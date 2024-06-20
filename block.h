#pragma once
#include<ctime>
#include<string>
#include<string.h>

#ifndef  __BLOCK_H__ 
#define __BLOCK_H__

struct BOOT_BLOCK {
};

struct SUPER_BLOCK {
	int free_inode;
	int free_data_block;
	int total_data_block;
	int first_data_block;
	int first_inode_block;
	int last_inode_block;
	int inode_bitmap_block;
	int data_bitmap_block;
	int sizeof_block;
	char cwdir[1024];
	char usrdir[1024];
};

struct I_NODE {
	int uid;
	int type;//0 is directory ,1 is data;
	time_t create_time;
	int current_size = 0;
	int max_size;
	int direct_addr[10];
	int indirect_addr[2];
};

struct DATA_BIT_MAP {
	bool data_bit_map[16 * 1024 - 1642];
};

struct INODE_BIT_MAP {
	bool inode_bit_map[16 * 1024 - 1642];
};

struct INDIRECT_ADDR_BLOCK {
	long addr[256];
};//1KB

struct DATA_BLOCK {
	char content[1024];
};//1KB

struct DIRECTORY {
	long inode_number;
	char name[16];
};

struct DIRECTORY_BLOCK
{
	char name[16];
	DIRECTORY directory[50];
};

struct DISK
{
	BOOT_BLOCK* boot_block;
	SUPER_BLOCK* super_block;
	INODE_BIT_MAP* i_node_bit_map;
	DATA_BIT_MAP* data_bit_map;
	I_NODE* i_node;

	//three types of the data blocks
	INDIRECT_ADDR_BLOCK* in_addr;
	DATA_BLOCK* data;
	DIRECTORY_BLOCK* Dire_Block;
};
extern DISK* disk;

DIRECTORY_BLOCK* newDirectory();
void format();

void createRootDirectory();

int createDirectoryCheck(char dir[]);

void checkAndCreateFile(const char* name, int size);

void createFile(int& p, const char* token, double size);

int findFile_INode(long p, char name[]);

int findFreeINode();

int make_Dir(long& p, char* token);

int findFreeDataBlock();

void assign_INode(int I);

bool deleteFileHelp(int& p, int i);

INDIRECT_ADDR_BLOCK* newINDIR_Addr();

void make_inDir(INDIRECT_ADDR_BLOCK* p, char* filecontent, int& totalBlocks, int& used_Blocks);

void cat(char* filename);

void cmdF(bool& flag);

int searchFile(char* filename);

int searchDir(char* dir);

void dirFix(char* dir);

void deleteFile(char* name);

bool deleteDir(char* dir);

void sum();

void cpyFile(char* obj, char* dest);

void showhelp();

void listFile(char* dir);

void changeDir(char* dir);

void save(DISK* disk);

#endif
