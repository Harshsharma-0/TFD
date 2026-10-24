#include <bitset>
#include <iostream>
#define TFD_ENABLE_ON_LINUX
#include "./tfd.hpp"

int main() {

  tfd tfd;

  tfd.init(3, 128);
  tfd.enable_reusable();

  for (int i = 0; i < 128; i++) {
    tfd.open(i);
  };

  for (int i = 0; i < 128; i++)
    tfd.close(i);

  for (int i = 0; i < 18; i++) {
    tfd.open(i);
  };

  for (int i = 0; i < 128; i++) {
    std::cout << "raw fd : " << tfd.rawsocket(i) << std::endl;
  };

  return 0;
};
