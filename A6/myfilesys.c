#include<stdio.h>
#include<stdlib.h>   /* for atoi() and system() */
#include<string.h>  /* for strlen() strcmp() etc */
#include<sys/types.h> /* for open() */
#include<sys/stat.h>  /* for open() */
#include<fcntl.h> 	   /* for open() write() */
#include<unistd.h>    /* for close() */
#include<math.h>       /*for ceil() */

#define SB_SIZE sizeof(sblk)
#define INODE_SIZE sizeof(i_node)
#define MAX_INODE 15
#define MAX_DATA_BLOCK 20
#define MAX 5

                /* ===================== SUPERBLOCK STRUCTURE ========================*/
typedef struct superblock							 
{
	unsigned long int 	fsize;					//file system size
	unsigned int 		root_inode_no;		//root inode 
	unsigned int 		block_size;
	unsigned int 		inode_size;
	unsigned int 		inode_count;
	unsigned long int 	inode_start_loc;
	unsigned int 		free_inode_count;
	unsigned long int 	dblock_start_loc;
	unsigned int 		dblock_count;
	unsigned int 		free_dblock_count;
	char dbm[MAX_DATA_BLOCK];
	char ibm[MAX_INODE];

}sblk;
        
                /* ===================== GLOBAL VARIABLES ========================*/
int mount_count = 0;
char curr_drive;
int osfile_count = 0;

            /* ===================== INODE STRUCTURE ========================*/

typedef struct inode 						
{
	char type;                     //type of data pointed by the inode : d for data, f for file
	int data_size;                 //size of the data that is kept in the datablocks,pointed by the inode
    int dblock_start_loc;          // starting location of datablocks
	int dblock_count;              //no of datablocks used
	int i_dbm[MAX_DATA_BLOCK];    //inode data block bitmap
    char filename[20];             //name of the file pointrd by the inode
    int inode_no;                   //inode no assigned to it
}i_node;


typedef struct mydrives
{
	char drivename;
	char osfile[MAX];
}drives;

drives arr[MAX];


void mkfs(char buf[])
{
	sblk sb;
	i_node inode;
	int i=0;
	char *token;
	int fd;
	size_t sz;
	char file1[20];
	int filesz,blocksz;

	token = strtok(buf," ");

	while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
        	printf("Filename: %s\n",token);
        	strcpy(file1,token);
        }
        else if(i == 2)
        {
        	printf("Block size: %d B\n",atoi(token));
        	blocksz = atoi(token);
        }
        else if(i == 3)
        {
        	filesz = atoi(token);
        }
        else
        	break;
    }

    fd = open(file1,O_RDWR | O_CREAT | O_EXCL);
    if(fd<0)
    {
        printf("Error opening file system created!");
        return;
    }							//opening OS file

    lseek(fd,0,SEEK_SET);
    int read_status = write(fd,(void *)&sb, sizeof(sb));          /* write to superblock*/
    if(read_status < 0)
    {
        fprintf(stderr, "Error : reading superblock\n" );
    }
        
    lseek(fd,sizeof(sb),SEEK_SET);
    read_status = write(fd,(void *)&inode,sizeof(inode));         /* Root Inode write */
    if(read_status < 0)
    {
        fprintf(stderr, "Error : reading root inode\n" );
    }
   				
                /* Superblock Specialization */
    char ch;
    int status;

    printf("Type 'k' for KB or 'm' for MB ? : ");
 Y: scanf("%c",&ch);
    getchar();
    printf("\n");
    if(ch == 'k')
    {
        printf("File size: %d kB\n",filesz);
        status = ftruncate(fd,filesz*1024);
        if(status == -1)
        {
            printf("Truncate error!\n");
            return;
        }
        sb.fsize = filesz*1024;
    }
    else if(ch == 'm')
    {
        printf("File size: %d MB\n",filesz);
        status = ftruncate(fd,filesz*1024*1024);
        if(status == -1)
        {
            printf("Truncate error!\n");
            return;
        }
        sb.fsize = filesz*1024*1024;

    }
    else
    {
        printf("re-enter filesize \n");
        goto Y;
    }
    
 
    sb.block_size = blocksz;
    sb.root_inode_no = 0;
    sb.inode_size = INODE_SIZE;
    sb.inode_count = MAX_INODE;
    sb.inode_start_loc = SB_SIZE;
    sb.free_inode_count = MAX_INODE;
    sb.dblock_start_loc = SB_SIZE + INODE_SIZE*MAX_INODE;
    sb.dblock_count = (sb.fsize - sb.dblock_start_loc)/sb.block_size;
    sb.free_dblock_count = sb.dblock_count;

    for(int i=0;i<MAX_INODE;i++)
    	   sb.ibm[i] = '0';

    for(int i=0;i<MAX_DATA_BLOCK;i++)
    	   sb.dbm[i] = '0';

    						/* Root Node Initialization */

    sb.ibm[0] = '1';			//root inode loaded
    sb.free_inode_count--;  	//decrementing free inode count
    inode.type = 'd';
    inode.data_size = 0;
    inode.dblock_count = 0;
    strcpy(inode.filename,"root");
    inode.dblock_start_loc = SB_SIZE + MAX_INODE*INODE_SIZE; 

    for(int i=0;i<MAX_DATA_BLOCK;i++)
    		inode.i_dbm[i] = 0;

    lseek(fd,0,SEEK_SET);
    int write_status = write(fd,(void *)&sb, SB_SIZE);			/* write to superblock*/
    if(write_status == SB_SIZE)
    	printf("Superblock Write successful!\n");
    else
    	printf("Error while writing to superblock!!\n");

    lseek(fd,sizeof(sb),SEEK_SET);
    write_status = write(fd,(void *)&inode,INODE_SIZE);			/* Root Inode write */
    if(write_status == INODE_SIZE)
    	printf("Root Inode Write successful!\n");
    else
    	printf("Error while writing to root inode!!\n");

    close(fd);
}

