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
	
	char input[10], arg1[1024], arg2[1024], buffer[2058];
	byte readArguments = 0;
	execState state = StateEnd;
	instruction inst = FAIL;
	listNode* children;
	
	while (1) {
		
		if(state == StateFetch){
			fgets(buffer, sizeof(buffer), stdin);
			readArguments = sscanf(buffer,"%s %s %s",input, arg1, arg2);
			if(!strcmp(input, "exit")){
				inst = EXIT;
			}else if(!strcmp(input, "ls")){
				inst = LS;
			}else if(!strcmp(input, "chmode")){
				inst = CHMOD;
			}else if(!strcmp(input, "mkdir")){
				inst = MKDIR;
			}else if(!strcmp(input, "chdir")){
				inst = CHDIR;
			}else if(!strcmp(input, "rm")){
				inst = RM;
			}else if(!strcmp(input, "echo")){
				inst = Echo;
			}else if(!strcmp(input, "cat")){
				inst = CAT;
			}else if(!strcmp(input, "mv")){
				inst = MV;
			}else if(!strcmp(input, "find")){
				inst = Find;
			}else if(!strcmp(input, "defrag")){
				inst = DEFRAG;
			}else {
				inst = FAIL;
			}
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
				if(readArguments == 2 && !strcmp(arg1, "-l")){
					
				}else{
					byte flip = 0;
					while (children!= NULL) {
						printf("%s",children->node.metadata.name);
						if (children->node.metadata.flags & FlagIsDir) {
							printf("/");
						}
						if (flip == 2) {
							printf("\n");
						}else{
							printf("\t");
						}
						
						flip = flip == 2? 0: flip +1;
						children = removeList(children);
					}
				}
				state = StateEnd;
			}else if(inst == MKDIR){
				createDirectory(arg1, ufs, currentDirectory);
				state = StateEnd;
			}
			
		}else if(state == StateEnd){
			printf("\n\n");
			printListNames(currentDirectory);
			printf(" $: ");
			state = StateFetch;
		}
		
	}
	
	
	fclose(ufs);
    return 0;
}
