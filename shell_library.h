#ifndef ERROR_CODES

#define ERROR_CODES
enum error_codes{
    file_not_found_exception,
    command_too_long_exception,
    input_exception,
    empty_command_exception,
    closing_shell,
    change_directory_failed,
    command_failed,
};
typedef struct{
    char* variable_name;
    char* value;
}variable;

void print_error(enum error_codes error);
char** remove_spaces(char* input);
int validate_input(char* input); //checks for string length  <= 512
int get_word_count(char* input);
void execute(char** command_params);
int compare_strings(char* a , char* b);
void make_system_calls(char* command , char** command_params);
void change_directory(char* path);
void signalHandler(int sig);
char* copy_directory_path(char* path);
void print_history();
void add_to_history(char* command);
int variable_exists(char* variable_name);
int is_valid_expression(char* input);
char** calculate_expression(char* command);
#endif // ERROR_CODES

