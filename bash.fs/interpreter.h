//
//  interpreter.h
//  bash.fs
//
//  Otavio Netto Zani - RA:103697
//  Flavio Matheus Muniz Ribeiro da Silva - RA:146098

#ifndef __bash_fs__interpreter__
#define __bash_fs__interpreter__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#endif /* defined(__bash_fs__interpreter__) */


//------------------- input interpreter --------------//
typedef enum ExecutionStates{
	StateFetch = 0,
	StateExec = 1,
	StateEnd = 2,
}execState;


typedef enum Instruction{
	FAIL = -1,
	LS = 0,
	CHMOD = 1,
	MKDIR = 2,
	CHDIR = 3,
	RM = 4,
	Echo = 5,
	CAT = 6,
	EXIT = 7,
	Find = 8,
	MV = 9,
	DEFRAG = 10,
}instruction;


//---------------------------------------------------//


//used to add a inverted list of the directories
typedef struct ListNode{
 
	inode node;
	struct ListNode* next;
	
}listNode;

//add a node as if the list is a stack
listNode* addList(listNode* root, inode node);

//remove a node as if the list is a stack
listNode* removeList(listNode* root);

//remove a node as if the list is a row
listNode* removeFirstNode(listNode* root);

//------------- manipulation of strings for the problem ---------------//

//create a full list of nodes from the given string
listNode* createListFromString(char* string, listNode*root, FILE* ufs);

//prints the full path of the list
void printListNames(listNode* list);



//------------- tasks execution ------------------//

//chdir implementation
listNode* changeCurrentDirectory(char* path, listNode* currentDir, FILE* ufs);

//ls implementation
listNode* createListOfChildren(inode parent, FILE* ufs);

//mkdir implementation
void createDirectory(char* path, FILE* ufs, listNode* currentDir);

//rm implementation
listNode* removePath(char* path, FILE* ufs, listNode* currentDir, halfWord blockSize, word maxBlocks);

//chmod implementation
void changePermissions(char* path, char* permissions, FILE* ufs, listNode* currentDir);

//echo implementation returns 0 if fail 1 otherwise
byte echoToInode(char* path, char* message, word blockSize, word maxBlocks ,FILE* ufs, listNode* currentDir);

//cat implementation
void catInode(char* path, word blockSize, FILE* ufs, listNode* currentDir);

//ls -l implementation
void printNodeInfo(inode node, word blockSize);
