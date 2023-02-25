#include "Date.h"
#include "head.h"


class Response{
  public:
    std::string input;
    std::string line;
    std::string statusCode;
    std::string etag;
    

    Date date_time;// for max-age & validation
    Date last_modified_time;
    Date expire_time;//
    Date max_age_time;

    bool canCache;
    bool needRevalidate;
    bool needCheckTime;
    
    //constructor
    Response():input(""),line(""),statusCode(""),etag(""),
        canCache(false),needRevalidate(false),needCheckTime(false){}
    Response(std::string input) : input(input),line(""),statusCode(""),etag(""),
        canCache(false),needRevalidate(false),needCheckTime(false){
        Parse();
        /*canCache=true;
        needRevalidate=true;
        needCheckTime=true;*/
    }

    void Parse(){
        
        //line
        std::stringstream ssLine(input);
        size_t pos = input.find_first_of("\r\n");
        line = input.substr(0, pos);

        //date
        std::string tmpLine;
        while (getline(ssLine, tmpLine, '\n')) {
            pos = tmpLine.find(" ");
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
        
        //etag, cache-control, status code
        std::stringstream ss(input);
        std::vector<std::string> parsedSpace((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
        statusCode=parsedSpace[1];
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
            else if(cache_control.find("must-revalidate") != std::string::npos){needRevalidate=true;needCheckTime=true;}
            else if(cache_control.find("max-age") != std::string::npos){//max-age
                std::size_t pos = cache_control.find("max-age=");
                std::string max_age_s=cache_control.substr(pos+8);
                int max_age=get_number(max_age_s);
                if(max_age>0){
                    max_age_time.renew(date_time,max_age);
                }
                needCheckTime=true;
            }
            else if(cache_control.find("public") != std::string::npos){}
        }
    }

    void print(){
        std::cout<<"line: "<<line<<"\n";
        std::cout<<"status code: "<<statusCode<<"\n";
        std::cout<<"etag: "<<etag<<"\n";
        if(!date_time.isEmpty()){
            std::cout<<"date_time: ";
            date_time.print();
        }
        if(!last_modified_time.isEmpty()){
            std::cout<<"last_modified_time: ";
            last_modified_time.print();
        }
        if(!expire_time.isEmpty()){
            std::cout<<"expire_time: ";
            expire_time.print();
        }
        if(!max_age_time.isEmpty()){
            std::cout<<"max_age_time: ";
            max_age_time.print();
        }
        std::cout<<"canCache: "<<canCache<<"\n";
        std::cout<<"needRevalidate: "<<needRevalidate<<"\n";
        std::cout<<"needCheckTime: "<<needCheckTime<<"\n";
    }
};


