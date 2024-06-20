#include <iostream>
#include <fstream>
#include <math.h>
#include "block.h"
#pragma warning( disable : 4996);
using namespace std;

int systemUsed = 1642;
int directoryMaxNum = 50;
DISK* disk;

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
}

int findFile_INode(long p, char name[])
{
	int pointer = -1;
	for (int i = 0; i < 50; i++)
	{
		if (strcmp(disk[p].Dire_Block->directory[i].name, name) == 0)
		{
			pointer = disk[p].Dire_Block->directory[i].inode_number;
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
	strcpy_s(disk[1].super_block->cwdir, disk[first_data_block_position].Dire_Block->name);
}

int findFreeDataBlock()
{
	for (int i = 0; i < disk[1].super_block->total_data_block; i++)
	{
		if (disk[3].data_bit_map->data_bit_map[i] == false)
		{
			return i + systemUsed;//1644.datablock file1 datablock|1643.datablock=dir1 directoryblock
		}
	}
	return -1;
}

void assign_INode(int I)
{
	disk[I].i_node->current_size = 0;
	for (int i = 0; i < 10; i++)
	{
		disk[I].i_node->direct_addr[i] = 0;
	}
	for (int i = 0; i < 2; i++)
	{
		disk[I].i_node->indirect_addr[i] = 0;
	}
}

int make_Dir(long& p, char* token)
{
	for (int i = 0; i < 50; i++)
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) == 0)
		{
			strcpy_s(disk[p].Dire_Block->directory[i].name, token);//1642.directory0.name=dir1
			int Inode = findFreeINode();//4.inode=dir1
			disk[p].Dire_Block->directory[i].inode_number = Inode;//1642.directory0.inode=4(dir1)
			assign_INode(Inode);//assign new directory with inode
			disk[Inode].i_node->type = 0;
			disk[2].i_node_bit_map->inode_bit_map[Inode - disk[1].super_block->first_inode_block] = true;

			int data_p = findFreeDataBlock();//assign new directory with data block
			disk[1].super_block->free_data_block--;
			disk[Inode].i_node->direct_addr[disk[Inode].i_node->current_size++] = data_p;
			disk[data_p].Dire_Block = newDirectory();
			disk[3].data_bit_map->data_bit_map[data_p - systemUsed] = true;
			strcpy_s(disk[data_p].Dire_Block->name, token);
			return data_p;
		}
	}
	return -1;
}


int createDirectoryCheck(char dir[]) // (char* = '/root/dir1/file1')
{
	int pointer = -1;
	long first_data_block_position = disk[1].super_block->first_data_block;
	if (strstr(dir, disk[1].super_block->usrdir) != NULL) // check whether the given dir is under user's dir
	{
		char* token = strtok(dir, "/");
		token = strtok(NULL, "/");
		int traversed_position;
		int last_traversed_position = 0;
		while (token != NULL)
		{
			if ((traversed_position = findFile_INode(first_data_block_position, token)) == -1) // check whether the dir is created
			{
				pointer = make_Dir(first_data_block_position, token);
				if (last_traversed_position != 0)
					return pointer;
			}
			else
			{
				first_data_block_position = disk[traversed_position].i_node->direct_addr[0]; //update the i node direct address is the current dir address
				last_traversed_position = traversed_position;
			}
			token = strtok(NULL, "/");
		};
		//pointer = 1643;//key
	}
	return first_data_block_position;
}


