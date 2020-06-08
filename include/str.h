#ifndef __STR_H
#define __STR_H

/**
 * Theses are the dependencies
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct Segment {
	const void *start_off;  /* Start Offset of a string/array_element/etc. */
	const void *end_off;    /* End Offset of a string/array_element/etc.   */
};


int 	str_ltrim(char *line);
int 	str_rtrim(char *line);
int 	str_trim(char *line);
int 	str_ltrims(char *line, char *tokens);
int 	str_rtrims(char *line, char *tokens);
bool 	str_find_next(const char *s, const char *pattern, struct Segment *seg);
char 	*str_replace(const char *s, const char *pattern, const char *substitue);

char 	*str_tolower(char *s);
char 	*str_toupper(char *s);
int 	str_split(char *in, char *delim, char **out, size_t ele);

#endif
