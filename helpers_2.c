#include "main.h"

void handle_line(char **line, ssize_t read);
ssize_t get_new_len(char *line);
void logical_ops(char *line, ssize_t *new_len);

/**
 * handle_line - Partitions a line read from standard input as needed.
 * @line: pointer to a line read from standard inputs
 * @read: The length of line.
 *
 * Description: Spaces are inserted to separate ";", "||", and "&&".
 *              Replaces "#" with '\0'.
 */
void handle_line(char **line, ssize_t read)
{
	char *old_line, *new_line;
	char previous, current, next;
	size_t a, j;
	ssize_t new_len;

	new_len = get_new_len(*line);
	if (new_len == read - 1)
		return;
	new_line = malloc(new_len + 1);
	if (!new_line)
		return;
	j = 0;
	old_line = *line;
	for (a = 0; old_line[a]; a++)
	{
		current = old_line[a];
		next = old_line[a + 1];
		if (a != 0)
		{
			previous = old_line[a - 1];
			if (current == ';')
			{
				if (next == ';' && previous != ' ' && previous != ';')
				{
					new_line[j++] = ' ';
					new_line[j++] = ';';
					continue;
				}
				else if (previous == ';' && next != ' ')
				{
					new_line[j++] = ';';
					new_line[j++] = ' ';
					continue;
				}
				if (previous != ' ')
					new_line[j++] = ' ';
				new_line[j++] = ';';
				if (next != ' ')
					new_line[j++] = ' ';
				continue;
			}
			else if (current == '&')
			{
				if (next == '&' && previous != ' ')
					new_line[j++] = ' ';
				else if (previous == '&' && next != ' ')
				{
					new_line[j++] = '&';
					new_line[j++] = ' ';
					continue;
				}
			}
			else if (current == '|')
			{
				if (next == '|' && previous != ' ')
					new_line[j++]  = ' ';
				else if (previous == '|' && next != ' ')
				{
					new_line[j++] = '|';
					new_line[j++] = ' ';
					continue;
				}
			}
		}
		else if (current == ';')
		{
			if (a != 0 && old_line[a - 1] != ' ')
				new_line[j++] = ' ';
			new_line[j++] = ';';
			if (next != ' ' && next != ';')
				new_line[j++] = ' ';
			continue;
		}
		new_line[j++] = old_line[a];
	}
	new_line[j] = '\0';

	free(*line);
	*line = new_line;
}

/**
 * get_new_len - Gets the new length of a line partitioned
 *               by ";", "||", "&&&", or "#".
 * @line: a line to check.
 *
 * Return: a new length of the line.
 *
 * Description: Cuts short line contain '#' comments with '\0'.
 */

ssize_t get_new_len(char *line)
{
	size_t a;
	ssize_t new_len = 0;
	char current, next;

	for (a = 0; line[a]; a++)
	{
		current = line[a];
		next = line[a + 1];
		if (current == '#')
		{
			if (a == 0 || line[a - 1] == ' ')
			{
				line[a] = '\0';
				break;
			}
		}
		else if (a != 0)
		{
			if (current == ';')
			{
				if (next == ';' && line[a - 1] != ' ' && line[a - 1] != ';')
				{
					new_len += 2;
					continue;
				}
				else if (line[a - 1] == ';' && next != ' ')
				{
					new_len += 2;
					continue;
				}
				if (line[a - 1] != ' ')
					new_len++;
				if (next != ' ')
					new_len++;
			}
			else
				logical_ops(&line[a], &new_len);
		}
		else if (current == ';')
		{
			if (a != 0 && line[a - 1] != ' ')
				new_len++;
			if (next != ' ' && next != ';')
				new_len++;
		}
		new_len++;
	}
	return (new_len);
}
/**
 * logical_ops - checks a line for logical operators "||" or "&&".
 * @line: A pointers to the char to check in the line.
 * @new_len: pointers to new_len in get_new_len func
 */
void logical_ops(char *line, ssize_t *new_len)
{
	char previous, current, next;

	previous = *(line - 1);
	current = *line;
	next = *(line + 1);

	if (current == '&')
	{
		if (next == '&' && previous != ' ')
			(*new_len)++;
		else if (previous == '&' && next != ' ')
			(*new_len)++;
	}
	else if (current == '|')
	{
		if (next == '|' && previous != ' ')
			(*new_len)++;
		else if (previous == '|' && next != ' ')
			(*new_len)++;
	}
}
