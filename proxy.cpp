#include "proxy.h"

#include <pthread.h>

#include "client_info.h"

#include "proxy_function.h"

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


//?copy
std::string sendContentLen(int send_fd,char * server_msg,int mes_len,int content_len) {
  int total_len = 0;
  int len = 0;
  std::string msg(server_msg, mes_len);

  while (total_len < content_len) {
    char new_server_msg[65536] = {0};
    if ((len = recv(send_fd, new_server_msg, sizeof(new_server_msg), 0)) <= 0) {
      break;
    }
    std::string temp(new_server_msg, len);
    msg += temp;
    total_len += len;
  }
  return msg;
}

//?copy
void httpPost(int client_fd,int server_fd,char * req_msg,int len,int id,const char * host) {
  int context_len = getContextLength(req_msg, len);  //get length of client request
  if (context_len != -1) {
    std::string request = sendContentLen(client_fd, req_msg, len, context_len);
    char send_request[request.length() + 1];
    strcpy(send_request, request.c_str());
    send(server_fd,
         send_request,
         sizeof(send_request),
         MSG_NOSIGNAL);  // send all the request info from client to server
    char response[BUFFER_LEN] = {0};
    int response_len = recv(server_fd,
                            response,
                            sizeof(response),
                            MSG_WAITALL);  //first time received response from server
    /*if (response_len != 0) {
      Response res;
      res.ParseLine(req_msg, len);
      pthread_mutex_lock(&mutex);
      logFile << id << ": Received \"" << res.getLine() << "\" from " << host
              << std::endl;
      pthread_mutex_unlock(&mutex);

      std::cout << "receive response from server which is:" << response << std::endl;

      send(client_fd, response, response_len, MSG_NOSIGNAL);

      pthread_mutex_lock(&mutex);
      logFile << id << ": Responding \"" << res.getLine() << std::endl;
      pthread_mutex_unlock(&mutex);
    }
    else {
      std::cout << "server socket closed!\n";
    }*/
  }
}

void * handle(void * info) {
  Client_Info * client_info = (Client_Info *)info;
  int client_fd = client_info->getFd();
  char buffer[BUFFER_LEN] = {0};
  ssize_t bytes_received=recv(client_fd, buffer, sizeof(buffer), 0);
  if (bytes_received<0){
    return NULL;
  }
  std::string buffer_s(buffer);
  //std::cout<<buffer_s<<std::endl;//?
  if (buffer_s == ""||buffer_s == "\r" || buffer_s == "\n" || buffer_s == "\r\n"){
    return NULL;
  }
  //parse http request
  Request request=Request(buffer_s);
  const char * host = request.host.c_str();
  const char * port = request.port.c_str();
  int server_fd = build_client(host,port);

  //connect
  if (request.method=="CONNECT"){
    httpConnect(client_fd, server_fd);
  }
  //post
  else if (request.method=="POST"){
    //todo
        
        ssize_t server_send = send(server_fd, buffer, sizeof(buffer), MSG_NOSIGNAL); 
        if (server_send<0){
          std::cerr << "Error: cannot send to server" << std::endl;
          return NULL;
        }
        char buffer_from_sever[BUFFER_LEN] = {0};
        ssize_t server_received = recv(server_fd, buffer_from_sever, sizeof(buffer_from_sever), 0);
        if (server_received<0){
          std::cerr << "Error: cannot received from server" << std::endl;
          return NULL;
        }
        ssize_t send_client = send(client_fd, buffer_from_sever, sizeof(buffer_from_sever), MSG_NOSIGNAL); 
        if (send_client<0){
          std::cerr << "Error: cannot received from server" << std::endl;
          return NULL;
        }
    //std::cout<<"POST"<<std::endl;
    //httpPost(client_fd, server_fd, buffer, bytes_received, client_info->getID(), host);
  }
  //get
  else if (request.method=="GET"){
    //todo
        ssize_t server_send = send(server_fd, buffer, sizeof(buffer), MSG_NOSIGNAL); 
        if (server_send<0){
          std::cerr << "Error: cannot send to server" << std::endl;
          return NULL;
        }
        char buffer_from_sever[BUFFER_LEN] = {0};
        ssize_t server_received = recv(server_fd, buffer_from_sever, sizeof(buffer_from_sever), 0);
        if (server_received<0){
          std::cerr << "Error: cannot received from server" << std::endl;
          return NULL;
        }
        ssize_t send_client = send(client_fd, buffer_from_sever, sizeof(buffer_from_sever), MSG_NOSIGNAL);
        if (send_client<0){
          std::cerr << "Error: cannot received from server" << std::endl;
          return NULL;
        } 
    //std::cout<<"get"<<std::endl;

  }
  //none
  else {
    return NULL;
  }


  close(server_fd);
  close(client_fd);

  return NULL;
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





  /*int server_fd_init = build_client("youtube.com", "443");
  char mes_buf[8192] = {0};

  if (send(server_fd_init, buffer, sizeof(buffer), MSG_NOSIGNAL) == 0) {
    std::cout << "buffer send to server is 0\n";
  }
  if (recv(server_fd_init, mes_buf, sizeof(mes_buf), 0) == 0) {
    std::cout << "before while loop closed\n";
  }*/