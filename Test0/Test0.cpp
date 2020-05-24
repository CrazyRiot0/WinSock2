#include <iostream>
#include <cstdio>
#include <atlimage.h>
#include <Windows.h>
#include <WinUser.h>
#include <fstream>
#include <string>

using namespace std;

char* bitmap = NULL;

char B[1000001];
int S;

void Capture()
{
	HDC hScreenDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	int width = 1920;
	int height = 1080;

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
	ofstream out;

	char _in[] = "Capture.jpg";
	char _out[] = "copyed1.jpg";

	in.open(_in, ios::binary);
	out.open(_out, ios::binary);
	char buf[1000001];
	string s;
	
	in.seekg(0, ios::end);
	int size = in.tellg();
	in.seekg(0, ios::beg);

	in.read(buf, size);
	out.write(buf, size);

	in.close();
	out.close();
}

int main()
{
	Capture();
	//ofstream out;
	//out.open("copyed.jpg", ios::binary);
	//out.write(B, S);
	//out.close();
	system("pause");
}
