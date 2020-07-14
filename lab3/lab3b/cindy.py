#!/usr/bin/env python3

import sys
import re
import csv
from collections import defaultdict

class superblock:
    def __init__(self,info):
        self.blocks_count = int(info[1])
        self.inodes_count = int(info[2])
        self.block_size = int(info[3])
        self.inode_size = int(info[4])
        self.blocks_per_group = int(info[5])
        self.inodes_per_group = int(info[6])
        self.first_ino = int(info[7])
        self.refs_per_block = int(self.block_size / 4)

class group:
    def __init__(self,info):
        self.bmap = int(info[6])
        self.imap = int(info[7])
        self.inode_table = int(info[8])

class inode:
    def __init__(self,info):
        # self.inode_no = int(info[1])
        self.file_type = info[2]
        self.mode = int(info[3])
        self.link_count = int(info[6])
        self.file_size = int(info[10])
        self.block_amt = int(info[11])
        self.iblock = list(map(int, info[12:]))

class dirent:
    def __init__(self,info):
        self.parent_inode_no = int(info[1])
        self.byte_offset = int(info[2])
        self.ref_inode = int(info[3])
        self.name = info[6]

class indirect:
    def __init__(self,info):
        self.inode_no = int(info[1])
        self.level = int(info[2])
        self.block_offset = int(info[3])
        self.scan_block_no = int(info[4])
        self.ref_block_no = int(info[5])

sb = None
gp = None

reserved = []
bfrees = []
ifrees = []
allocated_blocks = defaultdict(list)
inodes = defaultdict(inode)
dirents = defaultdict(list) # map from referenced inode no to list of directory entries that point to that inode no
indirects = []

def main():
    global sb
    global gp
    global reserved

    file = sys.argv[1]
    try:
        with open(sys.argv[1], 'r') as f:
            reader = csv.reader(f)
            for e in reader:
                if e[0] == "SUPERBLOCK":
                    sb = superblock(e)
                elif e[0] == "GROUP":
                    gp = group(e)
                elif e[0] == "BFREE":
                    bfrees.append(int(e[1]))
                elif e[0] == "IFREE":
                    ifrees.append(int(e[1]))
                elif e[0] == "INODE":
                    inodes[int(e[1])] = inode(e)
                    for i, block in enumerate(e[12:]):
                        if (block != "0"):
                            allocated_blocks[int(block)].append((i-11 if i > 11 else 0, e[1], get_initial_block_offset(i)))
                elif e[0] == "DIRENT":
                    dirents[int(e[3])].append(dirent(e))
                elif e[0] == "INDIRECT":
                    indirects.append(indirect(e))
                    if (e[5] != "0"):
                        allocated_blocks[int(e[5])].append((int(e[2])-1, int(e[1]), int(e[3])))
                else:
                    print("Error: the input csv file contains invalid elements!")
    except IOError:
        print('Unable to read file', file=sys.stderr)
        sys.exit(1)

    # store all reserved blocks into an array
    reserved = [0, 1, 2, gp.bmap, gp.imap]
    table_size = int(sb.inode_size * sb.inodes_count / sb.block_size)
    for i in range(table_size):
        reserved.append(gp.inode_table+i)

    check_block_consistency()

    for i in range(1, sb.inodes_count+1):
        check_inode_consistency(i)

    for inode_no, node in inodes.items():
        check_link_count(inode_no, node)

    for ref_inode_no, entries in dirents.items():
        check_directory_consistency(ref_inode_no, entries)


def check_block_consistency():
    check_invalid_reserved()
    check_allocation_consistency()
    check_duplicate_blocks()

def check_inode_consistency(inode_no):
    if inode_no in ifrees and inode_no in inodes:
        print("ALLOCATED INODE", inode_no, "ON FREELIST")
    elif inode_no >= sb.first_ino and inode_no not in ifrees and inode_no not in inodes:
        print("UNALLOCATED INODE", inode_no, "NOT ON FREELIST")
    
def check_link_count(inode_no, node):
    if (inode_no not in dirents):
        print("INODE", inode_no, "HAS 0 LINKS BUT LINKCOUNT IS", node.link_count)
    elif (len(dirents[inode_no]) != node.link_count):
        print("INODE", inode_no, "HAS", len(dirents[inode_no]), "LINKS BUT LINKCOUNT IS", node.link_count)

