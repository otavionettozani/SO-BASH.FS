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
	*/
	
	
	currentDirectory = changeCurrentDirectory("name_of_file3", currentDirectory, ufs);
	
	//createInodeInDirectory(&currentDirectory->node, "sub_dir", ufs, 1, 1, 1, 1);
	
	currentDirectory = changeCurrentDirectory("sub_dir", currentDirectory, ufs);
	
	currentDirectory = changeCurrentDirectory("../sub_dir", currentDirectory, ufs);
	
	printListNames(currentDirectory);
	
	
	fclose(ufs);
    return 0;
}
