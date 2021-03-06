#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell_library.h"

//defines
typedef int bool;
#define true 1
#define false 0
#define MAX_STRING_LENGTH 512
#define MAX_VARIABLES_COUNT 100

enum error_codes error;
char* input_line;
char* program_directory;
int variables_count = 0;
variable variables[MAX_VARIABLES_COUNT];

int main(int argc, char** argv)
{
    program_directory = malloc(512);
    program_directory = getcwd(program_directory , 512);
    input_line = malloc(512);
    int k = 0;
    for(k = 0 ; k < MAX_VARIABLES_COUNT; k++)
    {
        variables[k].variable_name = malloc(512);
        variables[k].value = malloc(600);
    }
    strcpy(variables[variables_count].variable_name , "PATH");
    strcpy(variables[variables_count].value , getenv("PATH") );
    variables_count++;
    if(argc < 2)
    {
        //interactive mode
        while(1)
        {
            printf("shell>");
            if(!fgets(input_line , 512 , stdin))
            {
                error = input_exception ;
                print_error(error);
                return 0;
            }
            else if(validate_input(input_line))
            {
                char** s = remove_spaces(input_line);
                execute(s);
            }
        }
    }
    else
    {
        //batch mode
        //file not found
        if(!freopen(argv[1],"r",stdin))
        {
            error = file_not_found_exception;
            print_error(error);
        }
        //file found
        else
        {
            while(fgets(input_line , 512 , stdin))
            {
                if(validate_input(input_line))
                {
                    printf("shell>%s\n" , input_line);
                    char** s = remove_spaces(input_line);
                    execute(s);
                }
                else
                {
                    error = command_too_long_exception;
                    print_error(error);
                }
            }
        }
    }
    return 0;
}

void print_error(enum error_codes error)
{
    switch(error)
    {
    case file_not_found_exception:
            printf("Batch file not found\n");
        break;
    case command_too_long_exception:
        printf("Your command must not exceed 512 characters\n");
        break;
    case input_exception:
        printf("Program ended\n");
        break;
    case empty_command_exception:
        printf("Empty command entered\n");
        break;
    case closing_shell:
        printf("closing shell\n");
        break;
    case change_directory_failed:
        printf("failed to change current directory\n");
        break;
    case command_failed:
        printf("failed to execute this command\n");
        break;
    }
}

char** remove_spaces(char* input)
{
    int string_count = get_word_count(input);
    char** arr;
    arr = malloc((string_count+1) * sizeof(char *));
    int i ,start;
    for(i = 0 ; i < string_count ; i ++ )
    {
        arr[i] = malloc(MAX_STRING_LENGTH);
    }
    //remove spaces and tokenize
    char* temp = malloc(512);
    i = 0;
    int arr_index = 0;
    while(input[i]!='\0')
    {
        //quotations
        if(input[i]=='"')
        {
            i++;
            start = i;
            while(input[i]!='"' && input[i]!='\0')
            {
                temp[i - start]=input[i];
                i++;
            }
            temp[i - start] = '\0';
            strcpy(arr[arr_index] , temp);
            arr_index++;
        }
        else if(input[i]=='\'')
        {
            i++;
            start = i;
            while(input[i]!='\'' && input[i]!='\0')
            {
                temp[i - start]=input[i];
                i++;
            }
            temp[i - start] = '\0';
            strcpy(arr[arr_index] , temp);
            arr_index++;
        }
        else if(input[i]==' ' || input[i]=='\t' || input[i]=='\n')i++;
        else
        {
            start = i;
            int quot_number = 0;
            while(input[i]!=' ' && input[i]!='\0' && input[i]!='\t' && input[i]!='\n')
            {
                if(input[i]=='"'){
                    quot_number++;
                    i++;
                    while(input[i]!='\0' && input[i]!='"')
                    {
                        temp[i - start-(quot_number)]=input[i];
                        i++;
                    }
                }
                else if(input[i]=='\''){
                    quot_number++;
                    i++;
                    while(input[i]!='\0' && input[i]!='\'')
                    {
                        temp[i - start-(quot_number)]=input[i];
                        i++;
                    }
                }
                else{
                    temp[i - start]=input[i];
                    i++;
                }
            }
            temp[i-start] = '\0';
            if(i-start>=1){
                strcpy(arr[arr_index] , temp);
                arr_index++;
            }
        }
    }
    arr[arr_index] = NULL;
    return arr;
}

