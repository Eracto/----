#include <stdio.h>

#include "cat.h"

int main(int argc, char* argv[]) {
  Options* options = get_options(argc, argv);
  Size_t_vector* paths_positions = get_paths_positions(argc, argv);
  if (options != NULL) {
    cat(options, paths_positions, argv);
  }
  return 0;
}
