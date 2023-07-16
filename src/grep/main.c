#include <regex.h>

#include "s21_grep.h"

int main(int argc, char* argv[]) {
  Options* options = calloc(1, sizeof(options));
  Templates* templates = calloc(1, sizeof(Templates));
  templates->strings_amount = 0;
  Filenames* filenames = calloc(1, sizeof(Filenames));
  filenames->strings_amount = 0;
  bool err_flag = get_options(options, templates, filenames, argc, argv);
  if (templates->strings_amount == 0) {
    usage();
    err_flag = true;
  }
  if (!err_flag) {
    grep(*filenames, *options, *templates);
  }
  destroy_string_vector(templates);
  destroy_string_vector(filenames);
  free(options);

  return 0;
}