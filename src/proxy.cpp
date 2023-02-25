#include "proxy.h"
#include "client_info.h"
#include "function.h"
#include "request.h"
#include "response.h"
#include "error.h"

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

std::map<std::string, Response> cache;
std::ofstream logFile("/var/log/erss/proxy.log");

std::string err502="HTTP/1.1 502 Bad Gateway\r\n\r\n";
std::string ok200="HTTP/1.1 200 OK\r\n\r\n";
std::string bad400="HTTP/1.1 400 Bad Request\r\n\r\n";

void error502(int client_fd, int client_id){
    /*char server_response[BUFFER_LEN] = {0};
    err502.copy(server_response, err502.size() + 1);
    server_response[err502.size()] = '\0';
    send(client_fd, server_response,err502.size(), 0);*/
    pthread_mutex_lock(&mutex1);
    logFile << client_id << ": Responding \"" << "HTTP/1.1 502 Bad Gateway" << "\"" << std::endl;
    pthread_mutex_unlock(&mutex1);
}

void okGood200(int client_fd, int client_id){
    char server_response[BUFFER_LEN] = {0};
    ok200.copy(server_response, ok200.size() + 1);
    server_response[ok200.size()] = '\0';
    send(client_fd, server_response,ok200.size(), 0);
    pthread_mutex_lock(&mutex1);
    logFile << client_id << ": Responding \"" << "HTTP/1.1 200 OK" << "\"" << std::endl;
    pthread_mutex_unlock(&mutex1);
}

void error400(int client_fd, int client_id){
    /*char server_response[BUFFER_LEN] = {0};
    bad400.copy(server_response, ok200.size() + 1);
    server_response[bad400.size()] = '\0';
    send(client_fd, server_response,bad400.size(), 0);*/
    pthread_mutex_lock(&mutex1);
    logFile << client_id << ": Responding \"" << "HTTP/1.1 400 Bad Request" << "\"" << std::endl;
    pthread_mutex_unlock(&mutex1);
}


int check502(int client_fd, char buffer[], ssize_t len, int cliend_id){
  std::string str(buffer);
  //valid
  if(str.find("Content-Type:") != std::string::npos){
    return 0;
  }
  //invalid
  else{
    error502(client_fd,cliend_id);
    return -1;
  }
}


int send_client_cache_directly(int client_fd, Request request, int client_id){
  std::string str=cache[request.line].input;
  char server_response[BUFFER_LEN] = {0};
  str.copy(server_response, str.size() + 1);
  server_response[str.size()] = '\0';
  
  ssize_t send_client = send(client_fd, server_response, str.size() , MSG_NOSIGNAL);
  //add
  pthread_mutex_lock(&mutex1);		
  logFile << client_id << ": Responding \"" << cache[request.line].line << "\"";		
  pthread_mutex_unlock(&mutex1); 

  if (send_client<0){
    std::cerr << "Error: send! client!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR send! client!" << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return -1;
  }
  return 0;
}

std::string request_directly(int client_fd, int server_fd,Request request, int client_id){
  std::string str=request.input;
  char buffer1[BUFFER_LEN] = {0};
  str.copy(buffer1, str.size() + 1);
  buffer1[str.size()] = '\0';

  //add
  pthread_mutex_lock(&mutex1);		
  logFile << client_id << ": " << "Requesting \"" << request.line << "\" from " << request.host << std::endl;		
  pthread_mutex_unlock(&mutex1);

  ssize_t server_send = send(server_fd, buffer1, str.size(), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: send! server!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR send! server!"  << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return "";
  }
  char buffer2[BUFFER_LEN] = {0};
  bool isChunk=false;
  bool isFirst=true;
  while (true) {  //use while loop for processing chunk data		
    ssize_t bytes_received = recv(server_fd, buffer2, sizeof(buffer2), 0);		
    if (bytes_received<=0){		
      std::cerr << "Error: send! client! or chunck end" << std::endl;	
      //add
      pthread_mutex_lock(&mutex1);
      logFile << client_id << ": ERROR recv! server!"  << std::endl;
      pthread_mutex_unlock(&mutex1);	

      break;		
    }		
    //add a lot
    if(isFirst){
      std::string input(buffer2);
      std::stringstream ssLine(input);
      std::string line;		
      getline(ssLine, line, '\r');
      pthread_mutex_lock(&mutex1);		
      logFile << client_id << ": Responding \"" << line << "\"" << std::endl;		
      pthread_mutex_unlock(&mutex1);
    }

    /*if(check502(client_fd,buffer2, bytes_received)==-1){
      return "";
    }*/
    std::string buffer2_ss(buffer2);
    if(buffer2_ss.find("chunked") != std::string::npos){isChunk=true;}

    ssize_t send_client = send(client_fd, buffer2, bytes_received , MSG_NOSIGNAL); 		
    if (send_client<0){		
      std::cerr << "Error: send! client!" << std::endl;		
      //add
      pthread_mutex_lock(&mutex1);		
      logFile << client_id << ": ERROR send! client!" << std::endl;		
      pthread_mutex_unlock(&mutex1);
      return "";		
    }		
    isFirst=false;
  }
  if(isChunk){
    return "chunk";
  }

  std::string buffer2_s(buffer2);
  return buffer2_s;
}


