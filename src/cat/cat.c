#include "cat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Options *get_options(int argc, char *argv[]) {
  Options *options = calloc(1, sizeof(Options));
  for (size_t n = 1; n < (size_t)argc && options != NULL; n++) {
    if (argv[n][0] == '-') {
      bool err = parse_option(argv[n], options);
      if (err != false) {
        free(options);
        options = NULL;
      }
    }
  }
  return options;
}

Size_t_vector *get_paths_positions(int argc, char *argv[]) {
  size_t paths_number = 0;
  for (size_t n = 1; n < (size_t)argc; n++) {
    if (argv[n][0] != '-') {
      paths_number++;
    }
  }
  Size_t_vector *paths_positions;
  if (paths_number != 0) {
    paths_positions = calloc(1, sizeof(Size_t_vector));
    paths_positions->array = calloc(1, sizeof(size_t) * paths_number);
    size_t i = 0;
    for (size_t n = 1; n < (size_t)argc; n++) {
      if (argv[n][0] != '-') {
        paths_positions->array[i] = n;
        i++;
      }
    }
    paths_positions->vector_size = i;
  } else {
    paths_positions = NULL;
  }
  return paths_positions;
}

bool is_wide(char *option) { return option[0] == '-' && option[1] == '-'; }

bool compare_wide_options(char *option1, char *option2) {
  return strlen(option1) == strlen(option2) &&
         !memcmp(option1, option2, strlen(option2));
}

bool set_short_option(char option_letter, Options *options) {
  bool err_flag = false;
  switch (option_letter) {
    case 'b':
      options->number_nonblank = true;
      options->number_all_lines = false;
      break;
    case 'E':
      options->show_EOL_symbols = true;
      break;
    case 'e':
      options->show_EOL_symbols = true;
      options->show_non_printing_symbols = true;
      break;
    case 'v':
      options->show_non_printing_symbols = true;
      break;
    case 'n':
      if (options->number_nonblank) {
        options->number_all_lines = false;
      } else {
        options->number_all_lines = true;
      }
      break;
    case 's':
      options->squeeze_blank = true;
      break;
    case 'T':
      options->show_tab_symbols = true;
      break;
    case 't':
      options->show_tab_symbols = true;
      options->show_non_printing_symbols = true;
      break;
    default:
      fprintf(stderr, "cat: illegal option %c%c\n", '-', option_letter);
      err_flag = true;
      break;
  }
  return err_flag;
}

bool set_wide_option(char *wide_option, Options *options) {
  bool err_flag = false;
  if (compare_wide_options(wide_option, "--number-nonblank")) {
    options->number_nonblank = true;
    options->number_all_lines = false;
  } else if (compare_wide_options(wide_option, "--squeeze-blank")) {
    options->squeeze_blank = true;
  } else if (compare_wide_options(wide_option, "--number")) {
    if (!options->number_nonblank) {
      options->number_all_lines = true;
    }
  } else {
    fprintf(stderr, "cat: illegal option %s\n", wide_option);
    err_flag = true;
  }
  return err_flag;
}

bool parse_option(char *option, Options *options) {
  bool err_flag = false;
  if (is_wide(option)) {
    err_flag = set_wide_option(option, options);
  } else {
    size_t i = 1;
    char option_letter = option[i];
    while (option_letter != '\0' && !err_flag) {
      err_flag = set_short_option(option_letter, options);
      i++;
      option_letter = option[i];
    }
  }
  return err_flag;
}

void print_options(Options *options) {
  printf("number_nonblank = %d\n", options->number_nonblank);
  printf("show_non_printing_symbols = %d\n",
         options->show_non_printing_symbols);
  printf("show_EOL_symbols = %d\n", options->show_EOL_symbols);
  printf("number_all_lines = %d\n", options->number_all_lines);
  printf("squeeze_blank = %d\n", options->squeeze_blank);
  printf("show_tab_symbols = %d\n", options->show_tab_symbols);
}

void transform_nonprinting_symbols(char symbol) {
  unsigned char usymbol = (unsigned char)symbol;
  if (usymbol < 0x20 && symbol != '\n' && symbol != 9) {
    printf("^%c", usymbol + '@');
  } else if (usymbol >= 0x20 && usymbol < 0x7f) {
    printf("%c", usymbol);
  } else if (usymbol == 0x7f) {
    printf("^?");
  } else if (usymbol > 0x7f && usymbol < 0xa0) {
    printf("M-^%c", usymbol + '@');
  } else if (usymbol >= 0xa0 && usymbol < 0xff) {
    printf("M-%c", usymbol + ' ');
  } else if (usymbol == 0xff) {
    printf("M-^?");
  } else {
    printf("%c", symbol);
  }
}

bool print_file(Options *options, char *filename, FILE *file) {
  bool err_flag = false;
  if (file != NULL) {
    char previous_symbol = ' ';
    char symbol = fgetc(file);

    size_t blank_line_counter = 0;
    size_t line_counter = 1;
    while (symbol != EOF) {
      if (symbol == '\n') {
        blank_line_counter++;
      }
      if (symbol != '\n') {
        blank_line_counter = 0;
      }

      if (options->squeeze_blank) {
        if (blank_line_counter >= 3) {
          previous_symbol = symbol;
          symbol = fgetc(file);
          continue;
        }
      }

      if (options->number_all_lines) {
        if (previous_symbol == '\n' || line_counter == 1) {
          printf("%6lu%c", line_counter, 9);
          line_counter++;
        }
      }
      if (options->number_nonblank) {
        if ((previous_symbol == '\n' && symbol != '\n') ||
            (line_counter == 1 && symbol != '\n')) {
          printf("%6lu%c", line_counter, 9);
          line_counter++;
        }
      }

      if (options->show_EOL_symbols) {
        if (symbol == '\n') {
          printf("$");
        }
      }
      if (options->show_tab_symbols) {
        if (symbol == 9) {
          printf("^I");
          previous_symbol = symbol;
          symbol = fgetc(file);
          continue;
        }
      }
      if (options->show_non_printing_symbols) {
        transform_nonprinting_symbols(symbol);
      } else {
        printf("%c", symbol);
      }
      previous_symbol = symbol;
      symbol = fgetc(file);
    }
    fclose(file);
  } else {
    fprintf(stderr, "cat: %s: No such file or directory\n", filename);
    err_flag = true;
  }
  return err_flag;
}

bool cat(Options *options, Size_t_vector *paths_positions, char *argv[]) {
  bool err_flag = false;
  if (paths_positions != NULL && options != NULL) {
    for (size_t n = 0; n < paths_positions->vector_size; n++) {
      FILE *file = fopen(argv[paths_positions->array[n]], "r");
      err_flag = print_file(options, argv[paths_positions->array[n]], file);
    }
  } else {
    FILE *file = stdin;
    print_file(options, "stdin", file);
  }
  free(options);
  free(paths_positions->array);
  free(paths_positions);
  return err_flag;
}
