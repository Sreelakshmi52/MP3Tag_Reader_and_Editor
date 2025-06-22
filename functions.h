#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Function prototypes (declarations)
// Main functions
void display(FILE *, char *);
void edit(FILE *, char *, char *, char *);
void help(char *);

// Sub functions
void endian_convert(int *);
int tags(char *, int, char *);

#endif // FUNCTIONS_H
