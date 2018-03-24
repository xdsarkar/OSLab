#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>			/*write(2) close(2)*/
#include <sys/types.h>		/*open(2)*/
#include <sys/stat.h>		/*open(2)*/
#include <fcntl.h>			/*open(2)*/
#include <error.h>			/*perror()*/

#define IBM_SIZE 10
#define DBM_SIZE 10
#define MAX	10
#define MAX_DRIVE 10
#define INODE_COUNT 10

#define PROMPT "myfs >> "
#define UNKNOWN_CMD "Unrecognized Command: "

#define EXIT "exit"

#define MKFS "mkfs"
#define MKFS_USAGE "Usage : mkfs [filename] [block size in bytes] [file system size in MB]"
#define MKFS_EXEC "Creating File System..."
#define SB_WRITE_FAILED "Super Block Write Failed"
#define SB_WRITE_SUCCESS "Super Block Write Successful"
#define SB_WRITE_INCMP "Super Block Write Incomplete"
#define RI_WRITE_FAILED "Root Inode Write Failed"
#define RI_WRITE_SUCCESS "Root Inode Write Successful"
#define RI_WRITE_INCMP "Root Inode Write Incomplete"
#define MKFS_FS_EXISTS "File System OS File Already Exists with the name "

#define GETFS "getfs"
#define GETFS_USAGE "Usage : getfs [-f or -d] [filename or drivename]"
#define SB_READ_FAILED "Super Block Read Failed"
#define SB_READ_SUCCESS "Super Block Read Successful"
#define SB_READ_INCMP "Super Block Read Incomplete"
#define RI_READ_FAILED "Root Inode Read Failed"
#define RI_READ_SUCCESS "Root Inode Read Successful"
#define RI_READ_INCMP "Root Inode Read Incomplete"
#define GETFS_FILE_NOT_FOUND "OS File Not Found"

#define USE "use"
#define USE_USAGE "Usage : use [filename] as [drivename]"
#define USE_EXEC "Creating OS File to Directory Mapping..."
#define USE_SUCCESS "OS File Mapped to Drive Successfully"
#define USE_FILE_EXISTS "OS File Already Mapped to Drive"
#define USE_DRIVE_EXISTS "Drive Exists"
#define USE_DRIVE_OVERFLOW "Cannot create any more drives"
#define USE_FILE_NOT_FOUND "OS File Not Found"
#define newline() printf("\n")

#define CP "cp"
#define CP_FILE_DOES_NOT_EXIST "No File of this name found"

/*Structure storing superblock*/
typedef struct
{
	long FSSize;		//File System Size
	int root_inode_num;	//Root INODE number
	int block_size;		//Block Size
	int inode_size;		//Inode Size
	int inode_start_loc;	//Starting Position of Inode
	int inode_cnt;		//Number of Inodes
	int free_inode_cnt;	//Number of free INODES
	int db_start_loc;	//Starting Location of Data Blocks
	int db_count;		//Number of Data Blocks
	int free_db_count;	//Number of Free Data Blocks
	char inode_bitmap[IBM_SIZE];
	char db_bitmap[DBM_SIZE];
}supernode;

/*Structure storing inode*/
typedef struct
{
	char type; //File or Folder(Directory)
	int size; 
	int db_count;
	int db[MAX];
}inode;

/*Mapping from file-name to drive i.e say osfile1 to C*/
typedef struct
{
	char file_name[100];
	char drive_name[10];
}file_drive;

file_drive fdmap[MAX_DRIVE];

void init_fdmap();
void create_fs(char *path, long size_FS, int size_B);
void create_dir(char *filename, char *dirname);
void get_filesys_info(char file[100]);
int check_err_mkfs(char cmd[][10]);
int check_err_use(char cmd[][10]);
int check_err_getfs(char cmd[][10]);
int check_err_cp(char cmd[][10]);
void implement_sys_call(char cmd[][10]);
void execute(char cmd[]);

int main()
{
	/*to store the command entered in the myfs prompt*/
	init_fdmap();
	char command[1000];
	printf("Welcome to MYFS Prompt. Enter \"exit\" to exit\n");
	while(1)
	{
		newline();
		/*diaplay the prompt*/
		fputs(PROMPT, stdout);

		/*input the command*/
		fgets(command, 1000, stdin);
		command[strlen(command) - 1] = 0;			/*Replace \n with \0*/

		/*exit*/
		if(strcmp(command, EXIT) == 0)
		{
			break;
		}

		/*parse the command and execute it*/
		else
		{
			execute(command);
		}
	}

}

