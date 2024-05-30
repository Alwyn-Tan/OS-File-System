#include <iostream>
#include "block.h"
#include <string.h>
#pragma warning( disable : 4996)
using namespace std;
int systemUsed = 1642;
int directoryMaxNum = 50;
struct DISK {
	BOOT_BLOCK* boot_block;
	SUPER_BLOCK* super_block;
	INODE_BIT_MAP* i_node_bit_map;
	DATA_BIT_MAP* data_bit_map;
	I_NODE* i_node;

	//three types of the data blocks
	INDIRECT_ADDR_BLOCK* in_addr;
	DATA_BLOCK* data;
	DIRECTORY_BLOCK* Dire_Block;
} *disk;

DIRECTORY_BLOCK* newDirectory()
{
	DIRECTORY_BLOCK* temp = new DIRECTORY_BLOCK;
	for (int i = 0; i < directoryMaxNum; i++)
		memset(temp->directory[i].name, 0, 16);
	return temp;
}

//
// The first block is boot block
// The second block is super block
// The third and fourth block is data bit map block and i-node bit map block
//
void format()
{
	disk = new DISK[16 * 1024];
	disk[0].boot_block = new BOOT_BLOCK;
	disk[0].boot_block->user_sum = 1;
	disk[0].boot_block->current_user = 0;
	strcpy_s(disk[0].boot_block->user_inf[0].user_id, "admin");
	strcpy_s(disk[0].boot_block->user_inf[0].pass_word, "admin");
	strcpy_s(disk[0].boot_block->user_inf[1].user_id, "user1");
	strcpy_s(disk[0].boot_block->user_inf[1].pass_word, "user1");
	strcpy_s(disk[0].boot_block->user_inf[2].user_id, "user2");
	strcpy_s(disk[0].boot_block->user_inf[2].pass_word, "user2");

	disk[1].super_block = new SUPER_BLOCK;
	disk[1].super_block->free_inode = 1638;
	disk[1].super_block->free_data_block = 16194;
	disk[1].super_block->total_data_block = 16194;
	disk[1].super_block->sizeof_block = 1024;
	disk[1].super_block->inode_bitmap_block = 2;
	disk[1].super_block->data_bitmap_block = 3;
	disk[1].super_block->first_inode_block = 4;
	disk[1].super_block->last_inode_block = 1641;
	disk[1].super_block->first_data_block = 1642;
	disk[2].i_node_bit_map = new INODE_BIT_MAP;
	memset(disk[2].i_node_bit_map->inode_bit_map, 0, sizeof(disk[2].i_node_bit_map->inode_bit_map));
	disk[3].data_bit_map = new DATA_BIT_MAP;
	memset(disk[3].data_bit_map->data_bit_map, 0, sizeof(disk[3].data_bit_map->data_bit_map));
	for (int i = 4; i < systemUsed; i++)
	{
		disk[i].i_node = new I_NODE;
	}
	disk[1].i_node_bit_map = new INODE_BIT_MAP;
}

int findFile_INode(long p, char name[])
{
	int pointer = -1;
	time_t ti;
	for (int i = 0; i < 50; i++)
	{
		if (strcmp(disk[p].Dire_Block->directory[i].name, name) == 0)
		{
			pointer = disk[p].Dire_Block->directory[i].inode_number;
			time(&ti);
			disk[pointer].i_node->access_time = ti;
			return pointer;
		}
	}
	return pointer;
}

void createRootDirectory()
{
	int first_data_block_position = disk[1].super_block->first_data_block;
	disk[first_data_block_position].Dire_Block = newDirectory();
	disk[1].super_block->free_data_block--;
	disk[3].data_bit_map->data_bit_map[0] = true;
	strcpy_s(disk[first_data_block_position].Dire_Block->name, "root");
	strcpy_s(disk[1].super_block->usrdir, disk[first_data_block_position].Dire_Block->name);
}

int findFreeDataBlock()
{
	for (int i = 0; i < disk[1].super_block->total_data_block; i++)
	{
		if (disk[3].data_bit_map->data_bit_map[i] == false)
		{
			return i + systemUsed;
		}
	}
}

void assign_INode(int I)
{
	time_t t;
	time(&t);
	disk[I].i_node->create_time = disk[I].i_node->access_time = disk[I].i_node->modification_time = t;
	disk[I].i_node->uid = disk[0].boot_block->current_user;
	disk[I].i_node->current_size = 0;
	disk[I].i_node->lock = 0;
	disk[I].i_node->shareDir = 0;
	disk[I].i_node->read_only_flag = 0;
	for (int i = 0; i < 10; i++)
	{
		disk[I].i_node->direct_addr[i] = 0;
	}
	for (int i = 0; i < 2; i++)
	{
		disk[I].i_node->indirect_addr[i] = 0;
	}
}

