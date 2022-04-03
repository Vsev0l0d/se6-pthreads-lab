#include <iostream>
#include <string>
#include "producer_consumer.h"

int main(int argc, char *argv[]) {
  if (argc == 3) {
    std::cout << run_threads(atoi(argv[1]), atoi(argv[2])) << std::endl;
  } else if (argc == 4) {
    std::cout << run_threads(atoi(argv[1]), atoi(argv[2]), true) << std::endl;
  } else
    return 1;
  return 0;
}
