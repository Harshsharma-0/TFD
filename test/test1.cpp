#include <bitset>
#include <iostream>
#define TFD_ENABLE_ON_LINUX
#include "../tfd.hpp"

#define TFD_PREFIX "[TFD TEST 1] "
/* function to test init on linux and windows the init code */
int test_init_enabled() {
  tfd tfd;

  std::cout << TFD_PREFIX
            << " checking the uninitlised exit code for tfd::exit() ";
  if (tfd.exit() != -1)
    return -1;

  std::cout << " [PASSED] " << std::endl;
  std::cout << TFD_PREFIX << " checking with wrong parameter -1,-1 ";

  if (tfd.init(-1, -1) != -1) {
    return -1;
  }

  std::cout << " [PASSED] " << std::endl;
  std::cout << TFD_PREFIX << "[checking if the min value passes or not] ";

  if (tfd.init(TFD_MIN_BUCKET, TFD_MIN_BUCKET_SIZE) != 0) {
    return -1;
  }
  std::cout << " [PASSED] " << std::endl;

  std::cout << TFD_PREFIX << "[checking the exit code] ";

  if (tfd.exit() != 0) {
    return -1;
  }
  std::cout << " [PASSED] " << std::endl;
  return 0;
};

// basic test file
int main() {
  if (test_init_enabled() != 0) {
    std::cout << " [FAILED] " << std::endl;
    return -1;
  }
  return 0;
};
