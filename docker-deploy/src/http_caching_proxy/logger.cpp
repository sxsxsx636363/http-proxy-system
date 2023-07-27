#include "logger.h"
#include <ctime>

std::string logger::findfirstLine(std::string str){
    std::size_t line_pos = str.find("\n");
    if (line_pos==std::string::npos){//s-maxage exist
        std::string Msg= "Invalid expire format: at logger::findfirstLine()";
        logWarnMsg("no-id",Msg);
        std::cerr << str<<std::endl;
        return "";
    }
    std::string first_line = str.substr(0,line_pos-1);
    return first_line;      
}

void logger::logServRecvReq(HttpRequest req){//receive request from client
    std::string ID = req.getField("ID");  
    std::string IP = req.getField("IP"); 
    std::string Requeststr= req.getRawDataStr(); 
    std::string RequestLine = findfirstLine(Requeststr);  
    if(Requeststr.empty()){
        return;
    }
    time_t now_time =time(0)+18000;   
    logFile << ID <<": " << "\"" << RequestLine<< "\" from " << IP << " @ " << ctime(&now_time);//log
    //std::cout<<ID <<"    cout: " << "\"" << RequestLine<< "\" from " << IP << " @ " << ctime(&now_time)<< std::endl;//log
    return;
}

void logger::logClientSendReq(HttpRequest req){//send request to origin server
    std::string ID = req.getField("ID"); 
    std::string HostServer = req.getField("Host"); 
    if(HostServer.empty()){
        std::string Msg= "No host name in the response received from origin server";
        logWarnMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        //return;
    }
    std::string Requeststr= req.getRawDataStr(); 
    std::string RequestLine = findfirstLine(Requeststr);
    logFile << ID <<": Requesting \"" <<RequestLine<<"\" from "<<HostServer<<std::endl;//log
    //std::cout<<ID <<": Requesting \"" <<RequestLine<<"\" from "<<HostServer<<std::endl;//log
    return;
}

void logger::logClientRecvResp(HttpResponse resp, std::string ID,std::string Host){//Receive response from origin server
    //std::string HostServer = resp.getField("Host"); 
    if(Host.empty()){
        std::string Msg= "No host name in the response received from origin server";
        logWarnMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        //return;
    }
    std::string Responstr= resp.getRawDataStr(); 
    std::string ResponLine = findfirstLine(Responstr);
    logFile << ID <<": Received \"" <<ResponLine<<"\" from "<<Host<<std::endl;//log
    //std::cout<<ID <<": Received \"" <<ResponLine<<"\" from "<<HostServer<<std::endl;//log
    return;
}

void logger::logServSendResp(HttpResponse resp, std::string ID){//Server send responds to the client
    std::string Responstr= resp.getRawDataStr(); 
    
    std::string ResponLine = findfirstLine(Responstr);
    logFile << ID <<": Responding \""<<ResponLine<<"\""<<std::endl;//log
    //std::cout<< ID <<": Responding \""<<ResponLine<<"\""<<std::endl;//log
    return;
}
