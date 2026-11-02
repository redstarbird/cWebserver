#ifndef FILEHANDLING_H
#define FILEHANDLING_H

// Standard IO library
#include <stdio.h>
// For handling strings
#include <string.h>
// Standard library
#include <stdlib.h>
// Booleans
#include <stdbool.h>
// File functions
#include <unistd.h>

#include "../internal_codes.h"

// Function definitions

bool FileExists(char *FilePath);
char *ReadDataFromFile(char *path, size_t *filesize, int *problems);

#endif