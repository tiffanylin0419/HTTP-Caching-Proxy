#include "proxy.h"

#include <pthread.h>

#include "client_info.h"
#include "function.h"
#include "request.h"
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#define RESPONSE_LEN 20
#define BUFFER_LEN 50000

void httpConnect(int client_fd, int server_fd){
  const char* response="HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response,RESPONSE_LEN, 0);
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);
    if(select(nfds, &readfds, NULL, NULL, NULL)<=0){
      continue;
    }
    int fd[2] = {server_fd, client_fd};

    for (int i = 0; i < 2; i++) {
      char buffer[BUFFER_LEN] = {0};//add
      if (FD_ISSET(fd[i], &readfds)) {
        ssize_t bytes_received = recv(fd[i], buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
          return;
        }
        send(fd[1 - i], buffer,bytes_received, 0) ;
      }
    }
  }
}


void proxy::run() {
  int temp_fd = build_server(this->port_num);
  if (temp_fd == -1) {
    return;
  }
  int client_fd;
  int id = 0;
  while (1) {
    std::string ip;
    client_fd = server_accept(temp_fd);
    if (client_fd == -1) {
      continue;
    }
    pthread_t thread;
    pthread_mutex_lock(&mutex1);
    Client_Info * client_info = new Client_Info();
    client_info->setFd(client_fd);
    client_info->setIP(ip);
    client_info->setID(id);
    id++;
    pthread_mutex_unlock(&mutex1);
    pthread_create(&thread, NULL, handle, client_info);
  }
}

void * proxy::handle(void * info) {
  Client_Info * client_info = (Client_Info *)info;
  int client_fd = client_info->getFd();
  char buffer[BUFFER_LEN] = {0};
  if (recv(client_fd, buffer, sizeof(buffer), 0)<0){
    return NULL;
  }
  std::string buffer_s(buffer);
  std::cout<<buffer<<std::endl;
  if (buffer_s == ""||buffer_s == "\r" || buffer_s == "\n" || buffer_s == "\r\n"){
    return NULL;
  }
  //parse http request
  Request request=Request(buffer_s);
  int server_fd = build_client(request.host.c_str(), request.port.c_str());

  //connect
  if (request.method=="CONNECT"){
    httpConnect(client_fd, server_fd);
  }
  //else
  else if (request.method != "POST" && request.method != "GET" && request.method != "CONNECT") {
    return NULL;
  }


  close(server_fd);
  close(client_fd);

  return NULL;
}



  /*int server_fd_init = build_client("youtube.com", "443");
  char mes_buf[8192] = {0};

  if (send(server_fd_init, buffer, sizeof(buffer), MSG_NOSIGNAL) == 0) {
    std::cout << "buffer send to server is 0\n";
  }
  if (recv(server_fd_init, mes_buf, sizeof(mes_buf), 0) == 0) {
    std::cout << "before while loop closed\n";
  }*/