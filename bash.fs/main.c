//
//  main.c
//  bash.fs
//
//  Created by Otávio Netto Zani on 26/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include <stdio.h>
#include "inode.h"
#include <stdlib.h>

#define FILE_SIZE 1024*1024*24


int main(int argc, const char * argv[]) {
	
	//test purposes
	const char filename[255] = "arquivo_fs";
	
	FILE* ufs = fopen(filename, "r+");
	
	inode root = getInodeFromRelativeAddress(0, ufs);
	
	
	
	createInodeInDirectory(&root,"name_of_file", ufs, 1, 1, 1, 1);
	inode a = getInodeFromRelativeAddress(1, ufs);
	
	
	
	fclose(ufs);
    return 0;
}