int validate_input(char* input)
{
    int valid = false;
    int i;
    for(i = 0 ; i < 512 ; i++)
    {
        if(input[i] == '\0')
        {
            valid = true;
            break;
        }
    }
    add_to_history(input);
    return valid;
}

int get_word_count(char* input)
{
    int counter = 0 ;
    int i = 0;
    while(input[i]!='\0' && input[i]!='\n')
    {
        counter++;
        if(input[i]!=' ' && input[i]!='\t' && input[i]!='\0' && input[i]!='\n')
        {
            while(input[i]!=' ' && input[i]!='\t' && input[i]!='\0' && input[i]!='\n')
            {
                i++;
            }
        }
        else  i++;
    }
    return counter;
}

void execute(char** command_params)
{
    bool background = false;
    char* command = command_params[0];
    if(command == NULL || command[0] == NULL)
    {
        error = empty_command_exception;
        print_error(error);
    }
    else
    {
        //check if exit to terminate main process
        if(compare_strings(command_params[0] , "exit"))
        {
            error = closing_shell;
            print_error(error);
            exit(0);
        }
        int i = 0;
        while(command_params[i]!=NULL)
        {
            if(command_params[i][0]=='&' && command_params[i+1]==NULL)
            {
                background = true;
                command_params[i] = NULL;
                break;
            }
            else if(command_params[i][0]==NULL)command_params[i] = NULL;
            else if(command_params[i]==NULL)break;
            i++;
        }
        if(compare_strings(command_params[0] , "cd"))
        {
            change_directory(command_params[1]);
        }
        else if(command_params[1]==NULL && is_valid_expression(command_params[0]))
        {
            char* variable_name;
            char* var_value;
            variable_name = malloc(512);
            var_value = malloc(512);
            char** t = calculate_expression(command_params[0]);
            strcpy(variable_name,t[0]);
            strcpy(var_value , t[1]);
            int index = variable_exists(variable_name);
            if(index!=-1)
            {
                strcpy(variables[index].value , var_value );
            }
            else
            {
                strcpy(variables[variables_count].variable_name , variable_name );
                strcpy(variables[variables_count].value , var_value );
                variables_count++;
            }
        }
        else
        {
            signal(SIGCHLD , signalHandler);
            int status;
            pid_t pid = fork();
            //child
            if(pid == 0)
            {
                make_system_calls(command , command_params);
            }
            //parent
            else
            {
                if(!background)
                {
                    waitpid(pid , &status , 0);
                }
            }
        }
    }
    return;
}

void change_directory(char* path)
{
    if(path == "" || path == NULL)
        path = getenv("HOME");
    int res = chdir(path);
    if(res != 0)
    {
        error = change_directory_failed;
        print_error(error);
    }
}

