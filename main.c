#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *str;
	size_t cap;
	size_t len;
} line;

typedef struct {
	line *lines;
	int capacity;
	int length;
} lines;

struct range {
	int start;
	int end;
};

void write_file(char *file, lines lines) {
	FILE *fp = fopen(file, "w");
	for (int i = 0; i < lines.length; i++) {
		fwrite(lines.lines[i].str, sizeof(*lines.lines[i].str), lines.lines[i].len, fp);
	}
}

int read_file(char *file, lines *result) {
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		result->capacity = 200;
		result->length = 0;
		result->lines = calloc(result->capacity, sizeof(*result->lines));
		return 0;
	}
	result->capacity = 200;
	result->length = 0;
	result->lines = calloc(result->capacity, sizeof(*result->lines));
	int nread = 0;
	int get_result = 0;
	while ((get_result = getline(&result->lines[result->length].str, &result->lines[result->length].cap, fp)) != -1) {
		result->lines[result->length].len = get_result;
		result->length++;
		nread += get_result;
	}
	return nread;
}

void free_lines(lines lines) {
	for (int i = 0; i < lines.length; i++) {
		free(lines.lines[i].str);
	}
	free(lines.lines);
}

void print_line(lines lines, int n) {
		printf("%s", lines.lines[n-1].str);
}

void print_lines(lines lines, struct range range) {
	for (int i = range.start; i <= range.end; i++) {
		printf("%s", lines.lines[i].str);
	}
}

void print_lines_numbers(lines lines, struct range range) {
	for (int i = range.start; i <= range.end; i++) {
		printf("%d       %s", i+1, lines.lines[i].str);
	}
}


struct range get_range(char **command, lines lines, int current_line) {
	if (**command == ',') {
		(*command)++;
		return (struct range) { .start = 0, .end = lines.length - 1 };
	}
	struct range result;
	if (!isdigit(**command)) {
		result.start = current_line-1;
		result.end = current_line-1;
		return result;
	}
	result.start = strtol(*command, command, 10);
	result.start--;
	if (**command != ',' ) {
		result.end = result.start;
		return result;
	}
	(*command)++;
	if (**command == '$') {
		result.end = lines.length - 1;
		(*command)++;
		return result;
	}
	if (!isdigit(**command)) {
		result.start = -1;
		return result;
	}
	result.end = strtol(*command, command, 10);
	result.end--;
	return result;
}


void delete_lines(lines *lines, struct range range) {
	assert(range.end >= range.start);
	for (int i = range.start; i <= range.end; i++) {
		free(lines->lines[i].str);
	}
	memmove(lines->lines+range.start, lines->lines+range.end+1, (lines->length-range.end+1)*sizeof(*lines->lines));
	lines->length -= (range.end+1)  - range.start;
}

void append_lines(lines *lines, int at_line) {
	char *line = NULL;
	size_t line_length = 0;
	int nread;
	while (-1 != (nread = getline(&line, &line_length, stdin))) {
		if (0 == strcmp(".\n", line)) {
			return;
		}
		if (lines->length == 0) {
			lines->length += 1;
			lines->lines[0].str = line;
			lines->lines[at_line].len = nread;
			lines->lines[at_line].cap = line_length;
			line = NULL;
			line_length = 0;
			continue;
		}
		lines->length += 1;
		if (lines->length > lines->capacity) {
			lines->capacity *= 2;
			lines->lines = realloc(lines->lines, lines->capacity * sizeof(*lines->lines));
		}
		memmove(lines->lines+at_line+2, lines->lines+at_line+1, ((lines->length - 1) - at_line)*sizeof(*lines->lines));
		at_line++;
		lines->lines[at_line].str = line;
		lines->lines[at_line].len = nread;
		lines->lines[at_line].cap = line_length;
		line = NULL;
		line_length = 0;
	}
}

void insert_lines(lines *lines, int at_line) {
	append_lines(lines, at_line-1);
}
char prompt = '*';
int prompting = 0;


__ssize_t my_getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __wur {

		if (prompting) {
			printf("%c", prompt);
		}
		return getline(__lineptr, __n, __stream);
}

int marks[256];

int main(int argc, char **argv) {
	lines lines;
	char filename[1024];
	if (argc == 2) {
		strcpy(filename, argv[1]);
	}
	printf("%d\n", read_file(filename, &lines));
	char *command = NULL;
	size_t command_len = 0;
	
	int current_line = 1;
	while (-1 != my_getline(&command, &command_len, stdin)) {
		struct range range = get_range(&command, lines, current_line);
		if (range.start < 0 || range.end > lines.length) {
			puts("?");
			puts("tehe");
			continue;
		}
		switch (*command) {
			case 'k':
				marks[*(++command)] = current_line;
				break;
			case '\'':
				current_line = marks[*(++command)];
				break;
			case '\n':
				current_line = range.start+1;
				break;
			case 'P':
				prompting = 1;
				command += 2;
				prompt = *command;
				break;
			case 'p':
				print_lines(lines, range);
				break;
			case 'n':
				print_lines_numbers(lines, range);
				break;
			case '-':
				current_line--;
				if (current_line < 1) {
					puts("?");
					current_line = 1;
					break;
				}
				print_line(lines, current_line);
				break;
			case '+':
				current_line++;
				if (current_line > lines.length) {
					puts("?");
					current_line = lines.length;
					break;
				}
				print_line(lines, current_line);
				break;
			case 'i':
				insert_lines(&lines, range.start);
				break;
			case 'a':
				append_lines(&lines, range.start);
				break;
			case 'c':
				delete_lines(&lines, range);
				insert_lines(&lines, range.start);
				break;
			case 'd':
				delete_lines(&lines, range);
				break;
			case 'e':
				free_lines(lines);
				command += 2;
				*strchr(command, '\n') = 0;
				strcpy(filename, command);
				printf("%d\n", read_file(filename, &lines));
				break;
			case 'w':
				if (*filename == 0) {
					puts("?");	
					continue;
				}
				write_file(filename, lines);
				if (command[1] != 'q') {
					break;
				}
			case 'q':
				free_lines(lines);
				return 0;
				break;
		}
	}
	return 0;
}
