/*NAME: Marie Chu
 * UID: 905116878
 * EMAIL: mariechu@ucla.euu
*/
#include <stdint.h>
#include "ext2_fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
void superblock();
void groupblock();
void freeblock(int blocknum);
void freeinode(int inodenum);
void inode();
void directory();
void indirectblock();
void freestuff();
void errorprocess(char* msg);
struct ext2_super_block super;
struct ext2_group_desc* groupdesc;
__u32* blockspgroup,*inodespgroup;
char* file;
ssize_t ret;
int fd, numgroups;
__u32 numblocks, numinodes, blocksize,inodesize,blockspgroup1,inodespgroup1,firstnot;
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
    inode();
    directory();
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
    blockspgroup1 = super.s_blocks_per_group;
    inodespgroup1 = super.s_inodes_per_group;
    firstnot = super.s_first_ino;
    if(blocksize <1024)
	errorprocess("Invalid block size");
    numgroups = 1+ (numblocks-1)/blockspgroup1;
    
    printf("SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", numblocks,numinodes,blocksize,inodesize,blockspgroup1,inodespgroup1,firstnot);
}

void groupblock()
{
    groupdesc = (struct ext2_group_desc*)malloc(numgroups*sizeof(struct ext2_group_desc));
    int bremain = numblocks;
    int iremain = numinodes;
    blockspgroup = (__u32*)malloc(numgroups*sizeof(__u32));
    inodespgroup = (__u32*)malloc(numgroups*sizeof(__u32));
    if(!groupdesc)
	errorprocess("Malloc failed\n");
    for(int i = 0; i <numgroups; i++)    
    {
        if(blocksize == 1024)
        {
	    ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 2048 + i*blockspgroup1*blocksize);
        }
        else if(blocksize > 1024)
        {
	    if(i == 0)
	        ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 2048 + i*blockspgroup1*blocksize);
	    else
	        ret = pread(fd, &groupdesc[i], sizeof(groupdesc[i]), 1024 + i*blockspgroup1*blocksize + blocksize);
        }
        if(ret <0)
	    errorprocess("pread failed");
        else if((unsigned)ret < sizeof(groupdesc[i]))
	    errorprocess("Pread read less than specified");
        if((unsigned)bremain >= blockspgroup)
            blockpgroup[i] = blockspgroup1;
        else
	    blockspgroup[i] = bremain;
        if((unsigned)iremain >= inodespgroup)
	    inodespgroup[i] = inodespgroup1;
 	else
	    inodespgroup[i] = iremain; 
        printf("GROUP,%u,%u,%u,%u,%u,%u,%u,%u\n", i,blockspgroup[i],inodesgroup[i],groupdesc[i].bg_free_blocks_count,groupdesc[i].bg_free_inodes_count,groupdesc[i].bg_block_bitmap,groupdesc[i].bg_inode_bitmap,groupdesc[i].bg_inode_table);
        bremain -= blockspgroup1;
        iremain -= inodespgroup1;
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
/*
void processtable()
{
    int table;
    for(int i = 0; i < numgroups; i++) 
    {
	table = groupdesc[i].bg_inode_table;
	for(int i = 0; i < inodespgroup
    }
}
void inode()
{
    printf("INODE,%u,%s,%o,%u,%u,%s,%s,%s,%u,%u\n",numofinode,filetype,mode,owner,group,linkcount,lastchange,modtime,lastaccess,filesize,diskusageblocks);
}
void directory()
{
    printf("DIRENT,%u,%u,%u,%u,%u,\"%s\"", parentinode,byteoffset,inodenumofile,entrylength,namelength,name);
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
