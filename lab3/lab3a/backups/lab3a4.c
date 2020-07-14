/*NAME: Marie Chu
 * UID: 905116878
 * EMAIL: mariechu@ucla.euu
*/
#include <stdint.h>
#include <stdio.h>
#include "ext2_fs.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <time.h>
void superblock();
void groupblock();
void freeblock(int blocknum);
void freeinode(int inodenum);
void inode();
void directory(int parent, struct ext2_inode* node);
void indirectblock();
void freestuff();
void processtable();
void errorprocess(char* msg);
struct ext2_super_block super;
struct ext2_group_desc* groupdesc;
char* file;
ssize_t ret;
int fd, numgroups;
int lastnumblocks, lastnuminodes;
__u32 numblocks, numinodes, blocksize,inodesize,blockspgroup,inodespgroup,firstnot;
int main(int argc, char** argv)
{
    if(argc != 2)
    {
	fprintf(stderr, "Must specify one file system image to read from\n");
    	exit(1);
    }
    file = argv[1];
    int len = strlen(file);
    char sub[5];
    strncpy(sub,file+len-4,5);
    if(strcmp(sub,".img"))
    {
	fprintf(stderr, "File must be a .img file\n");
    	exit(1);
    }
    
    fd = open(file, O_RDONLY);
    if(fd < 0)
    {
	fprintf(stderr, "Error opening file\n");
	exit(1);
    }
    superblock();  
    groupblock();
    freestuff();
    processtable();
    free(groupdesc);
}
void freestuff()
{
    int bbitmap, ibitmap;
    int bretval, iretval;
    for(int i = 0; i < numgroups; i++)
    {
	bbitmap = groupdesc[i].bg_block_bitmap;	
 	ibitmap = groupdesc[i].bg_inode_bitmap;
	printf("bbitmap: %u, ibitmap: %u\n", bbitmap, ibitmap);
        for(unsigned int j = 0; j < blocksize; j++)
        {
	    ret = pread(fd,&bretval,1,(bbitmap*blocksize)+j);
	    if(ret < 0)
		errorprocess("pread failed");
	    else if(ret < 1)
		errorprocess("Pread read less than specified");
	    ret = pread(fd,&iretval,1,(ibitmap*blocksize)+j); 
	    if(ret < 0)
		errorprocess("pread failed");
	    else if(ret < 1)
		errorprocess("Pread read less than specified");
	    for(int k = 0; k < 8; k++)
	    {
		if(((bretval >> k) & 1) == 0)
		    freeblock((i*blocksize)+(j*8)+k+1);		//ask this is it because the bitmap starts wtih block 1 and that each group has one table
		if(((iretval >> k) & 1) == 0)
		    freeinode((i*blocksize)+(j*8)+k+1);
	    }		
        }
    }
}
void superblock()
{
    ret = pread(fd, &super, sizeof(super), 1024);
    if(ret < 0)
	errorprocess("pread failed");
    else if((unsigned)ret < sizeof(super))
	errorprocess("Pread read less than specified");
    if (super.s_magic != EXT2_SUPER_MAGIC)
        errorprocess("Not a ext2 file system");
    numblocks = super.s_blocks_count;
    numinodes = super.s_inodes_count;
    blocksize = 1024 << super.s_log_block_size;
    inodesize = super.s_inode_size;
    blockspgroup = super.s_blocks_per_group;
    inodespgroup = super.s_inodes_per_group;
    firstnot = super.s_first_ino;
    if(blocksize <1024)
	errorprocess("Invalid block size");
    numgroups = 1+ (numblocks-1)/blockspgroup;
    
    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", numblocks,numinodes,blocksize,inodesize,blockspgroup,inodespgroup,firstnot);
}

