#!/usr/bin/python

import csv, sys
from optparse import OptionParser

numblocks, numinodes,bbitmap, ibitmap, tablesize, inodetable, grouploc, firstnonres =None,None,None,None,None,None,None,None;
freeblocks = [];
freeinodes = [];
inodes = [];
indirect = [];
dirent = [];
invalid = False;
validinodes = [];
inodelinks = {}; #creating an empty dictionary
def main():
    global grouploc,numblocks,numinodes,tablesize,bbitmap,ibitmap,inodetable,freeblocks, freeinodes, inodes, dirent, indirect,invalid,firstnonres;
    if(len(sys.argv) != 2):
	sys.stderr.write("Must have one argument.\n");
	exit(1);
    arg = sys.argv[1]
    if(arg.find('.csv') == -1):
	sys.stderr.write("Must be a csv file.\n"); 
	exit(1);
    with open(arg) as csvfile:
	readCSV = csv.reader(csvfile, delimiter=',')
	for row in readCSV:
    	    if(row[0] == 'SUPERBLOCK'):
#		numblocks = row[1];
#		numinodes = row[2];
		firstnonres = row[7];
                tablesize = (int(row[2])*int(row[4]))/int(row[3]);                
                if(int(row[3]) == 1024):
		    grouploc = 2;
		elif(int(row[3]) > 1024):
		    grouploc = 1;
            elif(row[0] == 'BFREE'):
		freeblocks.append(row[1]);
	    elif(row[0] == 'IFREE'):
		freeinodes.append(row[1]);
	    elif(row[0] == 'INODE'):
		inodes.append(row);
	    elif(row[0] == 'DIRENT'):
		dirent.append(row);
	    elif(row[0] == 'INDIRECT'):
		indirect.append(row);
	    elif(row[0] == 'GROUP'):
		numblocks = row[2];
		numinodes = row[3];
	 	bbitmap = row[6];
		ibitmap = row[7];
 		inodetable = row[8];
    blockconsist();
    inodeconsist();
   # directoryconsist();
	
    if(invalid):
	exit(2);
    else:
	exit(1);
def blockconsist():
    global validinodes,numinodes,tablesize,grouploc;
    blocks = {};
    reserved = [str(1),str(grouploc),ibitmap,bbitmap];
    for i in range(tablesize):
	reserved.append(str(int(inodetable)+i));
    for inode in inodes:
	if(str(inode[2]) == "l" and int(inode[10]) <= 60):
	    continue;
	if(str(inode[2]) == "d" or str(inode[2]) == "f" or str(inode[2]) == "l"):
	    validinodes.append(inode[1]);
	for i in range(12,24): #direct blocks
            if(inode[i] in reserved): 
		sys.stdout.write("RESERVED BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+ str(i-12)+"\n");
		invalid = True;
 	    if(int(inode[i]) < 0 or int(inode[i]) > int(numblocks)-1):#out of bounds
		sys.stdout.write("INVALID BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+str(i-12)+"\n");
		invalid = True;
	    if(inode[i] in freeblocks):#checks if on free list
		sys.stdout.write("ALLOCATED BLOCK "+inode[i]+" ON FREELIST\n");
		invalid = True;
	    if(inode[i] in blocks):#duplicate blocks
		if(blocks[inode[i]][0] == "False"):
	            blocks[inode[i]][0] = "True";
		    val = blocks[inode[i]];
		    sys.stdout.write("DUPLICATE BLOCK "+inode[i]+" IN INODE "+val[1]+" AT OFFSET "+val[2]+"\n");
		    invalid = True;
		sys.stdout.write("DUPLICATE BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+str(i-12)+"\n"); 
	    elif(int(inode[i]) != 0):#not duplicate
		blocks[inode[i]] = ["False"];
		blocks[inode[i]].append(inode[1]);
		blocks[inode[i]].append(str(i-12));
    		
    for i in range(1,int(numblocks)):
        if(str(i) not in reserved and str(i) not in freeblocks and str(i) not in blocks):
	    sys.stdout.write("UNREFERENCED BLOCK "+ str(i)+"\n");
	    invalid = True;
	
def inodeconsist():
    global validinodes;
    for i in range(int(firstnonres),int(numinodes)):#check all inodes
	if(str(i) not in freeinodes and str(i) not in validinodes): #if not allocated and not in free
	    sys.stdout.write("UNALLOCATED INODE "+str(i)+" NOT ON FREELIST\n");
	    invalid = True;
	if(str(i) in validinodes and str(i) in freeinodes): #if allocated but in free
	    sys.stdout.write("ALLOCATED INODE "+str(i)+" ON FREELIST\n");
	    invalid = True;

def directoryconsist():
    print("");
class Dup:
    def __init__(self, inum, links):
	self.inum = inum;
	self.links = links;

if __name__ =="__main__":
	main()

