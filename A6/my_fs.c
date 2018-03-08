#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <stdbool.h>

typedef unsigned long int ul;
typedef unsigned int ui;
typedef unsigned char uc;

#define SUPER_BLOCK_SIZE sizeof(superblock)
#define INODE_BLOCK_SIZE sizeof(inode)

#define IBM_SIZE 10
#define DBM_SIZE 10
#define MAX_DATA_BLOCKS_PER_INODE 10
#define MAX_DRIVE 10

#define PROMPT "myfs> "
#define EXIT "exit"
#define MKFS "mkfs"
#define nl "\n"

typedef struct {
	ul fssize; /* file system size in blocks */
	ui root_inode_no; /* stores root (/) inode number */
	ui block_size; /* size of block */
	ui inode_size; /* size of each inode */
	ul inode_start_location; /* inode starting location */
	ui inode_count; /* number of inodes present */
	ui free_inode_count; /* number of free inodes that are yet to be pointed to data blocks */
	ul data_block_start_location; /* data block starting location */
	ui data_block_count; /* count for data blocks */
	ui free_data_block_count; /* free data blocks */
	uc ibm[IBM_SIZE];
	uc dbm[DBM_SIZE];
} superblock;

typedef struct {
	char type; /* d: directory, f: file, l: symbolic link, etc. */
	int size; /* size of corresponding file */
	int datablock_count; /* data blocks allocated for storing the d,f,l,etc. type file */
	int datablocks[MAX_DATA_BLOCKS_PER_INODE]; /* max data blocks that can be allocated */
} inode;

typedef struct {
	char file[50];
	char drive[5];
} file_drive;

file_drive fdmap[MAX_DRIVE];

int main()
{
	char command[1024];
	while(true)
	{
		printf(PROMPT); fflush(stdout);
		fgets(command, 1024, stdin);
		if(strcmp(command, nl) == 0) continue;
		command[strlen(command)-1]='\0';
		if(strcmp(command, EXIT) == 0) break;
		//else execute(command);
	}
}

void init_fdmap()
{
	for(int i = 0; i < MAX_DRIVE; i++)
	{
		fdmap[i].file[0] = '\0';
		fdmap[i].drive[0] = '\0';
	}
}