void use(char buf[])
{
	int i=0;
	char *token;
	char file1[20],temp[10];
    char file2;
	token = strtok(buf," ");

	while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
        	strcpy(file1,token);
        }
        else if(i == 2)
        {
        	strcpy(temp,token);
        }
        else if(i == 3)
        {
        	file2 = *token;
        }
        else if(i >= 4)
        	break;
    }
    if(strcmp(temp,"as") != 0)
    {
    	printf("Wrong entry format!!\n");
        return;
    }

	for(int i = 0; i < mount_count; i++)
	{
		if(strcmp(arr[i].osfile, file1) == 0)
		{
			printf("%s already exists!!\n", file1);
			return;
		}
		else if(arr[i].drivename == file2)
		{ 
			printf("%c already exists!!\n",file2);
			return;
		}
	
	}
    int len1 = strlen(file1);

    strncpy(arr[mount_count].osfile,file1,len1);
    arr[mount_count].drivename = file2;
    mount_count++;

}


void copy(char buf[])
{
	int i=0,p=0,q=0;
	char *token;
    char file1[20];
    char file2[20];

	token = strtok(buf," ");

	while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
        	printf("source: %s\n",token);
            strcpy(file1,token);
        }
        else if(i == 2)
        {
        	printf("destination: %s\n",token);
            strcpy(file2,token);
        }
        else
        	break;
    }
    if(file1[1]==':' && file2[1]==':')
    {
    	char c1,c2;
    	c1= file1[0];
    	c2= file2[0];
    	char fs_name1[20],fs_name2[20];
    	for(int i=0; i<mount_count; i++)
    	{
    		if(c1==arr[i].drivename)
    			strcpy(fs_name1,arr[i].osfile);
    		if(c2==arr[i].drivename)
    			strcpy(fs_name2,arr[i].osfile);
    	}
    	int fd1, fd2;

    	fd1= open(fs_name1, O_RDWR);
    	if(fd1==-1)
    	{
    		fprintf(stderr, "Error: Unable to open file\n");
    	}

    	fd2= open(fs_name2,O_RDWR);
    	if(fd2==-1)
    	{
    		fprintf(stderr, "Error: Unable to open file\n");
    	} 

    	sblk sb1,sb2;
    	
        lseek(fd1,0,SEEK_SET);
    	int read_status= read(fd1,(void *)&sb1, sizeof(sb1));
    	if(read_status==-1)
    	{
    		fprintf(stderr, "Error: Unable to read file\n");
    	}

    	lseek(fd2,0,SEEK_SET);
    	read_status= read(fd2,(void *)&sb2, sizeof(sb2));
    	if(read_status==-1)
    	{
    		fprintf(stderr, "Error: Unable to read file\n");
    	}

    	char fname1[20];
    	int l = strlen(file1);
    	for(i=2; i<l; i++)
    	{
    		fname1[i-2]=file1[i];
    	}
    	fname1[i]='\0';

    	i_node in1, in2;

    	for(i=0; i<MAX_INODE; i++)
    	{
    		if(sb1.ibm[i]=='1')
    		{
    			lseek(fd1,sizeof(sb1)+sizeof(i_node)*i,SEEK_SET);

    			read_status = read(fd1,(void *)&in1, sizeof(in1));
    			
                if(read_status==-1)
    			{
    				fprintf(stderr, "Error: Unable to read inode\n");
    			}
    			if(strncmp(in1.filename,fname1,strlen(in1.filename))==0)
    				break;
    		}
    	}


        int temp1;
    	for(p=0; p<MAX_INODE; p++)
    	{
    		if(sb2.ibm[p]=='0')
    		{
    			sb2.ibm[p]='1';
    			sb2.free_inode_count--;
                temp1 = p;
    			break;
    		}
    	}

        char buffer[in1.dblock_count*sb1.block_size];

        lseek(fd1 ,0 ,SEEK_SET) ;
        lseek(fd1 ,sizeof(sb1) +(sizeof(in1)*MAX_INODE)+in1.dblock_start_loc ,SEEK_SET) ;
        
        read_status = read(fd1 ,&buffer ,sizeof(buffer)) ;
        if(read_status==-1)
        {
            fprintf(stderr, "Error: Unable to read into the buffer\n");
        }
         
    	in2 = in1;
    	in2.inode_no = temp1;
        int temp2;

    	for(p=0; p<MAX_DATA_BLOCK; p++)
    	{
    		if(sb2.dbm[p]=='0')
    		{
                temp2 =p;
    			break;
    		}
    	} 

    	sb2.free_dblock_count -= in1.dblock_count;
       

    	for(i=p; i<(in1.dblock_count+temp2); i++)
    	{
    		sb2.dbm[i]='1';
    	} 

    	in2.dblock_start_loc = temp2;

        lseek(fd2 ,0 ,SEEK_SET) ;
        lseek(fd2 ,sizeof(sb2) +(sizeof(in2)*MAX_INODE)+in2.dblock_start_loc ,SEEK_SET) ;

        int write_status= write(fd2,&buffer, sizeof(buffer));
        if(write_status==-1)
        {
            fprintf(stderr,"Error: Unable to write from the buffer\n");
        }

    	lseek(fd2,0,SEEK_SET);

    	write_status= write(fd2,(void *)&sb2, sizeof(sb2));
    	if(write_status==-1)
    	{
    		fprintf(stderr,"Error: Unable to write back into file\n");
    	}

    	lseek(fd2,sizeof(sb2)+sizeof(in2)*in2.inode_no, SEEK_SET);

    	write_status= write(fd2,(void *)&in2,sizeof(in2));

    	if(write_status==-1)
    	{
    		fprintf(stderr, "Error: Unable to write back into file\n");
    	}
        close(fd1);
        close(fd2);
    }

    else
    {
    	fprintf(stderr, "Error: Unable to find location\n");
    }
}


