#include "proxy.h"

#include <pthread.h>

#include "client_info.h"
#include "function.h"
#include "request.h"

void httpConnect(int client_fd, int server_fd){
  send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);
    select(nfds, &readfds, NULL, NULL, NULL);

    int fd[2] = {server_fd, client_fd};
    int len;
    for (int i = 0; i < 2; i++) {
      char message[50000] = {0};//add
      if (FD_ISSET(fd[i], &readfds)) {
        len = recv(fd[i], message, sizeof(message), 0);
        if (len <= 0) {
          return;
        }
        else {
          if (send(fd[1 - i], message, len, 0) <= 0) {
            return;
          }
        }
      }
    }
  }
}

void proxy::run() {
  Client_Info * client_info = new Client_Info();
  int temp_fd = build_server(this->port_num);
  int client_fd;
  while (1) {
    client_fd = server_accept(temp_fd);
    pthread_t thread;
    client_info->setFd(client_fd);
    pthread_create(&thread, NULL, handle, client_info);
  }
}

void * proxy::handle(void * info) {
  Client_Info * client_info = (Client_Info *)info;
  int client_fd = client_info->getFd();
  char req_msg[50000] = {0};
  recv(client_fd, req_msg, sizeof(req_msg), 0);
  //parse request
  std::string req_msg_s(req_msg);
  if (req_msg_s == ""||req_msg_s == "\r" || req_msg_s == "\n" || req_msg_s == "\r\n"){
    return NULL;
  }
  Request request=Request(req_msg_s);
  int server_fd = build_client(request.host.c_str(), request.port.c_str());
  
  if (request.method=="CONNECT"){
    
    httpConnect(client_fd, server_fd);
  }
  

  return NULL;
}



  /*int server_fd_init = build_client("youtube.com", "443");
  char mes_buf[8192] = {0};

  if (send(server_fd_init, req_msg, sizeof(req_msg), MSG_NOSIGNAL) == 0) {
    std::cout << "Message send to server is 0\n";
  }
  if (recv(server_fd_init, mes_buf, sizeof(mes_buf), 0) == 0) {
    std::cout << "before while loop closed\n";
  }*/