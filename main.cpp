#include"block.h"
#include <string.h>
#pragma warning( disable : 4996)
using namespace std;


int main() {
	format();
	createRootDirectory();
	char* content = new char[1024];
	strcpy(content, "this is file1");
	checkAndCreateFile("/root/dir1/file1", 1024, content);
	char* content1 = new char[1024];
	strcpy(content1, "this is file2");
	checkAndCreateFile("/root/dir1/dir2/file2", 1024, content1);

}