void move(char buf[])
{
    int i=0,p=0;
    char *token;
    char file1[20];
    char file2[20];

    token = strtok(buf," ");

    while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
            printf("source: %s\n",token);
            strcpy(file1,token);
        }
        else if(i == 2)
        {
            printf("destination: %s\n",token);
            strcpy(file2,token);
        }
        else
            break;
    }
    if(file1[1]==':' && file2[1]==':')
    {
        char c1,c2;
        c1= file1[0];
        c2= file2[0];
        char fs_name1[20],fs_name2[20];
        for(int i=0; i<mount_count; i++)
        {
            if(c1 == arr[i].drivename)
                strcpy(fs_name1,arr[i].osfile);
            if(c2 == arr[i].drivename)
                strcpy(fs_name2,arr[i].osfile);
        }
        int fd1, fd2;

        fd1= open(fs_name1, O_RDWR);
        if(fd1==-1)
        {
            fprintf(stderr, "Error: Unable to open file\n");
        }

        fd2= open(fs_name2,O_RDWR);
        if(fd2==-1)
        {
            fprintf(stderr, "Error: Unable to open file\n");
        } 

        sblk sb1,sb2;
        
        lseek(fd1,0,SEEK_SET);
        int read_status= read(fd1,(void *)&sb1, sizeof(sb1));
        if(read_status==-1)
        {
            fprintf(stderr, "Error: Unable to read file\n");
        }

        lseek(fd2,0,SEEK_SET);
        read_status= read(fd2,(void *)&sb2, sizeof(sb2));
        if(read_status==-1)
        {
            fprintf(stderr, "Error: Unable to read file\n");
        }

        char fname1[20];
        int l = strlen(file1);
        for(i=2; i<l; i++)
        {
            fname1[i-2]=file1[i];
        }
        fname1[i]='\0';

        i_node in1, in2;
        int temp;

        for(i=0; i<MAX_INODE; i++)
        {
            if(sb1.ibm[i]=='1')
            {
                lseek(fd1,sizeof(sb1)+sizeof(i_node)*i,SEEK_SET);

                read_status= read(fd1,(void *)&in1, sizeof(in1));
                
                if(read_status==-1)
                {
                    fprintf(stderr, "Error: Unable to read inode\n");
                }
                if(strncmp(in1.filename,fname1,strlen(in1.filename))==0)
                {
                    temp = i;
                    break;
                }
            }
        }
        
        int temp1;
        for(p=0; p<MAX_INODE; p++)
        {
            if(sb2.ibm[p]=='0')
            {
                sb2.ibm[p]='1';
                sb2.free_inode_count--;
                temp1 = p;
                break;
            }
        }

        char buffer[in1.dblock_count*sb1.block_size];

        lseek(fd1 ,0 ,SEEK_SET) ;
        lseek(fd1 ,sizeof(sb1) +(sizeof(in1)*MAX_INODE)+in1.dblock_start_loc ,SEEK_SET) ;
        
        read_status = read(fd1 ,&buffer ,sizeof(buffer)) ;
        if(read_status==-1)
        {
            fprintf(stderr, "Error: Unable to read into the buffer\n");
        }
         

        in2 = in1;
        in2.inode_no = temp1;
        int temp2;

        for(p=0; p<MAX_DATA_BLOCK; p++)
        {
            if(sb2.dbm[p]=='0')
            {
                temp2 = p;
                break;
            }
        } 

        sb2.free_dblock_count -= in1.dblock_count;
        

        for(i=temp2; i<(in1.dblock_count+temp2); i++)
        {
            sb2.dbm[i]='1';
        } 

        sb1.free_inode_count++;
        sb1.free_dblock_count += in1.dblock_count;

        for(p=in1.dblock_start_loc;p<in1.dblock_start_loc+in1.dblock_count;p++)
        {
            sb1.dbm[p] = '0';
        }

        sb1.ibm[temp] = '0';

        remove(in1.filename);

        in2.dblock_start_loc = temp2;


        lseek(fd2 ,0 ,SEEK_SET) ;
        lseek(fd2 ,sizeof(sb2) +(sizeof(in2)*MAX_INODE)+in2.dblock_start_loc ,SEEK_SET) ;

        int write_status= write(fd2,&buffer, sizeof(buffer));
        if(write_status==-1)
        {
            fprintf(stderr,"Error: Unable to write from the buffer\n");
        }

        bzero(buffer,in1.dblock_count*sb1.block_size);

        lseek(fd1 ,0 ,SEEK_SET) ;
        lseek(fd1 ,sizeof(sb1) +(sizeof(in1)*MAX_INODE)+in1.dblock_start_loc ,SEEK_SET) ;

        write_status= write(fd1,&buffer, sizeof(buffer));
        if(write_status==-1)
        {
            fprintf(stderr,"Error: Unable to clear memory\n");
        }


        lseek(fd1,0,SEEK_SET);

        write_status= write(fd1,(void *)&sb1, sizeof(sb1));
        if(write_status==-1)
        {
            fprintf(stderr,"Error: Unable to write back into file\n");
        }

        lseek(fd1,sizeof(sb1)+sizeof(in1)*in1.inode_no, SEEK_SET);

        write_status= write(fd1,(void *)&in1,sizeof(in2));

        if(write_status==-1)
        {
            fprintf(stderr, "Error: Unable to write back into file\n");
        }

        lseek(fd2,0,SEEK_SET);

        write_status= write(fd2,(void *)&sb2, sizeof(sb2));
        if(write_status==-1)
        {
            fprintf(stderr,"Error: Unable to write back into file\n");
        }

        lseek(fd2,sizeof(sb2)+sizeof(in2)*in2.inode_no, SEEK_SET);

        write_status= write(fd2,(void *)&in2,sizeof(in2));

        if(write_status==-1)
        {
            fprintf(stderr, "Error: Unable to write back into file\n");
        }

        close(fd1);
        close(fd2);
    }
    else
    {
        fprintf(stderr, "Error: Unable to find location\n");
    }
}


