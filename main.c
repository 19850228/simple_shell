#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10
#define MAX_ALIASES 100
#define MAX_ALIAS_LENGTH 50

typedef struct
{
    char name[MAX_ALIAS_LENGTH];
    char value[MAX_ALIAS_LENGTH];
} Alias;

Alias aliases[MAX_ALIASES];
int num_aliases = 0;

void print_aliases()
{
    int i;
    for (i = 0; i < num_aliases; i++)
    {
        printf("%s='%s'\n", aliases[i].name, aliases[i].value);
    }
}

Alias *find_alias(char *name)
{
    int i;
    for (i = 0; i < num_aliases; i++)
    {
        if (strcmp(aliases[i].name, name) == 0)
        {
            return &aliases[i];
        }
    }
    return NULL;
}

void add_alias(char *name, char *value)
{
    Alias *existing_alias = find_alias(name);
    if (existing_alias != NULL)
    {
        strcpy(existing_alias->value, value);
    }
    else
    {
        if (num_aliases < MAX_ALIASES)
        {
            strcpy(aliases[num_aliases].name, name);
            strcpy(aliases[num_aliases].value, value);
            num_aliases++;
        }
        else
        {
            printf("Maximum number of aliases reached.\n");
        }
    }
}

void parse_alias_command(char *command)
{
    char *token;
    char *saveptr;
    char *name;
    char *value;

    token = strtok_r(command, " \t", &saveptr);
    token = strtok_r(NULL, " \t", &saveptr);

    if (token == NULL)
    {
        print_aliases();
        return;
    }

    while (token != NULL)
    {
        if (strchr(token, '=') != NULL)
        {
            name = strtok_r(token, "=", &saveptr);
            value = strtok_r(NULL, "=", &saveptr);
            if (value != NULL)
            {
                add_alias(name, value);
            }
        }
        else
        {
            Alias *alias = find_alias(token);
            if (alias != NULL)
            {
                printf("%s='%s'\n", alias->name, alias->value);
            }
        }

        token = strtok_r(NULL, " \t", &saveptr);
    }
}

char *get_path(const char *command)
{
    char *path = getenv("PATH");
    char *path_copy = strdup(path);
    char *path_dir = strtok(path_copy, ":");

    while (path_dir)
    {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path_dir, command);
        if (access(full_path, X_OK) == 0)
        {
            free(path_copy);
            return strdup(full_path);
        }
        path_dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

int execute_cd(char *arguments[], int arg_count)
{
    char *dir;
    char *current_dir;
    char *new_dir;

    if (arg_count == 1)
    {
        dir = getenv("HOME");
    }
    else if (arg_count == 2)
    {
        if (strcmp(arguments[1], "-") == 0)
        {
            dir = getenv("OLDPWD");
        }
        else
        {
            dir = arguments[1];
        }
    }
    else
    {
        fprintf(stderr, "cd: Invalid number of arguments. Usage: cd [DIRECTORY]\n");
        return 1;
    }

    current_dir = getcwd(NULL, 0);
    if (current_dir == NULL)
    {
        fprintf(stderr, "cd: Failed to get current directory.\n");
        return 1;
    }

    if (chdir(dir) != 0)
    {
        fprintf(stderr, "cd: Failed to change directory to '%s'\n", dir);
        free(current_dir);
        return 1;
    }

    if (setenv("OLDPWD", current_dir, 1) != 0)
    {
        fprintf(stderr, "cd: Failed to set OLDPWD environment variable.\n");
        free(current_dir);
        return 1;
    }

    new_dir = getcwd(NULL, 0);
    if (new_dir == NULL)
    {
        fprintf(stderr, "cd: Failed to get current directory.\n");
        free(current_dir);
        return 1;
    }

    if (setenv("PWD", new_dir, 1) != 0)
    {
        fprintf(stderr, "cd: Failed to set PWD environment variable.\n");
        free(current_dir);
        free(new_dir);
        return 1;
    }

    free(current_dir);
    free(new_dir);

    return 0;
}

int execute_setenv(char *arguments[], int arg_count)
{
    int result;

    if (arg_count != 3)
    {
        fprintf(stderr, "setenv: Invalid number of arguments. Usage: setenv VARIABLE VALUE\n");
        return 1;
    }

    result = setenv(arguments[1], arguments[2], 1);
    if (result != 0)
    {
        fprintf(stderr, "setenv: Failed to set environment variable.\n");
        return 1;
    }

    return 0;
}

int execute_unsetenv(char *arguments[], int arg_count)
{
    int result;

    if (arg_count != 2)
    {
        fprintf(stderr, "unsetenv: Invalid number of arguments. Usage: unsetenv VARIABLE\n");
        return 1;
    }

    result = unsetenv(arguments[1]);
    if (result != 0)
    {
        fprintf(stderr, "unsetenv: Failed to unset environment variable.\n");
        return 1;
    }

    return 0;
}

void execute_command(char *command, char *program_name)
{
    char *token;
    char *program;
    char *arguments[MAX_ARGUMENTS];
    int arg_count = 0;
    char *path;

    pid_t pid;

    char *input = strdup(command);
    char *saveptr;

    token = strtok_r(input, " \n", &saveptr);
    program = strdup(token);

    while (token != NULL)
    {
        if (strpbrk(token, "|><&;"))
        {
            fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, token, "not found");
            free(input);
            free(program);
            return;
        }
        arguments[arg_count] = token;
        arg_count++;
        token = strtok_r(NULL, " \n", &saveptr);
    }

    arguments[arg_count] = NULL;

    free(input);

    if (arg_count == 0)
    {
        fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, "error", "No command provided.");
        free(program);
        return;
    }

    if (strcmp(program, "exit") == 0)
    {
        int status = 0;
        if (arg_count > 1)
        {
            status = atoi(arguments[1]);
        }
        exit(status);
    }
    else if (strcmp(program, "setenv") == 0)
    {
        int result = execute_setenv(arguments, arg_count);
        if (result != 0)
        {
            fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, "error", "Failed to set environment variable");
        }
    }
    else if (strcmp(program, "unsetenv") == 0)
    {
        int result = execute_unsetenv(arguments, arg_count);
        if (result != 0)
        {
            fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, "error", "Failed to unset environment variable");
        }
    }
    else
    {
        path = get_path(program);

        if (path == NULL)
        {
            fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, "error", "Command not found");
            free(program);
            return;
        }

        pid = fork();
        if (pid == 0)
        {
            if (execv(path, arguments) == -1)
            {
                fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, program, "error", "Execution failed");
                exit(1);
            }
        }
        else if (pid < 0)
        {
            fprintf(stderr, "%s: %s: 1: %s: %s\n", program_name, "error", "Fork failed", strerror(errno));
            perror("Fork failed");
        }
        else
        {
            wait(NULL);
            free(path);
        }
    }

    free(program);
}