/*Initialize the file to drive map storing NULL strings in both*/
void init_fdmap()
{
	int i;
	for(i = 0; i < MAX_DRIVE; i++)
	{
		strcpy(fdmap[i].file_name, "\0");
		strcpy(fdmap[i].drive_name, "\0");
	}
}


void create_fs(char *path, long size_FS, int size_B)
{
	/*initialize superblock*/
	supernode snode;
	snode.FSSize = size_FS;
	snode.root_inode_num = 0;
	snode.block_size = size_B;
	snode.inode_size = sizeof(inode);
	snode.inode_start_loc = sizeof(supernode);
	snode.inode_cnt = INODE_COUNT;
	snode.free_inode_cnt = snode.inode_cnt;
	snode.db_start_loc = sizeof(supernode) + INODE_COUNT * sizeof(inode);
	snode.db_count = (size_FS - (sizeof(supernode) + INODE_COUNT * sizeof(inode)))/size_B;
	snode.free_db_count = snode.db_count;

	int i;
	for(i = 0; i < IBM_SIZE - 1; i++)
		snode.inode_bitmap[i] = '0';
	snode.inode_bitmap[IBM_SIZE - 1] = '\0';
	for(i = 0; i < DBM_SIZE - 1; i++)
		snode.db_bitmap[i] = '0';
	snode.db_bitmap[DBM_SIZE - 1] = '\0';

	/*initialize root folder*/
	inode ind;
	ind.type = 'd'; // d means the inode points to a directory
	ind.size = 0;
	ind.db_count = 0;
	for(i = 0; i < MAX; i++)
		ind.db[i] = -1;

	/*open file*/
	int fd;     /*File Descriptor*/

	/*O_EXCL ensures that a new File System with the same pathname cannot be created again*/
	fd = open(path, O_RDWR|O_CREAT|O_EXCL);
	if(fd == -1)
	{
		fputs(MKFS_FS_EXISTS, stdout);
		fputs(path, stdout);
		newline();
		return;
	}

	/*write superblock to file*/
	/*ssize_t write(int fd, const void *buf, size_t count)*/
	int write_st;
	write_st = write(fd, (void *)&snode, sizeof(snode));


	if(write_st == sizeof(snode))
	{
		fputs(SB_WRITE_SUCCESS, stdout);
		newline();
	}

	/*Error in writing to Disk*/
	else
	{
		fputs(SB_WRITE_FAILED, stdout);
		newline();
		return;
	}


	/*Move file pointer to 512 bytes i.e 1 block to write the inode for root directory*/

	/*write root inode to file*/
	/*ssize_t write(int fd, const void *buf, size_t count);*/
	write_st = write(fd, (void *)&ind, sizeof(ind));

	if(write_st == sizeof(ind))
	{
		fputs(RI_WRITE_SUCCESS, stdout);
		newline();
	}

	/*Error in writing to Disk*/
	else
	{
		fputs(RI_WRITE_FAILED, stdout);
		newline();
		return;
	}	
	close(fd);
}

/*Create a new Directory*/
void create_dir(char *filename, char *dirname)
{
	int i;
	if(access(filename, F_OK) == -1 ) 
	{
		fputs(USE_FILE_NOT_FOUND, stdout);
		newline();
		return;
	}
		
	for(i = 0; i < MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].file_name, filename) == 0)
		{
			fputs(USE_FILE_EXISTS, stdout);
			newline();
			return;
		}

		if(strcmp(fdmap[i].drive_name, dirname) == 0)
		{
			fputs(USE_DRIVE_EXISTS, stdout);
			newline();
			return;
		}
	}	

	for(i = 0; i < MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].file_name, "\0") == 0)
		{
			strcpy(fdmap[i].file_name, filename);
			strcpy(fdmap[i].drive_name, dirname);
			fputs(USE_SUCCESS, stdout);
			newline();
			return;
		}
	}
	if(i == MAX_DRIVE)
		fputs(USE_DRIVE_OVERFLOW, stdout);

}

