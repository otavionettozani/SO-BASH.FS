//
//  inode.c
//  bash.fs
//
//  Created by Otávio Netto Zani on 25/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include "inode.h"



//--------------- auxiliar funcions--------------------//
void setBit(byte* destination, byte bit){
	*destination = *destination | bit;
	return;
}

void resetBit(byte* destination, byte bit){
	*destination = *destination & (~bit);
	return;
}



//------------inode use functions--------------//


halfWord convertRelativeAddressToAbsoluteAddress(halfWord relativeAddress){
	return SectionInodes + relativeAddress*sizeof(inode);
}


halfWord convertAbsoluteAddressToRelativeAddress(halfWord absoluteAddress){
	return (absoluteAddress - SectionInodes)/sizeof(inode);
}


inode getInodeFromRelativeAddress(halfWord address, FILE* ufs){
	
	halfWord absoluteAddress = convertRelativeAddressToAbsoluteAddress(address);
	
	inode node;
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	
	fread(&node, sizeof(inode), 1, ufs);
	
	return node;
	
}

inode getInodeFromAbsoluteAddress(halfWord address, FILE* ufs){
	
	inode node;
	
	fseek(ufs, address, SEEK_SET);
	
	fread(&node, sizeof(inode), 1, ufs);
	
	return node;
	
}


halfWord getFreeInode(FILE* ufs){
	
	byte inodeBitmap[InodeBitmapSize];
	int i,j,k;
	int found;
	
	//read the inode bitmap
	fseek(ufs, SectionInodeBitmap, SEEK_SET);
	fread(inodeBitmap, sizeof(byte), InodeBitmapSize, ufs);
	
	//find a free inode via inode bitmap;
	found = 0;
	for (i=0; i<InodeBitmapSize; i++) {
		for (j=0; j<7; j++) {
			k = 1<<j;
			if(!(k&inodeBitmap[i])){
				found = 8*i+j;
				break;
			}
		}
		if(found){
			break;
		}
	}
	
	if (found) {
		return SectionInodes + found*sizeof(inode);
	}
	
	return 0;
}

void setInodeBitmapAsUsed(inode node, FILE* ufs){
	
	halfWord selectByte = node.id/8;
	halfWord selectBit = node.id%8;
	
	halfWord absoluteAddress = SectionInodeBitmap+selectByte*sizeof(byte);
	
	byte bitmapByte;
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	fread(&bitmapByte, sizeof(byte), 1, ufs);
	
	setBit(&bitmapByte, 1<<selectBit);
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	
	fwrite(&bitmapByte, sizeof(byte), 1, ufs);
	
	return;
	
}

void setInodeBitmapAsUnused(inode node, FILE* ufs){
	
	halfWord selectByte = node.id/8;
	halfWord selectBit = node.id%8;
	
	halfWord absoluteAddress = SectionInodeBitmap+selectByte*sizeof(byte);
	
	byte bitmapByte;
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	fread(&bitmapByte, sizeof(byte), 1, ufs);
	
	resetBit(&bitmapByte, 1<<selectBit);
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	
	fwrite(&bitmapByte, sizeof(byte), 1, ufs);
	
	return;
	
	
}


halfWord seekInDirectory(inode directory, char* fileName, FILE* ufs){
	
	if (!(directory.metadata.flags & FlagIsDir)) {
		printf("Error in 'seekInDirectory': the given Inode is not a directory\n");
		return 0;
	}
	
	word i=0;
	
	for(i=0; i<1024; i++) {
		
		if(!directory.blocks[i]){
			continue;
		}
		
		inode node = getInodeFromRelativeAddress(directory.blocks[i], ufs);
		
		if(!strcmp(node.metadata.name,fileName)){
			return convertRelativeAddressToAbsoluteAddress(directory.blocks[i]);
		}
		
		
	}
	
	return 0;
	
}

void saveInode(inode* node, FILE* ufs){
	
	halfWord absolutePosition = convertRelativeAddressToAbsoluteAddress(node->id);
	
	fseek(ufs, absolutePosition, SEEK_SET);
	
	word tm = time(NULL);
	
	node->metadata.time = tm;
	
	fwrite(node, sizeof(inode), 1, ufs);
	
	return;
	
}

void changeInodePermissions(inode* node, byte read, byte write, byte execute){
	
	if (read) {
		setBit(&node->metadata.flags, FlagPmRead);
	}
	if (write){
		setBit(&node->metadata.flags, FlagPmWrite);
	}
	if (execute){
		setBit(&node->metadata.flags, FlagPmExec);
	}
	
}


halfWord createInodeInDirectory(inode* directory, char* filename, FILE* ufs, byte read,
							byte write, byte execute, byte isDirectory)
{
	//create the new inode
	inode newInode;
	halfWord newInodeAddress;
	
	newInodeAddress = getFreeInode(ufs);
	
	newInode = getInodeFromAbsoluteAddress(newInodeAddress, ufs);

	//set the name of the inode
	strcpy(newInode.metadata.name, filename);
	
	
	if (isDirectory){
		setBit(&newInode.metadata.flags, FlagIsDir);
	}
	
	changeInodePermissions(&newInode, read, write, execute);
	
	newInode.metadata.parent = directory->id;
	
	//save the new inode
	saveInode(&newInode, ufs);
	setInodeBitmapAsUsed(newInode, ufs);
	
	
	//change the current inode of directory so that its content contains the saved file address as its child
	int i=0;
	while (directory->blocks[i]!=0) {
		i++;
	}
	
	directory->blocks[i] = newInode.id;
	
	saveInode(directory, ufs);
	
	return convertRelativeAddressToAbsoluteAddress(newInode.id);
}

void deleteInode(inode node, inode* parent, FILE*ufs){
	
	
	word i =0;
	while (parent->blocks[i]!=node.id) {
		i++;
	}
	
	setInodeBitmapAsUnused(node, ufs);
	
	parent->blocks[i] = (halfWord)0;
	
	saveInode(parent, ufs);
	
}

