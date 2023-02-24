#include "proxy.h"

#include <pthread.h>

#include "client_info.h"

#include "proxy_function.h"

#include "function.h"
#include "request.h"
#include "response.h"
#include "error.h"

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
#define RESPONSE_LEN 20

std::map<std::string, Response> cache;

int check502(int client_fd, char buffer[], ssize_t len){
  std::string str(buffer);
  //valid
  if(str.find("Content-Type:") != std::string::npos){
    return 0;
  }
  //invalid
  else{
    error502(client_fd);
    return -1;
  }
}

int send_client_cache_directly(int client_fd, Request request){
  std::string str=cache[request.line].input;
  char server_response[BUFFER_LEN] = {0};
  str.copy(server_response, str.size() + 1);
  server_response[str.size()] = '\0';
  ssize_t send_client = send(client_fd, server_response, str.size() , MSG_NOSIGNAL); 
  if (send_client<0){
    std::cerr << "Error: send! client!" << std::endl;
    return -1;
  }
  return 0;
}

//todo
//
//like post but return the http response
std::string request_directly(int client_fd, int server_fd,Request request){
  std::string str=request.input;
  char buffer1[BUFFER_LEN] = {0};
  str.copy(buffer1, str.size() + 1);
  buffer1[str.size()] = '\0';

  ssize_t server_send = send(server_fd, buffer1, str.size(), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: send! server!" << std::endl;
    return "";
  }

  char buffer2[BUFFER_LEN] = {0};
  ssize_t bytes_received = recv(server_fd, buffer2, sizeof(buffer2), 0);
  if (bytes_received<=0){
    std::cerr << "Error: recv! server!" << std::endl;
    return "";
  }
  buffer2[bytes_received]='\0';
  if(check502(client_fd,buffer2, bytes_received)==-1){
    return "";
  }
  
  std::string buffer2_ss(buffer2);
  bool chunk=false;
  if(buffer2_ss.find("chunk") != std::string::npos){chunk=true;}
  ssize_t send_client = send(client_fd, buffer2, bytes_received , MSG_NOSIGNAL); 
  if (send_client<0){
    std::cerr << "Error: send! client!" << std::endl;
    return "";
  }
  //
  while (chunk) {  //use while loop for processing chunk data
    bytes_received = recv(server_fd, buffer2, sizeof(buffer2), 0);
    if (bytes_received<=0){
      std::cerr << "Chunck End" << std::endl;
      return "";
    }
    buffer2[bytes_received]='\0';
    send_client = send(client_fd, buffer2, bytes_received , MSG_NOSIGNAL); 
    if (send_client<0){
      std::cerr << "Error: send! client!" << std::endl;
      return "";
    }
  }
  std::string buffer2_s(buffer2);
  return buffer2_s;
}

int revalidate(int server_fd,int client_fd, Request request, Response response){
  //check response value, append to request
  std::string str=request.input;
  
  if (response.etag != "") {
    std::string ifNoneMatch = "If-None-Match: " + response.etag + "\r\n";
    str = str + ifNoneMatch;
  }
  if (!response.last_modified_time.isEmpty()){
    std::string ifModifiedSince = "If-Modified-Since: " + response.last_modified_time.toString() + "\r\n";
    str = str + ifModifiedSince;
  }
  //std::cout<<"\n\nmy response: \n\n"<<str<<"\n\n";
  char request_new[BUFFER_LEN] = {0};
  str.copy(request_new, str.size() + 1);
  request_new[str.size()] = '\0';

  //send to server
  ssize_t server_send = send(server_fd, request_new, sizeof(request_new), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: send! server!" << std::endl;
    return -1;
  }
  //recv from server
  char buffer[BUFFER_LEN] = {0};
  ssize_t bytes_received  = recv(server_fd, buffer, sizeof(buffer), 0);
  if (bytes_received <0){
    std::cerr << "Error: recv server!" << std::endl;
    return -1;
  }
  buffer[bytes_received]='\0';
  if(check502(client_fd,buffer, bytes_received)==-1){
    return -1;
  }
  std::string http_response(buffer, BUFFER_LEN);
  Response response_new=Response(http_response);
  std::string status_code = response_new.statusCode;

  //send to client
  if (status_code == "304" ){//same, 直接返還給客戶
    if(send_client_cache_directly(client_fd, request)==-1){
      return -1;
    }
  }
  else{//change, 更新map
    ssize_t send_client =send(client_fd, buffer,bytes_received, 0);
    if (send_client<0){
      std::cerr << "Error: send! client!" << std::endl;
      return -1;
    }
    
    if(response_new.canCache){
      cache[request.line] = response_new;
    }
  }
  return 0;
}

