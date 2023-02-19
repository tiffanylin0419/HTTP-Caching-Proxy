#include "head.h"

int get_number(std::string& s) {
  int ans=0;
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    char c = *it;
    if (!isdigit(c)) {
      return -1;
    }
    ans=ans*10+static_cast<int>(c)-static_cast<int>('0');
  }
  return ans;
}

int getContextLength(const char * server_msg, int mes_len) {
  std::string input(server_msg, mes_len);
  std::stringstream ss(input);
  std::vector<std::string> parsedSpace((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
  for(int i=0;i<parsedSpace.size();i++){
    if(parsedSpace[i]=="Content-Length:"){
      std::string hostLine=parsedSpace[i+1];
      std::cout<<hostLine<<std::endl;
      return get_number(hostLine);
    }
  }
  return -1;
}


