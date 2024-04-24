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
	assert(fp != NULL);
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

void append_lines(lines *lines, int at_line) {
	char *line = NULL;
	size_t line_length = 0;
	int nread;
	while (-1 != (nread = getline(&line, &line_length, stdin))) {
		if (0 == strcmp(".\n", line)) {
			return;
		}
		lines->length += 1;
		if (lines->length > lines->capacity) {
			lines->capacity *= 2;
			lines->lines = realloc(lines->lines, lines->capacity * sizeof(*lines->lines));
		}
		memmove(lines->lines+at_line+2, lines->lines+at_line+1, ((lines->length - 1) - at_line)*sizeof(*lines->lines));
		lines->lines[at_line+1].str = line;
		lines->lines[at_line+1].len = nread;
		lines->lines[at_line+1].cap = line_length;
		at_line++;
		line = NULL;
		line_length = 0;
	}
}


int main() {
	lines lines;
	printf("%d\n", read_file("./test", &lines));
	char *command = NULL;
	size_t command_len = 0;
	
	int current_line = 1;
	while (-1 != getline(&command, &command_len, stdin)) {
		struct range range = get_range(&command, lines, current_line);
		if (range.start < 0 || range.end >= lines.length) {
			puts("?");
			continue;
		}
		switch (*command) {
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

			case 'a':
				append_lines(&lines, range.start);
				break;
			case 'w':
				write_file("./test", lines);
				break;
			case 'q':
				free_lines(lines);
				return 0;
				break;
		}
	}
	return 0;
}
