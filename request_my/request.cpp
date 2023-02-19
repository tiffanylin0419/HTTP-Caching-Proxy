#include "request.h"


bool is_number(std::string& s) {
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    char c = *it;
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

void Request::Parse() {
  //line
  std::stringstream ssLine(input);
  getline(ssLine, line, '\n');

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
  std::string::size_type pos = 0;
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
