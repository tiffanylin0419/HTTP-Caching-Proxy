
#include"response.h"



int main() {  

  std::string str="HTTP/1.1 200 OK\n"
                  "Date: Fri, 11 Feb 2023 12:00:00 GMT\n"
                  "Server: Apache/2.4.46 (Ubuntu)\n"
                  "Last-Modified: Wed, 09 Feb 2023 10:00:00 GMT\n"
                  "ETag: \"abcd1234\"\n"
                  "Accept-Ranges: bytes\n"
                  "Content-Length: 12345\n"
                  "Cache-Control: max-age=3600, must-revalidate\n"
                  "\n"
                  "This is the content of the response body.\n";

  response req=response(str);
  req.print();
  if (req.max_age_time.getInput()==""){
    std::cout<<"no max-age\n\n";
  }
  std::cout<<"d"<<req.max_age_time.getInput()<<"d\n";
  std::cout<<req.expire_time.toString();
  return 0;
}

