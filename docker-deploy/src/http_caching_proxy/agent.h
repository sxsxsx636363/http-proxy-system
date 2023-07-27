#ifndef AGENT_H
#define AGENT_H
#include <list> // for std::list
#include <unordered_map> // for std::unordered_map
#include <mutex> // for std::mutex and std::lock_guard
#include <iostream>
#include <fstream> 
#include <string> 
#include "http_request.h"
#include "http_response.h"
#include "logger.h"
#include "cache.h"
#include "http_client.h"
#include "http_server.h"
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;
typedef std::shared_ptr<HttpRequest> HttpRequestPtr;

class agent {
    private:
        enum ErrorCode{
            ContainedAndValid, 
            ContainedAndNeedRevalidate,
            NotContained
        };
    private:
        //static agent agentInstance=0;
        agent(){}
        agent& operator=(const agent &);
        ~agent(){};
    private:
        HttpResponse updateResponse(HttpResponse resp,std::string ID);
        std::string editResp(std::string field, std::string oldVal, std::string newVal, std::string raw_cache_resp);
        bool checkfreshness(HttpResponse resp,HttpRequest req);
        std::string getrespExpireTime(HttpResponse resp,std::string ID);
        int respCurrentAge(HttpResponse resp,std::string ID);
        bool checkMaxStale(HttpResponse resp,int req_max_stale,std::string ID);
        bool checkrevalidation(std::string cacheControl);
        bool checkNoCache(std::string resp_cacheControl, std::string req_cacheControl);
        int findage(std::string cacheControl, std::string time_name);
        ErrorCode check(HttpRequest req);
        HttpResponsePtr GetResp(HttpRequestPtr req,HttpClient& client,std::string ID);
        bool checkCachebility(HttpResponse resp,std::string status,std::string ID);
    public:      
        static agent& getInstance() {
            static agent agentInstance;
            return agentInstance;
        }
        
        HttpResponsePtr getCorrespondResp(HttpRequestPtr req,HttpClient& client);

        std::string getRevalidreq(HttpRequest req);

};
#endif