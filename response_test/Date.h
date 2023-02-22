#include "head.h"

int getWeekDay(std::string s){
  std::string day[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  for(int i=0;i<7;i++){
    if (s==day[i]){
      return i;
    }
  }
  return -1;
}

int getMonth(std::string s){
  std::string months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  for(int i=0;i<12;i++){
    if (s==months[i]){
      return i;
    }
  }
  return -1;
}

int get_number(std::string s) {
  int ans=-1;
  for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
    char c = *it;
    if (!isdigit(c)) {
      return ans;
    }
    if(ans==-1){
      ans=0;
    }
    ans=ans*10+static_cast<int>(c)-static_cast<int>('0');
  }
  return ans;
}

class Date {
  private:
  std::string input;
  std::tm time;

  void Parse(){//"Wed, 23 Feb 2023 12:00:00 GMT"
    time.tm_wday=getWeekDay(input.substr(0,3));
    time.tm_mday=get_number(input.substr(5,2));
    time.tm_mon=getMonth(input.substr(8,3));
    time.tm_year=get_number(input.substr(12,4))-1900;
    time.tm_hour=get_number(input.substr(17,2));
    time.tm_min=get_number(input.substr(20,2));
    time.tm_sec=get_number(input.substr(23,2));
    time.tm_isdst = -1;
  }

 public:
 //constructor
  Date(){}
  Date(std::string input) : input(input){
    Parse();
  };

  Date(int wday,int mday, int mon, int year, int hour, int min, int sec) : input(""){
    time.tm_wday=wday;//0-6
    time.tm_mday=mday,//1-31
    time.tm_mon=mon;//0-11
    time.tm_year=year-1900;//year
    time.tm_hour=hour;//0-23
    time.tm_min=min;//0-59
    time.tm_sec=sec;//0-61
    time.tm_isdst = -1;
  };

  //get
  std::tm getTime(){return time;}
  std::string getInput(){return input;}

  //print
  void print(){
    std::cout << asctime(&time);
  }

  //compare time
  bool isLessThan(Date d2){
    std::tm t1=time;
    std::tm t2=d2.getTime();
    std::time_t time1 = std::mktime(&t1);
    std::time_t time2 = std::mktime(&t2);
    return time1 < time2;
  }
  void renew(Date d2,int max_age){
    std::tm t2=d2.getTime();
    std::time_t time_as_time_t = std::mktime(&t2);
    time_as_time_t += max_age;
    time = *localtime(&time_as_time_t);
  }
};
