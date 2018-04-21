#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#define IBM_SIZE 10
#define DBM_SIZE 10
#define MAX_DATA_BLOCKS_PER_INODE 10
#define MAX_DRIVE 10
#define INODE_COUNT 10
#define MAX_DB 10
#define MAX_DRIVE 10
#define MAX_FILE 50

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define EXIT "exit"
#define MKFS "mkfs"
#define LS "ls"
#define CP "cp"
#define USE "use"
#define MV "mv"
#define RM "rm"
#define MAN "man"

typedef struct {
	int fssize; /* file system size in blocks */
	int root_inode_no; /* stores root (/) inode number */
	int block_size; /* size of block */
	int inode_size; /* size of each inode */
	int inode_start_location; /* inode starting location */
	int inode_count; /* number of inodes present */
	int free_inode_count; /* number of free inodes that are yet to be pointed to data blocks */
	int data_block_start_location; /* data block starting location */
	int data_block_count; /* count for data blocks */
	int free_data_block_count; /* free data blocks */
	char ibm[IBM_SIZE];
	char dbm[DBM_SIZE];
} superblock;

typedef struct {
	char type; /* d: directory, f: file, l: symbolic link, etc. */
	int size; /* size of corresponding file */
	int datablock_count; /* data blocks allocated for storing the d,f,l,etc. type file */
	int datablocks[MAX_DATA_BLOCKS_PER_INODE]; /* max data blocks that can be allocated */
} inode;

#define SUPER_BLOCK_SIZE sizeof(superblock)
#define INODE_BLOCK_SIZE sizeof(inode)

typedef struct {
	char file[MAX_FILE];
	char drive[MAX_DRIVE];
} file_drive;

file_drive fdmap[MAX_DRIVE];

typedef struct {
	char file[MAX_FILE];
} file2delete;

file2delete file_del[MAX_DRIVE];

void init_bitmap();
void tokenize(char *);
void call_sys_calls(char [][10], int);
void mkfs(char *, long, int);
void use(char *, char *);
void writeScr(char *, int, int);
void cp(char *, char *);
void rm(char *);
void ls(char *);
void mv(char *, char *);
void exit_fs();
void init_file_delete();
void manual(char *);

int main()
{
	printf("\033[H\033[J");

	struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    int columns = w.ws_col;
    int rows = w.ws_row;
    writeScr("< [SASH] - [FILE] - [SYSTEM] >\n", w.ws_row, w.ws_col);
    writeScr("------------------------------\n", w.ws_row, w.ws_col);
    writeScr("< Manual Entries Also Available >\n", w.ws_row, w.ws_col);
    writeScr("< Subham Sarkar >\n", w.ws_row, w.ws_col);

	init_bitmap();
	init_file_delete();
	char command[1024];
	while(true)
	{
		printf(ANSI_COLOR_RED "$myfs>" ANSI_COLOR_RESET); fflush(stdout);
		fgets(command, 1024, stdin);
		if(strcmp(command, "\n") == 0) continue;
		command[strlen(command)-1]='\0';
		if(strcmp(command, EXIT) == 0)
		{
			exit_fs();
			exit(0);
		}
		else tokenize(command);
	}
}

static unsigned get_file_size (const char * file_name)
{
    struct stat sb;
    if (stat (file_name, & sb) != 0) exit (EXIT_FAILURE);
    return sb.st_size;
}

void tokenize(char *command)
{
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, " ");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, " ");
	}
	call_sys_calls(subcommands, no_commands);
}