//todo: check recv,send return value
void httpConnect(int client_fd, int server_fd){
  okGood200(client_fd);
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
      
      send(client_fd, buffer1,bytes_received, 0);
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



int httpPost(int client_fd, int server_fd,Request request){
  std::string server_response =request_directly (client_fd, server_fd, request);
  if(server_response==""){
    return -1;
  }
  return 0;
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
    if(httpPost(client_fd, server_fd,request)==-1){return NULL;}
  }
  //get
  else if (request.method=="GET"){
    std::cout<<"get"<<std::endl;
    //地一行不在cache裡
    if (cache.count(request.line) == 0){
      //DRY
      std::string server_response = request_directly(client_fd, server_fd,request);
      if(server_response==""){
        return NULL;
      }
      if(server_response!="chunk"){
        Response response = Response(server_response);
        std::cout<<"\n\n"<<response.input<<"\n";

        if(response.canCache){
          std::cout<<response.input;
          cache[request.line] = response;
        }
      }
    }
    //地一行在cache裡
    else{
      Response response = Response(cache[request.line]);
      //no-cache
      if(response.needRevalidate && !response.needCheckTime){
        std::cout<<"\n\nrevalidate suceed\n\n";
        if(revalidate(server_fd, client_fd, request, response)==-1){return NULL;}
      }
      //must-revalidate
      else if(response.needRevalidate && response.needCheckTime){
        time_t now = std::time(nullptr);
        std::tm* timeinfo = std::gmtime(&now);
        Date now_date=Date(*timeinfo);

        if((response.max_age_time.isEmpty() && response.expire_time.isEmpty()) ||
          (! response.max_age_time.isEmpty() && response.max_age_time.isLessThan(now_date))||
          (! response.expire_time.isEmpty() && response.expire_time.isLessThan(now_date))){//2都沒or任一過期，check
          if(revalidate(server_fd, client_fd, request, response)==-1){return NULL;}
        }
        else {//沒過期,用cache
          if(send_client_cache_directly(client_fd, request)==-1){return NULL;}
        }
      }

      //max-age or expire date
      else if((!response.needRevalidate && response.needCheckTime)|| !response.expire_time.isEmpty()){
        time_t now = std::time(nullptr);
        std::tm* timeinfo = std::gmtime(&now);
        Date now_date=Date(*timeinfo);

        if((! response.max_age_time.isEmpty() && response.max_age_time.isLessThan(now_date))||
          (! response.expire_time.isEmpty() && response.expire_time.isLessThan(now_date))){//已經過期，重新要
          //DRY
          std::string server_response = request_directly(client_fd, server_fd,request);
          if(server_response==""){
            return NULL;
          }
          if(server_response!="chunk"){
            Response response = Response(server_response);
            std::cout<<"\n\n"<<response.input<<"\n";

            if(response.canCache){
              std::cout<<response.input;
              cache[request.line] = response;
            }
          }
        }
        else {//沒過期,用cache
          if(send_client_cache_directly(client_fd, request)==-1){return NULL;}
        }
      }
      //public
      else if(!response.needRevalidate && !response.needCheckTime){//todo: change to if public 
        std::cout<<"\n\ncache suceed\n\n";
        if(send_client_cache_directly(client_fd,request)==-1){return NULL;}
      }
      else{
        std::cerr << "Error: 400" << std::endl;
        error400(client_fd);
        return NULL;
      }
    }
  }
  //none
  else {
    //todo: 
    //send client 400 error
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