void groupblock()
{
    groupdesc = (struct ext2_group_desc*)malloc(numgroups*sizeof(struct ext2_group_desc));
    int bremain = numblocks;
    int iremain = numinodes;
    int blockshere, inodeshere;    
    if(!groupdesc)
	errorprocess("Malloc failed\n");
    for(int i = 0; i <numgroups; i++)    
    {
        if(blocksize == 1024)
        {
	    ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 2048 + i*blockspgroup*blocksize);
        }
        else if(blocksize > 1024)
        {
	    if(i == 0)
	        ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 2048 + i*blockspgroup*blocksize);
	    else
	        ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 1024 + i*blockspgroup*blocksize + blocksize);
        }
        if(ret <0)
	    errorprocess("pread failed");
        else if((unsigned)ret < sizeof(groupdesc[i]))
	    errorprocess("Pread read less than specified");
        if((unsigned)bremain >= blockspgroup)
            blockshere = blockspgroup;
        else
	    blockshere = bremain;
        if((unsigned)iremain >= inodespgroup)
	    inodeshere = inodespgroup;
 	else
	    inodeshere = iremain; 
        if(i == numgroups-1)
        {
	    lastnumblocks = blockshere;
	    lastnuminodes = inodeshere;
        }
        printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", i,blockshere,inodeshere,groupdesc[i].bg_free_blocks_count,groupdesc[i].bg_free_inodes_count,groupdesc[i].bg_block_bitmap,groupdesc[i].bg_inode_bitmap,groupdesc[i].bg_inode_table);
        bremain -= blockspgroup;
        iremain -= inodespgroup;
    }	
}
void freeblock(int blocknum)
{
    printf("BFREE,%u\n", blocknum);
}
void freeinode(int inodenum)
{
    printf("IFREE,%u\n", inodenum);
}

void processtable()
{
    int tableblock,diskusageblocks=0; 
    char filetype;
    struct ext2_inode* table;
    char modtime[20];
    char accesstime[20];
    char changetime[20];
    time_t mod,acc,chg;
    
    for(int i = 0; i < numgroups; i++)
    {
        tableblock = groupdesc[i].bg_inode_table;
        table = (struct ext2_inode*)malloc(inodespgroup*inodesize);
        for(unsigned int j = 0; j < inodespgroup; j++)
        {
            ret = pread(fd,&table[j],inodesize,tableblock*blocksize+(j*inodesize));
            if(ret < 0)
                errorprocess("pread failed");
            else if(ret < 1)
                errorprocess("Pread read less than specified");
            if(table[j].i_mode == 0 || table[j].i_links_count == 0)
		continue;
            if(S_ISREG(table[j].i_mode))
		filetype = 'f';
            else if(S_ISLNK(table[j].i_mode))
		filetype = 's';
	    else if(S_ISDIR(table[j].i_mode))
            {
		filetype = 'd';
		directory(j+1, &table[j]);
            }
	    else
		filetype = '?';
            mod = table[j].i_mtime;
	    acc = table[j].i_atime;
	    chg = table[j].i_ctime;
            const char *format = "%x %X";
	    strftime(modtime,20,format,gmtime(&mod));
	    strftime(accesstime,20,format,gmtime(&acc));
	    strftime(changetime,20,format,gmtime(&chg));
            printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u,",j+1,filetype,table[j].i_mode & 0xFFF,table[j].i_uid,table[j].i_gid,table[j].i_links_count,changetime,modtime,accesstime,table[j].i_size,diskusageblocks);
            printf("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);		
         }
    }
    free(table);
}
void directory(int parent, struct ext2_inode* inode)
{    
    struct ext2_dir_entry* entry;
    __u32 size = 0;   
    char file_name[EXT2_NAME_LEN+1];
    char buf[blocksize];
    for(int i = 0; i < 12; i++)
    {
    ret = pread(fd, &buf, blocksize, blocksize*(inode->i_block[i]));
    if(ret < 0 )
	errorprocess("Pread failed");
    if(ret < blocksize)
	errorprocess("Pread read less than specified.");
    entry = (struct ext2_dir_entry*)&buf;
    size = 0;
    while(size < blocksize  && entry->file_type != 0)
    {
	if(!memcpy(file_name,entry->name,entry->name_len))
	    errorprocess("error in copying name");
	file_name[entry->name_len] = '\0';        
        if(entry->inode != 0)
	    printf("DIRENT,%u,%u,%u,%u,%u,'%s'\n",parent,(i*blocksize)+size,entry->inode,entry->rec_len,entry->name_len,file_name);
        size = entry->rec_len + size;
	entry = (void*)entry+entry->rec_len;
    }
    }
}
/*
void indirectblock()
{
     printf("INDIRECT,%u,%u,%u,%u,%u\n",inodenumofowningfile,levelofindirection,blockoffset,blocknumofindirect,blocknumofblock);
}
*/
void errorprocess(char*msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(2);
}
