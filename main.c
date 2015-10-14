#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include "shell_library.h"

//defines
typedef int bool;
#define true 1
#define false 0
#define MAX_STRING_LENGTH 512

enum error_codes error;
char* input_line;
//history count
int current_command_count = 0;
//history
char** commands_entered;

int main(int argc, char** argv)
{
    input_line = malloc(512);
    if(argc < 2){
        //interactive mode
        while(1){
            printf("shell>");
            if(!fgets(input_line , 512 , stdin)){
                error = input_exception ;
                print_error(error);
                return 0;
            }
            else if(validate_input(input_line)){
                char** s = remove_spaces(input_line);
                execute(s);
            }
        }
    }
    else {
        //batch mode
        //file not found
        if(!freopen(argv[1],"r",stdin)){
            error = file_not_found_exception;
            print_error(error);
        }
        //file found
        else{
            while(fgets(input_line , 512 , stdin)){
                if(validate_input(input_line)){
                    printf("%s\n" , input_line);
                    char** s = remove_spaces(input_line);
                    execute(s);
                }
                else{
                    error = command_too_long_exception;
                    print_error(error);
                }
            }
        }
    }
    return 0;
}

void print_error(enum error_codes error){
    switch(error){
        case file_not_found_exception:
        printf("Batch file not found\n");
        break;
        case command_too_long_exception:
        printf("Your command must not exceed 512 characters\n");
        break;
        case input_exception:
        printf("Failed to get input\n");
        break;
        case empty_command_exception:
        printf("Empty command entered\n");
        break;
    }
}

char** remove_spaces(char* input){
    int string_count = get_word_count(input);
    char** arr;
    arr = malloc((string_count+1) * sizeof(char *));
    int i ;
    for(i = 0 ; i < string_count ; i ++ ){
        arr[i] = malloc(MAX_STRING_LENGTH * sizeof(char *));
    }
    //remove spaces and tokenize
    char* temp;
    temp = strtok(input , " \t\n");
    i = 0;
    while(temp){
        arr[i] = temp;
        temp = strtok(NULL , "\t \n");
        i++;
    }
    return arr;
}

int validate_input(char* input){
    int valid = false;
    int i;
    for(i = 0 ; i < 512 ; i++){
        if(input[i] == '\0'){
            valid = true;
            break;
        }
    }
    return valid;
}

int get_word_count(char* input){
    int counter = 0 ;
    int i = 0;
    while(input[i]!='\0' && input[i]!='\n'){
        if(input[i]!=' ' && input[i]!='\t' && input[i]!='\0' && input[i]!='\n'){
            counter++;
            while(input[i]!=' ' && input[i]!='\t' && input[i]!='\0' && input[i]!='\n'){
                i++;
            }
        }
        else  i++;
    }
    return counter;
}

void execute(char** command_params){
    bool background = false;
    char* command = command_params[0];
    if(command == NULL){
        error = empty_command_exception;
        print_error(error);
    }
    else{
        //command_params = &command_params[1];
        int i = 0;
        while(command_params[i]!=NULL){
            if(command_params[i][0]=='&' && command_params[i+1]==NULL){
                background = true;
                break;
            }
            else if(command_params[i]==NULL)break;
            i++;
        }
        printf("%s\n" , command_params[0]);
        execvp(command , command_params);
    }
    return;
}
