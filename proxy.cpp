#include "proxy.h"

#include <pthread.h>

#include "client_info.h"

#include "proxy_function.h"

#include "function.h"
#include "request.h"
#include "response.h"


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#define RESPONSE_LEN 20
#define BUFFER_LEN 50000

std::map<std::string, Response> cache;


void send_client_cache_directly(int client_fd, Request request){
  std::string str=cache[request.line].input;
  char server_response[BUFFER_LEN] = {0};
  str.copy(server_response, str.size() + 1);
  server_response[str.size()] = '\0';
  ssize_t send_client = send(client_fd, server_response, str.size() , MSG_NOSIGNAL); 
  if (send_client<0){
    std::cerr << "Error: cannot received from server" << std::endl;
    return;
  }
}


//like post but return the http response
std::string request_directly(int client_fd, int server_fd,Request request){
  std::string str=request.input;
  char buffer1[BUFFER_LEN] = {0};
  str.copy(buffer1, str.size() + 1);
  ssize_t bytes_received=str.size();
  ssize_t server_send = send(server_fd, buffer1, bytes_received, MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: cannot send to server" << std::endl;
    return NULL;
  }
  char buffer2[BUFFER_LEN] = {0};
  bytes_received  = recv(server_fd, buffer2, sizeof(buffer2), 0);
  if (bytes_received <0){
    std::cerr << "Error: cannot received from server" << std::endl;
    return NULL;
  }
  ssize_t send_client = send(client_fd, buffer2, bytes_received , MSG_NOSIGNAL); 
  if (send_client<0){
    std::cerr << "Error: cannot received from server" << std::endl;
    return NULL;
  }
  std::string buffer2_s(buffer2);
  return buffer2_s;
}
/*

void revalidate(int server_fd,int client_fd, Request request, Response response){
  //check response value, append to request
  //todo: do not change request inputs
  if (response.etag != "") {
    std::string ifNoneMatch = "If-None-Match: " + response.etag ＋ "\r\n";
    std::string request.input = request.input + ifNoneMatch;
  }
  if (!response.last_modified_time.isEmpty()){
    std::string ifModifiedSince = "If-Modified-Since: " + response.last_modified_time.toString() ＋ "\r\n";
    std::string request.input = request.input + ifModifiedSince;
  }

  //send to server
  ssize_t server_send = send(server_fd, request.input, sizeof(request.input), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: cannot send to server" << std::endl;
    return NULL;
  }
  //recv from server
  char buffer[BUFFER_LEN] = {0};
  ssize_t bytes_received  = recv(server_fd, buffer, sizeof(buffer), 0);
  if (bytes_received <0){
    std::cerr << "Error: cannot received from server" << std::endl;
    return NULL;
  }
  std::string http_response(buffer, BUFFER_LEN);
  Response r=Response(http_response);
  std::string status_code = r.statusCode;

  //send to client
  if (status_code == "304" ){//same, 直接返還給客戶
    send_client_cache_directly(client_fd, request);
  }
  else{//change, 更新map
    ssize_t send_client = send(client_fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
    if (send_client<0){
    std::cerr << "Error: cannot received from server" << std::endl;
    return NULL;
    }
    std::string store_cache(buffer);
    cache_getrequest[getRequest->line] = getRequest;
    cache_response[getRequest->line] = store_cache;
  }
  
}*/




//todo: check recv,send return value
void httpConnect(int client_fd, int server_fd){
  const char* response="HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response,RESPONSE_LEN, 0);
  fd_set readfds;
  int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;//todo: change this line

  while (1) {

    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    FD_SET(client_fd, &readfds);
    if(select(nfds, &readfds, NULL, NULL, NULL)<=0){
      continue;
    }
    //send between server and client
    if (FD_ISSET(server_fd, &readfds)) {
      char buffer1[BUFFER_LEN] = {0};//add
      ssize_t bytes_received = recv(server_fd, buffer1, sizeof(buffer1), 0);
      if (bytes_received <= 0) {
        return;
      }
      buffer1[bytes_received]='\0';
      send(client_fd, buffer1,bytes_received, 0) ;
    }
    
    if (FD_ISSET(client_fd, &readfds)) {
      char buffer2[BUFFER_LEN] = {0};//add
      ssize_t bytes_received = recv(client_fd, buffer2, sizeof(buffer2), 0);
      if (bytes_received <= 0) {
        return;
      }
      buffer2[bytes_received]='\0';
      send(server_fd, buffer2,bytes_received, 0) ;
    }
  }
}



