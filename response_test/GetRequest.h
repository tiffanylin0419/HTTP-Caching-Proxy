#include "Date.h"
#include "head.h"


class GetRequest {
  public:
    std::string input;
    std::string line;
    std::string etag;

    Date date_time;// for max-age & validation
    Date last_modified_time;
    Date expire_time;//
    Date max_age_time;

    bool canCache;
    bool needRevalidate;
    bool needCheckTime;


    //constructor
    GetRequest(std::string input) : input(input),line(""),etag(""),
        canCache(false),needRevalidate(false),needCheckTime(false){
        Parse();
    }

    void Parse(){
        
        //line
        std::stringstream ssLine(input);
        getline(ssLine, line, '\n');

        //date
        std::string tmpLine;
        while (getline(ssLine, tmpLine, '\n')) {
            std::size_t pos = tmpLine.find(" ");
            std::string front=tmpLine.substr(0,pos);
            std::string back=tmpLine.substr(pos+1);
            
            if (front=="Date:"){
                date_time=Date(back);
            }
            else if (front=="Last-Modified:"){
                last_modified_time=Date(back);
            }
            else if (front=="Expires:"){
                expire_time=Date(back);
                needRevalidate=true;needCheckTime=true;
            }

        }
        
        //etag, cache-control
        std::stringstream ss(input);
        std::vector<std::string> parsedSpace((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
        std::string cache_control;
        for(int i=0;i<parsedSpace.size();i++){
            if(parsedSpace[i]=="ETag:"){
                etag=parsedSpace[i+1];
            }
            else if (parsedSpace[i]=="Cache-Control:"){
                cache_control=parsedSpace[i+1];
            }
        }

        //bool, max-age
        if (cache_control.find("no-store") != std::string::npos||cache_control.find("private") != std::string::npos){}
        else{
            canCache=true;
            if(cache_control.find("no-cache") != std::string::npos){needRevalidate=true;}
            else if(cache_control.find("max-age") != std::string::npos){//max-age
                std::size_t pos = cache_control.find("max-age=");
                std::string max_age_s=cache_control.substr(pos+8);
                int max_age=get_number(max_age_s);
                if(max_age>0){
                    max_age_time.renew(date_time,max_age);
                }
                needCheckTime=true;
            }
            else if(cache_control.find("must-revalidate") != std::string::npos){needRevalidate=true;needCheckTime=true;}
            else if(cache_control.find("public") != std::string::npos){}
        }
    }

    void print(){
        std::cout<<"line: "<<line<<"\n";
        std::cout<<"etag: "<<etag<<"\n";
        std::cout<<"date_time: ";
        date_time.print();
        std::cout<<"last_modified_time: ";
        last_modified_time.print();
        std::cout<<"expire_time: ";
        expire_time.print();
        std::cout<<"max_age_time: ";
        max_age_time.print();
        std::cout<<"canCache: "<<canCache<<"\n";
        std::cout<<"needRevalidate: "<<needRevalidate<<"\n";
        std::cout<<"needCheckTime: "<<needCheckTime<<"\n";
    }
};


