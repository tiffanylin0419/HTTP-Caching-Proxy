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
  //if input="", no Date
  // if input=" ", created by tm, or renewed
  //if input="some string", created by the string
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
  Date() : input(""){}
  Date(std::tm time) : input(" "),time(time){};
  Date(std::string input) : input(input){
    Parse();
  };
  

  //get
  std::tm getTime(){return time;}
  std::string getInput(){return input;}

  //print
  void print(){
    std::cout << asctime(&time);
  }

  //to HTTP form string , ex. Tue, 15 Nov 2022 08:12:31 GMT
  std::string toString(){
    std::stringstream timeStream;
    switch (time.tm_wday) {
      case 0: timeStream << "Sun"; break;
      case 1: timeStream << "Mon"; break;
      case 2: timeStream << "Tue"; break;
      case 3: timeStream << "Wed"; break;
      case 4: timeStream << "Thu"; break;
      case 5: timeStream << "Fri"; break;
      case 6: timeStream << "Sat"; break;
    }
    timeStream <<", "<< time.tm_mday << " ";
    switch (time.tm_mon+1) {
      case 1: timeStream << "Jan"; break;
      case 2: timeStream << "Feb"; break;
      case 3: timeStream << "Mar"; break;
      case 4: timeStream << "Apr"; break;
      case 5: timeStream << "May"; break;
      case 6: timeStream << "Jun"; break;
      case 7: timeStream << "Jul"; break;
      case 8: timeStream << "Aug"; break;
      case 9: timeStream << "Sep"; break;
      case 10: timeStream << "Oct"; break;
      case 11: timeStream << "Nov"; break;
      case 12: timeStream << "Dec"; break;
    }
    timeStream << " " << time.tm_year+1900 << " "<< std::setw(2) << std::setfill('0') << time.tm_hour << ":" << std::setw(2) << std::setfill('0') << time.tm_min << ":" << std::setw(2) << std::setfill('0') << time.tm_sec << " GMT";
    return timeStream.str();
  }


  //compare time
  bool isLessThan(Date d2){
    std::tm t1=time;
    std::tm t2=d2.getTime();
    std::time_t time1 = std::mktime(&t1);
    std::time_t time2 = std::mktime(&t2);
    return time1 < time2;
  }

  //renew used on max_age-date
  void renew(Date d2,int max_age){
    input=" ";
    std::tm t2=d2.getTime();
    std::time_t time_as_time_t = std::mktime(&t2);
    time_as_time_t += max_age;
    time = *localtime(&time_as_time_t);
  }

  bool isEmpty(){
    return input=="";
  }
};
