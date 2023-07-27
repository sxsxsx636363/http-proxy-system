#ifndef LOGGER_H
#define LOGGER_H
#include<fstream>
#include<iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include "http_request.h"
#include "http_response.h"
#include <time.h>
using namespace std;
class logger{
    private:
        std::ofstream logFile;
        logger(){
            logFile.open("/var/log/erss/proxy.log"); 
        }

        logger(const logger&) =delete;
        logger& operator=(const logger &)=delete;

    public:
        static logger& getInstance(){
            static logger loginstance;
            return loginstance;
        }
        ~logger(){
            if (logFile.is_open()) {
                logFile.close();
            }

        }
        
    std::string findfirstLine(std::string str);
    void logServRecvReq(HttpRequest req);
    void logClientSendReq(HttpRequest req);
    void logClientRecvResp(HttpResponse resp, std::string ID,std::string Host);
    void logServSendResp(HttpResponse resp, std::string ID);
    void logTunnelClose(std::string ID){
        logFile << ID <<": Tunnel closed"<<std::endl;
        return;
    }
    void logWarnMsg(std::string ID,std::string Msg){
        logFile << ID <<": WARNING "<<Msg<<std::endl;
        return;
    }
    void logErrMsg(std::string ID,std::string Msg){
        logFile << ID <<": ERROR "<<Msg<<std::endl;
        return;
    } 
    void logNoteMsg(std::string ID,std::string Msg){
        logFile << ID <<": Note "<<Msg<<std::endl;
        return;
    }
    void logCacheCheckReq(std::string logstring){//cache activity
        logFile <<logstring<<std::endl;
        return;
    }
    
};
#endif
