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
halfWord convertRelativeAddressToAbsoluteAddress(halfWord relativeAddress);

//returns the relative address given the absolute address
halfWord convertAbsoluteAddressToRelativeAddress(halfWord absoluteAddress);

//returns an inode from the given relative address
inode getInodeFromRelativeAddress(halfWord address, FILE* ufs);

//returns an inode from the given relative address
inode getInodeFromAbsoluteAddress(halfWord address, FILE* ufs);

//returns the absolute address of the first free inode that it founds
halfWord getFreeInode(FILE* ufs);

//save the inode in its position, also, update the time of modification of this node.
void saveInode(inode node, FILE* ufs);

//returns 0 if cant find the requested inode or a halfword containing the address of the requested inode in the file
halfWord seekInDirectory(inode directory, char* fileName, FILE* ufs);


//tries to create a new inode at the given directory and returns its address in the file. if fails returns 0
halfWord createInodeInDirectory(inode directory,halfWord directoryAddress ,char* filename, FILE* ufs, byte read,
							byte write, byte execute, byte isDirectory);



