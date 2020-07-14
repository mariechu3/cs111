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
void indirectblock(__u32 owner, __u32 first, __u32 second, __u32 third, __u32 offset);
void printentry(int parent,__u32* size, struct ext2_dir_entry** entry,char* file_name, int i);
void freestuff();
void processtable();
void errorprocess(char* msg);
void singleentry(__u32 parent, __u32 single, __u32 offset);
void doubleentry(__u32 parent, __u32 doubly, __u32 offset);
void tripleentry(__u32 parent, __u32 triple, __u32 offset);
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
    superblock();  //processes super block
    groupblock();  //processes groups
    freestuff();   //proceeses free blocks/inodes
    processtable(); //procceses inodes
    free(groupdesc);
}
//loops through bitmaps to check for free blocks and inodes
void freestuff()
{
    int bbitmap, ibitmap;
    int bretval, iretval;
    for(int i = 0; i < numgroups; i++)
    {
	bbitmap = groupdesc[i].bg_block_bitmap;	
 	ibitmap = groupdesc[i].bg_inode_bitmap;
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
//prints out the super block
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
//prints out the groupblocks
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
//free blocks
void freeblock(int blocknum)
{
    printf("BFREE,%u\n", blocknum);
}
//free inodes
void freeinode(int inodenum)
{
    printf("IFREE,%u\n", inodenum);
}
//processes inodes and calls the required functions to process directories or indirect blocks
void processtable()
{
    int tableblock; 
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
    	    {
		filetype = 'f';
		indirectblock(j+1, table[j].i_block[12], table[j].i_block[13], table[j].i_block[14],12);
	    }
            else if(S_ISLNK(table[j].i_mode))
		filetype = 's';
	    else if(S_ISDIR(table[j].i_mode))
            {
		filetype = 'd';
		directory(j+1, &table[j]);
		indirectblock(j+1, table[j].i_block[12], table[j].i_block[13], table[j].i_block[14],12); 
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
            printf("INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%u,%u",j+1,filetype,table[j].i_mode & 0xFFF,table[j].i_uid,table[j].i_gid,table[j].i_links_count,changetime,modtime,accesstime,table[j].i_size,table[j].i_blocks);
            if(filetype != '?')
            {
		if(filetype == 'l' && table[j].i_size <= 60)
		    printf("\n");
		else
		{
                    for(int k = 0; k < 15; k++)               
                        printf(",%u", table[j].i_block[k]);
	            printf("\n");
		}
	    }	
         }
    }
    free(table);
}
//prints out the directories in the given blocks
void directory(int parent, struct ext2_inode* inode)
{    
    struct ext2_dir_entry* entry;
    __u32 size = 0;   
    char file_name[EXT2_NAME_LEN+1];
    char buf[blocksize];
    for(int i = 0; i < 12; i++)
    {
	if(inode->i_block[i] == 0)
	   continue;
        ret = pread(fd, &buf, blocksize, blocksize*(inode->i_block[i]));
        if(ret < 0 )
	    errorprocess("Pread failed");
        if(ret < blocksize)
	    errorprocess("Pread read less than specified.");
        entry = (struct ext2_dir_entry*)&buf;
        size = 0;
        printentry(parent, &size, &entry, file_name, i); 
    }
    if(!inode->i_block[12] == 0)
        singleentry(parent,inode->i_block[12],12);
    if(!inode->i_block[13] == 0)
        doubleentry(parent,inode->i_block[13],268);
    if(!inode->i_block[14] == 0)
        tripleentry(parent,inode->i_block[14],65804);
}
//prints out the entries in the given block
void printentry(int parent,__u32* size, struct ext2_dir_entry** entry,char* file_name, int i)
{
    while(*size < blocksize && (*entry)->file_type != 0)
    {
	if(!memcpy(file_name,(*entry)->name,(*entry)->name_len))
	    errorprocess("error in copying name");
	file_name[(*entry)->name_len] = '\0';
	if((*entry)->inode != 0)
	    printf("DIRENT,%u,%u,%u,%u,%u,'%s'\n",parent,(i*blocksize)+*size,(*entry)->inode,(*entry)->rec_len,(*entry)->name_len,file_name);
	*size = (*entry)->rec_len + *size;
        (*entry) = (void*)(*entry)+(*entry)->rec_len;
    }
}
//deals with the case of finding directory entries in single indirection
void singleentry(__u32 parent,__u32 single,__u32 offset)
{
    struct ext2_dir_entry* entry;
    char buf[blocksize];
    char file_name[EXT2_NAME_LEN+1];
    __u32 size = 0; 
    __u32* address = (__u32*)malloc(blocksize);
    __u32 ref;
    for(__u32 i = 0; i < (blocksize/4); i++)
    {
	ret = pread(fd, &address[i], 4, (blocksize*single) + (i*4));
	if(ret <0)
	    errorprocess("Pread failed");
	if(ret <4)
	    errorprocess("Pread read less than specified");
	memcpy(&ref, &address[i], 4);
	if(ref != 0)
	{
	    ret = pread(fd, &buf, blocksize, blocksize*ref);
	    if(ret <0)
		errorprocess("Pread failed");
	    if(ret <blocksize)
		errorprocess("Pread read less than specified.");
	    entry = (struct ext2_dir_entry*)&buf;
	    size = 0;
	    printentry(parent, &size, &entry, file_name, (offset+i));
	}
    }
    free(address);        
}
//deals with the case of finding directory entries in double indirection
void doubleentry(__u32 parent,__u32 doubly,__u32 offset)
{
    __u32* address = (__u32*)malloc(blocksize);
    char buf[blocksize];
    __u32 ref;
    for(__u32 i = 0; i < (blocksize/4); i++)
    {
	ret = pread(fd, &address[i], 4, (blocksize*doubly) + (i*4));
	if(ret < 0)
	    errorprocess("Pread read failed");
        if(ret < 4)
	    errorprocess("Pread read less than specified");
	memcpy(&ref, &address[i], 4);
	if(ref != 0)
	{
	    ret = pread(fd, &buf, blocksize, blocksize*ref);
	    if(ret <0)
		errorprocess("Pread failed");
	    if(ret < blocksize)
		errorprocess("Pread read less than specified.");
            singleentry(parent, ref, offset+(256*i));
	}
    }
    free(address);      
}
//deals with the case of finding directory entries in triple indirection
void tripleentry(__u32 parent,__u32 triple, __u32 offset)
{
    __u32* address = (__u32*)malloc(blocksize);
    char buf[blocksize];
    __u32 ref;
    for(__u32 i =0; i < blocksize/4; i++)
    {
	ret = pread(fd, &address[i], 4, (blocksize*triple) + (i+4));
        if(ret < 0)
	    errorprocess("Pread read failed");
	if(ret <4)
	    errorprocess("Pread read less than specified");
        memcpy(&ref, &address[i], 4);
	if(ref != 0)
	{
	    ret = pread(fd, &buf, blocksize, blocksize*ref);
	    if(ret < 0)
		errorprocess("Pread failed");
	    if(ret < blocksize)
		errorprocess("Pread read less than specified.");
	    doubleentry(parent, ref, offset+(65536*i));
	}
    }
    free(address);
}
//deals with printing out the things for an indirect block
void indirectblock(__u32 owner, __u32 single, __u32 doubly, __u32 triple, __u32 offset)
{    
     __u32* buf = (__u32*)malloc(blocksize);
     __u32 ref;
     if(single)
     { 
        for(__u32 i = 0; i < (blocksize/4); i++)
        {
            ret = pread(fd, &buf[i], 4, (blocksize*single)+ (i*4));
            if(ret < 0)
	        errorprocess("Pread failed");
	    if(ret < 4)
	        errorprocess("Pread read less than specified.");
       	    memcpy(&ref,&buf[i],4);	    
	    if(ref != 0)
	        printf("INDIRECT,%u,1,%u,%u,%u\n", owner,offset,single,ref);    
	    offset++;
        }
     }
     if(doubly)
     {
	for(__u32 i = 0; i < (blocksize/4); i++)
        {
	    ret = pread(fd, &buf[i], 4, (blocksize*doubly)+ (i*4));
            if(ret < 0)
	        errorprocess("Pread failed");
	    if(ret < 4)
	        errorprocess("Pread read less than specified."); 
            memcpy(&ref,&buf[i],4);
            if(ref != 0)
	        printf("INDIRECT,%u,2,%u,%u,%u\n",owner, offset, doubly, ref );
            indirectblock(owner, ref, 0, 0, offset);
            offset += 256; //number of blocks in a single indirect
        }
     }
     if(triple)
     {
        for(__u32 i = 0; i < (blocksize/4); i++)
	{   ret = pread(fd, &buf[i], 4, (blocksize*triple) + i*4);
            if(ret < 0)
		errorprocess("Pread failed");
	    if(ret < 4)
		errorprocess("Pread read less than specified.");
	    memcpy(&ref, &buf[i],4);
	    if(ref != 0)
	        printf("INDIRECT,%u,3,%u,%u,%u\n",owner,offset,triple,ref);
	    indirectblock(owner, 0, ref, 0, offset);
            offset += 65536; //number of blocks in a doubly indirect (triple indirect entries are double indirect))
        }
     }
     free(buf);
}
//prints out the error message when there are system call fails or other failures during processing
void errorprocess(char*msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(2);
}
