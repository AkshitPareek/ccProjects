#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#define BUF_SIZE 16384

/* return number of bytes of file */
uint64_t numBytes(FILE* filename) {
    long current = ftell(filename);
    fseek(filename, 0L, SEEK_END);
    uint64_t size = ftell(filename);
    fseek(filename, current, SEEK_SET);
    return size;
}

/* return number of line of a file */
uint64_t numLines(FILE* filename) {

    char buffer[BUF_SIZE];
    uint64_t count = 0;
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, sizeof(buffer), filename)) > 0) {
        size_t i;
        for(i=0; i<bytes_read; i++) {
            if(buffer[i] == '\n') {
                count++;
            }
        }
    }

    rewind(filename);
    return count;
}

/* return number of words of a file */
uint64_t numWords(FILE* filename) {

    char buffer[BUF_SIZE];
    uint64_t count = 0;
    size_t bytes_read;
    unsigned int in_word = 0;

    while((bytes_read = fread(buffer, 1, sizeof(buffer), filename)) > 0) {
        size_t i;
        for(i=0; i<bytes_read; i++) {
            if(isspace((unsigned char)buffer[i]) || ispunct((unsigned char)buffer[i])) {
                in_word = 0;
            }
            else if(!in_word) {
                in_word = 1;
                count++;
            }
        }
    }

    if(in_word) {
        count++;
    }

    rewind(filename);
    return count;
}

/* return number of chars of a file */
uint64_t numChars(FILE* filename) {

    char buffer[BUF_SIZE];
    uint64_t count = 0;
    size_t bytes_read;

    while((bytes_read = fread(buffer, 1, sizeof(buffer), filename)) > 0) {
        count+=bytes_read;
    }

    rewind(filename);
    return count;
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    FILE *file; 
    char* filename = NULL;
    int use_stdin = 0;

    if(argc == 2 || (argc == 3 && strcmp(argv[2], "-") == 0)) {
        file = stdin;
        use_stdin = 1;
    }
    else if(argc == 3) {
        file = fopen(argv[2], "r");
        if(file == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        filename = argv[2];
    }
    else {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    switch(argv[1][1]) {
        case 'c':
            printf("%" PRIu64,
                numBytes(file));   
        break;
        case 'l':
            printf("%" PRIu64,
                numLines(file));
        break;
        case 'w':
            printf("%" PRIu64,
                numWords(file));
        break;
        case 'm':
            printf("%" PRIu64,
                numChars(file));
        break;
        default:
            printf("%" PRIu64 " %" PRIu64 " %" PRIu64,
                numLines(file), numWords(file), numChars(file));
        break;
    }
    if(filename)
        printf(" %s", filename);

    printf("\n"); /* ensuring newline for terminal output*/
    
    if(!use_stdin)
        fclose(file);

    exit(EXIT_SUCCESS);
}