std::string request_directly_post(int client_fd, int server_fd,Request request, int client_id){
  std::string str=request.input;
  char buffer1[BUFFER_LEN] = {0};
  str.copy(buffer1, str.size() + 1);
  buffer1[str.size()] = '\0';

  //add
  pthread_mutex_lock(&mutex1);		
  logFile << client_id << ": " << "Requesting \"" << request.line << "\" from " << request.host << std::endl;		
  pthread_mutex_unlock(&mutex1);

  ssize_t server_send = send(server_fd, buffer1, str.size(), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: send! server!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR send! server!"  << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return "";
  }
  char buffer2[BUFFER_LEN] = {0};
  ssize_t bytes_received  = recv(server_fd, buffer2, sizeof(buffer2), 0);
  if (bytes_received <0){
    std::cerr << "Error: recv! server!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR recv! server!"  << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return "";
  }

  //add a lot
  std::string input(buffer2); 		
  std::stringstream ssLine(input);		
  std::string line;		
  getline(ssLine, line, '\r');		
  pthread_mutex_lock(&mutex1);		
  logFile << client_id << ": Responding \"" << line << "\"" << std::endl;		
  pthread_mutex_unlock(&mutex1);


  ssize_t send_client = send(client_fd, buffer2, bytes_received , MSG_NOSIGNAL); 
  if (send_client<0){
    std::cerr << "Error: send! client!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR send! client!" << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return "";
    
  }
  std::string buffer2_s(buffer2);
  return buffer2_s;
}


int revalidate(int server_fd,int client_fd, Request request, Response response, int client_id){
  //check response value, append to request
  std::string str=request.input;
  
  if (response.etag != "") {
    std::string ifNoneMatch = "If-None-Match: " + response.etag + "\r\n";
    str = str + ifNoneMatch;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": NOTE ETag: " << response.etag <<"\r\n"<<std::endl;		
    pthread_mutex_unlock(&mutex1);
  }
  if (!response.last_modified_time.isEmpty()){
    std::string ifModifiedSince = "If-Modified-Since: " + response.last_modified_time.toString() + "\r\n";
    str = str + ifModifiedSince;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": NOTE If-Modified-Since: " << response.last_modified_time.toString() <<"\r\n"<<std::endl;		
    pthread_mutex_unlock(&mutex1);
  }
  //std::cout<<"\n\nmy response: \n\n"<<str<<"\n\n";
  char request_new[BUFFER_LEN] = {0};
  str.copy(request_new, str.size() + 1);
  request_new[str.size()] = '\0';

  //send to server
  ssize_t server_send = send(server_fd, request_new, sizeof(request_new), MSG_NOSIGNAL); 
  if (server_send<0){
    std::cerr << "Error: send! server!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR send! server!" << std::endl;;		
    pthread_mutex_unlock(&mutex1);
    return -1;
  }
  //recv from server
  char buffer[BUFFER_LEN] = {0};
  ssize_t bytes_received  = recv(server_fd, buffer, sizeof(buffer), 0);
  if (bytes_received <0){
    std::cerr << "Error: recv server!" << std::endl;
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": ERROR recv server!" << std::endl;;		
    pthread_mutex_unlock(&mutex1);
    return -1;
  }
  /*if(check502(client_fd,buffer, bytes_received)==-1){
    return -1;
  }*/
  std::string http_response(buffer, BUFFER_LEN);
  Response response_new=Response(http_response);
  std::string status_code = response_new.statusCode;

  //send to client
  if (status_code == "304" ){//same, 直接返還給客戶
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": Responding \"" << request.line << "\"" << std::endl;		
    pthread_mutex_unlock(&mutex1);
    if(send_client_cache_directly(client_fd, request,client_id)==-1){
      return -1;
    }
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": Responding \"" << request.line << "\"" << std::endl;		
    pthread_mutex_unlock(&mutex1);
  }
  else{//change, 更新map
    ssize_t send_client =send(client_fd, buffer,bytes_received, MSG_NOSIGNAL);
    
    if (send_client<0){
      std::cerr << "Error: send! client!" << std::endl;
      //add
      pthread_mutex_lock(&mutex1);		
      logFile << client_id << ": ERROR recv server!" << std::endl;;		
      pthread_mutex_unlock(&mutex1);
      return -1;
    }
    //add
    pthread_mutex_lock(&mutex1);		
    logFile << client_id << ": Responding \"" << response.line << "\""<< std::endl;		
    pthread_mutex_unlock(&mutex1);

    if(response_new.canCache){
      if(response_new.statusCode=="200"){cache[request.line] = response_new;}
    }
  }
  return 0;
}

