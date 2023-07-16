#ifndef SRC_CAT_CAT_H_
#define SRC_CAT_CAT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Size_t_vector {
  size_t vector_size;
  size_t* array;
} Size_t_vector;

typedef struct Options {
  bool number_nonblank;            //-b
  bool show_non_printing_symbols;  //-v
  bool show_EOL_symbols;           //-E
  bool number_all_lines;           //-n
  bool squeeze_blank;              //-s
  bool show_tab_symbols;           //-T
} Options;

Options* get_options(int argc, char* argv[]);
Size_t_vector* get_paths_positions(int argc, char* argv[]);
bool is_wide(char* option);
bool parse_option(char* option, Options* options);
bool compare_wide_options(char* option1, char* option);
bool set_wide_option(char* wide_option, Options* options);
bool set_short_option(char option_letter, Options* options);
void transform_nonprinting_symbols(char symbol);
bool cat(Options* options, Size_t_vector* paths_positions, char* argv[]);
bool print_file(Options* options, char* filename, FILE* file);

void print_options(Options* options);

#endif  // SRC_CAT_CAT_H_
