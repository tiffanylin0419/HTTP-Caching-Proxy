#include "proxy.h"

#include <pthread.h>

#include "client_info.h"
#include "function.h"

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
  char req_msg[8192] = {0};
  recv(client_fd, req_msg, sizeof(req_msg), 0);
  //need parsing

  //cout<<req_msg;

  const char * hostname="google.com";
  send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  int server_fd = build_client(hostname, "443");
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;

  FD_ZERO(&readfds);
  while (1) {
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);
    select(nfds, &readfds, NULL, NULL, NULL);

    int fd[2] = {server_fd, client_fd};
    for (int i = 0; i < 2; i++) {
      if (FD_ISSET(fd[i], &readfds)) {
        int len;
        char message[8192] = {0};
        len = recv(fd[i], message, sizeof(message), 0);
        if (len == 0) {
          // std::cout << "Error: Receive length 0\n";
          break;
        }
        else {
          send(fd[1 - i], message, len, 0);
          // std::cout << "Error in Sending!\n" << fd[i] << std::endl;
        }
      }
    }
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