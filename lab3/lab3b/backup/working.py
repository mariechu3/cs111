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
validinodes = {};
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
    directoryconsist();
	
    if(invalid):
	exit(2);
    else:
	exit(0);
def blockconsist():
    global validinodes,numinodes,tablesize,grouploc,invalid;
    blocks = {};
    reserved = ["1",str(grouploc),ibitmap,bbitmap];
    indirectval = [24,25,26];
    level = "";
    levelnum = "0";
    for i in range(tablesize):
	reserved.append(str(int(inodetable)+i));
    for inode in inodes:
	if(str(inode[2]) == "d" or str(inode[2]) == "f" or str(inode[2]) == "l"):
            validinodes[inode[1]] = inode[6]; #Map to link count
        if(str(inode[2]) == "l" and int(inode[10]) <= 60):
	    continue;
	for i in range(12,27): #direct blocks
	    offset = str(i-12)
	    level = "";
	    if(i == 24):
		level = "INDIRECT ";
	    elif(i == 25):
		level = "DOUBLE INDIRECT "
		offset = "268";
	    elif(i == 26):
		level = "TRIPLE INDIRECT " #MAKE SURE TO CHECK IF NEED TO DO THIS ONE
		offset = "65804"; 
            if(inode[i] in reserved):#reserved blocks (ex superblock, bitmaps, etc) 
		sys.stdout.write("RESERVED "+level+"BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+ offset+"\n");
		invalid = True;
 	    if(int(inode[i]) < 0 or int(inode[i]) > int(numblocks)-1):#out of bounds
		sys.stdout.write("INVALID "+level+"BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+offset+"\n");
		invalid = True;
	    if(inode[i] in freeblocks):#checks if on free list
		sys.stdout.write("ALLOCATED "+level+"BLOCK "+inode[i]+" ON FREELIST\n");
		invalid = True;
	    if(inode[i] in blocks):#duplicate blocks
		if(blocks[inode[i]][0] == "False"):
	            blocks[inode[i]][0] = "True";
		    val = blocks[inode[i]];
		    sys.stdout.write("DUPLICATE "+val[3]+"BLOCK "+inode[i]+" IN INODE "+val[1]+" AT OFFSET "+val[2]+"\n");
		    invalid = True;
		sys.stdout.write("DUPLICATE "+level+"BLOCK "+inode[i]+" IN INODE "+inode[1]+" AT OFFSET "+offset+"\n"); 
	    elif(int(inode[i]) != 0):#not duplicate
		blocks[inode[i]] = ["False"];
		blocks[inode[i]].append(inode[1]);
		blocks[inode[i]].append(offset);
		blocks[inode[i]].append(level);
    
    level = "";
    for entry in indirect: #indirect blocks
	if(str(entry[2]) == "1"):
	    level = ""
	elif(str(entry[2]) == "2"):
	    level = "DOUBLE ";
	elif(str(entry[2]) == "3"):
	    level = "TRIPLE ";
        if(int(entry[5]) < 0 or int(entry[5]) > int(numblocks)-1): #out of bounds
            sys.stdout.write("INVALID "+level+"INDIRECT BLOCK "+entry[5]+" IN INODE "+entry[1]+" AT OFFSET "+entry[3]+"\n");
            invalid = True;
	    continue;
        if(entry[5] in reserved): #reserved blocks
	    sys.stdout.write("RESERVED "+level+"INDIRECT BLOCK "+entry[5]+" IN INODE "+entry[1]+" AT OFFSET "+entry[3]+"\n");
            invalid = True;
	if(entry[5] in freeblocks):#checks if on free list
	    sys.stdout.write("ALLOCATED BLOCK "+entry[5]+" ON FREELIST\n");
	    invalid = True;
	if(entry[5] in blocks):#duplicate blocks
	    if(blocks[entry[5]][0] == "False"):
	        blocks[entry[5]][0] = "True";
	        val = blocks[entry[5]];
	        sys.stdout.write("DUPLICATE "+level+"BLOCK"+entry[5]+" IN INODE "+val[1]+" AT OFFSET "+val[2]+"\n");
		invalid = True;
	    if(str(entry[2]) == "1"):
                level = "";
            elif(str(entry[2]) == "2"):
                level = "DOUBLE ";
            elif(str(entry[2]) == "3"):
                level = "TRIPLE ";
            sys.stdout.write("DUPLICATE "+level+"INDIRECT BLOCK "+entry[5]+" IN INODE "+entry[1]+" AT OFFSET "+entry[3]+"\n");
	elif(entry[5] != 0): #not duplicates
            blocks[entry[5]] = ["False"];
	    blocks[entry[5]].append(entry[1]);
	    blocks[entry[5]].append(entry[3]);
            blocks[entry[5]].append(level);
                		
    for i in range(1,int(numblocks)):
        if(str(i) not in reserved and str(i) not in freeblocks and str(i) not in blocks):
	    sys.stdout.write("UNREFERENCED BLOCK "+ str(i)+"\n");
	    invalid = True;
def inodeconsist():
    global validinodes,invalid;
    reserved = [];
    for i in range(1, int(firstnonres)):
	reserved.append(str(i));
    for i in range(1,int(numinodes)):#check all inodes
	if(str(i) not in freeinodes and str(i) not in validinodes and str(i) not in reserved): #if not allocated and not in free
	    sys.stdout.write("UNALLOCATED INODE "+str(i)+" NOT ON FREELIST\n");
	    invalid = True;
	if(str(i) in validinodes and str(i) in freeinodes): #if allocated but in free
	    sys.stdout.write("ALLOCATED INODE "+str(i)+" ON FREELIST\n");
	    invalid = True;

def directoryconsist():
    global invalid;
    link = {};
    parent = {};
    for entry in dirent:
	if(int(entry[3]) < 1 or int(entry[3]) > int(numinodes)): #out of bounds
            sys.stdout.write("DIRECTORY INODE "+entry[1]+" NAME "+entry[6]+" INVALID INODE "+entry[3]+"\n");
	    invalid = True;
	    continue;
	if(entry[3] not in validinodes): #check if this is in freeinodes or valid inodes
	    sys.stdout.write("DIRECTORY INODE "+entry[1]+" NAME "+entry[6]+" UNALLOCATED INODE "+entry[3]+"\n");
	    invalid = True;
	if(entry[3] in link):
	    link[entry[3]]+=1;
	else:
	    link[entry[3]] = 1;
        if(entry[6] == "'.'"):#if cur dir does not point to itself
            if(str(entry[1]) != str(entry[3])):
                sys.stdout.write("DIRECTORY INODE "+entry[1]+" NAME '.' LINK TO INODE "+entry[3]+" SHOULD BE "+entry[1]+"\n");
                invalid = True;
            continue;    #ARE WE ALLOWED TO CONTINUE
        if(entry[6] == "'..'"):#if parent dir is not correct
            if(entry[1] in parent and entry[3] != parent[entry[1]]):
                sys.stdout.write("DIRECTORY INODE "+entry[1]+" NAME '..' LINK TO INODE "+entry[3]+" SHOULD BE "+parent[entry[1]]+"\n");
                invalid = True;
            elif(int(entry[1]) == 2 and str(entry[1]) != str(entry[3])): #check this for the root dir
		sys.stdout.write("DIRECTORY INODE "+entry[1]+" NAME '..' LINK TO INODE "+entry[3]+" SHOULD BE "+entry[1]+"\n");
		invalid = True;
	    continue;
        parent[entry[3]] = entry[1];

    for key in validinodes:
	if(key in link):
	    if(link[key] != int(validinodes[key])):
		sys.stdout.write("INODE "+key+" HAS "+str(link[key])+" LINKS BUT LINKCOUNT IS "+validinodes[key]+"\n");
		invalid = True;
	else:
	    sys.stdout.write("INODE "+key+" HAS 0 LINKS BUT LINKCOUNT IS "+validinodes[key]+"\n");
	    invalid = True;
		
if __name__ =="__main__":
	main()

