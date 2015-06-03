//
//  inode.c
//  bash.fs
//
//  Created by Otávio Netto Zani on 25/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include "inode.h"
#include <stdlib.h>


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


word convertRelativeAddressToAbsoluteAddress(halfWord relativeAddress){
	return SectionInodes + relativeAddress*sizeof(inode);
}


halfWord convertAbsoluteAddressToRelativeAddress(word absoluteAddress){
	return (absoluteAddress - SectionInodes)/sizeof(inode);
}


inode getInodeFromRelativeAddress(halfWord address, FILE* ufs){
	
	word absoluteAddress = convertRelativeAddressToAbsoluteAddress(address);
	
	inode node;
	
	fseek(ufs, absoluteAddress, SEEK_SET);
	
	fread(&node, sizeof(inode), 1, ufs);
	
	return node;
	
}

inode getInodeFromAbsoluteAddress(word address, FILE* ufs){
	
	inode node;
	
	fseek(ufs, address, SEEK_SET);
	
	fread(&node, sizeof(inode), 1, ufs);
	
	return node;
	
}


word getFreeInode(FILE* ufs){
	
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


word seekInDirectory(inode directory, char* fileName, FILE* ufs){
	
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
	
	word absolutePosition = convertRelativeAddressToAbsoluteAddress(node->id);
	
	fseek(ufs, absolutePosition, SEEK_SET);
	
	word tm = time(NULL);
	
	node->metadata.time = tm;
	
	fwrite(node, sizeof(inode), 1, ufs);
	
	return;
	
}

void changeInodePermissions(inode* node, byte read, byte write, byte execute){
	
	if (read) {
		setBit(&node->metadata.flags, FlagPmRead);
	}else{
		resetBit(&node->metadata.flags, FlagPmRead);
	}
	if (write){
		setBit(&node->metadata.flags, FlagPmWrite);
	}else{
		resetBit(&node->metadata.flags, FlagPmWrite);
	}
	if (execute){
		setBit(&node->metadata.flags, FlagPmExec);
	}else{
		resetBit(&node->metadata.flags, FlagPmExec);
	}
	
}



byte directoryHasChildWithName(inode directory, byte* name, FILE* ufs){
	
	word i=0;
	if (!(directory.metadata.flags & FlagIsDir)) {
		printf("Error: Given inode is not a directory\n");
		return 2;
	}
	while (directory.blocks[i]!=0) {
		inode node = getInodeFromRelativeAddress(directory.blocks[i], ufs);
		
		if (!strcmp((char*)node.metadata.name,(char*)name)) {
			return 1;
		}
		i++;
	}
	
	return 0;
}


word createInodeInDirectory(inode* directory, char* filename, FILE* ufs, byte read,
							byte write, byte execute, byte isDirectory)
{
	//create the new inode
	inode newInode;
	word newInodeAddress;
	
	if (directoryHasChildWithName(*directory, (byte*)filename,ufs) == 1) {
		printf("Error: Given name already exists\n");
		return 0;
	}else if (directoryHasChildWithName(*directory, (byte*)filename,ufs) == 2){
		return 0;
	}
	
	newInodeAddress = getFreeInode(ufs);
	
	if (!newInodeAddress) {
		printf("Error: No free inodes found\n");
		return 0;
	}
	
	newInode = getInodeFromAbsoluteAddress(newInodeAddress, ufs);

	//set the name of the inode
	strcpy(newInode.metadata.name, filename);
	
	//setup if is dir or file -- also update superblock
	superBlock sBlock;
	fseek(ufs, 0, SEEK_SET);
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	if (isDirectory){
		setBit(&newInode.metadata.flags, FlagIsDir);
		sBlock.inodesDirectories++;
	}else{
		sBlock.inodesFiles++;
	}
	fseek(ufs, 0, SEEK_SET);
	fwrite(&sBlock, sizeof(superBlock), 1, ufs);
	
	//setup permissions
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

byte deleteInode(inode* node, inode* parent, halfWord blockSize, word maxBlocks, FILE*ufs){
	
	
	word i =0;
	
	superBlock sBlock;
	fseek(ufs, 0, SEEK_SET);
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	
	if (!((node->metadata.flags&FlagPmWrite)&&(parent->metadata.flags&FlagPmWrite))) {
		printf("Permission Denied For File '%s' at Directory '%s'\n",node->metadata.name,parent->metadata.name);
		return 0;
	}
	
	if(node->metadata.flags & FlagIsDir){
		
		sBlock.inodesDirectories--;
		fseek(ufs, 0, SEEK_SET);
		fwrite(&sBlock, sizeof(superBlock), 1, ufs);
		
		inode child;
		halfWord childAddr;
		byte allChildrenDeleted = 1;
		
		for(i=0; i<1024;i++){
			childAddr = node->blocks[i];
			if(childAddr){
				child = getInodeFromRelativeAddress(childAddr, ufs);
				allChildrenDeleted &= deleteInode(&child, node,blockSize,maxBlocks, ufs);
			}
		}
		
		if (!allChildrenDeleted) {
			printf("Directory %s has children without write permission\n",node->metadata.name);
			return 0;
		}
		
	}else{
		
		sBlock.inodesFiles--;
		fseek(ufs, 0, SEEK_SET);
		fwrite(&sBlock, sizeof(superBlock), 1, ufs);
		
		//is a file, must remove the blocks it occupies
		byte* zeros = (byte*) malloc(blockSize*sizeof(byte));
		word i;
		for (i=0;i<blockSize;i++) {
			zeros[i] = 0;
		}
		
		i=0;
		while (i<1024) {
			if (node->blocks[i]) {
				word addr = convertBlockRelativeAddressToAbsoluteAddress(node->blocks[i], blockSize, ufs);
				copyBytesToBlock(zeros, blockSize, addr, ufs, blockSize, maxBlocks);
				setBlockBitmapAsUnused(node->blocks[i], ufs);
				node->blocks[i] = 0;
			}else{
				break;
			}
			i++;
		}
		
		free(zeros);
	}
	
	i=0;
	while (parent->blocks[i]!=node->id) {
		i++;
	}
	
	setInodeBitmapAsUnused(*node, ufs);
	
	parent->blocks[i] = (halfWord)0;
	
	for (i=0; i<1024; i++) {
		node->blocks[i]=0;
	}
	node->metadata.parent = 0;
	node->metadata.flags = 0;
	node->metadata.time = 0;
	
	for (i=0;i<256;i++) {
		node->metadata.name[i]=0;
	}
	saveInode(parent, ufs);
	saveInode(node, ufs);
	
	return 1;
	
}


//----------------------- Blocks -----------------------//

word convertBlockRelativeAddressToAbsoluteAddress(halfWord relativeAddress, halfWord blockSize ,FILE* ufs){
	
	
	return SectionDataBlocks + (relativeAddress-1)*blockSize;
	
}

halfWord convertBlockAbsoluteAddressToRelativeAddress(word absoluteAddress, halfWord blockSize ,FILE* ufs){
	
	return ((absoluteAddress-SectionDataBlocks)/blockSize)+1;
	
}

word getFreeBlock(FILE* ufs , halfWord blockSize, word maxBlocks){
	
	
	byte blockBitmap[DataBitmapSize];
	
	fseek(ufs, SectionDataBitmap, SEEK_SET);
	
	fread(blockBitmap, sizeof(byte), DataBitmapSize, ufs);
	word i, j, freeBlock = 0;
	byte found = 0;
	
	for (i=0; i<DataBitmapSize; i++) {
		for (j=0;j<8;j++) {
			if(!(blockBitmap[i]&(1<<j))){
				freeBlock = i*8+j;
				found = 1;
				break;
			}
		}
		if(found){
			break;
		}
	}
	
	if (freeBlock<maxBlocks) {
		return freeBlock*blockSize + SectionDataBlocks+1;
	}
	
	return 0;
	
}

void copyBytesToBlock(byte* bytes, halfWord size, word block,FILE* ufs, halfWord blockSize ,word maxBlocks){
	
	if (size>blockSize) {
		printf("Error: size of requested data is bigger than the size of the block\n");
		return;
	}
	
	fseek(ufs, block, SEEK_SET);
	
	fwrite(bytes, sizeof(byte), size, ufs);
	
	return;
	
}



void setDataToInode(byte* bytes, halfWord size, inode* node, FILE* ufs, halfWord blockSize ,word maxBlocks){
	
	if ((node->metadata.flags & FlagIsDir)) {
		printf("Error: Can't allocate blocks to directory!");
		return;
	}
	
	//clear old data
	byte* zeros = (byte*) malloc(blockSize*sizeof(byte));
	word i;
	for (i=0;i<blockSize;i++) {
		zeros[i] = 0;
	}
	
	i=0;
	while (i<1024) {
		if (node->blocks[i]) {
			word addr = convertBlockRelativeAddressToAbsoluteAddress(node->blocks[i], blockSize, ufs);
			copyBytesToBlock(zeros, blockSize, addr, ufs, blockSize, maxBlocks);
			setBlockBitmapAsUnused(node->blocks[i], ufs);
			node->blocks[i] = 0;
		}else{
			break;
		}
		i++;
	}
	
	free(zeros);
	
	
	//get free blocks
	word blocksQuantity = size/blockSize + ((size%blockSize)!=0);
	word newBlockAddr;
	
	i=0;
	while (i<blocksQuantity) {
		newBlockAddr = getFreeBlock(ufs, blockSize, maxBlocks);
		if (!newBlockAddr) {
			printf("Error: No free blocks to save the requested data\n");
			return;
		}
		node->blocks[i] = convertBlockAbsoluteAddressToRelativeAddress(newBlockAddr, blockSize, ufs);
		setBlockBitmapAsUsed(node->blocks[i], ufs);
		i++;
	}
	
	//write new data
	
	byte* dataPointer;
	word blockAddr;
	word writeSize;
	for (i=0; i<blocksQuantity; i++) {
		dataPointer=&bytes[i*blockSize];
		writeSize = blockSize<(size-i*blockSize)?blockSize:(size-i*blockSize);
		blockAddr = convertBlockRelativeAddressToAbsoluteAddress(node->blocks[i], blockSize, ufs);
		copyBytesToBlock(dataPointer, writeSize, blockAddr, ufs, blockSize, maxBlocks);
	}
	
	//save inode
	saveInode(node, ufs);
	
}


void printInodeData(inode node, halfWord blockSize ,FILE* ufs){
	
	word blockAddr;
	word i = 0, j=0;
	byte* block = (byte*) malloc(blockSize*sizeof(byte));
	while (node.blocks[i]) {
		
		j = 0;
		blockAddr = convertBlockRelativeAddressToAbsoluteAddress(node.blocks[i], blockSize, ufs);
		
		fseek(ufs, blockAddr, SEEK_SET);
		fread(block, sizeof(byte), blockSize, ufs);
		
		while (j<blockSize && block[j] != 0) {
			printf("%c",block[j]);
			j++;
		}
		
		i++;
		
	}
	
	printf("\n");
	
	free(block);
	
	return;
}

void setBlockBitmapAsUsed(halfWord block, FILE* ufs){
	
	byte bitmapPosition;
	word addrByte = SectionDataBitmap + block/8;
	byte addrBit = block%8;
	
	fseek(ufs, addrByte, SEEK_SET);
	fread(&bitmapPosition, sizeof(byte), 1, ufs);
	
	setBit(&bitmapPosition, (1<<(addrBit-1)));
	
	fseek(ufs, addrByte, SEEK_SET);
	fwrite(&bitmapPosition, sizeof(byte), 1, ufs);
	
	
	//write the data of consumption to superblock
	
	superBlock sBlock;
	
	fseek(ufs, 0, SEEK_SET);
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	sBlock.usedBlocks ++;
	fseek(ufs, 0, SEEK_SET);
	fwrite(&sBlock, sizeof(superBlock), 1, ufs);
	
	return;
	
}

void setBlockBitmapAsUnused(halfWord block, FILE* ufs){
	byte bitmapPosition;
	word addrByte = SectionDataBitmap + block/8;
	byte addrBit = block%8;
	
	fseek(ufs, addrByte, SEEK_SET);
	fread(&bitmapPosition, sizeof(byte), 1, ufs);
	
	resetBit(&bitmapPosition, (1<<(addrBit-1)));
	
	fseek(ufs, addrByte, SEEK_SET);
	fwrite(&bitmapPosition, sizeof(byte), 1, ufs);
	
	
	//write the data of consumption to superblock
	
	superBlock sBlock;
	
	fseek(ufs, 0, SEEK_SET);
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	sBlock.usedBlocks --;
	fseek(ufs, 0, SEEK_SET);
	fwrite(&sBlock, sizeof(superBlock), 1, ufs);
	
	
	
	return;
}


