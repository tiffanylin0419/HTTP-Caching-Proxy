
#include "head.h"
std::ofstream logFile("/var/log/erss/proxy.log");
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

std::string err502="HTTP/1.1 502 Bad Gateway\r\n\r\n";
std::string ok200="HTTP/1.1 200 OK\r\n\r\n";
std::string bad400="HTTP/1.1 400 Bad Request\r\n\r\n";

void error502(int client_fd){
    char server_response[BUFFER_LEN] = {0};
    err502.copy(server_response, err502.size() + 1);
    server_response[err502.size()] = '\0';
    send(client_fd, server_response,err502.size(), 0);
    pthread_mutex_lock(&mutex2);
    logFile << "Responding \""<<"HTTP/1.1 502 Bad Gateway"<<"\"\n" << std::endl;
    pthread_mutex_unlock(&mutex2);
}

void okGood200(int client_fd){
    char server_response[BUFFER_LEN] = {0};
    ok200.copy(server_response, ok200.size() + 1);
    server_response[ok200.size()] = '\0';
    send(client_fd, server_response,ok200.size(), 0);
    pthread_mutex_lock(&mutex2);
    logFile << "Responding \""<<"HTTP/1.1 200 OK"<<"\"\n" << std::endl;
    pthread_mutex_unlock(&mutex2);
}

void error400(int client_fd){
    char server_response[BUFFER_LEN] = {0};
    bad400.copy(server_response, ok200.size() + 1);
    server_response[bad400.size()] = '\0';
    send(client_fd, server_response,bad400.size(), 0);
    pthread_mutex_lock(&mutex2);
    logFile << "Responding \""<<"HTTP/1.1 400 Bad Request"<<"\"\n" << std::endl;
    pthread_mutex_unlock(&mutex2);
}