void ls(char buf[])
{
	int i=0;

    sblk sb;

    char file_name[20];
    for(int i=0; i<mount_count; i++)
    {
        if(arr[i].drivename == curr_drive)
        {
            strcpy(file_name,arr[i].osfile);
            break;
        }
    }

    int fd = open(file_name,O_RDWR);
    if(fd < 0)
    {
        printf("Error while opening file system\n");
        return;
    }

    i_node inode;

    lseek(fd,0,SEEK_SET);

    int read_status= read(fd,(void *)&sb, sizeof(sb));
    if(read_status==-1)
    {
        printf("Unable to read superblock\n");
        return;
    }

    printf("Used Inodes: %d\n",sb.inode_count-sb.free_inode_count);
    for(int i=1; i<MAX_INODE; i++)
    {
        if(sb.ibm[i]=='0')
            continue;
        lseek(fd,0,SEEK_SET);
        lseek(fd,sizeof(sb)+sizeof(inode)*(i),SEEK_SET);
        read_status= read(fd,(void *)&inode, sizeof(inode));
        if(read_status==-1)
        {
            printf("Unable to read inode\n");
            return;
        }
        printf("%s : ",inode.filename);
        printf("%dB\n",inode.data_size);
    }
    close(fd);
}


void rm(char buf[])
{
    int i=0;
    char *token;
    char file1[20];

    token = strtok(buf," ");

    while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
            strcpy(file1,token);
        }
        else
            break;
    }

    sblk sb;

    char file_name[20];
    for(int i=0; i<mount_count; i++)
    {
        if(arr[i].drivename == curr_drive)
        {
            strcpy(file_name,arr[i].osfile);
            break;
        }
    }

    int fd = open(file_name,O_RDWR);
    if(fd < 0)
    {
        printf("Error while opening file system\n");
        return;
    }
    
    lseek(fd,0,SEEK_SET);

    int read_status= read(fd,(void *)&sb, sizeof(sb));
    if(read_status==-1)
    {
        printf("Unable to read superblock\n");
        return;
    }

    i_node inode;
    int temp ;
    for(int i=0; i<MAX_INODE; i++)
    {
        if(sb.ibm[i]=='1')
        {    
            lseek(fd,0,SEEK_SET);
            lseek(fd,sizeof(sb)+sizeof(inode)*(i),SEEK_SET);
            read_status= read(fd,(void *)&inode, sizeof(inode));
            if(read_status==-1)
            {
                printf("Unable to read inode\n");
                return;
            }
            if(strncmp(inode.filename,file1 ,strlen(inode.filename))==0)
            {
                temp = i;
                break;
            }
        }
    }
    
    sb.ibm[temp] = '0';
    for(i=inode.dblock_start_loc;i<inode.dblock_start_loc+inode.dblock_count;i++)
    {
        sb.dbm[i] = '0';
    }
    remove(inode.filename);

    sb.free_inode_count++;

    sb.free_dblock_count += inode.dblock_count;

    for(int k=0; k<MAX_DATA_BLOCK; k++)
        inode.i_dbm[k] = 0;

    char buffer[inode.dblock_count*sb.block_size];

    bzero(buffer,inode.dblock_count*sb.block_size);

    
    read_status = read(fd ,&buffer ,sizeof(buffer)) ;
    if(read_status==-1)
    {
        fprintf(stderr, "Error: Unable to read the buffer\n");
    }

    lseek(fd ,0 ,SEEK_SET) ;
    lseek(fd ,sizeof(sb) +(sizeof(inode)*MAX_INODE)+inode.dblock_start_loc ,SEEK_SET) ;

    int write_status= write(fd,&buffer, sizeof(buffer));
    if(write_status==-1)
    {
        fprintf(stderr,"Error: Unable to clear memory\n");
    }


    lseek(fd,0,SEEK_SET);

    write_status= write(fd,(void *)&sb, sizeof(sb));
    if(write_status==-1)
    {
        fprintf(stderr,"Error: Unable to write back into file\n");
    }

    lseek(fd,0,SEEK_SET);
    lseek(fd,sizeof(sb)+sizeof(inode)*inode.inode_no, SEEK_SET);

    write_status= write(fd,(void *)&inode,sizeof(inode));

    if(write_status==-1)
    {
        fprintf(stderr, "Error: Unable to write back into file\n");

    }
    
    close(fd);

}



