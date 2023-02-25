#include "connect_func.h"
#include "proxy.h"

#include <pthread.h>

#define PORT "12345"

int main() {  
  const char * port = PORT;
  proxy * myproxy = new proxy(port);
  myproxy->run();
  return 1;
}