void httpPost(int client_fd, int server_fd,Request request){
  request_directly(client_fd, server_fd, request);
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
  //std::cout<<buffer<<std::endl;//?
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
    std::cout<<"connect"<<std::endl;
    httpConnect(client_fd, server_fd);
  }
  //post
  else if (request.method=="POST"){
    std::cout<<"post"<<std::endl;
    //todo
    httpPost(client_fd, server_fd,request);
  }
  //get
  else if (request.method=="GET"){
    //todo
    /*std::cout<<"get"<<std::endl;
    ssize_t server_send = send(server_fd, buffer, sizeof(buffer), MSG_NOSIGNAL); 
    if (server_send<0){
      std::cerr << "Error: cannot send to server" << std::endl;
      return NULL;
    }
    char buffer_from_sever[BUFFER_LEN] = {0};
    ssize_t bytes_received  = recv(server_fd, buffer_from_sever, sizeof(buffer_from_sever), 0);
    if (bytes_received <0){
      std::cerr << "Error: cannot received from server" << std::endl;
      return NULL;
    }
    ssize_t send_client = send(client_fd, buffer_from_sever, bytes_received , MSG_NOSIGNAL); 
    if (send_client<0){
      std::cerr << "Error: cannot received from server" << std::endl;
      return NULL;
    }*/

    //地一行不在cache裡
    if (cache.count(request.line) == 0){
      std::string server_response = request_directly(client_fd, server_fd,request);
      Response response = Response(server_response);
      
      if(response.canCache){
        std::cout<<response.input;//
        cache[request.line] = response;
      }
    }
    else{
      std::cout<<"\n\ncache suceed\n\n";
      send_client_cache_directly(client_fd,request);
    }
    //地一行在cache裡
    /*else{
      //no-cache
      if(response.needRevalidate && !response.needCheckTime){
        revalidate(server_fd, client_fd, request);
      }
      //must-revalidate
      else if(getRequest.needRevalidate && getRequest.needCheckTime){
        time_t now = std::time(nullptr);
        std::tm* timeinfo = std::gmtime(&now);
        Date now_date=new Date(timeinfo);

        if (getRequest.expire_time.isLessThan(now_date)){//過期,直接向server請求//?不是請求而是check 304
            std::string server_response = Request_directly(server_fd, client_fd, buffer);
            cache_getrequest[getRequest->line] = getRequest;
            cache_response[getRequest->line] = server_response;
        }
        else{//沒過期，直接給
            char send_client_rec = cache_response[getRequest->line].c_str();
            ssize_t send_client = send(client_fd, send_client_rec, sizeof(send_client_rec), MSG_NOSIGNAL);
            if (send_client<0){
              std::cerr << "Error: cannot received from server" << std::endl;
              return NULL;
          } 
        }
      }
      //max-age
      else if(!getRequest.needRevalidate && getRequest.needCheckTime){
        time_t now = std::time(nullptr);
        std::tm* timeinfo = std::gmtime(&now);
        Date now_date=new Date(timeinfo);

        if (getRequest.expire_time.isLessThan(now_date)){//過期,要驗證
            void revalidate(server_fd, client_fd, getRequest);
        }
        else{//直接給
            char* send_client_rec = cache_response[getRequest->line].c_str();
            ssize_t send_client = send(client_fd, send_client_rec, sizeof(send_client_rec), MSG_NOSIGNAL);
            if (send_client<0){
              std::cerr << "Error: cannot received from server" << std::endl;
              return NULL;
          } 
        }
      }
      //public
      else if(!getRequest.needRevalidate && !getRequest.needCheckTime){
        char* send_client_rec = cache_response[getRequest->line].c_str();
        ssize_t send_client = send(client_fd, send_client_rec, sizeof(send_client_rec), MSG_NOSIGNAL);
        if (send_client<0){
          std::cerr << "Error: cannot received from server" << std::endl;
          return NULL;
        } 
      }
    }*/
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
  close(temp_fd);
}
