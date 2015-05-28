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



listNode* createListFromString(char* string, listNode*root, FILE* ufs){
	
	word i=0;
	word currentNameCounter =0;
	char currentName[256];
	
	listNode* list = NULL;
	inode aux;
	halfWord auxAddr;
	
	if(string[0]=='/'){
		aux = getInodeFromRelativeAddress(0,ufs);
		list = addList(list, aux);
		i++;
	}else if(string[0]=='.' && string[1] == '.'){
		aux = getInodeFromRelativeAddress(root->node.metadata.parent, ufs);
		list = addList(list, aux);
		i+=2;
	}else{
		list = addList(list, root->node);
		if(string[0]=='.'){
			i++;
		}
	}
	
	
	while (string[i]!=0) {
		if(string[i]=='/'){
			
			if(currentName[0]=='.' && currentName[1]=='.' && currentName[2]==0){
				list = removeList(list);
			}else if((currentName[0]=='.' && currentName[1]==0) || currentName[0]==0){
				
			}else{
				auxAddr = seekInDirectory(list->node, currentName, ufs);
				if(auxAddr == 0){
					printf("Error: No such file or directory!");
					while (list!=NULL) {
						list = removeList(list);
					}
					return NULL;
				}
				aux = getInodeFromAbsoluteAddress(auxAddr, ufs);
				list = addList(list, aux);
			}
			for (;currentNameCounter!=0;currentNameCounter--) {
				currentName[currentNameCounter] = 0;
			}
			currentName[0] = 0;
			
		}else{
			currentName[currentNameCounter] = string[i];
			
			currentNameCounter++;
		}
		
		i++;
	}
	
	if(currentName[0]=='.' && currentName[1]=='.' && currentName[2]==0){
		if(list->next!=NULL){
			list = removeList(list);
		}
	}else if((currentName[0]=='.' && currentName[1]==0) || currentName[0]==0){
		
	}else{
		auxAddr = seekInDirectory(list->node, currentName, ufs);
		if(auxAddr == 0){
			printf("Error: No such file or directory!");
			while (list!=NULL) {
				list = removeList(list);
			}
			return NULL;
		}
		aux = getInodeFromAbsoluteAddress(auxAddr, ufs);
		list = addList(list, aux);
	}
	
	return list;
}


void printListNames(listNode* list){
	
	if(list == NULL){
		return;
	}
	
	printListNames(list->next);
	printf("%s/",list->node.metadata.name);
	
}
