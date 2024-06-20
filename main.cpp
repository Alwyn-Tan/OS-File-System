#include"block.h"
#include<iostream>
#pragma warning( disable : 4996)
using namespace std;


int main() {
	format();
	createRootDirectory();
	// checkAndCreateFile("/root/dir1/file1", 1024);
	// checkAndCreateFile("/root/dir1/dir2/file2", 1024);
	bool flag = true;
	while (flag)
	{
		cmdF(flag);
	};

	return 0;
}
