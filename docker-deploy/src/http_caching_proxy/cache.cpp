#include "cache.h"
boost::shared_mutex cache::s_mutex;

bool cache::checkexist(std::string host){
    boost::shared_lock<boost::shared_mutex> lock(s_mutex);   
    if(cache_map.count(host)==0){//response don't exist in cache
        // std::cout << "cache::checkexist(): reponse can't find in cache" << std::endl;
        return false;
    }
    return true;//response is found in cache
}

// precondition: must call checkexist() before get()
HttpResponse cache::get(std::string host, std::string ID){
    //std::cerr << "cache::get(): start get host: "<< host<< std::endl;
    boost::unique_lock<boost::shared_mutex> lock(s_mutex);
    auto itr = cache_map.find(host);
    if(itr == cache_map.end()) {
        logger& loginstance = logger::getInstance();      
        std::string logline = "cache::get(): reponse can't find in cache";//log errornote
        loginstance.logErrMsg(ID,logline);
        std::cerr << logline<<std::endl;    
    }
    cacheList.splice(cacheList.begin(), cacheList, itr->second);
     std::cerr << "cache::get(): start finish" << std::endl; 
    return *(itr->second);
}

bool cache::put(HttpResponse resp,std::string host,std::string ID){
    boost::unique_lock<boost::shared_mutex> lock(s_mutex);
   // std::cout<<"cache::put() Start put into cache"<<std::endl;
    logger& loginstance = logger::getInstance();
    std::string statusCode = resp.getField("StatusCode");
    std::string cacheControl = resp.getField("Cache-Control");
    //check the exist of Date
    if(resp.getField("Date").empty()){//If no Date exist

        std::string Msg= "agent::GetResp() no Date exist in response";
        loginstance.logWarnMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        
        std::string raw_cache_resp = resp.getRawDataStr();
        std::size_t bodyPos = raw_cache_resp.find("\r\n\r\n");
        std::time_t now_time =time(0)+18000; 
        std::ostringstream oss;
        oss << "Date: "<<ctime(&now_time)<<"\r\n";
        std::string Datestr = oss.str();
        //std::string Datestr = "Date: "+ctime(&now_time)+"\r\n";
        raw_cache_resp.insert(bodyPos+2,Datestr);
        vector<char> newRespVec(raw_cache_resp.begin(), raw_cache_resp.end());
        resp.setRawdata(newRespVec);
        if (!resp.parse()) {
        std::string Msg= "agent::GetResp() Added Date parse error";
        loginstance.logErrMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
    // HttpResponsePtr resp_err = std::make_shared<HttpResponse>(BAD_GATEWAY_502);
        }
    }
    //Cache store response
    if(cache_map.count(host)==0){  //url response exist before  
        if(cacheList.size()>capacity){
            std::string host = cacheList.rbegin()->getField("Url");
            cacheList.pop_back();
            cache_map.erase(host);
        }
        cacheList.push_front(resp);
        cache_map[host] = cacheList.begin();
        // std::cout<<"cache.put():replace the origin response"<<std::endl;
    }  
    else{ 
        // std::cout<<"cache::put() url response didn't exist before "<<std::endl;
         std::unordered_map<std::string, std::list<HttpResponse>::iterator>::iterator response_it = cache_map.find(host); 
        cacheList.erase(response_it->second);      
        cacheList.push_front(resp);
        cache_map[host] = cacheList.begin();
        // std::cout<<"cache.put():insert new response"<<" :    " << (*cache_map[host]).getField("Url")<<std::endl;
       // std::cout<<"response string is: "<<(*cache_map[host]).getRawDataStr()<<std::endl;
    }
    return true;   
}

void cache::printCache(){
    //std::cout<<"printCache(): start "<<std::endl;
    boost::shared_lock<boost::shared_mutex> lock(s_mutex);     
    std::list<HttpResponse> ::iterator itr = cacheList.begin();
    //std::cout<<"printCache(): start while"<<std::endl;
    while(itr!=cacheList.end()){
        std::cout<<"i'm print: "<< (*itr).getField("Date") <<std::endl;
        itr++;
    }
}