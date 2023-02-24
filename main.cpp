#include "function.h"
#include "proxy.h"

#include <pthread.h>



int main() {  
  //proxy
  const char * port = "3333";
  proxy * myproxy = new proxy(port);
  myproxy->run();
  return 1;
}
