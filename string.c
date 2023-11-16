#include "main.h"

int _strlen(const char *s);
char *_strcpy(char *dest, const char *src);
char *_strcat(char *dest, const char *src);
char *_strncat(char *dest, const char *src, size_t n);

/**
 * _strlen - Returns the length of a strings
 * @s: a pointers to the char strings
 *
 * Return: The length of the character string
 */
int _strlen(const char *s)
{
	int length = 0;

	if (!s)
		return (length);
	for (length = 0; s[length]; length++)
		;
	return (length);
}

/**
 * _strcpy - Copies the string pointed to by src, including the
 * terminating null byte, to the buffer pointed by dest
 * @dest: Pointer to the destination of copy string
 * @src: Pointer to the src of the source strings
 *
 * Return: Pointer to dest.
 */
char *_strcpy(char *dest, const char *src)
{
	size_t b;

	for (b = 0; src[b] != '\0'; b++)
		dest[b] = src[b];
	dest[b] = '\0';
	return (dest);
}

/**
 * _strcat - Concantenates two strings
 * @dest:A Pointers to destination stringS
 * @src: Pointers to source stringS
 *
 * Return: Pointer to destination string
 */
char *_strcat(char *dest, const char *src)
{
	char *destTemp;
	const char *srcTemp;

	destTemp = dest;
	srcTemp =  src;

	while (*destTemp != '\0')
		destTemp++;

	while (*srcTemp != '\0')
		*destTemp++ = *srcTemp++;
	*destTemp = '\0';
	return (dest);
}

/**
 * _strncat -a  Concantenates two strings where n number
 *  of bytes are copied from source
 * @dest: Pointers to destination strings
 * @src: Pointers to source strings
 * @n: n bytes to copy from src
 *
 * Return: Pointer to destination string
 */
char *_strncat(char *dest, const char *src, size_t n)
{
	size_t dest_len = _strlen(dest);
	size_t b;

	for (b = 0; b < n && src[b] != '\0'; b++)
		dest[dest_len + b] = src[b];
	dest[dest_len + b] = '\0';

	return (dest);
}
