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

listNode* removeFirstNode(listNode* root){
	
	if (root->next==NULL) {
		free(root);
		return NULL;
	}
	
	root->next = removeFirstNode(root->next);
	return root;
	
}



listNode* createListFromString(char* string, listNode*root, FILE* ufs){
	
	word i=0;
	word currentNameCounter =0;
	char currentName[256] = {0};
	
	listNode* list = NULL;
	inode aux;
	word auxAddr;
	
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
				if(list->node.id!=0){
					if(list->next == NULL){
						inode nodeDotDot = getInodeFromRelativeAddress(list->node.metadata.parent, ufs);
						list = removeList(list);
						list = addList(list, nodeDotDot);
					}else{
						list = removeList(list);
					}
				}
			}else if((currentName[0]=='.' && currentName[1]==0) || currentName[0]==0){
				
			}else{
				auxAddr = seekInDirectory(list->node, currentName, ufs);
				if(auxAddr == 0){
					printf("Error:'%s' No such file or directory!\n",string);
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
		if(list->node.id!=0){
			if(list->next == NULL){
				inode nodeDotDot = getInodeFromRelativeAddress(list->node.metadata.parent, ufs);
				list = removeList(list);
				list = addList(list, nodeDotDot);
			}else{
				list = removeList(list);
			}
		}
		
	}else if((currentName[0]=='.' && currentName[1]==0) || currentName[0]==0){
		
	}else{
		auxAddr = seekInDirectory(list->node, currentName, ufs);
		if(auxAddr == 0){
			printf("Error:'%s' No such file or directory!\n",string);
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




//------------------ tasks execution -------------------//

listNode* changeCurrentDirectory(char* path, listNode* currentDir, FILE* ufs){
	
	listNode* list2 = createListFromString(path, currentDir, ufs);
	listNode* aux;
	
	if(list2 == NULL){
		return currentDir;
	}
	
	aux = list2;
	
	while (aux->next!=NULL) {
		aux = aux->next;
	}
	
	if(aux->node.id==0){
		
		while (currentDir!=NULL) {
			currentDir = removeList(currentDir);
		}
		return list2;
	}
	
	
	while (currentDir!=NULL) {
		if(aux->node.id == currentDir->node.id){
			list2 = removeFirstNode(list2);
			aux = list2;
			if(aux == NULL){
				return currentDir;
			}
			while (aux->next!=NULL) {
				aux = aux->next;
			}
			aux->next = currentDir;
			return list2;
		}else{
			currentDir = removeList(currentDir);
		}
	}
	
	
	
	return currentDir;
}


listNode* createListOfChildren(inode parent, FILE* ufs){
	
	listNode* children = NULL;
	word i;
	inode auxNode;
	for(i=0; i<1024;i++){
		if (parent.blocks[i]) {
			auxNode = getInodeFromRelativeAddress(parent.blocks[i], ufs);
			children = addList(children, auxNode);
		}
	}
	
	return children;
}



void createDirectory(char* path, FILE* ufs, listNode* currentDir){
	
	char* lastBar;
	char nameCopy[256]={0};
	lastBar = strrchr(path, '/');
	
	if (lastBar) {
		strcpy(nameCopy, lastBar+sizeof(char));
		
		if(lastBar!=path){
			lastBar[0] = 0;
		}else{
			lastBar[1]=0;
		}
	}else{
		strcpy(nameCopy, path);
		path[0] = 0;
	}
	
	
	listNode* list = createListFromString(path, currentDir, ufs);
	
	if(list == NULL){
		return;
	}
	
	//verify if the parent dir is already at the memory
	listNode* aux = currentDir;
	while (aux!= NULL) {
		if(aux->node.id == list->node.id){
			break;
		}
		aux = aux->next;
	}
	
	if(!aux){
		createInodeInDirectory(&list->node, nameCopy, ufs, 1, 1, 1, 1);
	}else{
		createInodeInDirectory(&aux->node, nameCopy, ufs, 1, 1, 1, 1);
	}
	while (list != NULL) {
		list = removeList(list);
	}
	
	return;
	
}

listNode* removePath(char* path, FILE* ufs, listNode* currentDir){
	
	listNode* list = createListFromString(path, currentDir, ufs);
	
	inode removableNode;
	
	if (list == NULL) {
		return currentDir;
	}
	if(list->node.id == 0){
		printf("Can't remove root directory!\n");
		return currentDir;
	}
	
	listNode* aux = currentDir;
	
	//find if the desired removed path is parent of your directory
	while (aux!= NULL) {
		if(list->node.id == aux->node.id){
			break;
		}
		aux = aux->next;
	}
	
	if(aux!= NULL){
		//remove a file within your current path
		while (currentDir!=aux) {
			currentDir = removeList(currentDir);
		}
		
		removableNode = currentDir->node;
		currentDir = removeList(currentDir);
		
		deleteInode(&removableNode, &currentDir->node, ufs);
		
		printf("The selected path was removed, it was upper than you in your current path, you got moved to its parent\n");
		
	}else{
		//remove a file beyond your path
		removableNode = list->node;
		list = removeList(list);
		
		deleteInode(&removableNode, &list->node, ufs);
	}
	
	return currentDir;
}




void changePermissions(char* path, char* permissions, FILE* ufs, listNode* currentDir){
	
	listNode* list = createListFromString(path, currentDir, ufs);
	
	if (list == NULL) {
		return;
	}
	
	if(list->node.id == 0){
		printf("Can't change root permissions!\n");
		return;
	}
	
	if (strlen(permissions)!=3) {
		printf("Error: Invalid Argument '%s'",permissions);
		return;
	}
	
	byte read, write, execute;
	
	read = permissions[0] == '1' ? 1 : permissions[0] == '0' ? 0 : 2;
	write = permissions[1] == '1' ? 1 : permissions[1] == '0' ? 0 : 2;
	execute = permissions[2] == '1' ? 1 : permissions[2] == '0' ? 0 : 2;
	
	if(read == 2 || write == 2 || execute == 2){
		printf("Error: Invalid Argument '%s'",permissions);
		return;
	}
	
	listNode* aux = currentDir;
	
	//find if the desired path is parent of your directory
	while (aux!= NULL) {
		if(list->node.id == aux->node.id){
			break;
		}
		aux = aux->next;
	}
	

	
	
	if(aux!= NULL){
		changeInodePermissions(&aux->node, read, write, execute);
		saveInode(&aux->node, ufs);
	}else{
		changeInodePermissions(&list->node, read, write, execute);
		saveInode(&list->node, ufs);
	}
	
	return;
	
	
}