void mkfile(char buf [])
{
    int i=0;
    char *token;
    char file1[20];
    char filesz[5] ;
    token = strtok(buf," ");
    while (token != NULL)
    {
        token = strtok(NULL, " ");
        i++;
        if(i == 1)
        {
            strcpy(file1,token);
        }
        else if(i == 2)
        {
            filesz[0] = token[0] ;
            filesz[1] = '\0';

        }
        else
            break;
    }
    int file_sz;
    sscanf(filesz,"%d",&file_sz);

    sblk sb;
    char file_name[20];
    long int file_sz_bytes = file_sz*1024;
    for(int i=0; i<mount_count; i++)
    {
        if(arr[i].drivename==curr_drive)
        {
            strcpy(file_name,arr[i].osfile);
            break;
        }
    }
    printf("File system Name: %s\n",file_name);
    int fd = open(file_name,O_RDWR);
    if(fd<0)
    {
        printf("Error while opening file system\n");
        return;
    }

    lseek(fd,0,SEEK_SET);

    int read_status= read(fd,(void *)&sb, sizeof(sb));
    if(read_status==-1) 
    {
        printf("Error while reading superblock!\n");
        return;
    }
    
    i_node inode;
    
    int j;

    for(i=0; i<MAX_INODE; i++)
    {
        if(sb.ibm[i]=='0')
        {
            sb.ibm[i]='1';
            sb.free_inode_count--;
            printf("Empty Slot Inode found: %d\n",i);
            for(j=0; j<MAX_DATA_BLOCK; j++)
            {
                if(sb.dbm[j]=='0')
                {
                    sb.dbm[j]='1';
                    printf("Empty Slot Data Block found: %d\n",j);
                    break;
                }
            }
            break;
        }
    }
    
    inode.inode_no = i;
    inode.type = 'f';
    inode.data_size = file_sz_bytes;
    strcpy(inode.filename,file1);
    int dblocks = ceil(file_sz_bytes/sb.block_size);
    inode.dblock_count = dblocks;
    inode.dblock_count++;
    inode.dblock_start_loc = j;
    for(int k=0; k<MAX_DATA_BLOCK; k++)
        inode.i_dbm[k] = -1;

    lseek(fd,0,SEEK_SET);
    lseek(fd,sizeof(sb)+sizeof(inode)*i, SEEK_SET);

    int write_status= write(fd, (void *)&inode, sizeof(inode));
    if(write_status==-1)
    {
        printf("Error while writing into the inode!\n");
        return;
    }

    for(int k=j; k < (dblocks+j); k++) //filled data blocks
    {
        sb.dbm[k]='1';
    }
    
    sb.free_dblock_count-= dblocks;
    lseek(fd,0,SEEK_SET);

    write_status= write(fd, (void *)&sb, sizeof(sb));
    if(write_status == -1)
    {
        printf("Error while writing back into the superblock");
        return;
    }

    close(fd);
}

void cd(char *buf)
{
    curr_drive = buf[3];
    return;
}


int main()
{
	char str[50];

	while(1)
	{
		fflush(stdin);
	 X:	printf("myfs> ");
	 	fgets(str,50,stdin);

		if(strncmp(str,"exit",4) == 0)						//exit
			break;

	 	else if(strncmp(str,"clr",3) == 0)						//clearing the screen
	 	{
			system("clear");
	 	}

		else if(strncmp(str,"cp",2)==0)
		{
			copy(str);
	 	}

		else if(strncmp(str,"mv",2)==0)
		{
			move(str);
		}

		else if(strncmp(str,"mkfs",4)==0)
		{
			mkfs(str);
		}

		else if(strncmp(str,"use",3)==0)
		{
			use(str);
		}
		else if(strncmp(str,"ls",2)==0)
		{
			ls(str);
		}
		else if(strncmp(str,"rm",2)==0)
		{
			rm(str);
		}
        else if(strncmp(str,"mkfile",6)==0)
        {
            mkfile(str);
        }
        else if(strncmp(str,"cd",2)==0)
        {
            cd(str);
        }
		else
		{
			printf("Wrong command.......Please re-enter!\n");
			goto X;
		}

	}

	return 0;
}