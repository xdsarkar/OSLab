#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <sys/ioctl.h>
#include <math.h>
#include <stdbool.h>

#define SB_SIZE sizeof(superblock)
#define INODE_SIZE sizeof(inode)
#define MAX_INODE 10
#define MAX_DATA_BLOCK 20
#define MAX_FILE 5
#define MAX_DRIVE 5
#define MAX 5


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
#define CD "cd"
#define MKFILE "mkfile"

typedef struct {
	int fsize; /* file system size in blocks */
	int root_inode_no; /* stores root (/) inode number */
	int block_size; /* size of block */
	int inode_size; /* size of each inode */
	int inode_start_location; /* inode starting location */
	int inode_count; /* number of inodes present */
	int free_inode_count; /* number of free inodes that are yet to be pointed to data blocks */
	int data_block_start_location; /* data block starting location */
	int data_block_count; /* count for data blocks */
	int free_data_block_count; /* free data blocks */
	char ibm[MAX_INODE];
	char dbm[MAX_DATA_BLOCK];
} superblock;

typedef struct {
	char type; /* d: directory, f: file, l: symbolic link, etc. */
	int data_size; /* size of corresponding file */
    int dblock_start_loc; /* starting location of datablocks */
	int dblock_count; /* no of datablocks used */
	int i_dbm[MAX_DATA_BLOCK]; /* inode data block bitmap */
    char filename[20]; /* name of the file pointrd by the inode */
    int inode_no; /* inode no assigned to it */
} inode;

typedef struct {
	char osfile[MAX_FILE];
	char drivename;
} drives;

drives fdmap[MAX];

int mount_count = 0;
char curr_drive;
int osfile_count = 0;

void tokenize(char *);
void call_sys_calls(char [][10], int);
void create_fs(char *, int, int);

