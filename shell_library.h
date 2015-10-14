#ifndef ERROR_CODES

#define ERROR_CODES
enum error_codes{
    file_not_found_exception,
    command_too_long_exception,
    input_exception,
    empty_command_exception,
};

void print_error(enum error_codes error);
char** remove_spaces(char* input);
int validate_input(char* input); //checks for string length  <= 512
int get_word_count(char* input);
void execute(char** command_params);
#endif // ERROR_CODES

