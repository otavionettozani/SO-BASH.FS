//
//  inode.h
//  bash.fs
//
//  Created by Otávio Netto Zani on 25/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#ifndef __bash_fs__inode__
#define __bash_fs__inode__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#endif /* defined(__bash_fs__inode__) */

//generic definitions
typedef uint8_t byte;
typedef uint32_t word;
typedef uint16_t halfWord;


//----------super block definitions ------------//

typedef struct SuperBlock{
	//we will use the magic number to save the block size selected by the user
	word magicNumber;
	halfWord rootInode;
	halfWord usedBlocks;
	halfWord inodesQuantity;
} superBlock;

//---------inode definitions-----------//

//location of flags in the metadata
typedef enum MetaFlags{
	FlagIsDir = 0x1<<0,
	FlagPmWrite = 0x1<<1,
	FlagPmRead = 0x1<<2,
	FlagPmExec = 0x1<<3
} metaFlags;


//the metadata itself
typedef struct Meta{
	byte name[256];
	byte flags;
	halfWord parent;
	word time;
} metaData;


//the inode itself
typedef struct Inode{
	halfWord id;
	metaData metadata;
	halfWord blocks[1024];
	
} inode;


//------------auxiliar functions---------------//

void setBit(byte* destination, byte bit);
void resetBit(byte* destination, byte bit);


//---------sections definitions---------//
#define InodeBitmapSize 128
#define DataBitmapSize 6144
typedef enum Sections{
	SectionSuperBlock = 0,
	SectionInodeBitmap = sizeof(superBlock),
	SectionDataBitmap = sizeof(superBlock)+InodeBitmapSize,
	SectionInodes = sizeof(superBlock)+InodeBitmapSize+DataBitmapSize,
	SectionDataBlocks =  sizeof(superBlock)+InodeBitmapSize+DataBitmapSize+1024*sizeof(inode)
} sections;



//------------inode use functions--------------//


//returns the absolute address given the relative address
word convertRelativeAddressToAbsoluteAddress(halfWord relativeAddress);

//returns the relative address given the absolute address
halfWord convertAbsoluteAddressToRelativeAddress(word absoluteAddress);

//returns an inode from the given relative address
inode getInodeFromRelativeAddress(halfWord address, FILE* ufs);

//returns an inode from the given relative address
inode getInodeFromAbsoluteAddress(word address, FILE* ufs);

//returns the absolute address of the first free inode that it founds
word getFreeInode(FILE* ufs);

//save the inode in its position, also, update the time of modification of this node.
void saveInode(inode* node, FILE* ufs);

//change the permissions of the given inode
void changeInodePermissions(inode* node, byte read, byte write, byte execute);

//returns 0 if cant find the requested inode or a halfword containing the absolute address of the requested inode
word seekInDirectory(inode directory, char* fileName, FILE* ufs);

//tries to create a new inode at the given directory and returns its absolute address. if fails returns 0
word createInodeInDirectory(inode* directory,char* filename, FILE* ufs, byte read,
							byte write, byte execute, byte isDirectory);

//delete the selected inode
void deleteInode(inode* node,inode* parent, halfWord blockSize, word maxBlocks,FILE*ufs);

//set the bit relative to the inode as used in the inode bitmap
void setInodeBitmapAsUsed(inode node, FILE* ufs);

//set the bit relative to the inode as unused in the inode bitmap
void setInodeBitmapAsUnused(inode node, FILE* ufs);

//returns 1 if dir has a child with the name or 0 if not. Returns 2 if node is not a directory
byte directoryHasChildWithName(inode directory, byte* name, FILE* ufs);

//-------------- Blocks ----------//

//converts from relative to absolute address the block address
word convertBlockRelativeAddressToAbsoluteAddress(halfWord relativeAddress, halfWord blockSize ,FILE* ufs);

//returns the relative address given the absolute address
halfWord convertBlockAbsoluteAddressToRelativeAddress(word absoluteAddress, halfWord blockSize ,FILE* ufs);

//returns the absolute address of the desired block
word getFreeBlock(FILE* ufs , halfWord blockSize, word maxBlocks);

//copy the given bytes to the block (give the absolute address of the block)
void copyBytesToBlock(byte* bytes, halfWord size, word block,FILE* ufs, halfWord blockSize ,word maxBlocks);

//set the given data to the given inode
void setDataToInode(byte* bytes, halfWord size, inode* node, FILE* ufs, halfWord blockSize ,word maxBlocks);

//print all the inode data
void printInodeData(inode node, halfWord blockSize ,FILE* ufs);

//set the bit relative to the data block as used in the data block bitmap (use relative address)
void setBlockBitmapAsUsed(halfWord block, FILE* ufs);

//set the bit relative to the data block as unused in the data block bitmap (use relative address)
void setBlockBitmapAsUnused(halfWord block, FILE* ufs);

