#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

int main(int argc, char *argv[])
{
    FILE *fptr; // file pointer

    // step1: Open file and validate if its open or not
    fptr = fopen(argv[2], "rb");
    if (fptr == NULL)
    {
        perror("Error opening file!");
        help(NULL);
        return 1;
    }
    // step 1.1 check if its mp3 file or not
    char signature[4], version[1]; //,version[3];
    fread(signature, 3, 1, fptr);
    signature[3] = '\0';

    fread(version, 1, 1, fptr);
    // printf("Version ID: 2.%d\n", version[0]);

    fseek(fptr, 0, SEEK_SET);
    if (!strcmp(signature, "ID3")) // if its true then its mp3 file(returns 0 if its equal)
    {
        if (argc < 3)
        {
            printf("Not enough arguments!");
            help(version);
            return 0;
        }
        else if ((!strcmp(argv[1], "-v")) && argc == 3) // if its equal enter loop
        {
            fseek(fptr, 10, SEEK_SET); // skips 10 bytes header
            display(fptr, version);
        }
        else if ((!strcmp(argv[1], "-e") && argc == 5))
        {
            edit(fptr, argv[3], argv[4], argv[2]); // Passing file pointer and the tag to edit
            return 0;
        }
        else if ((!strcmp(argv[1], "-h") && argc == 3))
        {
            help(version); 
            return 0;
        }
        else
        {
            printf("Pass correct command line arguments!\n");
            help(NULL);
            return 0;
        }
    }
    else
    {
        printf("This file is not an MP3 file\n");
        return 1;
    }
}
