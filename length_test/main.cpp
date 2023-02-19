
#include"length.h"



int main() {  
  //std::string str="GET http://detectportal.firefox.com/canonical.html HTTP/1.1\nHost: detectportal.firefox.com\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/110.0\nAccept: */*\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nCache-Control: no-cache\nPragma: no-cache\nConnection: keep-alive";
  //std::string str="CONNECT www.gstatic.com:443 HTTP/1.1\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/110.0\nProxy-Connection: keep-alive\nConnection: keep-alive\nHost: www.gstatic.com:443";
  std::string str="POST http://ocsp.sectigo.com/ HTTP/1.1\nHost: ocsp.sectigo.com\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:109.0) Gecko/20100101 Firefox/110.0\nAccept: */*\nAccept-Language: en-US,en;q=0.5\nAccept-Encoding: gzip, deflate\nContent-Type: application/ocsp-request\nContent-Length: 84\nConnection: keep-alive\nPragma: no-cache\nCache-Control: no-cache";
  const char *c=str.c_str();
  std::cout<<getContextLength(c, 365) <<std::endl;


  return 0;
}