void httpConnect(int client_fd, int server_fd, int client_id){
  okGood200(client_fd,client_id);
  fd_set readfds;
  int nfds = max(server_fd , client_fd)+1;

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
        //add
        pthread_mutex_lock(&mutex1);		
        logFile << client_id << ": ERROR recv server!" << std::endl;;		
        pthread_mutex_unlock(&mutex1);
        return;
      }
      buffer1[bytes_received]='\0';
      if(send(client_fd, buffer1,bytes_received, MSG_NOSIGNAL)<0){
        std::cerr << "Error: send! client!" << std::endl;   
        pthread_mutex_lock(&mutex1);		
        logFile << client_id << ": ERROR send! client!" << std::endl;		
        pthread_mutex_unlock(&mutex1);
        return ;    
      }
    }
    
    if (FD_ISSET(client_fd, &readfds)) {
      char buffer2[BUFFER_LEN] = {0};//add
      ssize_t bytes_received = recv(client_fd, buffer2, sizeof(buffer2), 0);
      if (bytes_received <= 0) {
        //add
        pthread_mutex_lock(&mutex1);		
        logFile << client_id << ": ERROR recv server!" << std::endl;;		
        pthread_mutex_unlock(&mutex1);
        return;
      }
      buffer2[bytes_received]='\0';
      if(send(server_fd, buffer2,bytes_received, MSG_NOSIGNAL)<0){
        std::cerr << "Error: send! client!" << std::endl;   
        pthread_mutex_lock(&mutex1);		
        logFile << client_id << ": ERROR send! client!" << std::endl;		
        pthread_mutex_unlock(&mutex1);
        return ;    
      }
    }
  }
}



void httpPost(int client_fd, int server_fd,Request request, int client_id){
  std::string server_response =request_directly_post (client_fd, server_fd, request,client_id);
  if(server_response==""){
    pthread_mutex_lock(&mutex1);
    logFile << client_id << ": ERROR server_response!" << std::endl;;		
    pthread_mutex_unlock(&mutex1);
    return;
  }
  Response new_response = Response(server_response);
  pthread_mutex_lock(&mutex1);		
  logFile << client_id << ": Responding \"" << new_response.line << "\""<< std::endl ;		
  pthread_mutex_unlock(&mutex1);
  return;
}