/*cmd >> mkfs filename BLK_SIZE FILE_SIZE */
int check_err_mkfs(char cmd[][10])
{
	int i;
	for(i = 0; strcmp(cmd[i],"\0") != 0; i++);
	if(i != 4)
	{
		fputs(MKFS_USAGE, stdout);
		newline();
		return -1;
	}
	else
	{
		fputs(MKFS_EXEC, stdout);
		newline();
		return 0;
	}
}


int check_err_use(char cmd[][10])
{
	int i;
	for(i = 0; strcmp(cmd[i],"\0") != 0; i++);
	if(i != 4 || strcmp(cmd[2], "as") != 0)
	{/*ssize_t write(int fd, const void *buf, size_t count);*/
		fputs(USE_USAGE, stdout);
		newline();
		return -1;
	}
	else
	{
		fputs(USE_EXEC, stdout);
		newline();
		return 0;
	}
}

int check_err_getfs(char cmd[][10])
{
	int i;
	for(i = 0; strcmp(cmd[i],"\0") != 0; i++);
	if(i != 3)
	{
		fputs(GETFS_USAGE, stdout);
		newline();
		return -1;
	}
	else if(strcmp(cmd[1], "-d") != 0 && strcmp(cmd[1], "-f") != 0)
	{
		fputs(GETFS_USAGE, stdout);
		newline();
		return -1;
	}
	else
	{
		return 0;
	}
}

int check_err_cp(char cmd[][10])
{
	int i;
	for(i=0; strcmp(cmd[i],"\0") != 0; i++);
	if(i!=2)
	{
		fputs(USE_USAGE,stdout);
		newline();
		return -1;	
	}

	else
	{
		return 0;
	}
}

/*Print the Information Present on the virtual Disk*/
void get_filesys_info(char file[100])
{

	/*open file*/
	/*int open(const char *pathname, int flags);*/
	int fd;						/*File Descriptor*/
	fd = open(file, O_RDONLY);

	supernode sn;
	inode in;	
	int i;

	/*Read and display superblock info*/
	/*ssize_t read(int fd, void *buf, size_t count);*/
	int read_st;
	read_st = read(fd, (void *)&sn, sizeof(sn));
	if(read_st == sizeof(sn))
	{
		printf("\nREADING SUPERBLOCK...");
		newline();
		fputs(SB_READ_SUCCESS, stdout);
		printf("\nFILE SYSTEM SIZE: %ld", sn.FSSize);
		printf("\nROOT INODE NUMBER: %d", sn.root_inode_num);
		printf("\nBLOCK SIZE: %d", sn.block_size);
		printf("\nINODE SIZE: %d", sn.inode_size);
		printf("\nINODE START LOCATION: %d", sn.inode_start_loc);
		printf("\nINODE COUNT: %d", sn.inode_cnt);
		printf("\nFREE INODE COUNT: %d", sn.free_inode_cnt);
		printf("\nDATA BLOCK START LOCATION: %d", sn.db_start_loc);
		printf("\nDATA BLOCK COUNT: %d", sn.db_count);
		printf("\nFREE DATA BLOCK COUNT: %d", sn.free_db_count);
		printf("\nINODE BITMAP: %s", sn.inode_bitmap);
		printf("\nDATA BLOCK BITMAP: %s", sn.db_bitmap);
	}

	else
	{	
		newline();
		fputs(SB_READ_FAILED, stdout);
	}

	/*Moving File Pointer to Root Inode Location*/

	/*Read and Display Inode info*/
	read_st = read(fd, (void *)&in, sizeof(in));
	if(read_st == sizeof(in))
	{

		printf("\n\nREADING ROOT INODE...");
		newline();
		fputs(RI_READ_SUCCESS, stdout);
		printf("\nTYPE: %c", in.type);
		printf("\nSIZE: %d", in.size);
		printf("\nDATA BLOCK COUNT: %d", in.db_count);
		printf("\nDATA BLOCK:");
		for(i = 0; i < MAX; i++){
			if(in.db[i] == -1)
				break;
			else
				printf(" %d", in.db[i]);
		}
		printf(" END\n");

	}

	else
	{
		newline();
		fputs(RI_READ_FAILED, stdout);
	}

}