bool deleteFileHelp(int& p, int i)
{
	int inode;
	inode = disk[p].Dire_Block->directory[i].inode_number;//get the I_node of the file
	strcpy_s(disk[p].Dire_Block->directory[i].name, "");
	disk[p].Dire_Block->directory[i].inode_number = 0;

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
			tp1 = (long long)disk[tp].in_addr->addr;
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

void checkAndCreateFile(const char* name, int size) // (char* = '/root/dir1/file1')
{
	if (strstr(name, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Error! User Directory does not exist!" << endl;
		return;
	}
	const char* filename = strrchr(name, '/');
	char dir[1024];
	memset(dir, 0, sizeof(dir));
	strncpy(dir, name, strlen(name) - strlen(filename));// directory: /root/dir1  filename:/file1
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
		createFile(pointer, filename, size);
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
	return -1;
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

void createFile(int& p, const char* filename, double size)
{
	int Inode = disk[1].super_block->first_inode_block;
	for (int i = 0; i < directoryMaxNum; i++)//find the proper place in directory for the filename and allowcate inode 
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) == 0)//find the first empty dir entry
		{
			strcpy_s(disk[p].Dire_Block->directory[i].name, filename);//1642.1 file1 directory
			Inode = findFreeINode(); //5.inode=file1 inode
			if (Inode == -1)//not free Inode
				return;
			disk[p].Dire_Block->directory[i].inode_number = Inode; //inode5 file1 inode
			assign_INode(Inode);//allowcate inode for new directory
			disk[Inode].i_node->type = 1;
			disk[2].i_node_bit_map->inode_bit_map[Inode - disk[1].super_block->first_inode_block] = true;
			p = Inode;
			break;
		}
	}
	//calculate data size, usedSpace and update inode,
	//then calculate number of blocks written data and number of total blocks taken

	char* filecontent = new char[10];
	char c;
	for (int i = 0; i < 10; i++)
	{
		c = 'a' + rand() % 26;
		filecontent[i] = c;
	}


	int usedSpace = 0;
	if (disk[p].i_node != nullptr) {
		disk[p].i_node->current_size = strlen(filecontent);
		usedSpace = disk[p].i_node->current_size;
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

int searchFile(char* filename)
{
	int pointer = -1;
	char* token = strtok(filename, "/");
	long p = disk[1].super_block->first_data_block;
	token = strtok(NULL, "/");
	int tp;
	while (token != NULL)
	{
		if ((tp = findFile_INode(p, token)) == -1)
			return pointer;
		p = disk[tp].i_node->direct_addr[0];//4.inode.direct_address0=1643
		token = strtok(NULL, "/");
	}
	pointer = tp;
	return pointer;
}

int searchDir(char* dir)//����Ŀ¼�ļ��ĵ�ַ
{
	long pointer = -1;
	char* sub = strtok(dir, "/");
	long p = disk[1].super_block->first_data_block;
	sub = strtok(NULL, "/");
	int tp;
	while (sub != NULL)
	{
		if ((tp = findFile_INode(p, sub)) == -1)
			return pointer;
		p = disk[tp].i_node->direct_addr[0];//��ȡ��һ��Ŀ¼�ĵ�ַ

		sub = strtok(NULL, "/");
	}
	pointer = p;
	return pointer;
}

void deleteFile(char* name)
{
	if (strstr(name, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Permission denied" << endl;
		return;
	}
	char* filename = strrchr(name, '/');
	char dir[16];
	memset(dir, 0, sizeof(dir));
	strncpy(dir, name, (strlen(name) - strlen(filename)));//��ȡ�ļ�·��
	filename++;//��ȡ�ļ���
	int tp;
	if ((tp = searchDir(dir)) == -1)
	{
		cout << "not such a directory" << endl;
		return;
	}
	for (int i = 0; i < 50; i++)
		if (strcmp(disk[tp].Dire_Block->directory[i].name, filename) == 0)
		{
			deleteFileHelp(tp, i);
		}
}

bool deleteDir(char* dir)
{
	if (strcmp(dir, disk[1].super_block->cwdir) == 0 || strstr(disk[1].super_block->cwdir, dir) != NULL || strcmp(dir, disk[1].super_block->usrdir) == 0)
	{
		cout << "Permission denied" << endl;
		return false;
	}
	int p;
	char td[1024];
	strcpy(td, dir);
	if ((p = searchDir(dir)) == -1)
	{
		cout << "Directory doesn't exist" << endl;
		return false;
	}
	bool flag = true;
	for (int i = 0; i < 50; i++)//delete file
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) > 0)
		{
			int inode = disk[p].Dire_Block->directory[i].inode_number;
			if (disk[inode].i_node->type == 1)
				flag = deleteFileHelp(p, i);
			else {
				char temp[1024];
				strcpy(temp, td);
				strcat(temp, "/");
				strcat(temp, disk[p].Dire_Block->directory[i].name);
				flag = deleteDir(temp);

			}
		}
	}
	if (flag)
	{
		delete disk[p].Dire_Block;
		disk[1].super_block->free_data_block++;
		disk[3].data_bit_map->data_bit_map[p - systemUsed] = false;

		char* sub1 = strrchr(td, '/');
		char sub2[1024];
		memset(sub2, 0, sizeof(sub2));
		strncpy(sub2, td, strlen(td) - strlen(sub1));
		sub1++;
		int tp = searchDir(sub2);
		int tp1;
		for (int i = 0; i < 50; i++)
		{
			if (strcmp(sub1, disk[tp].Dire_Block->directory[i].name) == 0)
			{
				tp1 = disk[tp].Dire_Block->directory[i].inode_number;
				strcpy(disk[tp].Dire_Block->directory[i].name, "");
				disk[tp].Dire_Block->directory[i].inode_number = 0;
				break;
			}
		}
		delete disk[tp1].i_node;
		disk[tp].i_node = new I_NODE;
		disk[2].i_node_bit_map->inode_bit_map[tp - disk[1].super_block->first_inode_block] = false;
	}
	else {
		cout << "can't no delete locked file" << endl;
		return false;
	}

	return flag;
}

