#include <iostream>
#include "block.h"
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

void createRootDirectory()
{
	int first_data_block_position = disk[1].super_block->first_data_block;
	disk[first_data_block_position].Dire_Block = newDirectory();
	disk[1].super_block->free_data_block--;
	disk[3].data_bit_map->data_bit_map[0] = true;
	strcpy_s(disk[first_data_block_position].Dire_Block->name, "root");
	strcpy_s(disk[1].super_block->usrdir, disk[first_data_block_position].Dire_Block->name);
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

void checkAndCreateFile(char* name, int size, char* filecontent) // (char* = '/root/dir1/file1')
{
	if (strstr(name, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Error! User Directory does not exist!" << endl;
		return;
	}
	char* filename = strrchr(name, '/');
	char dir[1024];
	memset(dir, 0, sizeof(dir));
	strncpy(dir, name, strlen(name) - strlen(filename));
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

void createFile(int& p, char* filename, double size, char* filecontent) 
{
	int Inode;
	for (int i = 0; i < directoryMaxNum; i++)//find the proper place in directory for the filename and allowcate inode 
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) == 0)//find the first empty dir entry
		{
			strcpy(disk[p].Dire_Block->directory[i].name, filename);
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
	int usedSpace = disk[p].i_node->current_size = strlen(filecontent);
	disk[p].i_node->max_size = size;
	int used_Blocks = (int)ceil((double) usedSpace / 1024);
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
			strncpy(disk[temp].data->content, filecontent, 1024);
			filecontent += 1024;
		}
	}

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
	// delete []filecontent;
}

