#ifndef CONTENT_TYPES_H
#define CONTENT_TYPES_H

// For reading the CSV file
#include "./util/file_util.h"

// String handling
#include <string.h>

// Global definitions
extern char **fileExtensions;
extern char **contentTypes;
extern int contentTypesLength;

// Sets up the content type globals
void setupContentTypes();

// Returns the content type for a given file path
char *getTypeFromPath(const char *path);

#endif // !CONTENTTYPES_H