#include "s21_grep.h"

#include <getopt.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage() {
  fprintf(stderr,
          "usage: ./s21_grep [-chilnosv] [-e pattern] [-f file with patterns] "
          "pattern file\n");
}

void append_templates_from_file(FILE* file, Templates* templates) {
  char* template = NULL;
  size_t len = 0;
  while (getline(&template, &len, file) != -1) {
    append_to_string_vector(templates, template);
    if (!(strlen(template) == 1 && template[0] == '\n')) {
      template[strcspn(template, "\n")] = '\0';
    }
    template = NULL;
  }
  free(template);
  template = NULL;
  fclose(file);
}

bool append_to_string_vector(String_vector* strings_vector, char* string) {
  bool err_flag = false;
  char** old_pointer = strings_vector->strings;
  strings_vector->strings =
      realloc(strings_vector->strings,
              (strings_vector->strings_amount + 1) * sizeof(char*));
  if (strings_vector->strings == NULL) {
    free(old_pointer);
    fprintf(
        stderr,
        "s21_grep: Error while trying to append_to_string_vector. Probably out "
        "of memory.");
    err_flag = true;
  } else {
    strings_vector->strings[strings_vector->strings_amount] = string;
    strings_vector->strings_amount++;
  }
  return err_flag;
}

void print_strings(String_vector strings) {
  for (size_t string_num = 0; string_num < strings.strings_amount;
       string_num++) {
    printf("%s\n", strings.strings[string_num]);
  }
}

bool get_options(Options* options, Templates* templates, Filenames* filenames,
                 int argc, char* argv[]) {
  const struct option long_options[] = {{0, 0, 0, 0}};
  int option_index;
  bool err_flag = false;
  bool ef_appeared = false;
  int opt =
      getopt_long(argc, argv, "chif:e:lnosv", long_options, &option_index);
  if (opt == 'e' || opt == 'f') {
    ef_appeared = true;
  }
  while (opt != -1 && !err_flag) {
    err_flag = set_option(opt, optarg, options, templates);
    opt = getopt_long(argc, argv, "chif:e:lnosv", long_options, &option_index);
    if (opt == 'e' || opt == 'f') {
      ef_appeared = true;
    }
  }

  if (optind < argc && !err_flag) {
    if (!ef_appeared) {
      char* template_value = calloc(strlen(argv[optind]) + 1, sizeof(char));
      strcpy(template_value, argv[optind]);
      err_flag = append_to_string_vector(templates, template_value);
      optind++;
    }
    while (optind < argc) {
      char* file_value = calloc(strlen(argv[optind]) + 1, sizeof(char));
      strcpy(file_value, argv[optind]);
      err_flag = append_to_string_vector(filenames, file_value);
      optind++;
    }
  }
  return err_flag;
}

bool set_option(int opt, char* optarg, Options* options, Templates* templates) {
  bool err_flag = false;
  char* e_value = NULL;
  FILE* f_value = NULL;
  switch (opt) {
    case 'i':
      options->ignore_case = true;
      break;
    case 'v':
      options->invert_match = true;
      options->only_matching = false;
      break;
    case 'c':
      options->count = true;
      options->line_number = false;
      options->only_matching = false;
      break;
    case 'l':
      options->files_with_matches = true;
      options->line_number = false;
      break;
    case 'n':
      if (!options->files_with_matches && !options->count) {
        options->line_number = true;
      }
      break;
    case 'h':
      options->no_filename = true;
      break;
    case 's':
      options->no_messages = true;
      break;
    case 'o':
      if (!options->count && !options->invert_match) {
        options->only_matching = true;
      }
      break;
    case 'e':
      e_value = calloc(strlen(optarg) + 1, sizeof(char));
      strcpy(e_value, optarg);
      err_flag = append_to_string_vector(templates, e_value);
      break;
    case 'f':
      f_value = fopen(optarg, "r");
      if (f_value) {
        append_templates_from_file(f_value, templates);
      } else {
        err_flag = true;
        fprintf(stderr, "s21_grep: %s: No such file or directory\n", optarg);
      }
      break;
    default:
      if (!options->no_messages) {
        usage();
      }
      err_flag = true;
      break;
  }
  return err_flag;
}