void make_system_calls(char* command , char** command_params)
{
    if(compare_strings(command_params[0] , "history"))
    {
        print_history();
    }
    else
    {
        char* path = getenv("PATH");
        //check if calls parameters
        if(compare_strings(command , "echo"))
        {
            if(command_params[1]!= NULL && command_params[1][0]=='$')
            {
                //check if variable exists
                char* temp_alloc = malloc(strlen(command_params[1])-1);
                int j = 0 , index = 0;
                while(command_params[1][j]!='\0')
                {
                    if(command_params[1][j]!='$')
                    {
                        temp_alloc[index] = command_params[1][j];
                        index++;
                    }
                    j++;
                }
                temp_alloc[index]='\0';
                int place = variable_exists(temp_alloc);
                if(place!=-1)
                {
                    strcpy(command_params[1] , variables[place].value);
                }
                else strcpy(command_params[1] , "");
            }
        }
        execv(command , command_params);
        char** arr;
        arr = malloc((512) * sizeof(char *));
        int i;
        for(i = 0 ; i < 512; i ++ )
        {
            arr[i] = malloc(MAX_STRING_LENGTH);
        }
        i = 0;
        i = 0;
        char* temp = malloc(512);
        temp = strtok(path , ":");
        while(temp!=NULL)
        {
            strcpy(arr[i] , temp);
            i++;
            temp = strtok(NULL,":");
        }

        char* original_command = malloc(512);
        strcpy(original_command , command);
        //law comment
        if(original_command[0]!='#')
        {
            i = 0;
            while(arr[i]!=NULL)
            {
                char* new_command = malloc(512);
                strcpy(new_command , copy_directory_path(arr[i]));
                strncat(new_command , original_command , 512);
                strcpy(command_params[0] , new_command);
                execv(new_command,command_params);
                i++;
            }
        }
        error = command_failed;
        print_error(error);
    }
    exit(0);
}

bool compare_strings(char* a , char* b)
{
    int i = 0;
    while(a[i]==b[i])
    {
        if(a[i]=='\0')return true;
        i++;
    }
    return false;
}

void signalHandler(int sig)
{
    char* address = malloc(strlen(program_directory)*2 + 8);
    strcpy(address , copy_directory_path(program_directory));
    strncat(address , "log.txt" , strlen("log.txt"));
    FILE *f;
    f = fopen(address , "a");
    fputs("child process terminated\n" , f);
    fclose(f);
}

void add_to_history(char* command)
{
    char* address = malloc(strlen(program_directory)*2 + 8);
    strcpy(address , copy_directory_path(program_directory));
    strncat(address , "history.txt" , strlen("history.txt"));
    FILE *f;
    f = fopen(address , "a");
    fputs(command , f);
    fclose(f);
}

void print_history()
{
    char* temp = malloc(600);
    char* address = malloc(strlen(program_directory)*2 + 8);
    strcpy(address , copy_directory_path(program_directory));
    strncat(address , "history.txt" , strlen("history.txt"));
    FILE *f;
    f = fopen(address , "r");
    temp = fgets(temp , 512 , f);
    while(temp!=NULL)
    {
        puts(temp);
        temp = fgets(temp , 512 , f);
    }
    fclose(f);
}

char* copy_directory_path(char* path)
{
    char* temp = malloc(strlen(path)*2);
    int t_index = 0 , p_index = 0;
    while(path[p_index]!='\0')
    {
        temp[t_index] = path[p_index];
        p_index++;
        t_index++;
    }
    temp[t_index] = '/';
    t_index++;
    temp[t_index] = '\0';
    return temp;
}

int variable_exists(char* variable_name)
{
    int i ;
    for(i = 0 ; i < variables_count; i++)
    {
        if(compare_strings(variable_name , variables[i].variable_name))
        {
            return i;
        }
    }
    return -1;
}

bool is_valid_expression(char* input)
{
    int c = 0;
    int i = 0;
    bool valid = false;
    while(input[i]!='\0')
    {
        if(input[i] == '=')
        {
            valid = true;
        }
       // else if(input[i]==' ')return false;
        i++;
    }
    return valid;
}

char** calculate_expression(char* command)
{
    printf("%s\n" , command);
    char** arr;
    arr = malloc(2 * sizeof(char *));
    int i;
    for(i = 0 ; i < 2 ; i ++)
    {
        arr[i] = malloc(512);
    }
    char* temp = malloc(512);
    temp = strtok(command , "=");
    strcpy(arr[0] , temp);
    temp = strtok(NULL , "");
    strcpy(arr[1] , temp);
    return arr;
}
