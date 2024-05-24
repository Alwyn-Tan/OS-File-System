#pragma once
#include<ctime>
#include<string.h>
#ifndef  __BLOCK_H__ 
#define __BLOCK_H__


struct USER_INF {
	char user_id[22];
	char pass_word[20];
};

struct BOOT_BLOCK {
	USER_INF user_inf[3];
	int user_sum;
	int current_user;
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
	int lock;//0 can be delete,1can;'t delete
	int read_only_flag;//0 can be for read and wirte ,1 read only.
	int type;//0 is directory ,1 is data;
	time_t create_time;
	time_t modification_time;
	time_t access_time;
	int current_size;
	int max_size;
	int direct_addr[10];
	int indirect_addr[2];
	int shareDir;
	int shareOffset;
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



DIRECTORY_BLOCK* newDirectory();

void createRoot();

void format();

#endif
