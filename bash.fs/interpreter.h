//
//  interpreter.h
//  bash.fs
//
//  Created by Otávio Netto Zani on 27/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#ifndef __bash_fs__interpreter__
#define __bash_fs__interpreter__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#endif /* defined(__bash_fs__interpreter__) */



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

listNode* changeCurrentDirectory(char* path, listNode* currentDir, FILE* ufs);