void call_sys_calls(char subcommands[][10], int no_commands)
{
	if(strcmp(subcommands[0], MKFS) == 0) /* make filesystem */
	{
		if(no_commands == 4)
		{
			long fs_size = atoi(subcommands[3]) * 1024 * 1024;
			int block_size = atoi(subcommands[2]);
			mkfs(subcommands[1], fs_size, block_size);
			return;
		}
		else 
		{
			printf("sash: mkfs command failed: expected > mkfs [fs_file] [block size] [fs_size]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], USE) == 0) /* use */
	{
		if(no_commands == 4)
		{
			use(subcommands[1], subcommands[3]);
			return;
		}
		else if((no_commands != 4) || (strcmp(subcommands[2], "as") != 0))
		{
			printf("sash: use command failed: expected > use [filename] [as] [drive]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], CP) == 0) /* copy */
	{
		if(no_commands == 3)
		{
			cp(subcommands[1], subcommands[2]);
			return;
		}
		else
		{
			printf("sash: cp command failed: expected > cp [file a] [file b]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], LS) == 0) /* list */
	{
		if(no_commands == 2)
		{
			ls(subcommands[1]);
			return;
		}
		else
		{
			printf("sash: ls command failed: expected > ls [drive]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], RM) == 0) /* remove */
	{
		if(no_commands == 2)
		{
			rm(subcommands[1]);
			return;
		}
		else
		{
			printf("sash: rm command failed: expected > rm [file]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], MV) == 0) /* move */ 
	{
		if(no_commands == 3)
		{
			mv(subcommands[1], subcommands[2]);
			return;
		}
		else
		{
			printf("sash: mv command failed: expected > mv [source] [destination]\n");
			return;
		}
	}
	else if(strcmp(subcommands[0], MAN) == 0) /* manual */
	{
		if(no_commands == 2)
		{
			manual(subcommands[1]);
			return;
		}
		else
		{
			printf("sash: man command failed: expected > man [sys_call]\n");
			return;
		}
	}
	else
	{
		printf("sash: command not found: %s\n", subcommands[0]);
		return;
	}
}

void init_file_delete()
{
	for(int i = 0; i < MAX_DRIVE; i++) strcpy(file_del[i].file, "\0");
}

void init_bitmap()
{
	for(int i = 0; i < MAX_DRIVE; i++)
	{
		strcpy(fdmap[i].file, "\0");
		strcpy(fdmap[i].drive, "\0");
	}
}

void mkfs(char *path, long fs_size, int block_size)
{
	superblock sb; inode ino;
	sb.fssize = fs_size; /* file system size in blocks */
	sb.root_inode_no = 0; /* stores root (/) inode number */
	sb.block_size = block_size; /* size of block */
	sb.inode_size = INODE_BLOCK_SIZE; /* size of each inode */
	sb.inode_start_location = SUPER_BLOCK_SIZE; /* inode starting location */
	sb.inode_count = INODE_COUNT; /* number of inodes present */
	sb.free_inode_count = sb.inode_count; /* number of free inodes that are yet to be pointed to data blocks */
	sb.data_block_start_location = SUPER_BLOCK_SIZE + INODE_COUNT*INODE_BLOCK_SIZE; /* data block starting location */
	sb.data_block_count = (fs_size - (SUPER_BLOCK_SIZE + (INODE_COUNT*INODE_BLOCK_SIZE)))/block_size; /* count for data blocks */
	sb.free_data_block_count = sb.data_block_count; /* free data blocks */
	
	for(int i = 0; i < IBM_SIZE-1; i++) sb.ibm[i] = 0; /* inode bit map, free */
	for(int i = 0; i < DBM_SIZE - 1; i++) sb.dbm[i] = 0; /* superblock bit map, free */
	
	sb.dbm[DBM_SIZE - 1] = '\0'; /* terminate unsigned char with NULL string */
	sb.ibm[IBM_SIZE - 1] = '\0'; /* terminate unsigned char with NULL string */

	ino.type = 'd'; /* directory */
	ino.size = 0;
	ino.datablock_count = 0;
	for(int i = 0; i < MAX_DB; i++) ino.datablocks[i] = -1; /* Intialise data blocks with -1, indicating free */
	
	int file = open(path, O_RDWR | O_CREAT | O_EXCL);
	if(file == -1) 
	{
		printf("sash: File system exists %s\n", path);
		close(file);
		return;
	}

	int write_sb = write(file, (void *)&sb, SUPER_BLOCK_SIZE);
	if(write_sb == SUPER_BLOCK_SIZE) 
	{
		printf("sash: Superblock write successful\n");
	}
	else
	{
		printf("sash: Superblock write failed\n");
		close(file);
		return;
	}

	int write_in = write(file, (void *)&ino, INODE_BLOCK_SIZE);
	if(write_in == INODE_BLOCK_SIZE) 
	{
		printf("sash: Root inode write successful\n");
	}
	else
	{
		printf("sash: Root inode write failed\n");
		close(file);
		return;
	}

	printf("Size of Super Block: %ld B\n", SUPER_BLOCK_SIZE);
	printf("Size of Inode Block: %ld B\n", INODE_BLOCK_SIZE);
	printf("File System Size: %d KB\n", (int)fs_size/(1024));
	printf("Block Size: %d B\n", (int)block_size);

	for(int i = 0; i < MAX_DRIVE; i++)
	{
		if(strcmp(file_del[i].file, "\0") == 0)
		{
			strcpy(file_del[i].file, path);
			break;
		}
	}
	truncate(path, fs_size);
	close(file);
	return;
}

void use(char *filename, char *dirname)
{
	char drive_n[MAX_DRIVE];
	int i=0; while(dirname[i] != ':')
	{
		drive_n[i] = dirname[i];
		i++;
	}
	drive_n[i]='\0';

	if(access(filename, F_OK) == -1 )
	{
		printf("sash: %s not found. Use [mkfs] first.\n", filename);
		return;
	}
	
	for(int i = 0; i < MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].file, filename) == 0)
		{	
			printf("sash: %s already exists\n", filename);
			return;
		}
		if(strcmp(fdmap[i].drive, drive_n) == 0) 
		{
			printf("sash: %s already exists\n", dirname);
			return;
		}
	}	

	for(int i = 0; i < MAX_DRIVE; i++)
	{
		if((strcmp(fdmap[i].file, "\0") == 0))
		{
			printf("sash: Creating OS File to directory mapping...\n");
			strcpy(fdmap[i].file, filename);
			strcpy(fdmap[i].drive, drive_n);
			printf("sash: [use] system call success\n");
			return;
		}
	}
}

void cp(char *filename, char *drive_file)
{
	superblock sn; inode in;
	int fd_source, fd_destination, fd_dest;
	char drive[MAX_DRIVE];
	char file_n[MAX_FILE];
	int i=0; while(drive_file[i] != ':')
	{
		drive[i] = drive_file[i];
		i++;
	}
	drive[i]='\0';
	int j=0; while(drive_file[i] != '\0')
	{
		i++;
		file_n[j] = drive_file[i];
		j++;
	}
	file_n[j]='\0';
	
	fd_source = open(filename, O_RDONLY);
	if(fd_source == -1)
	{
		printf("sash: File <%s> doesn't exists\n", filename);
		close(fd_source);
		return;
	}

	for(int i=0; i<MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].drive, drive) == 0)
		{
			if(strcmp(fdmap[i].file, "\0")==0)
			{
				fd_dest = open(fdmap[i].file, O_WRONLY | O_CREAT);
				printf("sash: Copy file %s to %s\n", filename, drive_file);
				strcpy(fdmap[i].file, file_n);
				int read_st = read(fd_source, (void *)&sn, sizeof(sn));
				int wr_st = write(fd_dest, (void*)&sn, sizeof(sn));
				read_st = read(fd_source, (void*)&in, sizeof(in));
				wr_st = write(fd_dest, (void*)&in, sizeof(in));
				close(fd_source);
				close(fd_dest);
				return;
			}
			else
			{
				fd_destination = open(fdmap[i].file, O_WRONLY | O_CREAT | O_EXCL);
				if(fd_destination == -1)
				{
					printf("sash: %s already exists. Want to overwrite? Press 1, else 0\n", fdmap[i].file);
					int take; scanf("%d", &take);
					if(take == 0)
					{
						printf("sash: Can't overwrite\n");
						close(fd_source);
						close(fd_destination);
						return;
					}
					else
					{
						strcpy(fdmap[i].file, file_n);
						printf("sash: Copy file %s to %s\n", filename, drive_file);
						int fd_over = open(fdmap[i].file, O_WRONLY | O_CREAT);
						int read_st = read(fd_source, (void *)&sn, sizeof(sn));
						int wr_st = write(fd_over, (void*)&sn, sizeof(sn));
						read_st = read(fd_source, (void*)&in, sizeof(in));
						wr_st = write(fd_over, (void*)&in, sizeof(in));
						close(fd_source);
						close(fd_destination);
						return;
					}
				}
			}
		}
	}
	printf("sash: First use file %s as %s\n", filename, drive_file);
	return;
}

void rm(char *drive_file)
{
	char drive[MAX_DRIVE];
	char file_n[MAX_FILE];
	int i=0; while(drive_file[i] != ':')
	{
		drive[i] = drive_file[i];
		i++;
	}
	drive[i]='\0';
	int j=0; while(drive_file[i] != '\0')
	{
		i++;
		file_n[j] = drive_file[i];
		j++;
	}
	file_n[j]='\0';
	for(int i=0; i<MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].drive, drive) == 0)
		{
			if(strcmp(fdmap[i].file, file_n) == 0)
			{
				remove(file_n);
				strcpy(fdmap[i].file, "\0");
				return;
			}
			else
			{
				printf("sash: File %s isn't present in the Drive %s:\n", file_n, drive);
				return;
			}
		}
	}
	printf("sash: Drive %s: doesn't exists. Cannot proceed operation.\n", drive);
	return;
}

void exit_fs()
{
	for(int i=0; i<MAX_DRIVE; i++)
	{
		if(strcmp(file_del[i].file, "\0") != 0) remove(file_del[i].file);
	}
	return;
}

void ls(char *drive)
{
	char drive_name[MAX_DRIVE];
	int i=0; while(drive[i] != ':')
	{
		drive_name[i] = drive[i];
		i++;
	}
	drive_name[i] = '\0';
	for(int i=0; i<MAX_DRIVE; i++)
	{
		if((strcmp(fdmap[i].drive, drive_name)==0) && (strcmp(fdmap[i].file, "\0") != 0))
		{
			printf("-> %s -> size: %d KB\n", fdmap[i].file, get_file_size(fdmap[i].file)/1024);
			return;
		}
		else if((strcmp(fdmap[i].drive, drive_name)==0) && (strcmp(fdmap[i].file, "\0") == 0))
		{
			printf("sash: No files to be listed\n");
			return;
		}
	}
	printf("sash: Drive %s doesn't exists\n", drive);
	return;
}

void mv(char *source, char *destination)
{
	int fd_source, fd_destination;
	superblock sn; inode in;
	int flagsource = 0, flagdest = 0;
	char source_drive[MAX_DRIVE];
	char dest_drive[MAX_DRIVE];
	char file_source[MAX_FILE], file_dest[MAX_FILE];
	int pos_source, pos_dest;

	int i=0; while(source[i] != ':')
	{
		source_drive[i] = source[i];
		i++;
	}
	source_drive[i] = '\0';
	int j=0; while(source[i] != '\0')
	{
		i++;
		file_source[j] = source[i];
		j++;
	}
	file_source[j]='\0';

	int k=0; while(destination[k] != ':')
	{
		dest_drive[k] = destination[k];
		k++;
	}
	dest_drive[k] = '\0';
	int l=0; while(destination[k] != '\0')
	{
		k++;
		file_dest[l] = destination[k];
		l++;
	}
	file_dest[l]='\0';

	for(int i=0; i<MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].drive, source_drive) == 0)
		{
			printf("Source Drive %s: exists. Proceed.\n", source_drive);
			pos_source = i;
			flagsource = 1;
		}
	}
	
	for(int i=0; i<MAX_DRIVE; i++)
	{
		if(strcmp(fdmap[i].drive, dest_drive) == 0)
		{
			printf("Destination Drive %s: exists. Proceed.\n", dest_drive);
			pos_dest = i;
			flagdest = 1;
		}
	}

	if(flagsource == 0 || flagdest == 0)
	{
		if(flagsource == 0)
		{
			printf("sash: Source Drive %s: doesn't exists\n", source_drive);
		}
		if(flagdest == 0)
		{
			printf("sash: Destination Drive %s: doesn't exists\n", dest_drive);
		}
		return;
	}
	else
	{
		fd_source = open(fdmap[pos_source].file, O_RDONLY);
		if(fd_source == -1)
		{
			printf("sash: %s doesn't exists in %s:\n", file_source, source_drive);
			close(fd_source);
			return;
		}
		fd_destination = open(fdmap[pos_dest].file, O_WRONLY | O_CREAT | O_EXCL);
		if(fd_destination == -1)
		{
			printf("sash: %s already exists. Want to overwrite? Press 1, else 0\n", file_dest);
			int take; scanf("%d", &take);
			if(take == 0)
			{
				printf("sash: Can't overwrite\n");
				close(fd_source);
				close(fd_destination);
				return;
			}
			else
			{
				strcpy(fdmap[pos_dest].file, file_dest);
				int fd_over = open(fdmap[pos_dest].file, O_WRONLY | O_CREAT);
				int read_st = read(fd_source, (void *)&sn, sizeof(sn));
				int wr_st = write(fd_over, (void*)&sn, sizeof(sn));
				read_st = read(fd_source, (void*)&in, sizeof(in));
				wr_st = write(fd_over, (void*)&in, sizeof(in));
				strcpy(fdmap[pos_source].file, "\0");
				close(fd_source);
				close(fd_destination);
				return;
			}
		}
		else
		{
			strcpy(fdmap[pos_dest].file, file_dest);
			int read_st = read(fd_source, (void *)&sn, sizeof(sn));
			int wr_st = write(fd_destination, (void*)&sn, sizeof(sn));
			read_st = read(fd_source, (void*)&in, sizeof(in));
			wr_st = write(fd_destination, (void*)&in, sizeof(in));
			strcpy(fdmap[pos_source].file, "\0");
			close(fd_source);
			close(fd_destination);
			return;
		}
	}
}

#define EXIT "exit"
#define MKFS "mkfs"
#define USE "use"
#define MV "mv"
#define RM "rm"

void manual(char *path)
{
	if(strcmp(path, MV) == 0)
	{
	printf("sash: mv [drive_source_name][:][file_source_name] [drive_dest_name][:][file_dest_name]\n");
		return;
	}
	else if(strcmp(path, LS) == 0)
	{
		printf("sash: ls [drive_name][:]\n");
		return;
	}
	else if(strcmp(path, CP) == 0)
	{
		printf("sash: cp [file_source_name] [drive_dest_name][:][file_dest_name]\n");
		return;
	}
	else if(strcmp(path, EXIT) == 0)
	{
		printf("sash: exit\n");
		return;
	}
	else if(strcmp(path, RM) == 0)
	{
		printf("sash: rm [drive_name][:][file_name]\n");
	}
	else if(strcmp(path, USE) == 0)
	{
		printf("sash: use [file] [as] [drive_name][:]\n");
		return;
	}
	else if(strcmp(path, MKFS) == 0)
	{
		printf("sash: mkfs [file] [block_size in B] [file_system_size in MB]\n");
		return;
	}
	else
	{
		printf("sash: No manual entry\n");
		return;
	}
}

void writeScr(char *string, int rows, int cols)
{
    int vertl = rows/2;
    int hortl = 0;
    int stringLength = strlen(string) / 2;
    hortl = (cols - strlen(string))/2;
    for (int x = 0; x <= rows; x++) if (x == vertl) printf("%*s", cols / 2 + stringLength, string );
}