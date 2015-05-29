//
//  main.c
//  bash.fs
//
//  Created by Otávio Netto Zani on 26/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include <stdio.h>
#include "interpreter.h"
#include <stdlib.h>

#define FILE_SIZE 1024*1024*24





int main(int argc, const char * argv[]) {
	
	//test purposes
	const char filename[255] = "arquivo_fs";
	
	FILE* ufs = fopen(filename, "r+");
	
	inode root = getInodeFromRelativeAddress(0, ufs);
	
	listNode* currentDirectory = NULL;
	
	currentDirectory = addList(currentDirectory, root);
	
	/*
	createInodeInDirectory(&root,"name_of_file3", ufs, 1, 1, 1, 1);
	inode a = getInodeFromRelativeAddress(1, ufs);
	
	halfWord b = seekInDirectory(root, "name_of_file3", ufs);
	
	if(b){
		inode deletable = getInodeFromAbsoluteAddress(b, ufs);
		deleteInode(deletable, &root,ufs);
	}
	
	
	
	currentDirectory = changeCurrentDirectory("name_of_file3", currentDirectory, ufs);
	
	//createInodeInDirectory(&currentDirectory->node, "sub_dir", ufs, 1, 1, 1, 1);
	
	currentDirectory = changeCurrentDirectory("sub_dir", currentDirectory, ufs);
	
	currentDirectory = changeCurrentDirectory("../sub_dir", currentDirectory, ufs);
	
	printListNames(currentDirectory);
	
	 */
	
	//end of test purposes
	
	char input[10], arg1[1024], arg2[1024];
	execState state = StateEnd;
	instruction inst = FAIL;
	listNode* children;
	listNode* auxNode;
	
	while (1) {
		
		if(state == StateFetch1){
			scanf("%s",input);
			
			if(!strcmp(input, "exit")){
				inst = EXIT;
				state = StateExec;
			}else if(!strcmp(input, "ls")){
				inst = LS;
				state = StateExec;
			}else if(!strcmp(input, "chmode")){
				inst = CHMOD;
				state = StateFetch2;
			}else if(!strcmp(input, "mkdir")){
				inst = MKDIR;
				state = StateFetch2;
			}else if(!strcmp(input, "chdir")){
				inst = CHDIR;
				state = StateFetch2;
			}else if(!strcmp(input, "rm")){
				inst = RM;
				state = StateFetch2;
			}else if(!strcmp(input, "echo")){
				inst = Echo;
				state = StateFetch2;
			}else if(!strcmp(input, "cat")){
				inst = CAT;
				state = StateFetch2;
			}else if(!strcmp(input, "mv")){
				inst = MV;
				state = StateFetch2;
			}else if(!strcmp(input, "find")){
				inst = Find;
				state = StateFetch2;
			}else if(!strcmp(input, "defrag")){
				inst = DEFRAG;
				state = StateExec;
			}else {
				inst = FAIL;
				state = StateExec;
			}
			
			
		}else if(state == StateFetch2){
			if(inst == MKDIR || inst == CHDIR || inst == RM || inst == CAT){
				scanf("%s",arg1);
				state = StateExec;
			}else{
				scanf("%s",arg1);
				state = StateFetch3;
			}
			
		}else if(state == StateFetch3){
			scanf("%s",arg2);
			state = StateExec;
		}else if(state == StateExec){
			if(inst == EXIT){
				printf("Bye Bye!\n");
				break;
			}else if(inst == FAIL){
				printf("Can't find the given instruction: %s\n",input);
				state = StateEnd;
			}else if (inst == CHDIR){
				currentDirectory = changeCurrentDirectory(arg1, currentDirectory, ufs);
				state = StateEnd;
			}else if(inst == LS){
				children = createListOfChildren(currentDirectory->node, ufs);

				
				state = StateEnd;
			}
			
		}else if(state == StateEnd){
			printf("\n\n");
			printListNames(currentDirectory);
			printf(" $: ");
			state = StateFetch1;
		}
		
	}
	
	
	fclose(ufs);
    return 0;
}
