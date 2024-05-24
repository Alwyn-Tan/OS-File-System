#include <iostream>
#include "block.h"

int systemUsed = 1642;

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
	for (int i = 0; i < 50; i++)
		memset(temp->directory[i].name, 0, 16);
	return temp;
}
void createRoot()
{
	int p = disk[1].super_block->first_data_block;
	disk[p].Dire_Block = newDirectory();
	disk[1].super_block->free_data_block--;
	disk[3].data_bit_map->data_bit_map[0] = true;
	strcpy_s(disk[p].Dire_Block->name, "root");
	strcpy_s(disk[1].super_block->usrdir, disk[p].Dire_Block->name);
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
	createRoot();
}