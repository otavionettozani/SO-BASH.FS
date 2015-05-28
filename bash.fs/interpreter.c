//
//  interpreter.c
//  bash.fs
//
//  Created by Otávio Netto Zani on 27/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include "interpreter.h"


listNode* addList(listNode* root, inode node){
	
	listNode* newNode = (listNode*)malloc(sizeof(listNode));
	
	newNode->node = node;
	
	newNode->next = root;
	
	return newNode;
	
}


listNode* removeList(listNode* root){
	
	if(root == NULL){
		return NULL;
	}
	
	listNode* node = root->next;
	
	root->next = NULL;
	
	free(root);
	
	return node;
	
}