void * handle(void * info) {
  Client_Info * client_info = (Client_Info *)info;
  int client_fd = client_info->getFd();
  char buffer[BUFFER_LEN] = {0};
  //now time
  time_t now = std::time(nullptr);
  std::tm* timeinfo = std::gmtime(&now);
  Date now_date=Date(*timeinfo);

  ssize_t bytes_received=recv(client_fd, buffer, sizeof(buffer), 0);
  if (bytes_received<0){
    pthread_mutex_lock(&mutex1);
    logFile << client_info->getID() << ": WARNING Invalid Request" << std::endl;
    pthread_mutex_unlock(&mutex1);
    return NULL;
  }
  std::string buffer_s(buffer);
  //std::cout<<buffer<<std::endl;//?
  if (buffer_s == ""||buffer_s == "\r" || buffer_s == "\n" || buffer_s == "\r\n"){
    return NULL;
  }
  //parse http request
  Request request=Request(buffer_s);
  //add
  pthread_mutex_lock(&mutex1);		
  logFile << client_info->getID() << ": " << "\"" <<request.line<< "\"" << " from "<< client_info->getIP() << " @ " << now_date.toPrintString();		
  pthread_mutex_unlock(&mutex1);
  const char * host = request.host.c_str();
  const char * port = request.port.c_str();
  int server_fd = build_client(host,port);

  //connect
  if (request.method=="CONNECT"){
    pthread_mutex_lock(&mutex1);
    logFile << client_info->getID() << ": "		<< "Requesting \"" << request.line << "\" from " << host << std::endl;		
    pthread_mutex_unlock(&mutex1);
    httpConnect(client_fd, server_fd,client_info->getID());
  }
  //post
  else if (request.method=="POST"){
    pthread_mutex_lock(&mutex1);
    logFile << client_info->getID() << ": "	<< "Requesting \"" << request.line << "\" from " << host << std::endl;		
    pthread_mutex_unlock(&mutex1);
    httpPost(client_fd, server_fd,request, client_info->getID());
  }
  //get
  else if (request.method=="GET"){
    std::cout<<"get"<<std::endl;
    //cache size control
    while(cache.size()>100){
      cache.erase(cache.begin());
    }
    //地一行不在cache裡
    if (cache.count(request.line) == 0){
      pthread_mutex_lock(&mutex1);		
      logFile << client_info->getID() << ": not in cache" << std::endl;		
      pthread_mutex_unlock(&mutex1);
      //DRY
      std::string server_response = request_directly(client_fd, server_fd,request,client_info->getID());
      if(server_response==""){
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": ERROR server_response!" << std::endl;;		
        pthread_mutex_unlock(&mutex1);
        return NULL;
      }
      else if(server_response=="chunk"){
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": NOTE chunk!" << std::endl;;		
        pthread_mutex_unlock(&mutex1);
        return NULL;
      }
      
      Response response = Response(server_response);
      //add a lot
      pthread_mutex_lock(&mutex1);		      std::cout<<"\n\n"<<response.input<<"\n";
      logFile << client_info->getID() << ": " << "Received \"" << response.line << "\" from " << request.host << std::endl;		
      pthread_mutex_unlock(&mutex1);		
      pthread_mutex_lock(&mutex1);		
      logFile << client_info->getID() << ": Responding \"" << response.line << "\""<< std::endl;		
      pthread_mutex_unlock(&mutex1);		
      //std::cout<<"\n\n"<<response.input<<"\n";		
      //for log		
      if(!response.canCache){		
        pthread_mutex_lock(&mutex1);		
        //no-store 和 private 不知道理由要不要分開寫，但要找方法把他們分開，要補 chunked 的 not cacheable		
        logFile << client_info->getID() << ": " << "not cacheable because no-store or private" << std::endl;		
        pthread_mutex_unlock(&mutex1);		
      }		
      if (! response.expire_time.isEmpty() ) {		
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": cached, expires at "<< response.expire_time.toPrintString() << std::endl;		
        pthread_mutex_unlock(&mutex1);		
      }		
      if (response.needRevalidate && response.needCheckTime) {		
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": " << "cached, but requires re-validation" << std::endl;		
        pthread_mutex_unlock(&mutex1);		
      }


      if(response.canCache){
        std::cout<<response.input;
        if(response.statusCode=="200"){cache[request.line] = response;}
      }
      
    }
    //地一行在cache裡
    else{
      Response response = Response(cache[request.line]);
      pthread_mutex_lock(&mutex1);		
      logFile << client_info->getID() << ": in cache, but expired at "<< response.expire_time.toPrintString() << std::endl;		
      pthread_mutex_unlock(&mutex1);
      //no-cache
      if(response.needRevalidate && !response.needCheckTime){
        std::cout<<"\n\nrevalidate suceed\n\n";
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": NOTE Cache-Control: no-cache" << std::endl;		
        pthread_mutex_unlock(&mutex1);		
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getFd() << ": in cache, requires validation" << std::endl;		
        pthread_mutex_unlock(&mutex1);
        if(revalidate(server_fd, client_fd, request, response, client_info->getID())==-1){return NULL;}
      }
      //must-revalidate
      else if(response.needRevalidate && response.needCheckTime){
        pthread_mutex_lock(&mutex1);		        
        logFile << client_info->getID() << ": NOTE Cache-Control: must-revalidate" << std::endl;		
        pthread_mutex_unlock(&mutex1);

        if((response.max_age_time.isEmpty() && response.expire_time.isEmpty()) ||
          (! response.max_age_time.isEmpty() && response.max_age_time.isLessThan(now_date,request.max_stale))||
          (! response.expire_time.isEmpty() && response.expire_time.isLessThan(now_date,request.max_stale))){//2都沒or任一過期，check
          
          pthread_mutex_lock(&mutex1);
          logFile << client_info->getFd() << ": in cache, requires validation" << std::endl;		
          pthread_mutex_unlock(&mutex1);

          if(revalidate(server_fd, client_fd, request, response, client_info->getID())==-1){return NULL;}
        }
        else {//沒過期,用cache
          pthread_mutex_lock(&mutex1);		
          logFile << client_info->getID()  << ": Responding \"" << response.line << "\"" << std::endl;		
          pthread_mutex_unlock(&mutex1);
          if(send_client_cache_directly(client_fd, request,client_info->getID())==-1){return NULL;}
        }
      }

      //max-age or expire date
      else if((!response.needRevalidate && response.needCheckTime)){
        pthread_mutex_lock(&mutex1);	
        logFile << client_info->getID() << ": NOTE Cache-Control: max-age" << std::endl;	
        pthread_mutex_unlock(&mutex1);
        if((! response.max_age_time.isEmpty() && response.max_age_time.isLessThan(now_date,request.max_stale))||
          (! response.expire_time.isEmpty() && response.expire_time.isLessThan(now_date,request.max_stale))){//已經過期，重新要
          //DRY
          std::string server_response = request_directly(client_fd, server_fd,request,client_info->getID());		
          if(server_response==""){
            pthread_mutex_lock(&mutex1);		
            logFile << client_info->getID() << ": ERROR request_directly!" << std::endl;;		
            pthread_mutex_unlock(&mutex1);
            return NULL;
          }	
          else if(server_response=="chunk"){
            pthread_mutex_lock(&mutex1);		
            logFile << client_info->getID() << ": NOTE chunked!" << std::endl;;		
            pthread_mutex_unlock(&mutex1);
            return NULL;
          }
          Response response = Response(server_response);		
          std::cout<<"\n\n"<<response.input<<"\n";		
          if(response.canCache){		
            std::cout<<response.input;		
            if(response.statusCode=="200"){cache[request.line] = response;}
          }		
        }
        else {//沒過期,用cache
          if(send_client_cache_directly(client_fd, request,client_info->getID())==-1){return NULL;}
        }

      }
      //public
      else if(!response.needRevalidate && !response.needCheckTime){ 
        pthread_mutex_lock(&mutex1);		
        logFile << client_info->getID() << ": NOTE Cache-Control: public" << std::endl;		
        pthread_mutex_unlock(&mutex1);
        if(send_client_cache_directly(client_fd,request,client_info->getID())==-1){
          pthread_mutex_lock(&mutex1);		
          logFile << client_info->getID() << ": ERROR send_client_cache_directly!" << std::endl;;		
          pthread_mutex_unlock(&mutex1);
          return NULL;
        }
      }
      else{return NULL;}
    }

  }
  //none
  else {
    error400(client_fd,client_info->getID());
    return NULL;
  }
  pthread_mutex_lock(&mutex1);		
  logFile << client_info->getID() << ": Tunnel closed" << std::endl;		
  pthread_mutex_unlock(&mutex1);
  close(server_fd);
  close(client_fd);
  return NULL;
}



void proxy::run() {
  int temp_fd = build_server(this->port_num);
  if (temp_fd == -1) {
    pthread_mutex_lock(&mutex1);		
    logFile << "(no-id): ERROR in creating socket to accept" << std::endl;		
    pthread_mutex_unlock(&mutex1);
    return;
  }
  int client_fd;
  int id = 0;
  while (1) {
    std::string ip;
    client_fd = server_accept(temp_fd);
    if (client_fd == -1) {
      pthread_mutex_lock(&mutex1);		
      logFile << "(no-id): ERROR in connecting client" << std::endl;		
      pthread_mutex_unlock(&mutex1);
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