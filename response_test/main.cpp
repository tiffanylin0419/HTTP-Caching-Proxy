
#include"GetRequest.h"



int main() {  

  std::string str="GET http://detectportal.firefox.com/canonical.html HTTP/1.1\n"
"Host: detectportal.firefox.com\n"
"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/97.0.4692.99 Safari/537.36\n"
"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\n"
"Accept-Encoding: gzip, deflate, br\n"
"Accept-Language: en-US,en;q=0.9\n"
"Connection: keep-alive\n"
"Date: Thu, 24 Feb 2023 15:30:00 GMT\n"
"Expires: Thu, 24 Feb 2023 16:30:00 GMT\n"
"Last-Modified: Wed, 23 Feb 2023 12:00:00 GMT\n"
"ETag: \"example-12345\"\n"
"Cache-Control: must-revalidate,max-age=3601,max-age=3601\n";

  GetRequest req=GetRequest(str);
  req.print();
  if (req.max_age_time.getInput()==""){
    std::cout<<"no max-age\n\n";
  }
  std::cout<<"d"<<req.max_age_time.getInput()<<"d\n";
  std::cout<<req.expire_time.toString();
  return 0;
}

