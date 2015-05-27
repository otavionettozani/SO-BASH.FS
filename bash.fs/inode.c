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


halfWord seekInDirectory(inode directory, char* fileName, FILE* ufs){
	
	if (!(directory.metadata.flags & FlagIsDir)) {
		printf("Error in 'seekInDirectory': the given Inode is not a directory\n");
		return 0;
	}
	
	word i=0;
	
	while (directory.blocks[i] != 0) {
		
		inode node = getInodeFromRelativeAddress(directory.blocks[i], ufs);
		
		if(!strcmp(node.metadata.name,fileName)){
			return convertRelativeAddressToAbsoluteAddress(directory.blocks[i]);
		}
		
	}
	
	return 0;
	
}

void saveInode(inode node, FILE* ufs){
	
	halfWord absolutePosition = convertRelativeAddressToAbsoluteAddress(node.id);
	
	fseek(ufs, absolutePosition, SEEK_SET);
	
	node.metadata.time = (halfWord)time(NULL);
	
	fwrite(&node, sizeof(inode), 1, ufs);
	
	return;
	
}


halfWord createInodeInDirectory(inode directory, halfWord directoryAddress ,char* filename, FILE* ufs, byte read,
							byte write, byte execute, byte isDirectory)
{
	inode newInode;
	halfWord newInodeAddress;
	
	newInodeAddress = getFreeInode(ufs);
	
	newInode = getInodeFromAbsoluteAddress(newInodeAddress, ufs);
	
	//load the given free inode from memory
	fseek(ufs, newInodeAddress, SEEK_SET);
	fread(&newInode, sizeof(inode), 1, ufs);
	
	strcpy(newInode.metadata.name, filename);
	
	if (read) {
		setBit(&newInode.metadata.flags, FlagPmRead);
	}
	if (write){
		setBit(&newInode.metadata.flags, FlagPmWrite);
	}
	if (isDirectory){
		setBit(&newInode.metadata.flags, FlagIsDir);
	}
	if (execute){
		setBit(&newInode.metadata.flags, FlagPmExec);
	}
	
	
	return 0;
}

