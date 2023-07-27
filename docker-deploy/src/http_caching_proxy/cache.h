#ifndef CACHE_H
#define CACHE_H
#include <list> // for std::list
#include <unordered_map> // for std::unordered_map
// #include <mutex> // for std::mutex and std::lock_guard
#include <boost/thread.hpp>
#include <iostream> 
#include <string> 
#include "http_request.h"
#include "http_response.h"
#include "logger.h"
class cache {
    private:
        std::list<HttpResponse> cacheList;
        std::unordered_map<std::string, std::list<HttpResponse>::iterator> cache_map;//map<url,reponse>
        int capacity;
        static boost::shared_mutex s_mutex; // read/write lock

    private: 
        cache():capacity(20) {}

        cache(int _capacity):capacity(_capacity){}
        cache& operator=(const cache &);
    public:    
        void printCache();
        static cache& getInstance() {
            static cache cacheInstance;
            return cacheInstance;
        }
        ~cache(){};  
        bool checkexist(std::string host);
        HttpResponse get(std::string host,std::string ID); // always return a valid HttpResponse, because we assume that server will    // invoke check() first
        bool put(HttpResponse resp,std::string host,std::string ID);// if check != ContainedAndValid, server will invoke put() to update the cache ContainedAndNeedRevalidate然后invalid的情况会调用put，就是key会重复的唯一情况
        void setCapcity(int cap){
            capacity = cap;
        }     
        
};
#endif

 

