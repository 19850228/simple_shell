# Simple Shell

## Version: 1.0

### Description
This is a simple command-line shell application developed in C. It provides basic shell functionality, allowing users to execute single-word commands. Advanced features, such as pipes, redirections, and multiple arguments, are not supported.

### Compilation
To compile the Simple Shell program, use the following command:

```bash
gcc -Wall -Werror -Wextra -pedantic -std=gnu89 main.c -o simple_shell
```

### Running the Shell
To run the Simple Shell, execute the compiled program:
```bash
./simple_shell
```

Once the shell is running, it will display a prompt like this:

```bash
Simple Shell >
```

You can enter one-word commands, and the shell will execute them. To exit the shell, type "exit" and press Enter or Ctrl + D.

### Example
Here's an example of running the shell and executing a command:

```plaintext
Simple Shell > ls
file1.txt  file2.txt  main.c  simple_shell
Simple Shell > exit
Exiting the shell.
``