void destroy_string_vector(String_vector* string_vector) {
  for (size_t i = 0; i < string_vector->strings_amount; i++) {
    free(string_vector->strings[i]);
  }
  free(string_vector->strings);
  free(string_vector);
}

void grep(Filenames filenames, Options options, Templates templates) {
  Regex_vector* regexs = get_regexs(templates, options.ignore_case);
  bool is_stdin = filenames.strings_amount ? false : true;
  if (templates.strings_amount == 1 && regexs->vector_size == 0) {
    free(regexs->regexs);
    free(regexs);
    fprintf(stderr, "s21_grep: template error\n");
  } else {
    for (size_t filenum = 0; filenum < filenames.strings_amount || is_stdin;
         filenum++) {
      FILE* file;
      char* filename;
      if (is_stdin) {
        is_stdin = false;
        file = stdin;
        filename = "(standart input)";
      } else {
        filename = filenames.strings[filenum];
        file = fopen(filename, "r");
      }
      if (file != NULL) {
        if (options.files_with_matches) {
          bool is_match = is_match_in_file(file, regexs, options);
          print_files_with_matching(is_match, options, filename,
                                    filenames.strings_amount);
        } else if (options.count) {
          size_t line_counter = count_strings(file, regexs, options);
          print_counting_results(line_counter, filename,
                                 filenames.strings_amount, options);
        } else {
          if (options.only_matching) {
            print_only_matches(filenames.strings_amount, file, regexs, options,
                               filename);
          } else {
            print_searching_results(filenames.strings_amount, file, regexs,
                                    options, filename);
          }
        }
        fclose(file);
      } else if (!options.no_messages) {
        fprintf(stderr, "s21_grep: %s: No such file or directory\n",
                filenames.strings[filenum]);
      }
    }
    if (regexs && regexs->vector_size != 0) {
      destroy_regexs(regexs);
    }
  }
}

char* is_there_match(FILE* file, Regex_vector regexs, bool* eof,
                     bool* is_match) {
  size_t len = 0;
  char* string_for_searching = NULL;
  ssize_t read = getline(&string_for_searching, &len, file);
  *is_match = false;
  if (read != -1) {
    regmatch_t pmatch[1];
    for (size_t i = 0; i < regexs.vector_size && !(*is_match); i++) {
      *is_match = !regexec(&(regexs.regexs[i]), string_for_searching,
                           ARRAY_SIZE(pmatch), pmatch, 0);
    }
  } else {
    free(string_for_searching);
    string_for_searching = NULL;
    *eof = true;
  }
  return string_for_searching;
}

Regex_vector* get_regexs(Templates templates, bool ignore_case) {
  Regex_vector* regexs = calloc(1, sizeof(Regex_vector));
  regexs->regexs = calloc(templates.strings_amount, sizeof(regex_t));
  bool err_flag = false;
  regexs->vector_size = 0;
  for (size_t i = 0; i < templates.strings_amount && !err_flag; i++) {
    err_flag =
        regcomp(regexs->regexs + i, templates.strings[regexs->vector_size],
                ignore_case ? REG_EXTENDED | REG_ICASE | REG_NEWLINE
                            : REG_EXTENDED | REG_NEWLINE);
    if (!err_flag) {
      regexs->vector_size++;
    }
  }
  return regexs;
}

void destroy_regexs(Regex_vector* regexs) {
  for (size_t i = 0; i < regexs->vector_size; i++) {
    regfree(&(regexs->regexs[i]));
  }
  free(regexs->regexs);
  free(regexs);
}

void print_files_with_matching(bool is_match, Options options, char* filename,
                               size_t filenum) {
  if (is_match) {
    if (options.count && filenum > 1) {
      if (options.no_filename) {
        printf("1\n%s\n", filename);
      } else {
        printf("%s:1\n%s\n", filename, filename);
      }
    } else if (options.count && filenum == 1) {
      printf("1\n%s\n", filename);
    } else {
      printf("%s\n", filename);
    }
  } else {
    if (options.count && filenum > 1 && !options.no_filename) {
      printf("%s:0\n", filename);
    } else if (options.count) {
      printf("0\n");
    }
  }
}

