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

#define FILE_SIZE (1024*1024*24)





int main(int argc, const char * argv[]) {
	
	//test purposes
	const char filename[255] = "arquivo_fs";
	
	
	//end of test purposes
	
	FILE* ufs = fopen(filename, "r+");
	
	superBlock sBlock;
	
	fseek(ufs, SectionSuperBlock, SEEK_SET);
	
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	
	inode root = getInodeFromRelativeAddress(0, ufs);
	
	
	listNode* currentDirectory = NULL;
	
	currentDirectory = addList(currentDirectory, root);
	
	halfWord blockSize = sBlock.magicNumber;
	
	halfWord maxBlocks = (FILE_SIZE - SectionDataBlocks)/blockSize;
	
	
	//--------------------------  BASH MODE ----------------------------------//
	char input[10], arg1[1024], arg2[1024], buffer[2058];
	byte readArguments = 0;
	execState state = StateEnd;
	instruction inst = FAIL;
	listNode* children;
	
	while (1) {
		
		if(state == StateFetch){
			fgets(buffer, sizeof(buffer), stdin);
			strcpy(input, "");
			strcpy(arg1, "");
			strcpy(arg2, "");
			readArguments = sscanf(buffer,"%s %s %s",input, arg1, arg2);
			
			
			
			
			//case arg1 between quotes
			if (arg1[0]=='"') {
				sscanf(buffer, "%s \"%[^\"]\" %s",input,arg1,arg2);
				if(arg1[0]==arg1[1] && arg1[0] == '"'){
					arg1[0] = arg1[1] = 0;
				}
			}
			
			
			if (buffer[0]=='\n') {
				state = StateEnd;
			}else if(!strcmp(input, "exit")){
				inst = EXIT;
				state = StateExec;
			}else if(!strcmp(input, "ls")){
				inst = LS;
				state = StateExec;
			}else if(!strcmp(input, "chmod")){
				inst = CHMOD;
				state = StateExec;
			}else if(!strcmp(input, "mkdir")){
				inst = MKDIR;
				state = StateExec;
			}else if(!strcmp(input, "chdir")){
				inst = CHDIR;
				state = StateExec;
			}else if(!strcmp(input, "rm")){
				inst = RM;
				state = StateExec;
			}else if(!strcmp(input, "echo")){
				inst = Echo;
				state = StateExec;
			}else if(!strcmp(input, "cat")){
				inst = CAT;
				state = StateExec;
			}else if(!strcmp(input, "mv")){
				inst = MV;
				state = StateExec;
			}else if(!strcmp(input, "find")){
				inst = Find;
				state = StateExec;
			}else if(!strcmp(input, "defrag")){
				inst = DEFRAG;
				state = StateExec;
			}else {
				inst = FAIL;
				state = StateExec;
			}
			
			
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
					//print with -l flag
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
			}else if(inst == RM){
				currentDirectory = removePath(arg1, ufs, currentDirectory);
				state = StateEnd;
			}else if(inst == CHMOD){
				changePermissions(arg2, arg1, ufs, currentDirectory);
				state = StateEnd;
			}else if(inst == Echo){
				echoToInode(arg2, arg1, blockSize, maxBlocks, ufs, currentDirectory);
				state = StateEnd;
			}else if(inst == CAT){
				catInode(arg1, blockSize, ufs, currentDirectory);
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
