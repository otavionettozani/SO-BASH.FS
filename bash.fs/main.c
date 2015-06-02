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

typedef enum Mode {
	modeI=0,
	modeO,
	modeB,
	modeD,
	modeFail,
} programMode;

int main(int argc, const char * argv[]) {
	
	
	char* fileName = NULL;
	
	char* sourceFileName= NULL;
	
	char* destinationFileName = NULL;
	
	programMode mode;
	
	if (argc == 3) {
		fileName = argv[2];
		if (!strcmp("-b", argv[1])) {
			mode = modeB;
		}else if(!strcmp("-d", argv[1])) {
			mode = modeD;
		}else{
			mode = modeFail;
		}
	}else if(argc == 5){
		sourceFileName = argv[2];
		destinationFileName = argv[3];
		fileName = argv[4];
		if (!strcmp("-i", argv[1])) {
			mode = modeI;
		}else if(!strcmp("-o", argv[1])) {
			mode = modeO;
		}else{
			mode = modeFail;
		}
	}else{
		mode = modeFail;
	}
	
	//fail mode
	if (mode == modeFail) {
		printf("Use one of the following options:\n");
		printf("-i <path_source_fs> <path_destination_ufs> <ufs>\n");
		printf("-o <path_source_fs> <path_destination_ufs> <ufs>\n");
		printf("-b <ufs>\n");
		printf("-d <ufs>\n");
		return 0;
	}
	
	FILE* ufs = fopen(fileName, "r+");
	
	superBlock sBlock;
	
	fseek(ufs, SectionSuperBlock, SEEK_SET);
	
	fread(&sBlock, sizeof(superBlock), 1, ufs);
	
	inode root = getInodeFromRelativeAddress(0, ufs);
	
	
	listNode* currentDirectory = NULL;
	
	currentDirectory = addList(currentDirectory, root);
	
	halfWord blockSize = sBlock.magicNumber;
	
	halfWord maxBlocks = (FILE_SIZE - SectionDataBlocks)/blockSize;
	
	
	//running modes
	if (mode == modeI) {
		
		//----------------------------- MODE INPUT -----------------------------------//
		FILE* external = fopen(sourceFileName, "r");
		if (!external) {
			printf("The given source file does not exist!\n");
			fclose(ufs);
			return 0;
		}
		
		
		char* lastBar;
		char nameCopy[256]={0};
		lastBar = strrchr(destinationFileName, '/');
		
		if (lastBar) {
			strcpy(nameCopy, lastBar+sizeof(char));
			
			if(lastBar!=destinationFileName){
				lastBar[0] = 0;
			}else{
				lastBar[1]=0;
			}
		}else{
			strcpy(nameCopy, destinationFileName);
			destinationFileName[0] = 0;
		}
		
		
		listNode* list = createListFromString(destinationFileName, currentDirectory, ufs);
		
		if(list == NULL){
			printf("Destination Path Does Not Exists!\n");
			fclose(ufs);
			fclose(external);
			return 0;
		}
		
		
		if(directoryHasChildWithName(list->node, (byte*)nameCopy, ufs)==1){
			printf("Destination File Already Exists!\n");
			while (list != NULL) {
				list = removeList(list);
			}
			fclose(ufs);
			fclose(external);
			return 0;
		}else if (directoryHasChildWithName(list->node, (byte*)nameCopy, ufs)==0){
			//size of file
			byte buffer[1024*2*1024];
			fseek(external, 0, SEEK_END);
			word size = ftell(external);
			fseek(external, 0, SEEK_SET);
			fread(buffer, sizeof(byte), size, external);
			
			if(!(list->node.metadata.flags&FlagPmWrite)){
				printf("Error: Permission Denied!\n");
				while (list != NULL) {
					list = removeList(list);
				}
				return 0;
			}
			
			
			word addr = createInodeInDirectory(&list->node, nameCopy, ufs, 1, 1, 1, 0);
			inode node = getInodeFromAbsoluteAddress(addr, ufs);
			setDataToInode((byte*)buffer, size, &node, ufs, blockSize, maxBlocks);
			
			printf("Transfer of file was successfull! Copied %d bytes",size);
			
		}
		
		while (list != NULL) {
			list = removeList(list);
		}
		fclose(ufs);
		fclose(external);
		return 0;
	}
	
	if (mode == modeO) {
		
		//-------------------------- MODE OUTPUT -------------------------------//
		FILE* external = fopen(destinationFileName, "w");
		
		listNode* list = createListFromString(sourceFileName, currentDirectory, ufs);
		
		if(list == NULL){
			printf("Source Path does no exists!\n");
			fclose(external);
			fclose(ufs);
			return 0;
		}
		
		if (list->node.metadata.flags&FlagIsDir) {
			printf("Source Path is a directory!\n");
			fclose(external);
			fclose(ufs);
			return 0;
		}
		
		if (!(list->node.metadata.flags&FlagPmRead)) {
			printf("Permission Denied!\n");
			fclose(external);
			fclose(ufs);
			return 0;
		}
		
		word i = 0, j;
		byte* readByte = (byte*) malloc(blockSize*sizeof(byte));
		while (list->node.blocks[i] && i<1024) {
			
			word addr =	convertBlockRelativeAddressToAbsoluteAddress(list->node.blocks[i], blockSize, ufs);
			fseek(ufs, addr, SEEK_SET);
			
			
			
			fread(readByte, sizeof(byte), blockSize, ufs);
			
			j=0;
			while (readByte[j] && j<blockSize) {
				j++;
			}
			
			fwrite(readByte, sizeof(byte), j, external);
			
			i++;
		}
		
		printf("File Copied successfully!\n");
		
		
		free(readByte);
		
		
		fclose(external);
		fclose(ufs);
		return 0;
	}
	
	if (mode== modeD){
		
		//-------------------------- MODE D ------------------------------------//
		fclose(ufs);
		return 0;
	}
	
	if (mode == modeB) {
		
		//--------------------------  BASH MODE ----------------------------------//
		char input[10], arg1[1024], arg2[1024], buffer[2058];
		byte readArguments = 0;
		execState state = StateEnd;
		instruction inst = FAIL;
		listNode* children;
		
		printf("Welcome to Bash Mode\n");
		printf("Here you can use many comands in order to access your UFS\n");
		printf("We support more than one access level for all instructions except ls\n");
		printf("We support name of files and directories up to 255 character\n");
		printf("Supported commands:\n");
		printf("ls(-l)\tmkdir\tchdir\trmdir\techo\tcat\tchmod\texit\n");
		
		while (1) {
			
			if(state == StateFetch){
				fgets(buffer, sizeof(buffer), stdin);
				word i;
				for (i=0; i<10; i++) {
					input[i] = 0;
				}
				for (i=0; i<1024; i++) {
					arg1[i] = 0;
					arg2[i] = 0;
				}
				
				
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
					currentDirectory = removePath(arg1, ufs, currentDirectory, blockSize, maxBlocks);
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
}
