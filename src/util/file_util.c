#include "file_util.h"

// Returns whether a given file exists
bool FileExists(char *FilePath)
{
    return access(FilePath, F_OK) == 0;
}

// Reads contents of the file at the given path and returns it as a string
char *ReadDataFromFile(char *path, size_t *filesize, int *problems)
{
    // Open the file in read binary mode
    FILE *filePTR = fopen(path, "rb");

    // If there was an error opening the file, set the problem to CANT_OPEN_FILE and return NULL
    if (filePTR == NULL)
    {
        *problems = CANT_OPEN_FILE;
        return NULL;
    }

    // Go to the end of the file with fseek to find the length
    fseek(filePTR, 0, SEEK_END);

    // Set the length value with ftell
    long int length = ftell(filePTR);

    // Check the file size pointer is provided before attempting to set it
    if (filesize != NULL)
    {
        // Set the given filesize pointer to the length
        *filesize = length;
    }

    // Go back to the start of the file
    fseek(filePTR, 0, SEEK_SET);

    // Create the buffer with the required length including a null terminator
    char *buffer = malloc(sizeof(char) * (length + 1));

    // If there was an error creating the buffer, set the problem variable and return NULL
    if (buffer == NULL)
    {
        printf("Error creating buffer\n");
        *problems = CANT_CREATE_BUFFER;
        return NULL;
    }

    // Read the entire file into the buffer
    fread(buffer, 1, length, filePTR);

    // Close the file
    fclose(filePTR);

    // Set the null terminator at the end of the buffer string
    buffer[length] = '\0';

    // Return the buffer
    return buffer;
}