def check_directory_consistency(ref_inode_no, entries):
    # GOAL: check if directory entries point to an inode number that is valid and allocated
    if ref_inode_no < 1 or ref_inode_no > sb.inodes_count: # check if ref_inode_no is invalid (out of range)
        for entry in entries:
            print("DIRECTORY INODE", entry.parent_inode_no, "NAME", entry.name, "INVALID INODE", ref_inode_no)
    elif ref_inode_no not in inodes: # check if ref_inode_no is unallocated (not in list of inodes)
        for entry in entries:
            print("DIRECTORY INODE", entry.parent_inode_no, "NAME", entry.name, "UNALLOCATED INODE", ref_inode_no)

    # GOAL: check for correctness of '.' and '..' links
    for entry in entries:
        # if the '.' dirent points to an inode number other than that of the directory it belongs in
        if entry.name.strip('\'') == '.' and ref_inode_no != entry.parent_inode_no:
            print("DIRECTORY INODE", entry.parent_inode_no, "NAME '.' LINK TO INODE", ref_inode_no, "SHOULD BE", entry.parent_inode_no)
        
        elif entry.name.strip('\'') == '..':
            # root directory, '..' should point to itself
            if entry.parent_inode_no == 2 and ref_inode_no != 2:
                print("DIRECTORY INODE 2 NAME '..' LINK TO INODE", ref_inode_no, "SHOULD BE 2")
                continue

            # loop through all directory entries that point to the directory to which current dirent belongs
            if entry.parent_inode_no not in dirents: continue
            for entry_in_parent_dir in dirents[entry.parent_inode_no]:
                # find the valid directory entry (i.e. not the "current directory" '.' entry or "parent directory" '..' entries)
                if entry_in_parent_dir.name.strip('\'') != '.' and entry_in_parent_dir.name.strip('\'') != '..':
                    # if current '..' entry does not point to the directory that contains current directory
                    if ref_inode_no != entry_in_parent_dir.parent_inode_no:
                        print("DIRECTORY INODE", entry.parent_inode_no, "NAME '..' LINK TO INODE", ref_inode_no, "SHOULD BE", entry_in_parent_dir.parent_inode_no)

def check_invalid_reserved():
    # TODO: TAKE CARE OF SYMBOLIC LINKS
    # check the 12 direct block pointers and 3 indirect block pointers in each inode's iblock array
    for inode_no, node in inodes.items():
        if (node.file_type == 's' and node.file_size <= 60):
            continue

        for i, ptr in enumerate(node.iblock):
            s = get_string(i-11 if i > 11 else 0)
            offset = get_initial_block_offset(i)

            if ptr < 0 or ptr >= sb.blocks_count:   # if block pointer out of range -- invalid
                print("INVALID", s, ptr, "IN INODE", inode_no, "AT OFFSET", offset)
            elif ptr != 0 and ptr in reserved:      # if block pointer is reserved
                print("RESERVED", s, ptr, "IN INODE", inode_no, "AT OFFSET", offset)

    # check each of the recursive block pointers recorded by the indirect csv entries
    for ind in indirects:
        s = get_string(ind.level-1)
        
        if ind.ref_block_no < 0 or ind.ref_block_no >= sb.blocks_count: # if block pointer out of range -- reserved
            print("INVALID", s, ind.ref_block_no, "IN INODE", ind.inode_no, "AT OFFSET", ind.block_offset)
        elif ind.ref_block_no != 0 and ind.ref_block_no in reserved:    # if block pointer is reserved
            print("RESERVED", s, ind.ref_block_no, "IN INODE", ind.inode_no, "AT OFFSET", ind.block_offset)

def check_allocation_consistency():
    for i in range(sb.blocks_count):
        # if i in bfrees and (i in allocated_blocks or i in reserved):
        if i in bfrees and i in allocated_blocks:
            print("ALLOCATED BLOCK", i, "ON FREELIST")
        elif i not in bfrees and i not in allocated_blocks and i not in reserved:
            print("UNREFERENCED BLOCK", i)

def get_initial_block_offset(index):
    if index <= 12:
        return index
    elif index == 13:
        return 12 + sb.refs_per_block
    elif index == 14:
        return 12 + sb.refs_per_block + sb.refs_per_block*sb.refs_per_block

def check_duplicate_blocks():
    for block, lst in allocated_blocks.items():
        if len(lst) > 1:
            for item in lst:
                level, inode_no, block_offset = item[:]
                s = get_string(level)
                print("DUPLICATE", s, block, "IN INODE", inode_no, "AT OFFSET", block_offset)

def get_string(index):
    if index == 0:
        return "BLOCK"
    elif index == 1:
        return "INDIRECT BLOCK"
    elif index == 2:
        return "DOUBLE INDIRECT BLOCK"
    elif index == 3:
        return "TRIPLE INDIRECT BLOCK"

if __name__== "__main__":
    main()