void print_environment()
{
    extern char **environ;
    char **env;
    for (env = environ; *env != NULL; env++)
    {
        printf("%s\n", *env);
    }
}

#define BUFFER_SIZE 1024

static char buffer[BUFFER_SIZE];
static char *buf_ptr = buffer;
static int buf_remaining = BUFFER_SIZE;

char *my_getline(char **lineptr)
{
    char *ret;
    char *newline;

    if (buf_remaining == 0)
    {
        buf_remaining = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        buf_ptr = buffer;
    }

    if (buf_remaining == 0)
    {
        return NULL;
    }

    ret = buf_ptr;

    newline = strchr(buf_ptr, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
        buf_ptr = newline + 1;
        buf_remaining -= (buf_ptr - buffer);
    }
    else
    {
        *lineptr = buf_ptr;
        buf_remaining = 0;
    }

    return ret;
}

void execute_commands(char *commands, char *program_name)
{
    char *token;
    char *saveptr;
    size_t len;

    token = strtok_r(commands, ";", &saveptr);

    while (token != NULL)
    {
        char *command = token;
        while (*command == ' ' || *command == '\t')
        {
            command++;
        }

        len = strlen(command);
        while (len > 0 && (command[len - 1] == ' ' || command[len - 1] == '\t'))
        {
            command[len - 1] = '\0';
            len--;
        }

        if (strlen(command) > 0)
        {
            execute_command(command, program_name);
        }

        token = strtok_r(NULL, ";", &saveptr);
    }
}

void replace_variables(char *command)
{
    char *token;
    char *saveptr;
    char *variable;
    char *value;

    token = strtok_r(command, " \t\n", &saveptr);

    while (token != NULL)
    {
        if (token[0] == '$')
        {
            variable = token + 1;

            if (strcmp(variable, "?") == 0)
            {
                value = strerror(errno);
                printf("%s ", value);
            }
            else if (strcmp(variable, "$") == 0)
            {
                value = malloc(sizeof(char) * 10);
                sprintf(value, "%d", getpid());
                printf("%s ", value);
                free(value);
            }
            else
            {
                Alias *alias = find_alias(variable);
                if (alias != NULL)
                {
                    printf("%s ", alias->value);
                }
            }
        }
        else
        {
            printf("%s ", token);
        }

        token = strtok_r(NULL, " \t\n", &saveptr);
    }

    printf("\n");
}

void execute_file_commands(char *filename)
{
    FILE *file = fopen(filename, "r");
    char command[MAX_COMMAND_LENGTH];
    char *comment_pos = strchr(command, '#');

    if (file == NULL)
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    while (fgets(command, MAX_COMMAND_LENGTH, file) != NULL)
    {
        if (command[0] == '\0')
        {
            continue;
        }

        command[strcspn(command, "\n")] = '\0';

        if (comment_pos != NULL)
        {
            *comment_pos = '\0';
        }

        replace_variables(command);

        system(command);
    }

    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        execute_file_commands(argv[1]);
    }
    else
    {
        char command[MAX_COMMAND_LENGTH];
        char *program_name = "simple_shell";
        char *comment_pos;

        while (1)
        {
            printf(":) ");
            if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL)
            {
                printf("Exiting the shell.\n");
                break;
            }

            if (command[0] == '\0')
            {
                continue;
            }

            /* Check if the user entered "exit" */
            if (strcmp(command, "exit") == 0)
            {
                printf("Exiting the shell.\n");
                break;
            }

            /* Check if the user entered "env" */
            if (strcmp(command, "env") == 0)
            {
                print_environment();
                continue;
            }

            if (strcmp(command, "alias\n") == 0)
            {
                print_aliases();
                continue;
            }

            if (strncmp(command, "alias ", 6) == 0)
            {
                parse_alias_command(command + 6);
                continue;
            }

            comment_pos = strchr(command, '#');
            if (comment_pos != NULL)
            {
                *comment_pos = '\0';
            }

            replace_variables(command);

            if (strchr(command, ';') != NULL || strchr(command, '&') != NULL || strchr(command, '|') != NULL)
            {
                execute_commands(command, program_name);
            }
            else
            {
                execute_command(command, program_name);
            }
        }

        return 0;
    }
    return 0;
}

