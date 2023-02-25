#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class ClientData {

public:
  int id;
  int client_fd;
    //原本是ip
    std::string ip_address;

};