void print_searching_results(size_t filenum, FILE* file, Regex_vector* regexs,
                             Options options, char* filename) {
  bool eof = false;
  size_t line_number = 0;
  bool is_match = false;
  while (!eof) {
    char* string_for_searching = is_there_match(file, *regexs, &eof, &is_match);
    line_number++;
    if (options.invert_match) {
      is_match = !is_match;
    }
    if (is_match && string_for_searching) {
      if (!options.no_filename && filenum > 1) {
        printf("%s:", filename);
      }
      if (options.line_number) {
        printf("%lu:", line_number);
      }
      printf("%s", string_for_searching);
      if (string_for_searching[strlen(string_for_searching) - 1] != '\n') {
        printf("\n");
      }
    }
    free(string_for_searching);
  }
}

size_t count_strings(FILE* file, Regex_vector* regexs, Options options) {
  bool eof = false;
  bool is_match = false;
  size_t line_counter = 0;
  while (!eof) {
    char* string_for_searching = is_there_match(file, *regexs, &eof, &is_match);
    free(string_for_searching);
    if (options.invert_match) {
      is_match = !is_match;
    }
    if (is_match) {
      line_counter++;
    }
  }
  if (options.invert_match) {
    line_counter--;
  }
  return line_counter;
}

bool is_match_in_file(FILE* file, Regex_vector* regexs, Options options) {
  bool eof = false;
  bool is_match = false;
  while (!eof && !is_match) {
    char* string_for_searching = is_there_match(file, *regexs, &eof, &is_match);
    if (options.invert_match && !eof) {
      is_match = !is_match;
    }
    free(string_for_searching);
  }
  return is_match;
}

void print_counting_results(size_t line_counter, char* filename, size_t filenum,
                            Options options) {
  if (options.no_filename || filenum == 1 || filenum == 0) {
    printf("%lu\n", line_counter);
  } else {
    printf("%s:%lu\n", filename, line_counter);
  }
}

void print_only_matches(size_t filenum, FILE* file, Regex_vector* regexs,
                        Options options, char* filename) {
  size_t line_number = 0;
  bool is_match = false;
  size_t len = 0;
  char* string_for_searching = NULL;
  ssize_t read = getline(&string_for_searching, &len, file);
  while (read != EOF) {
    String_vector* matches =
        get_all_matches_from_line(string_for_searching, *regexs);
    is_match = matches->strings_amount ? true : false;
    free(string_for_searching);
    string_for_searching = NULL;
    len = 0;
    line_number++;
    if (is_match) {
      if (!options.no_filename && filenum > 1) {
        printf("%s:", filename);
      }
      if (options.line_number) {
        printf("%lu:", line_number);
      }
      print_strings(*matches);
    }
    read = getline(&string_for_searching, &len, file);
    destroy_string_vector(matches);
  }
  free(string_for_searching);
}

String_vector* get_all_matches_from_line(char* string_for_searching,
                                         Regex_vector regexs) {
  char* moving_pointer = string_for_searching;
  String_vector* matches = calloc(1, sizeof(String_vector));
  matches->strings_amount = 0;
  matches->strings = NULL;
  bool is_match = true;
  regmatch_t pmatch[1];
  for (size_t i = 0; i < regexs.vector_size; i++) {
    while (is_match) {
      is_match = !regexec(&(regexs.regexs[i]), moving_pointer,
                          ARRAY_SIZE(pmatch), pmatch, 0);
      if (is_match) {
        size_t len = pmatch[0].rm_eo - pmatch[0].rm_so;
        char* new_match = calloc(len + 1, sizeof(char));
        sprintf(new_match, "%.*s", (int)len, moving_pointer + pmatch[0].rm_so);
        append_to_string_vector(matches, new_match);
        moving_pointer += pmatch[0].rm_eo;
      }
    }
    is_match = true;
  }

  return matches;
}