/*cmd>>myfs>cp osfile3 C:tesfile1*/
void copy_osfile_to_drive(char *filename, char *drive_file)
{
	int fdsrc,fddest,i;
	char drive[10];
	while(drive_file[i] != ':')
	{
		drive[i] = drive_file[i];
		i++;	
	}
	drive[i]='\0';

	/*open file*/
	/*int open(const char *pathname, int flags);*/
	fdsrc = open(filename,O_RDONLY);
	if(fdsrc == -1)
	{
		fputs(MKFS_FS_EXISTS, stdout);
		fputs(filename, stdout);
		newline();
		return;
	} 

	for(int i=0;i<MAX;i++)
	{
		if(strcmp(fdmap[i].drive_name,drive) == 0)
		{
			fddest = open(fdmap[i].file_name,O_WRONLY);
			if(fddest == -1)
			{
				fputs(MKFS_FS_EXISTS, stdout);
				fputs(drive, stdout);
				newline();
				return;
			}
			break;
		}
		else
		{
			fputs(CP_FILE_DOES_NOT_EXIST,stdout);
			newline();
			return;
		}

	}

	supernode sn;
	inode in;

	/*ssize_t read(int fd, void *buf, size_t count);*/
	int read_st;
	read_st = read(fdsrc, (void *)&sn, sizeof(sn));

	/*ssize_t write(int fd, const void *buf, size_t count);*/
	int wr_st;
	wr_st = write(fddest,(void*)&sn, sizeof(sn));

	read_st = read(fdsrc,(void*)&in, sizeof(in));
	wr_st = write(fddest,(void*)&in,sizeof(in));


}

/*Function to implement required system_call for a command*/
void implement_sys_call(char cmd[][10])
{
	/*Makefile Command*/
	if(strcmp(cmd[0], MKFS) == 0)
	{
		if(check_err_mkfs(cmd) == 0)
		{
			long fssize = atoi(cmd[3]) * 1024 * 1024; // 3rd field is File system size
			int bsize = atoi(cmd[2]); // 2nd field is Block Size
			create_fs(cmd[1], fssize, bsize); // cmd[1] will have the filename or pathname
		}
	}

	/*Use OSFILE as command*/	
	else if(strcmp(cmd[0], USE) == 0)
	{
		if(check_err_use(cmd) == 0)
		{
			create_dir(cmd[1], cmd[3]);
		}
	}

	/*getfs command*/
	else if(strcmp(cmd[0], GETFS) == 0)
	{
		char filename[100];
		int flag = 0;
		if(check_err_getfs(cmd) == 0)
		{
			if(strcmp(cmd[1], "-d") == 0)
			{
				int i;
				for(i = 0; strcmp(fdmap[i].drive_name, "\0") != 0; i++)
				{
					if(strcmp(fdmap[i].drive_name, cmd[2]) == 0)
					{
						strcpy(filename, fdmap[i].file_name);
						flag = 1;
						break;
					}
				}
				if(flag == 0)
				{
					fputs(GETFS_FILE_NOT_FOUND, stdout);
					newline();
					return;
				}
			}
			else
			{
				strcpy(filename, cmd[2]);
				if(access(filename, F_OK) == -1 ) 
				{
					fputs(GETFS_FILE_NOT_FOUND, stdout);
					newline();
					return;
				} 	
			}
			get_filesys_info(filename);
		}
	}
	/*cp command*/
	else if(strcmp(cmd[0],CP) == 0)
	{
		if(check_err_cp(cmd)==0)
		{
			copy_osfile_to_drive(cmd[1],cmd[2]);	
		}

	}	

	else
	{
		fputs(UNKNOWN_CMD, stdout);
		fputs(cmd[0], stdout);
		newline();
	}
}

/*Function to parse and implement a command*/
void execute(char cmd[])
{
	newline();
	/*store each word in a separate string*/
	char cmd_elements[10][10];
	int i, j, k;
	j = 0;
	k = 0;
	for(i = 0; i <= strlen(cmd); i++)
	{
		if (cmd[i] == '\0')
		{
			cmd_elements[j][k] = '\0';
		}
		else if(cmd[i] == ' ')
		{
			if(cmd[i + 1] != ' ' && cmd[i + 1] != 0){
				cmd_elements[j][k] = '\0';
				j++;
				k = 0;
				bzero(cmd_elements[j], 10);
			}
		}
		else
		{
			cmd_elements[j][k] = cmd[i];
			k++;
		}
	}
	/*Put null at the end to denote end of command*/
	strcpy(cmd_elements[j + 1], "\0");

	/*implement required system call*/
	implement_sys_call(cmd_elements);
}
