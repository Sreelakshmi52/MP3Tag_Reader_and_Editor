#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions.h"

struct tagmatch
{
    char tags[5], label[20], edit[3];
} mp3tags[] = {{"TPE1", "Artist", "-a"}, {"TIT2", "Title", "-t"}, {"TALB", "Album", "-A"}, {"TYER", "Year", "-y"}, {"TCON", "Content", "-C"}, {"COMM", "Comment", "-c"}};

// Display function
void display(FILE *fptr, char *version)
{
    int tagcount = 0,count=0; // For counting the 6 tags 
    do
    {
        count++;//to track no.of iterations

        char tag[5];            // where the values read will be stored
        fread(tag, 4, 1, fptr); // read tag
        tag[4] = '\0';          // Added null character for future handling
        int size;
        fread(&size, 4, 1, fptr); // Want to convert into little endian
        endian_convert(&size);    // convert size into little endian
        // printf("%d\n",size);

        if (tags(tag, 0, version)) // if match found enter loop else it will read next 4 characters and try to find match
        {
            char *content = malloc(size); // one null character at beginning size includes that
            if (content == NULL)
            {
                printf("Memory not allocated\n");
                return;
            }

            fseek(fptr, 3, SEEK_CUR);          // 2 byte flag 1 byte null character
            fread(content, size - 1, 1, fptr); // size -1 because last character is NULL
            content[size - 1] = '\0';
            printf("    ");

            for (int k = 0; k < size; k++)
            {
                printf("%c", content[k]);
            }
            printf("\n");
            tagcount++;
            free(content);
        }
        else
        {
            fseek(fptr, size + 2, SEEK_CUR);
        }

    } while (tagcount <= 5 && count<=80);
    if (tagcount > 0)
        printf("-------------------------------------------------------------------------\n");
}

void endian_convert(int *size)
{
    // Bigendian value to little endian conversion
    unsigned char temp;
    unsigned char *size1 = (unsigned char *)size;
    for (int j = 0; j < 2; j++)
    {
        temp = size1[j];
        size1[j] = size1[3 - j];
        size1[3 - j] = temp;
    }
}

int tags(char *tag, int switch_label, char *version)
{
    static int x = 0;           // for printing -------
    for (int j = 0; j < 6; j++) // for the structure
    {
        if (!switch_label)
        {
            if (!strcmp(tag, mp3tags[j].tags)) // if its a match enters loop
            {

                if (x == 0)
                {
                    printf("-------------------------------------------------------------------------\n");
                    printf("MP3 Tag  Reader and Editor\n");
                    // printf("Version ID : %s\n",version);
                    printf("Version ID: 2.%d\n", version[0]);

                    printf("-------------------------------------------------------------------------\n");
                    x++;
                }
                printf("%-10s:", mp3tags[j].label);
                return 1;
            }
        }
        else
        {
            if (!strcmp(tag, mp3tags[j].edit)) // if its a match enters loop
            {
                // printf("%s", mp3tags[j].label);
                return j + 1; // returning inex+1
            }
        }
    }
}

void edit(FILE *fptr, char *str, char *newinfo, char *old_file_name)
{
    FILE *fp;
    fp = fopen("sample_edit.mp3", "wb"); // New file
    if (fp == NULL)
    {
        printf("Couldn't open file!");
        return;
    }

    int index_1 = tags(str, 1, NULL); // get index of tag in structure(index of -t -y -c etc)

    // printf("ret = %d\n", index_1); //********************************************/

    char header[10];
    fread(header, 10, 1, fptr);
    fwrite(header, 10, 1, fp); // copied first 10 bytes to new file including tag

    char value[5]; // where the values of tag read will be stored
    while (1)      // for finding Tag match corresponding to -t -y option....
    {

        int size;
        fread(value, 4, 1, fptr); // read tag
        value[4] = '\0';
        fwrite(value, 4, 1, fp); // write tag
        fread(&size, 4, 1, fptr);
        if (strcmp(value, mp3tags[index_1 - 1].tags) != 0) // not a match
        {
            fwrite(&size, 4, 1, fp);
            endian_convert(&size);
            char *ptr = malloc(size);
            if (ptr == NULL)
            {
                perror("Memory allocation failed");
                return;
            }
            fread(ptr, size + 2, 1, fptr); // 2 flag,1 null,content copied
            fwrite(ptr, size + 2, 1, fp);
            free(ptr);
            if (index_1 > 6 || index_1 < 1)
            {
                printf("Not a valid option!");
                return;
            }
        }
        else
        {
            int new_size = strlen(newinfo) + 1; // include '\0'
            int copy_new_size = new_size - 1;   // subtracted one to exclude null while storing
            endian_convert(&new_size);          // converted to big endian
            fwrite(&new_size, 4, 1, fp);        // Writes the new size as 4 bytes into the file in big endian format

            char content[3];
            fread(content, 3, 1, fptr);
            fwrite(content, 3, 1, fp);
            endian_convert(&size); // big endian size read converting to little endian

            fseek(fptr, size - 1, SEEK_CUR);       // skipping old content
            fwrite(newinfo, copy_new_size, 1, fp); // store new content skips null at the end (null already stored at beginning)

            char rest_content;
            while (fread(&rest_content, 1, 1, fptr) > 0) // not using EOF
            {
                fputc(rest_content, fp);
            }
            printf("Successfully updated %s to '%s'\n", mp3tags[index_1 - 1].label, newinfo);
            fclose(fp);
            fclose(fptr);

            const char *old_file = old_file_name;
            const char *new_file = "sample_edit.mp3";

            // First, remove the original file
            if (remove(old_file) != 0)
            {
                perror("Error while saving data!");
                return;
            }

            // Then rename the new file to the original file's name
            if (rename("sample_edit.mp3", old_file_name) != 0)
            {
                perror("Error while saving data!");
                return;
            }

            return;
        }
    }
}

void help(char *version)
{
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("MP3 Tag Reader and Editor\n");
    if (version != NULL)
    printf("Version ID: 2.%d\n", version[0]);
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("Usage:\n    ./a.out [OPTION] [ARGS...]\nOptions:\n");
    printf("    -e <file_name> <specifier> <new_value>   Edit the specified tag in the MP3 file\n");
    printf("    -h <file_name>                           Show help and usage information\n");
    printf("    -v <file_name>                           View ID3v2 tags of the MP3 file\n");

    printf("Tag Specifiers(used with -e option):\n");
    printf("    -A  Modifies  Album Tag\n");
    printf("    -a  Modifies  Artist Tag\n");
    printf("    -C  Modifies  Content Tag\n");
    printf("    -c  Modifies  Comment Tag\n");
    printf("    -t  Modifies  Title Tag\n");
    printf("    -y  Modifies  Year Tag\nExamples:\n");

    printf("    To edit a specific tag          :       ./a.out  -e sample.mp3 -y 2025\n");
    printf("    To include spaces in new data, wrap it in double quotes. Example: ./a.out -e sample.mp3 -A \"Sunny Sunny\"\n");
    printf("    To show help and usage info:    :       ./a.out -h sample.mp3\n");
    printf("    To view MP3 file details:       :       ./a.out -v sample.mp3\n");
    printf("-------------------------------------------------------------------------------------------------------------\n");
}
