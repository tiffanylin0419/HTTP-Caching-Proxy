#include "proxy.h"

#include <pthread.h>

#include "client_info.h"
#include "function.h"
#include "request.h"
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void httpConnect(int client_fd, int server_fd){
  const char* response = "HTTP/1.1 200 OK\r\n";
  send(client_fd, response, strlen(response), 0);
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
      char message[50000] = {0};//add
      if (FD_ISSET(fd[i], &readfds)) {
        ssize_t bytes_received = recv(fd[i], message, sizeof(message), 0);
        if (bytes_received <= 0) {
          return;
        }
        else {
          if (send(fd[1 - i], message,bytes_received, 0) <= 0) {
            return;
          }
        }
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
  char req_msg[70000] = {0};
  if (recv(client_fd, req_msg, sizeof(req_msg), 0)<0){
    return NULL;
  }
  std::string req_msg_s(req_msg);
  std::cout<<req_msg<<std::endl;
  if (req_msg_s == ""||req_msg_s == "\r" || req_msg_s == "\n" || req_msg_s == "\r\n"){
    return NULL;
  }
  //parse http request
  Request request=Request(req_msg_s);
  int server_fd = build_client(request.host.c_str(), request.port.c_str());

  
  //connect
  if (request.method=="CONNECT"){
    httpConnect(client_fd, server_fd);
  }
  //?change to else
  else if (request.method != "POST" && request.method != "GET" && request.method != "CONNECT") {
    return NULL;
  }

  close(server_fd);
  close(client_fd);

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