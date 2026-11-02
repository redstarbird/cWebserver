#include "./content_types.h"

// Definitions for globals
int contentTypesLength = 0;
char **contentTypes = NULL;
char **fileExtensions = NULL;

// Counts the number of content types in the CSV file content and updates the global variable
void countContentTypes(const char *data)
{
    // Count the number of lines/rows in the file
    for (int i = 0; i < strlen(data); i++)
    {
        if (data[i] == '\n')
        {
            contentTypesLength++;
        }
    }
}

// Returns the file extension from a given file path
char *getFileExtension(const char *path)
{
    // Find the index of the last fullstop
    int lastFullstopIndex = 0;
    for (int i = strlen(path); i > 0; i--)
    {
        if (path[i] == '.')
        {
            lastFullstopIndex = i + 1;
            break;
        }
    }

    // Return the path after the last full stop
    return strdup(path + lastFullstopIndex);
}

// Takes a path string and returns the content type header for that file
char *getTypeFromPath(const char *path)
{
    // Get the file extension for the path
    char *fileExtension = getFileExtension(path);

    // Search through the file extensions array until one is found matching the provided path's extension
    for (int i = 0; i < contentTypesLength - 1; i++)
    {
        if (strcmp(fileExtension, fileExtensions[i]) == 0)
        {
            // Return a copy of the content type for the file
            return strdup(contentTypes[i]);
        }
    }

    // Default content type
    return strdup("text/plain");
}

// Sets up the content type and file extension array from the CSV file "contentTypes.csv"
void setupContentTypes()
{
    // Read the data of the content types CSV file
    int problem = -1;
    size_t size = 0;
    char *contentTypesSVGData = ReadDataFromFile("build/contentTypes.csv", &size, &problem);

    // Check the file was read correctly
    if (problem != -1)
    {
        printf("Error reading content types file: %d\n", problem);
        exit(1);
    }

    // Find the number of content types in the file
    countContentTypes(contentTypesSVGData);

    // Allocate memory for the file extension and content type arrays
    fileExtensions = malloc(sizeof(char *) * contentTypesLength);
    contentTypes = malloc(sizeof(char *) * contentTypesLength);

    // Stores the start of the current set of parentheses
    int startParenthesisIndex = 0;
    // Tracks whether the start of a parenthesis has been found yet
    bool startParenthesisFound = false;

    // Storse the index for the current content type
    int currentContentTypeIndex = 0;

    int endParenthesisIndex = 0;

    // Boolean tracking whether the loop is currently looking at the extension or the content type
    bool onStartPart = true;

    // Iterate through the CSV data
    for (int i = 0; i < size; i++)
    {
        // Check if the current character is a parenthesis
        if (contentTypesSVGData[i] == '"')
        {
            // If the start parenthesis has already been found, set the end parenthesis index to the current index
            if (startParenthesisFound)
            {
                endParenthesisIndex = i;
            }
            // This parenthesis is the start parenthesis so store its index and update the boolean to show that the start of the next entry has been found
            else
            {
                startParenthesisIndex = i + 1;
                startParenthesisFound = true;
            }
        }

        // If either a comma or a newline character are found, the end of the current entry has been found and needs to be stored in the appropriate array
        if (contentTypesSVGData[i] == ',' || contentTypesSVGData[i] == '\n')
        {

            // Calculate the length
            int length = endParenthesisIndex - startParenthesisIndex;

            // Work out which array the entry should be added to based on whether it is a extension or a content type
            char **targetArray = onStartPart ? &fileExtensions[currentContentTypeIndex] : &contentTypes[currentContentTypeIndex];

            // Allocate the string
            *targetArray = malloc(1 + length);

            // Copy the entry into the string
            strncpy(*targetArray, contentTypesSVGData + startParenthesisIndex, length);

            // Add a null terminator to the end of the string
            (*targetArray)[length] = '\0';

            // Set startParenthesis found back to false so the program starts looking for the next parenthesis
            startParenthesisFound = false;

            // If the previous entry was a content type, increment the current index as the next entries will be a new content type
            if (!onStartPart)
            {
                currentContentTypeIndex++;
            }

            // Invert onStartPart value
            onStartPart = !onStartPart;
        }
    }
}