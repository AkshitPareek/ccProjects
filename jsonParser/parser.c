#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define BUF_SIZE 16384
#define MAX_TOKEN_SIZE 1024

char* input;
int idx;

typedef enum {
    T_EOF,
    T_STRING,
    T_NUMBER,
    T_TRUE,
    T_FALSE,
    T_NULL,
    T_LBRACE,
    T_RBRACE,
    T_LBRACKET,
    T_RBRACKET,
    T_COMMA,
    T_COLON,
    T_INVALID
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_SIZE];
} Token;

int parse_value();

Token lexer() {
    Token token;
    char c;

    while(isspace(input[idx])) {
        idx++;
    }

    c = input[idx];

    switch(c) {
        case '\0':
            token.type = T_EOF;
        break;
        case '{':
            token.type = T_LBRACE;
            idx++;
        break;
        case '}':
            token.type = T_RBRACE;
            idx++;
        break;
        case '[':
            token.type = T_LBRACKET;
            idx++;
        break;
        case ']':
            token.type = T_RBRACKET;
            idx++;
        break;
        case ',':
            token.type = T_COMMA;
            idx++;
        break;
        case ':':
            token.type = T_COLON;
            idx++;
        break;
        case '"':
            token.type = T_STRING;
            int pos = 0;
            idx++;
            while(input[idx] != '"' && input[idx] != '\0') {
                token.value[pos++] = input[idx++];
            }
            token.value[pos] = '\0';
            if(input[idx] == '"')
                idx++;
        break;
        case '-':
        default:
            if(isdigit(c) || c == '-') {
                token.type = T_NUMBER;
                int pos = 0;
                while(input[idx] != '\0' && pos < MAX_TOKEN_SIZE - 1 && 
                    (isdigit(input[idx]) || input[idx] == '.' || input[idx] == '-' || input[idx] == 'e' || input[idx] == 'E')) {
                    token.value[pos++] = input[idx++];
                }
                token.value[pos] = '\0';
            }
            else if(strncmp(&input[idx], "true", 4) == 0) {
                token.type = T_TRUE;
                idx += 4;
            }
            else if(strncmp(&input[idx], "false", 5) == 0) {
                token.type = T_FALSE;
                idx += 5;
            }
            else if(strncmp(&input[idx], "null", 4) == 0) {
                token.type = T_NULL;
                idx += 4;
            }
            else {
                token.type = T_INVALID;
                idx++;
            }
        break;
    }

    return token;
}

int parse_object() {
    Token token = lexer();

    if(token.type != T_LBRACE)
        return 0;

    token = lexer();

    if(token.type == T_RBRACE)
        return 1;

    while(1) {
        if(token.type != T_STRING)
            return 0;

        token = lexer();

        if(token.type != T_COLON)
            return 0;

        if(!parse_value())
            return 0;

        token = lexer();

        if(token.type == T_RBRACE)
            return 1;

        if(token.type != T_COMMA)
            return 0;

        token = lexer();
    }

}

int parse_array() {
    Token token = lexer();

    if(token.type != T_LBRACKET)
        return 0;

    token = lexer();

    if(token.type == T_RBRACKET)
        return 1;

    while(1) {
        if(!parse_value())
            return 0;

        token = lexer();

        if(token.type == T_RBRACKET)
            return 1;
        
        if(token.type != T_COMMA)
            return 0;
    }
}

int parse_value() {
    Token token = lexer();

    switch(token.type) {
        case T_STRING:
        case T_NUMBER:
        case T_TRUE:
        case T_FALSE:
        case T_NULL:
            return 1;
        case T_LBRACE:
            idx--;
            return parse_object();
        case T_LBRACKET:
            idx--;
            return parse_array();
        default:
            return 0;
    }
}

int parse_json(FILE* file) {
    char buffer[BUF_SIZE];

    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytesRead] = '\0';

    input = buffer;

    if(parse_value()) {
        Token token = lexer();
        return token.type == T_EOF;
    }

    // rewind(file);
    return 0;
}


int main(int argc, char* argv[]) {
    if(argc < 1) {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* file;
    char* filename = NULL;
    int use_stdin = 0;

    if(argc == 1) {
        file = stdin;
        use_stdin = 1;
    }
    else if(argc == 2) {
        file = fopen(argv[1], "r");
        if(file == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        filename = argv[1];
    }
    else {
        fprintf(stderr, "Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }

    if(parse_json(file)) {
        if(filename)
            printf("%s ", filename);
        printf("is a valid json file\n");
        exit(EXIT_SUCCESS);
    }
    else {
        if(filename)
            printf("%s ", filename);
        printf(" :invalid json file\n");
        exit(EXIT_FAILURE);
    }

    if(!use_stdin)
        fclose(file);

    exit(EXIT_SUCCESS);
}