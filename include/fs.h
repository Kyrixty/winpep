#ifndef FS_H
#define FS_H

#include "common.h"
#include <stdio.h>
#include <stdlib.h>

FILE* open_file(const char* fpath, const char* mode);

#endif