//
//  directory.h
//  bash.fs
//
//  Created by Otávio Netto Zani on 26/05/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#ifndef __bash_fs__directory__
#define __bash_fs__directory__

#include <stdio.h>
#include "inode.h"
#endif /* defined(__bash_fs__directory__) */



typedef struct Directory{
 
	struct Directory* nextDir;
	halfWord inodePosition;
	
}directory;