void cat(char* filename)
{
	int p;
	if (strstr(filename, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Permission denied" << endl;
		return;
	}
	if ((p = searchFile(filename)) == -1)
	{
		cout << "file doesn't exist in this directory!" << endl;
		return;
	}
	int tp;
	for (int i = 0; i < 10; i++)
	{
		tp = (int)disk[p].i_node->direct_addr[i];
		if (tp == 0)
			break;
		cout << disk[tp].data->content;
	}
	tp = (int)disk[p].i_node->indirect_addr[0];
	int tp1;
	if (tp != 0)
	{

		for (int i = 0; i < 256; i++)
		{
			tp1 = (int)disk[tp].in_addr->addr[i];
			if (tp1 == 0)
				break;
			cout << disk[tp1].data->content;
		}
	}
	tp = (int)disk[p].i_node->indirect_addr[1];
	int tp2;
	if (tp != 0)
	{
		for (int i = 0; i < 256; i++)
		{
			tp1 = (int)disk[tp].in_addr->addr[i];
			if (tp1 == 0)
				break;
			for (int j = 0; j < 256; j++)
			{
				tp2 = (int)disk[tp1].in_addr->addr[i];
				if (tp2 == 0)
					break;
				cout << disk[tp2].data->content;
			}
		}
	}
	cout << endl;
}

void dirFix(char* dir)
{
	if (strchr(dir, '/') == NULL)
	{
		char temp[1024];
		strcpy(temp, dir);
		strcpy(dir, disk[1].super_block->cwdir);
		strcat(dir, "/");
		strcat(dir, temp);
	}
}

void showhelp()
{

}

void cpyFile(char* obj, char* dest)
{
	int pointer;
	if ((pointer = searchFile(obj)) == -1)
	{
		cout << "Source file doesn't exist in this directory!" << endl;
		return;
	}
	int size = disk[pointer].i_node->max_size;
	char* buf = new char[disk[pointer].i_node->current_size];
	memset(buf, 0, sizeof(buf));
	int traversed_pointer;
	for (int i = 0; i < 10; i++)
	{
		traversed_pointer = (int)disk[pointer].i_node->direct_addr[i];
		if (traversed_pointer == 0)
			break;
		strcat(buf, disk[traversed_pointer].data->content);
	}
	traversed_pointer = (int)disk[pointer].i_node->indirect_addr[0];
	int tp1;
	if (traversed_pointer != 0)
	{
		for (int i = 0; i < 256; i++)
		{
			tp1 = (int)disk[traversed_pointer].in_addr->addr[i];
			if (tp1 == 0)
				break;
			strcat(buf, disk[tp1].data->content);
		}
	}
	traversed_pointer = (int)disk[pointer].i_node->indirect_addr[1];
	int traversed_pointer2;
	if (traversed_pointer != 0)
	{

		for (int i = 0; i < 256; i++)
		{
			tp1 = (int)disk[traversed_pointer].in_addr->addr[i];
			if (tp1 == 0)
				break;
			for (int j = 0; j < 256; j++)
			{
				traversed_pointer2 = (int)disk[tp1].in_addr->addr[i];
				if (traversed_pointer2 == 0)
					break;
				strcat(buf, disk[traversed_pointer2].data->content);
			}
		}
	}
	checkAndCreateFile(dest, size);

}

void sum()
{
	cout << "system used space : 191kb" << endl;
	cout << "Occupied blocks : " << (disk[1].super_block->total_data_block - disk[1].super_block->free_data_block) << endl;
	cout << "Available blocks : " << disk[1].super_block->free_data_block << endl;
	cout << "Total blocks for user data : " << disk[1].super_block->total_data_block << endl;
}


void listFile(char* dir)
{
	int p;
	if ((p = searchDir(dir)) == -1)
	{
		cout << "Directories doesn't exist" << endl;
		return;
	}
	cout << "#################################################" << endl;
	for (int i = 0; i < 50; i++)
	{
		if (strlen(disk[p].Dire_Block->directory[i].name) > 0)
		{
			int inode = disk[p].Dire_Block->directory[i].inode_number;
			cout << "Name : " << disk[p].Dire_Block->directory[i].name << endl;
			if (disk[inode].i_node->type == 0)
			{
				cout << "Data type : directory" << endl;
				cout << "Size : 1KB" << endl;
			}
			else
			{
				cout << "Data type : user file" << endl;
				cout << "Current size : " << disk[inode].i_node->current_size << " Bytes" << endl;
				cout << "Max size : " << disk[inode].i_node->max_size << " Bytes" << endl;
			}
			cout << "Create time : " << ctime(&disk[inode].i_node->create_time);
			cout << "#################################################" << endl;
		}
	}
}

void changeDir(char* dir)
{
	if (strstr(dir, disk[1].super_block->usrdir) == NULL)
	{
		cout << "Permission denied" << endl;
		return;
	}
	char td[1024];
	strcpy(td, dir);
	if (searchDir(td) != -1)
		strcpy(disk[1].super_block->cwdir, dir);
	else cout << "Directory doesn't exist" << endl;
}

void cmdF(bool& flag)
{
	char cmd[1024];
	cout << "@ " << disk[1].super_block->cwdir << ">";
	cin.sync();
	cin >> cmd;
	if (strcmp(cmd, "createF") == 0)
	{
		char filename[1024];
		memset(filename, 0, sizeof(filename));
		int size = 0;
		cin >> filename >> size;
		dirFix(filename);
		if (size >= disk[1].super_block->free_data_block * 1024)
		{
			cout << "error:file size greater available space" << endl;
			return;
		}
		checkAndCreateFile(filename, size);
	}

	if (strcmp(cmd, "cat") == 0)
	{
		char filename[1024];
		memset(filename, 0, sizeof(filename));
		cin >> filename;
		dirFix(filename);
		cat(filename);
	}

	if (strcmp(cmd, "deleteFile") == 0)
	{
		char filename[1024];
		memset(filename, 0, sizeof(filename));
		cin >> filename;
		dirFix(filename);
		deleteFile(filename);
	}

	if (strcmp(cmd, "deleteDir") == 0)
	{
		char  dir[1024];
		memset(dir, 0, sizeof(dir));
		cin >> dir;
		dirFix(dir);
		deleteDir(dir);
	}

	if (strcmp(cmd, "createDir") == 0)
	{
		char  dir[1024];
		memset(dir, 0, sizeof(dir));
		cin >> dir;
		dirFix(dir);
		createDirectoryCheck(dir);
	}

	if (strcmp(cmd, "listF") == 0)
	{
		char  dir[1024];
		memset(dir, 0, sizeof(dir));
		cin >> dir;
		listFile(dir);
	}

	if (strcmp(cmd, "cp") == 0)
	{
		char dest[1024];
		char sour[1024];
		cin >> sour >> dest;
		dirFix(sour);
		cpyFile(sour, dest);
	}
	if (strcmp(cmd, "sum") == 0)
	{
		sum();
	}
	if (strcmp(cmd, "cd") == 0)
	{
		char dir[1024];
		cin >> dir;
		changeDir(dir);
	}
	if (strcmp(cmd, "changeS") == 0)
	{
		char dir[1024];
		char lock[10];
		cin >> dir >> lock;
		dirFix(dir);
	}
	if (strcmp(cmd, "exit") == 0)
	{
		flag = false;
		save(disk);
	}
}
void save(DISK* disk)
{
	ofstream file("content.txt");
	int* superblock = new int[2];
	superblock[0] = disk[1].super_block->free_inode;
	superblock[1] = disk[1].super_block->free_data_block;
	for (int i = 0; i < 2; i++)
	{
		file << superblock[i] << endl;
	}

	for (int i = 0; i < 14742; i++)
	{
		file << disk[2].i_node_bit_map->inode_bit_map[i] << endl;
	}
	for (int i = 0; i < 14742; i++)
	{
		file << disk[3].data_bit_map->data_bit_map[i] << endl;
	}
	file.close();
}
