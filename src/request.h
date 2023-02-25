#include "head.h"

int is_number(std::string& s) {
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    char c = *it;
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

class Request {
 public:
  std::string input;//all
  std::string line;//第一個換行
  std::string method;//第一個空格
  std::string host;//ex. detectportal.firefox.com
  std::string port;//ex. 443 or 80
  //constructor
  Request(std::string request) : input(request) {
    Parse();
  }
  //function
  void Parse() {
    //line
    std::stringstream ssLine(input);
    size_t pos = input.find_first_of("\r\n");
    line = input.substr(0, pos);

    //method
    std::stringstream ss(input);
    std::vector<std::string> parsedSpace((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
    method=parsedSpace[0];
    
    //host & port
    std::string hostLine;
    for(int i=0;i<parsedSpace.size();i++){
      if(parsedSpace[i]=="Host:"){
        hostLine=parsedSpace[i+1];
        break;
      }
    }
    pos = 0;
    std::string::size_type prev_pos = 0;
    while ((pos = hostLine.find(':', prev_pos)) != std::string::npos) {
      prev_pos = pos + 1;
    }
    
    
    port=hostLine.substr(prev_pos);
    if (is_number(port)){
      host=hostLine.substr(0,prev_pos-1);
    }else{
      host=hostLine;
      port="80";
    }
    
  }
};