void use(char *, char*);
void writeScr(char *, int, int);
void cp(char *, char *);
void ls();
void cd( char *);
void mkfile(char *, char*);

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

	char command[1024];
	while(true)
	{
		printf(ANSI_COLOR_RED "$myfs>" ANSI_COLOR_RESET); fflush(stdout);
		fgets(command, 1024, stdin);
		if(strcmp(command, "\n") == 0) continue;
		command[strlen(command)-1]='\0';
		if(strcmp(command, EXIT) == 0)
		{
			exit(0);	
		}
		else tokenize(command);
	}
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
			long fs_size = atoi(subcommands[3])*1024*1024;
			int block_size = atoi(subcommands[2]);
			create_fs(subcommands[1], fs_size, block_size);
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
		if(no_commands == 1)
		{
			ls();
			return;
		}
		else
		{
			printf("sash: ls command failed: expected > ls\n");
			return;
		}
	}
    else if(strcmp(subcommands[0], CD) == 0) /* change directory */
    {
        if(no_commands == 2)
        {
            cd(subcommands[1]);
            return;
        }
        else
        {
            printf("sash: cd command failed: expected > cd [drive]\n");
            return;
        }
    }
	else if(strcmp(subcommands[0], RM) == 0) /* remove */
	{
		if(no_commands == 2)
		{
			//remove_file(subcommands[1]);
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
			//move_file(subcommands[1], subcommands[2]);
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
	else if(strcmp(subcommands[0], MKFILE) == 0)
	{
		if(no_commands <= 3)
		{
			mkfile(subcommands[1], subcommands[2]);
			return;
		}
		else
		{
			printf("sash: mkfile command failed: expected > mkfile [filename]\n");
			return;
		}
	}
	else
	{
		printf("sash: command not found: %s\n", subcommands[0]);
		return;
	}
}

void create_fs(char *path, int fs_size, int block_size)
{
	superblock sb; inode in;
	int fd = open(path, O_RDWR | O_CREAT | O_EXCL, 0777);
    if(fd<0)
    {
        printf("Error opening file system created!\n");
        return;
    }
    lseek(fd, 0, SEEK_SET);
    int read_status = write(fd,(void *)&sb, sizeof(sb));
    if(read_status < 0)
    {
        fprintf(stderr, "Error: reading superblock\n");
        return;
    }
    lseek(fd, sizeof(sb), SEEK_SET);
    read_status = write(fd,(void *)&in,sizeof(in));
    if(read_status < 0)
    {
        fprintf(stderr, "Error: reading root inode\n");
        return;
    }

    printf("File size: %d MB\n", fs_size/(1024*1024));
    int status = ftruncate(fd,fs_size*1024*1024);
    if(status == -1)
    {
        printf("sash: Truncate error!\n");
        return;
    }

    sb.fsize = fs_size;
    sb.block_size = block_size;
    sb.root_inode_no = 0;
    sb.inode_size = INODE_SIZE;
    sb.inode_count = MAX_INODE;
    sb.inode_start_location = SB_SIZE;
    sb.free_inode_count = MAX_INODE;
    sb.data_block_start_location = SB_SIZE + INODE_SIZE*MAX_INODE;
    sb.data_block_count = (sb.fsize - sb.data_block_start_location)/sb.block_size;
    sb.free_data_block_count = sb.data_block_count;

    for(int i=0;i<MAX_INODE;i++) sb.ibm[i] = '0';
    for(int i=0;i<MAX_DATA_BLOCK;i++) sb.dbm[i] = '0';
    sb.ibm[0] = '1';
    sb.free_inode_count--;
    in.type = 'd';
    in.data_size = 0;
    in.dblock_count = 0;
    strcpy(in.filename, "root");
    in.dblock_start_loc = SB_SIZE + MAX_INODE*INODE_SIZE; 

    for(int i=0;i<MAX_DATA_BLOCK;i++) in.i_dbm[i] = 0;

    lseek(fd,0,SEEK_SET);
    int write_status = write(fd,(void *)&sb, SB_SIZE); /* write to superblock*/
    if(write_status == SB_SIZE) printf("sash: Superblock write successful!\n");
    else 
    {
    	printf("sash: Error: Writing to superblock!\n");
    	return;
    }
    lseek(fd,sizeof(sb),SEEK_SET);
    write_status = write(fd,(void *)&in,INODE_SIZE); /* Root Inode write */
    if(write_status == INODE_SIZE) printf("sash: Root inode write successful!\n");
    else
    {
    	printf("sash: Error: Writing to root inode!\n");
    	return;
    }
    close(fd);
}

void use(char *file1, char *file2)
{
	for(int i = 0; i < mount_count; i++)
	{
		if(strcmp(fdmap[i].osfile, file1) == 0)
		{
			printf("%s already exists!\n", file1);
			return;
		}
		else if(fdmap[i].drivename == file2[0])
		{ 
			printf("%c already exists!\n", file2[0]);
			return;
		}
	}
    strncpy(fdmap[mount_count].osfile, file1, strlen(file1));
    fdmap[mount_count].osfile[strlen(file1)+1] = '\0';
    fdmap[mount_count].drivename = file2[0];
    printf("sash: Filesystem %s mounted as %c:\n", fdmap[mount_count].osfile, fdmap[mount_count].drivename);
    ++mount_count;
    return;
}

void ls()
{
    char file_name[20];
    superblock sb; inode in;

    for(int i=0; i<mount_count; i++)
    {
        if(fdmap[i].drivename == curr_drive)
        {
            strcpy(file_name, fdmap[i].osfile);
            break;
        }
    }

    int fd = open(file_name, O_RDWR);
    if(fd < 0)
    {
        printf("sash: Error while opening file system\n");
        return;
    }

    lseek(fd, 0, SEEK_SET);

    int read_status= read(fd, (void *)&sb, sizeof(sb));
    if(read_status == -1)
    {
        printf("sash: Unable to read superblock\n");
        return;
    }

    printf("sash: Used inodes: %d\n", sb.inode_count - sb.free_inode_count);
    for(int i=1; i<MAX_INODE; i++)
    {
        if(sb.ibm[i] == '0') continue;
        lseek(fd, 0, SEEK_SET);
        lseek(fd, sizeof(sb) + sizeof(in)*(i),SEEK_SET);
        read_status= read(fd,(void *)&in, sizeof(in));
        if(read_status==-1)
        {
            printf("sash: Unable to read inode\n");
            return;
        }
        printf("%s : %d KB\n", in.filename, in.data_size/(1024));
    }
    close(fd);
}

void cd(char *buf)
{
    curr_drive = buf[0];
    printf("Current working directory -> %c:\n", buf[0]);
    return;
}

void cp(char *file1, char *file2)
{
    if(file1[1]==':' && file2[1]==':')
    {
        int i, p, tmp1, tmp2;
    	char c1 = file1[0];
    	char c2 = file2[0];
        superblock sb1, sb2;
        inode in1, in2;
    	char fs_name1[20], fs_name2[20], fname1[20];;
    	
        for(i=0; i<mount_count; i++)
    	{
    		if(c1 == fdmap[i].drivename) strcpy(fs_name1, fdmap[i].osfile);
    		if(c2 == fdmap[i].drivename) strcpy(fs_name2, fdmap[i].osfile);
    	}
    	
        int fd1 = open(fs_name1, O_RDWR);
    	if(fd1 < 0)
    	{
    		fprintf(stderr, "sash: Error: Unable to open file\n");
    		return;
    	}
    	
        int fd2 = open(fs_name2,O_RDWR);
    	if(fd2 < 0)
    	{
    		fprintf(stderr, "sash: Error: Unable to open file\n");
    		return;
    	} 
    	
        lseek(fd1, 0, SEEK_SET);
    	
        int read_status = read(fd1, (void *)&sb1, sizeof(sb1));
    	if(read_status == -1)
    	{
    		fprintf(stderr, "sash: Error: Unable to read file\n");
    		return;
    	}
    	
        lseek(fd2, 0, SEEK_SET);
    	
        read_status = read(fd2, (void *)&sb2, sizeof(sb2));
        if(read_status == -1)
    	{
    		fprintf(stderr, "sash: Error: Unable to read file\n");
    		return;
    	}
    	
        for(i=2; i<strlen(file1); i++) fname1[i-2]=file1[i]; /* C:[file] */
    	fname1[i]='\0';

    	for(i=0; i<MAX_INODE; i++)
    	{
    		if(sb1.ibm[i]=='1')
    		{
    			lseek(fd1, sizeof(sb1)+sizeof(inode)*i, SEEK_SET);
    			read_status = read(fd1, (void *)&in1, sizeof(in1));
                if(read_status==-1)
    			{
    				fprintf(stderr, "sash: Error: Unable to read inode\n");
    				return;
    			}
    			if(strncmp(in1.filename, fname1, strlen(in1.filename)) == 0) break; /* success reading inode */
    		}
    	}

        for(p=0; p<MAX_INODE; p++)
    	{
    		if(sb2.ibm[p]=='0')
    		{
    			sb2.ibm[p]='1';
    			sb2.free_inode_count--;
                tmp1 = p; /* inode no just allocated */
    			break;
    		}
    	}
    	
        char buffer[in1.dblock_count*sb1.block_size];

        lseek(fd1, 0, SEEK_SET) ;
        lseek(fd1, sizeof(sb1) + (sizeof(in1)*MAX_INODE) + in1.dblock_start_loc, SEEK_SET);
        
        read_status = read(fd1, &buffer, sizeof(buffer)); 
    	in2 = in1;
    	in2.inode_no = tmp1;

    	for(p=0; p<MAX_DATA_BLOCK; p++)
    	{
    		if(sb2.dbm[p]=='0')
    		{
                tmp2 = p;
    			break;
    		}
    	} 

    	sb2.free_data_block_count -= in1.dblock_count;
    	for(i=p; i<(in1.dblock_count+tmp2); i++) sb2.dbm[i] = '1';

    	in2.dblock_start_loc = tmp2;

        lseek(fd2, 0, SEEK_SET);
        lseek(fd2,sizeof(sb2) + (sizeof(in2)*MAX_INODE) + in2.dblock_start_loc, SEEK_SET);
        int write_status= write(fd2, &buffer, sizeof(buffer));
        if(write_status == -1)
        {
            fprintf(stderr,"sash: Error: Unable to write from the buffer\n");
            return;
        }
    	lseek(fd2,0,SEEK_SET);
    	write_status = write(fd2,(void *)&sb2, sizeof(sb2));
    	if(write_status == -1)
    	{
    		fprintf(stderr,"sash: Error: Unable to write back into file\n");
    		return;
    	}
    	lseek(fd2, sizeof(sb2)+sizeof(in2)*in2.inode_no, SEEK_SET);
    	write_status= write(fd2,(void *)&in2,sizeof(in2));
    	if(write_status == -1)
    	{
    		fprintf(stderr, "sash: Error: Unable to write into file\n");
    		return;
    	}
        close(fd1);
        close(fd2);
    }
    else
    {
    	fprintf(stderr, "sash: Error: Unable to locate\n");
    	return;
    }
}

void mkfile(char *file, char *file_size)
{
    char file1[20], file_name[20];
    int file_sz;
    superblock sb;
    inode in;
    int i=0, j=0; 

    strcpy(file1, file);
    file_sz = atoi(file_size);

    long int file_sz_bytes = file_sz*1024;
    for(int i=0; i<mount_count; i++)
    {
        if(fdmap[i].drivename == curr_drive)
        {
            strcpy(file_name, fdmap[i].osfile);
            break;
        }
    }
    printf("sash: File system (OSFILE): %s\n", file_name);
    int fd = open(file_name, O_RDWR);
    if(fd < 0)
    {
        printf("sash: Error while opening file system\n");
        return;
    }
    lseek(fd,0,SEEK_SET);
    int read_status= read(fd,(void *)&sb, sizeof(sb));
    if(read_status==-1)
    {
        printf("sash: Error while reading superblock\n");
        return;
    }
    
    for(i=0; i<MAX_INODE; i++)
    {
        if(sb.ibm[i]=='0')
        {
            sb.ibm[i]='1';
            sb.free_inode_count--;
            printf("sash: Free Inode Count: %d. Success.\n",sb.free_inode_count);
            for(j=0; j<MAX_DATA_BLOCK; j++)
            {
                if(sb.dbm[j]=='0')
                {
                    sb.dbm[j]='1';
                    break;
                }
            }
            break;
        }
    }
    
    int dblocks = ceil(file_sz_bytes/sb.block_size);
    in.inode_no = i;
    in.type = 'f';
    in.data_size = file_sz_bytes;
    strcpy(in.filename, file1);
    in.dblock_count = dblocks;
    in.dblock_count++;
    in.dblock_start_loc = j;
    for(int k=0; k<MAX_DATA_BLOCK; k++) in.i_dbm[k] = -1;
    lseek(fd,0,SEEK_SET);
    lseek(fd,sizeof(sb)+sizeof(in)*i, SEEK_SET);
    int write_status= write(fd, (void *)&in, sizeof(in));
    if(write_status == -1)
    {
        printf("sash: Error while writing into the inode\n");
        return;
    }
    for(int k=j; k < (dblocks+j); k++) sb.dbm[k]='1';
    sb.free_data_block_count -= dblocks;
    lseek(fd,0,SEEK_SET);
    write_status= write(fd, (void *)&sb, sizeof(sb));
    if(write_status == -1)
    {
        printf("sash: Error while writing back into the superblock\n");
        return;
    }
    close(fd);
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
	else if(strcmp(path, MKFILE) == 0)
	{
		printf("sash: mkfile [file] [file_size in KB]\n");
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