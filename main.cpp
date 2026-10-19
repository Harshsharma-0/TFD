#include <iostream>
#include <bitset>
#define TFD_ENABLE_ON_LINUX
#include "./tfd.hpp"

int main(){

  tfd tfd;

  tfd.init(3,128);

  for(int i = 0; i < 128; i++){
        tfd.open(i);
   };

  for(int i = 0; i < 128 * 2 ; i++){
    std::cout<<"raw fd : "<<tfd.rawsocket(i)<<std::endl;
  };
   
 
  return 0;
};