void make_Dir(long& p, char* token)
{
	for (int i = 0; i < 50; i++)
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) == 0)
		{
			strcpy_s(disk[p].Dire_Block->directory[i].name, token);
			int Inode = findFreeINode();
			disk[p].Dire_Block->directory[i].inode_number = Inode;
			assign_INode(Inode);//assign new directory with inode
			disk[Inode].i_node->type = 0;
			disk[2].i_node_bit_map->inode_bit_map[Inode - disk[1].super_block->first_inode_block] = true;

			int data_p = findFreeDataBlock();//assign new directory with data block
			disk[1].super_block->free_data_block--;
			disk[Inode].i_node->direct_addr[disk[Inode].i_node->current_size++] = data_p;
			disk[data_p].Dire_Block = newDirectory();
			disk[3].data_bit_map->data_bit_map[data_p - systemUsed] = true;
			strcpy_s(disk[data_p].Dire_Block->name, token);
		}
	}
}


int createDirectoryCheck(char dir[]) // (char* = '/root/dir1/file1')
{
	int pointer = -1;
	if (strstr(dir, disk[1].super_block->usrdir) != NULL) // check whether the given dir is under user's dir
	{
		char* token = strtok(dir, "/");
		long first_data_block_position = disk[1].super_block->first_data_block;
		token = strtok(NULL, "/");
		int traversed_position;
		int last_traversed_position = 0;
		while (token != NULL)
		{
			if ((traversed_position = findFile_INode(first_data_block_position, token)) == -1) // check whether the dir is created
			{
				make_Dir(first_data_block_position, token);
				time_t ti;
				time(&ti);
				if (last_traversed_position != 0)
					disk[last_traversed_position].i_node->modification_time = ti;
			}
			else
			{
				first_data_block_position = disk[traversed_position].i_node->direct_addr[0]; //update the i node direct address is the current dir address
				last_traversed_position = traversed_position;
			}
			token = strtok(NULL, "/");
		};
		pointer = first_data_block_position;
	}
	else cout << "Permission denied! You can not create file under other uers directory" << endl;
	return pointer;
}


bool deleteFileHelp(int& p, int i)
{
	int inode;
	inode = disk[p].Dire_Block->directory[i].inode_number;//get the I_node of the file
	if (disk[inode].i_node->lock == 1)//check wheter the file is locked
	{
		cout << disk[p].Dire_Block->directory[i].name << " is locked ,permission denied" << endl;
		return false;
	}
	strcpy_s(disk[p].Dire_Block->directory[i].name, "");
	disk[p].Dire_Block->directory[i].inode_number = 0;//在目录上清除

	if (disk[inode].i_node->shareDir != 0)//delete soft link
	{
		int p = disk[inode].i_node->shareDir;
		int o = disk[inode].i_node->shareOffset;
		strcpy_s(disk[p].Dire_Block->directory[o].name, "");
		disk[p].Dire_Block->directory[o].inode_number = 0;
		return true;
	}

	int tp;//delete data
	for (int i = 0; i < 10; i++)
	{
		tp = (int)disk[inode].i_node->direct_addr[i];
		if (tp == 0)
			break;
		delete disk[tp].data;
		disk[3].data_bit_map->data_bit_map[tp - systemUsed] = false;
		disk[1].super_block->free_data_block++;

	}

	tp = (int)disk[inode].i_node->indirect_addr[0];
	int tp1;
	if (tp != 0)
	{
		for (int i = 0; i < 256; i++)
		{
			tp1 = (int)disk[tp].in_addr->addr;
			if (tp1 == 0)
				break;
			delete disk[tp1].data;
			disk[3].data_bit_map->data_bit_map[tp1 - systemUsed] = false;
			disk[1].super_block->free_data_block++;
		}
	}

	tp = (int)disk[inode].i_node->indirect_addr[1];
	if (tp != 0)
	{
		int tp2;
		for (int i = 0; i < 256; i++)
		{
			tp1 = disk[tp].in_addr->addr[i];
			if (tp1 == 0)
				break;
			for (int j = 0; j < 256; j++)
			{
				tp2 = disk[tp1].in_addr->addr[j];
				if (tp2 == 0)
					break;
				delete disk[tp2].data;
				disk[3].data_bit_map->data_bit_map[tp2 - systemUsed] = false;
				disk[1].super_block->free_data_block++;
			}
		}
	}
	disk[2].i_node_bit_map->inode_bit_map[inode - disk[1].super_block->first_inode_block] = false;
	delete disk[inode].i_node;
	disk[inode].i_node = new I_NODE;
	return true;
}

