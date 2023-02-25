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

int get_number_stale(std::string s) {
  int ans=0;
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    char c = *it;
    if (!isdigit(c)) {
      return ans;
    }
    ans=ans*10+static_cast<int>(c)-static_cast<int>('0');
  }
  return ans;
}

class Request {
 public:
  std::string input;//all
  std::string line;//第一個換行
  std::string method;//第一個空格
  std::string host;//ex. detectportal.firefox.com
  std::string port;//ex. 443 or 80
  int max_stale;
  //constructor
  Request(std::string request) : input(request),max_stale(0) {
    Parse();
  }
  //function
  void Parse() {
    //line
    std::stringstream ssLine(input);
    getline(ssLine, line, '\r');

    //method
    std::stringstream ss(input);
    std::vector<std::string> parsedSpace((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
    method=parsedSpace[0];
    
    //host & port * max-stale
    std::string hostLine;
    for(int i=0;i<parsedSpace.size();i++){
      if(parsedSpace[i]=="Host:"){
        hostLine=parsedSpace[i+1];
        break;
      }
      size_t pos = parsedSpace[i].find("max-stale");
      if(pos!=std::string::npos){
        int tmp=get_number_stale(parsedSpace[i].substr(pos+1));
        if(tmp!=-1){
          max_stale=tmp;
        }
      }
    }
    size_t pos = 0;
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