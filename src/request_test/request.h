#include "head.h"

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
  void Parse();
};