void checkAndCreateFile(const char* name, int size, char* filecontent) // (char* = '/root/dir1/file1')
{
	if (strstr(name, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Error! User Directory does not exist!" << endl;
		return;
	}
	const char* filename = strrchr(name, '/');
	char dir[1024];
	memset(dir, 0, sizeof(dir));
	strncpy(dir, name, /*strlen(name) - */ strlen(filename));
	filename++;

	int pointer;
	if ((pointer = createDirectoryCheck(dir)) == -1)
	{
		cout << "Directory creating denied" << endl;
	}
	else {
		for (int i = 0; i < directoryMaxNum; i++)
			if (strcmp(disk[pointer].Dire_Block->directory[i].name, filename) == 0)
			{
				cout << "Same file name in the directory, would you like to replace it? <y/n>" << endl;
				char n;
				cin >> n;
				if (n == 'n')
					return;
				else if (!deleteFileHelp(pointer, i))
					return;
			}
		createFile(pointer, filename, size, filecontent);
	}
}

int findFreeINode()
{
	for (int i = 0; i < disk[1].super_block->total_data_block; i++)
	{
		if (disk[2].i_node_bit_map->inode_bit_map[i] == false)
		{
			return i + disk[1].super_block->first_inode_block;
		}
	}
}

INDIRECT_ADDR_BLOCK* newINDIR_Addr()
{
	INDIRECT_ADDR_BLOCK* temp = new INDIRECT_ADDR_BLOCK;
	for (int i = 0; i < 256; i++)
		temp->addr[i] = 0;
	return temp;
}

void make_inDir(INDIRECT_ADDR_BLOCK* block, char* filecontent, int& totalBlocks, int& used_Blocks)
{
	int count = totalBlocks < 256 ? totalBlocks : 256;
	int temp;
	for (int i = 0; i < count; i++)
	{
		temp = block->addr[i] = findFreeDataBlock();
		disk[1].super_block->free_data_block--;
		disk[3].data_bit_map->data_bit_map[temp - systemUsed] = true;
		disk[temp].data = new DATA_BLOCK;
		memset(disk[temp].data->content, 0, 1024);
		totalBlocks--;
		if (used_Blocks > 0)//copy the file content into the new space;
		{
			used_Blocks--;
			strncpy_s(disk[temp].data->content, filecontent, 1024);
			filecontent += 1024;
		}
	}

}

void createFile(int& p, const char* filename, double size, char* filecontent)
{
	int Inode=disk[1].super_block->first_inode_block;
	for (int i = 0; i < directoryMaxNum; i++)//find the proper place in directory for the filename and allowcate inode 
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) == 0)//find the first empty dir entry
		{
			strcpy_s(disk[p].Dire_Block->directory[i].name, filename);
			Inode = findFreeINode();
			if (Inode == -1)//not free Inode
				return;
			disk[p].Dire_Block->directory[i].inode_number = Inode;
			assign_INode(Inode);//allowcate inode for new directory
			disk[Inode].i_node->type = 1;
			disk[2].i_node_bit_map->inode_bit_map[Inode - disk[1].super_block->first_inode_block] = true;
			p = Inode;
			break;
		}
	}
	//calculate data size, usedSpace and update inode,
	//then calculate number of blocks written data and number of total blocks taken
	int usedSpace = 0;
	if (disk[p].i_node != nullptr && filecontent != nullptr && strlen(filecontent) > 0) {
		usedSpace = disk[p].i_node->current_size = strlen(filecontent);
	}
	disk[p].i_node->max_size = size;
 	int used_Blocks = (int)ceil((double)usedSpace / 1024);
	int totalBlocks = (int)ceil(size / 1024);
	int used_Blocks2 = used_Blocks;

	int count = totalBlocks < 10 ? totalBlocks : 10; //write data into direct data block
	int temp;
	for (int i = 0; i < count; i++)
	{
		temp = disk[Inode].i_node->direct_addr[i] = findFreeDataBlock();// find free data block as temp and load data
		disk[3].data_bit_map->data_bit_map[temp - systemUsed] = true;
		disk[temp].data = new DATA_BLOCK;
		memset(disk[temp].data->content, 0, 1024);
		disk[1].super_block->free_data_block--;
		totalBlocks--;
		if (used_Blocks > 0)
		{
			used_Blocks--;
			strncpy_s(disk[temp].data->content, filecontent, 1024);
			filecontent += 1024;
		}
	}
	cout << disk[temp].data->content << endl;//test if the file is successfully created
	if (totalBlocks > 0) // data writing finished, but still needs to allocate data blocks
	{
		int new_p = findFreeDataBlock();
		disk[new_p].in_addr = newINDIR_Addr();
		disk[Inode].i_node->indirect_addr[0] = new_p;
		disk[3].data_bit_map->data_bit_map[new_p - systemUsed] = true;
		make_inDir(disk[new_p].in_addr, filecontent, totalBlocks, used_Blocks);
	}

	if (totalBlocks > 0)
	{
		int new_p1 = findFreeDataBlock();//request data block for new data;
		disk[new_p1].in_addr = newINDIR_Addr();
		disk[Inode].i_node->indirect_addr[1] = new_p1;
		disk[3].data_bit_map->data_bit_map[new_p1 - systemUsed] = true;
		int count = 0;
		while (totalBlocks > 0 && count < 256)
		{
			int new_p2 = findFreeDataBlock();//request data block for new data;
			disk[new_p2].in_addr = newINDIR_Addr();
			disk[new_p1].in_addr->addr[count++] = new_p2;
			disk[3].data_bit_map->data_bit_map[new_p2 - systemUsed] = true;
			make_inDir(disk[new_p2].in_addr, filecontent, totalBlocks, used_Blocks);
		}
	}
	filecontent -= used_Blocks2 * 1024;
	//delete []filecontent;
}