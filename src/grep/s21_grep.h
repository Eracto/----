#ifndef SRC_GREP_GREP_H_
#define SRC_GREP_GREP_H_

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct String_vector {
  size_t strings_amount;
  char** strings;
} String_vector;

typedef String_vector Templates;
typedef String_vector Filenames;

typedef struct Regex_vector {
  size_t vector_size;
  regex_t* regexs;
} Regex_vector;

typedef struct Options {
  bool ignore_case;         // -i
  bool invert_match;        // -v
  bool count;               // -c
  bool files_with_matches;  // -l
  bool line_number;         // -n
  bool no_filename;         // -h
  bool no_messages;         // -s
  bool only_matching;       // -o
} Options;

void usage();
void append_templates_from_file(FILE* file, Templates* templates);
bool append_to_string_vector(String_vector* string_vector, char* string);
void print_strings(String_vector strings);
bool get_options(Options* options, Templates* templates, Filenames* filenames,
                 int argc, char* argv[]);
bool set_option(int opt, char* optarg, Options* options, Templates* template);
void destroy_string_vector(String_vector* string_vector);
void grep(Filenames filenames, Options options, Templates templates);
char* is_there_match(FILE* file, Regex_vector regexs, bool* eof,
                     bool* is_match);
Regex_vector* get_regexs(Templates templates, bool ignore_case);
void destroy_regexs(Regex_vector* regexs);
void print_files_with_matching(bool is_match, Options options, char* filename,
                               size_t filenum);
void print_searching_results(size_t filenum, FILE* file, Regex_vector* regexs,
                             Options options, char* filename);
size_t count_strings(FILE* file, Regex_vector* regexs, Options options);
void print_counting_results(size_t line_counter, char* filename, size_t filenum,
                            Options options);
bool is_match_in_file(FILE* file, Regex_vector* regexs, Options options);
void print_only_matches(size_t filenum, FILE* file, Regex_vector* regexs,
                        Options options, char* filename);
String_vector* get_all_matches_from_line(char* string_for_searching,
                                         Regex_vector regexs);
#endif  // SRC_GREP_GREP_H_
