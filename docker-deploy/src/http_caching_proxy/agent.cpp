#include "agent.h"

HttpResponsePtr agent::getCorrespondResp(HttpRequestPtr req,HttpClient& client){
    std::cout<<"Start getCorrespondResp()"<<std::endl;
    //HttpClient client;
    cache& cacheinstance = cache::getInstance();
    logger& loginstance = logger::getInstance();
    std::string host = req->getField("Url"); 
    std::string Method = req->getField("Method"); 
    std::string ID = req->getField("ID");
    if(Method == "GET"){
        std::cout<<"Start getCorrespondResp() check()"<<std::endl;
        int checkVal = check(*req);
        //Check response status
        if(checkVal==ContainedAndValid){
            std::cout << "agent::getCorrespondResp() start ContainedAndValid"<<endl;
            HttpResponse received_resp = cacheinstance.get(host,ID);
            //std::cout << "agent::getCorrespondResp() ContainedAndValid: "<<received_resp.getRawDataStr()<<endl;
           
            //std::string respStr = received_resp.getRawDataStr();
            std::vector<char> RespVec =  received_resp.getRawData();
            //std::vector<char> RespVec(respStr.begin(), respStr.end());
             std::cout << "agent::getCorrespondResp point 1"<<std::endl;
            return std::make_shared<HttpResponse>(RespVec);
        }
        else if(checkVal==ContainedAndNeedRevalidate){
            std::string revalidReq = getRevalidreq(*req);
            //std::cout << "agent::getCorrespondResp() get revalidate: "<< revalidReq << std::endl;
            std::vector<char> revalidReqVec(revalidReq.begin(), revalidReq.end());
            HttpRequestPtr revalidreqPtr = std::make_shared<HttpRequest>(revalidReqVec);
            if (!revalidreqPtr->parse()) {
                std::string Msg= "agent::getCorrespondResp() HttpRequest parse error";
                loginstance.logErrMsg(ID,Msg);
                std::cerr << Msg<<std::endl;
                return NULL;
            }
            //std::cout << "agent::getCorrespondResp() get if non match: "<<revalidreqPtr->getField("If-None-Match")<<std::endl;

            HttpResponsePtr resp = GetResp(revalidreqPtr,client,ID);
            if (!resp) {
                return std::shared_ptr<HttpResponse>(nullptr);
            }
            //Check the received response status
            //std::cout << "agent::getCorrespondResp() Status"<<std::endl;
            std::string Status = resp->getField("StatusCode");
            //std::cout << "agent::getCorrespondResp() StatusCode"<<std::endl;
            if(Status=="200"||Status=="304"){//if response status is 200, or 304:store in cache
             //std::cout << "agent::getCorrespondResp() updateResponse"<<std::endl;
                std::string respStr;
                if(checkCachebility(*resp,Status,ID) == true && !host.empty()){
                    if(Status == "304"){//Update the cache response and send to client
                        HttpResponse updatedRep = updateResponse(*resp,ID);
                        cacheinstance.put(updatedRep,host,ID);//Update response in cache                   
                        //std::cout<<"agent::getCorrespondResp(): 304 update the response"<<std::endl;
                        HttpResponse received_resp = cacheinstance.get(host,ID);
                        respStr = received_resp.getRawDataStr(); 
                    }
                    else{
                        HttpResponse updatedRep = *resp;
                        cacheinstance.put(updatedRep,host,ID);//Update response in cache                   
                        //std::cout<<"agent::getCorrespondResp(): 304 update the response"<<std::endl;
                        HttpResponse received_resp = cacheinstance.get(host,ID);
                        respStr = received_resp.getRawDataStr(); 
                    }
                     
                }else{
                    std::string Msg= "agent::getCorrespondResp(): response is not cachable, can't update";
                    loginstance.logNoteMsg(ID,Msg);
                    std::cout << Msg<<std::endl;
                    respStr = resp->getRawDataStr();          
                }
                std::vector<char> RespVec(respStr.begin(), respStr.end());
                return std::make_shared<HttpResponse>(RespVec);
            }                            
            else{//if response status is not 200, or 304: not store in cache, use directly
                std::string Msg= "agent::getCorrespondResp(): response don't can't update, use forward directly";
                loginstance.logNoteMsg(ID,Msg);
                std::cout << Msg<<std::endl;
                //std::cout<<"agent::getCorrespondResp(): response don't can't update, use forward directly"<<std::endl;
                return resp;
            }         
        }
        //NotContained
        //std::cout<<"agent::getCorrespondResp(): not contained start"<<endl;
        loginstance.logServRecvReq(*req);
        HttpResponsePtr new_resp = GetResp(req,client,ID);
        //std::cout<<"agent::getCorrespondResp(): get Expires: "<<new_resp->getField("Expires")<<std::endl;
        if (!new_resp) {
            return std::shared_ptr<HttpResponse>(nullptr);
        }
        std::string Status = new_resp->getField("StatusCode");
        std::string respStr;
        if(checkCachebility(*new_resp,Status,ID) == true && !host.empty()&&Status=="200"){
            cacheinstance.put(*new_resp,host,ID);
            //print reposne revalidation and expire date in logfile              
            //std::cout<<"agent::getCorrespondResp(): add reponse into cache"<<std::endl;
        }
        return new_resp;     
    }else{
        return GetResp(req,client,ID);
    }      
}

HttpResponsePtr agent::GetResp(HttpRequestPtr req,HttpClient& client,std::string ID){
    logger& loginstance = logger::getInstance();
    if (!client.init(req)) {
        std::string Msg= "agent::GetResp() HttpClient init() error";
        loginstance.logErrMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        return NULL;
    }        
    if (!client.sendRequest(req)) {
        std::string Msg= "agent::GetResp() HttpClient sendRequest() error";
        loginstance.logErrMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        return NULL;
    }     
    std::string Host = req->getField("Url");
    loginstance.logClientSendReq(*req);  
    HttpResponsePtr resp = client.recvResponse();
    if (!resp) {
        std::string Msg= "agent::GetResp() client.recvResponse error";
        loginstance.logErrMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
        return NULL;
    }
    if (!resp->parse()) {
        std::string Msg= "agent::GetResp() HttpResponse parse error";
        loginstance.logErrMsg(ID,Msg);
        std::cerr << Msg<<std::endl;
    // HttpResponsePtr resp_err = std::make_shared<HttpResponse>(BAD_GATEWAY_502);
    }
    loginstance.logClientRecvResp(*resp,ID,Host);
    return resp;
}
agent::ErrorCode agent::check(HttpRequest req){  
    cache& cacheinstance = cache::getInstance();
    logger& loginstance = logger::getInstance();
    std::string host = req.getField("Url");   
    std::string ID = req.getField("ID");  
    std::cout <<"agent::check() check start"<<std::endl;
    //check exist
    if(cacheinstance.checkexist(host)==true){//response exist, check the response status
        std::cout <<"agent::check() response is in cache. host: "<<host<<std::endl;
        //Fetch response from cache
        HttpResponse cachedResponese = cacheinstance.get(host,ID);
        //Check no-cache
        //std::cout <<"agent::check() response 2"<<std::endl;
        std::string resp_cacheControl = cachedResponese.getField("Cache-Control");
        std::string req_cacheControl = req.getField("Cache-Control");
        if(checkNoCache(resp_cacheControl, req_cacheControl) == true){
            std::string logline = ID+": in cache, requires validation";//log           
            loginstance.logCacheCheckReq(logline);   
            std::cout <<logline<<std::endl;        
            //std::cout <<"agent::check(): return ContainedAndNeedRevalidate"<<std::endl;
            return ContainedAndNeedRevalidate;
        }
        //no-cache don't exist       
        //Check freshness
        if(checkfreshness(cachedResponese,req) == false){//response staled
            //std::cout <<"agent::check():response staled"<<std::endl;
            std::string respExpireTime = getrespExpireTime(cachedResponese,ID);
            int req_max_stale = findage(req_cacheControl, "max-stale");
            if(req_max_stale >0){//max_stale exist
                //std::cout <<"agent::check():Max - Stale exist"<<std::endl;
                if(checkMaxStale(cachedResponese,req_max_stale,ID)==false){//exceed max stale
                    std::cout <<"agent::check():checkMaxStale: return NotContained"<<std::endl;
                    if(checkrevalidation(resp_cacheControl)==true){//need revalid
                        std::string logline = ID+": in cache, requires validation";//log
                        std::cout <<logline<<std::endl;   
                        //std::cout <<"agent::check() checkMaxStale: requires validation"<<std::endl;
                        loginstance.logCacheCheckReq(logline);                       
                    }
                    else{//no revalidation require, print expired date
                        std::string logline = ID+": in cache, but expired at "+respExpireTime;//log 
                        //std::cout <<"agent::check() checkMaxStale: revalidation after print expire date"<<logline<<std::endl;
                        std::cout <<logline<<std::endl;  
                        loginstance.logCacheCheckReq(logline);
                    }
                    return ContainedAndNeedRevalidate;
                }
                else{
                    //contained
                    std::string logline = ID+": in cache, valid";//log 
                    //std::cout <<"agent::check():checkMaxStale: stale response within max-stale, valid"<<std::endl;
                    loginstance.logCacheCheckReq(logline);
                    std::cout <<logline<<std::endl;  
                    return ContainedAndValid;              
                }
            }
            else if(req_max_stale == 0){//ContainedAndValid
                std::string logline = ID+": in cache, valid";//log 
                loginstance.logCacheCheckReq(logline);
                //std::cout <<"agent::check():checkMaxStale: max-stale = 0, always valid"<<std::endl;
                std::cout <<logline<<std::endl;               
                return ContainedAndValid;
            }
            else{//max_stale not exist
            //check needrevalid
                std::cout <<"agent::check():checkMaxStale not exist"<<std::endl;
                if(checkrevalidation(resp_cacheControl)==true){//need revalid
                    std::string logline = ID+": in cache, requires validation";//log 
                    //std::cout <<"agent::check() stale response: requires validation "<<logline<<std::endl;  
                    std::cout <<logline<<std::endl;                          
                    loginstance.logCacheCheckReq(logline);
                }
                else{
                    std::string logline = ID+": in cache, but expired at "+respExpireTime;//log 
                   // std::cout <<"agent::check() checkrevalidation:stale response print expired date "<<logline<<std::endl; 
                   std::cout <<logline<<std::endl;                    
                    loginstance.logCacheCheckReq(logline);                  
                }
                return ContainedAndNeedRevalidate;
            }
        }
        else{//response not staled, its fresh
            //check resquest max-age
            int current_age = respCurrentAge(cachedResponese,ID);
            int req_maxage = findage(req_cacheControl, "max-age");
            int min_fresh = findage(req_cacheControl, "min-fresh");
            if(req_maxage!=-1&&current_age>=req_maxage){ //current request not fit the max-age in the request,need revalidation
                std::string logline = ID+": in cache, requires validation";//log 
                //std::cout<< "agent::check() req_maxage: need revalidation"<<std::endl;
                std::cout <<logline<<std::endl;  
                loginstance.logCacheCheckReq(logline);
                return ContainedAndNeedRevalidate;
            }
            else if(req_maxage!=-1&&current_age<=min_fresh){//check request min-fresh, if current age smaller than min_fresh, need revalidation
                //need revalid
                std::string logline = ID+": in cache, requires validation";//log 
                //std::cout <<"agent::check() min_fresh: need revalidation"<<std::endl;
                std::cout <<logline<<std::endl;  
                loginstance.logCacheCheckReq(logline);
                return ContainedAndNeedRevalidate;
            }            
            //contained
            std::string logline = ID+": in cache, valid";//log 
            loginstance.logCacheCheckReq(logline);
            //std::cout <<"agent::check() return ContainedAndValid"<<std::endl;
            std::cout <<logline<<std::endl;  
            return ContainedAndValid;
        }
    }
    //response don't exist, return 
    //std::cout<<"agent::check() return NotContained  "<<NotContained<<std::endl;
    std::string logline;  
    logline =ID+": not in cache";//log
    loginstance.logCacheCheckReq(logline);
    std::cout <<logline<<std::endl;  
    //response not exist
    return NotContained;
}

HttpResponse agent::updateResponse(HttpResponse resp,std::string ID){
    cache& cacheinstance = cache::getInstance();
    std::string raw_response = resp.getRawDataStr();  
    std::string host = resp.getField("Url");
   // std::cout<<"agent::updateResponse() start host: "<<host<<std::endl;
    if(!host.empty()){
        HttpResponse cachedresp = cacheinstance.get(host,ID);
            std::string raw_cache_resp =  cachedresp.getRawDataStr();
            std::string oldCacheConVal = cachedresp.getField("Cache-Control");
            std::string newCacheConVal = resp.getField("Cache-Control");
            std::string newresp;
            //update cache_control

            newresp = editResp("Cache-Control",oldCacheConVal,newCacheConVal,raw_cache_resp);
            
            //update Last-Modified
            std::string oldlastmodVal = cachedresp.getField("Last-Modified");
            std::string newlastmodVal = resp.getField("Last-Modified");
            newresp = editResp("Last-Modified",oldlastmodVal,newlastmodVal,newresp);

            //update ETag
            std::string oldETagVal = cachedresp.getField("ETag");
            std::string newETagVal = resp.getField("ETag");
            newresp = editResp("ETag",oldETagVal,newETagVal,newresp);

            //update Expires
            std::string oldExpiresVal = cachedresp.getField("Expires");
            std::string newExpiresVal = resp.getField("Expires");
            newresp = editResp("Expires",oldExpiresVal,newExpiresVal,newresp);

            vector<char> newRespVec(newresp.begin(), newresp.end());
            cachedresp.setRawdata(newRespVec);
            std::string Msg= "agent updated the response";
            std::cerr << Msg<<std::endl;
            return cachedresp;
    }
    //Assume only 304 and 200 will call this function
    std::string logline = ID+": WARNING host name is unknown";//log
    logger& loginstance = logger::getInstance(); 
    loginstance.logWarnMsg(logline,ID);
    return resp;
}

std::string agent::editResp(std::string field, std::string oldVal, std::string newVal, std::string raw_cache_resp){
    //update field
    if(oldVal!=""||newVal!=""){
        if(oldVal!=""){
            std::size_t oldPos = raw_cache_resp.find(field);
            std::string tempOld = raw_cache_resp.substr(oldPos);
            std::size_t endLinePos = tempOld.find("\n");
            int length = endLinePos-oldPos+1;
            raw_cache_resp.erase (oldPos,length); 
        }
        if(newVal!=""){
            std::size_t newPos = raw_cache_resp.find("\r\n\r\n");
            std::string newstr = field + ": "+newVal+"\r\n";
            raw_cache_resp.insert(newPos+2,newstr);
        }
    }
    return raw_cache_resp;
}

bool agent::checkfreshness(HttpResponse resp,HttpRequest req){ 
    logger& loginstance = logger::getInstance();   
    std::string ID = req.getField("ID");
    std::string respExpireTime = getrespExpireTime(resp,ID);
    if(respExpireTime.empty()){       
         std::string logwarn="Warning response cant' find expired time";//log errornote
       std::cout<< ID<<": "<<logwarn;
        loginstance.logWarnMsg(ID,logwarn);
        return false;
    }
    std::tm expireTm;
    expireTm.tm_isdst = -1;
    //std::cout<< ID<<"agent::checkfreshness(): "<<respExpireTime<<std::endl;
    strptime(respExpireTime.c_str(), "%a %b %d %H:%M:%S %Y", &expireTm);
    //std::cout<<"agent::chec"<<std::endl;
    std::time_t now_time =time(0)+18000; 
    std::time_t Expire_time = std::mktime(&expireTm);//Expire_time may be invalid
    std::cout <<"agent::checkfreshness() expireTm:"<<ctime(&Expire_time)<<"now: "<<ctime(&now_time)<<std::endl; 
   // std::cout<<"agent::checkfreshness() error in strptime() expireTm:"<<ctime(&Expire_time_utc)<<std::endl;
    if(Expire_time==-1||now_time>=Expire_time){                             
        return false;
    }         

    //std::cout<<"agent::checkfreshness() not expired"<<std::endl;
    return true;
}

std::string agent::getRevalidreq(HttpRequest req){
    logger& loginstance = logger::getInstance(); 
    cache& cacheinstance = cache::getInstance();
    std::string ID = req.getField("ID");
    std::string host = req.getField("Url");
    HttpResponse req_response = cacheinstance.get(host,ID);
    std::string last_modified_time = req_response.getField("Last-Modified");
    std::string ETagVal = req_response.getField("ETag");
    std::string raw_req = req.getRawDataStr();
    
    if(last_modified_time.empty()&&ETagVal.empty()){//Didn't find last modified header
        //std::cout<<"agent::getRevalidreq() no find "<<std::endl;
        std::string logwarn="No \"ETag\" or \"Last-Modified\" can be used for revalidation";//log errornote
        std::cout<< ID<<": "<<logwarn;
        loginstance.logWarnMsg(ID,logwarn);
        return raw_req;
    }
    std::string Etag;
    std::string last_modified;
    if(!ETagVal.empty()){//Use ETag to revalid
        Etag = "If-None-Match: "+ETagVal + "\r\n";
        //std::cout<<"agent::getRevalidreq() find ETag"<<Etag<<std::endl;
    }
    if(!last_modified_time.empty()){
        last_modified = "If-Modified-Since: "+last_modified_time + "\r\n";
        // std::cout<<"agent::getRevalidreq find last mod"<<last_modified<<std::endl;
    }
    
    //Find the first line after the header field
    std::size_t afterHeaderPos = raw_req.find("\r\n\r\n");
    std::string addedSinceReq;
    if(afterHeaderPos!=std::string::npos){
        //Add If-None-Match/If-Modified-Since header into the request
        if(!Etag.empty()){
            raw_req= raw_req.insert(afterHeaderPos+2,Etag);           
        }
        afterHeaderPos = raw_req.find("\r\n\r\n");
        if(afterHeaderPos!=std::string::npos&&!last_modified.empty()){
            
            raw_req= raw_req.insert(afterHeaderPos+2,last_modified);            
        }
        
        //std::cout<<"agent::getRevalidreq finial request is "<<raw_req<<std::endl;
        return raw_req;
    }
    std::cerr<<"can't find the first line after the header field"<<std::endl;
    std::string logwarn="Can't find the header end to insert control header";//log errornote
    std::cout<< ID<<": "<<logwarn;
    loginstance.logErrMsg(ID,logwarn);
    return raw_req;
}

std::string agent::getrespExpireTime(HttpResponse resp,std::string ID){
    logger& loginstance = logger::getInstance();
    std::time_t now_time =time(0); 
    std::string Date = resp.getField("Date");
    std::cout<<"agent::getrespExpireT() resp date:"<<Date<<endl;
      
    if(!Date.empty()){
        std::tm dateTm;
         dateTm.tm_isdst = -1;
      if (strptime(Date.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &dateTm) == nullptr) {     
        } 
        //std::tm = gmtime(&exipTime);
        std::time_t Date_time = std::mktime(&dateTm);
       //std::cout<<"agent::getrespExpireTime() response date is: "<<ctime(&Date_time)<<" or "<<Date<<std::endl;
        std::string resp_cacheControl = resp.getField("Cache-Control");
        //find the max-age in response
        //check s-maxage
        int resp_maxage = findage(resp_cacheControl,"s-maxage");
        //s-max-age don't exist, check max-age
        if(resp_maxage==-1){
            // std::cout<<"agent::getrespExpireTime() smax-age don't exist, check max-age"<<std::endl;
            resp_maxage = findage(resp_cacheControl,"max-age");
            //max-age don't exist, check Expire
            if(resp_maxage==-1){
                //std::cout<<"agent::getrespExpireTime() max-age don't exist, check Expires"<<std::endl;
                std::string Expirestr = resp.getField("Expires");
                //std::cout<<"agen  Start chech Expire"<<Expirestr<<std::endl;
                //check expiretime 
                if(Expirestr!=""){ 
                    //std::cout<<"agent::getrespExpireTime()  Expirestr exist: "<<Expirestr<<std::endl;
                    std::tm ExpirestrTm;
                    ExpirestrTm.tm_isdst = -1;
                    strptime(Expirestr.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &ExpirestrTm);  
                    //std::cout<<"agent::getrespExpireTime()  point 1 exist: "<<std::endl;
                    std::time_t Expires_time = std::mktime(&ExpirestrTm);
                    //std::cout<<"agent::getrespExpireTime() exprire_time"<<ctime(&Expires_time)<<std::endl;
       
                    return ctime(&Expires_time);
                }else{
                    std::cout<<"agent::getrespExpireTime()  expiretime don't exist, check max-age"<<std::endl;
                    return "";
                }       
            }           
        }
        time_t exprire_time = Date_time + resp_maxage;
        std::cout<<"agent::getrespExpireTime() find the exprire_time"<<ctime(&exprire_time)<<std::endl;
        return ctime(&exprire_time);  
	}
    std::string logwarn="No Date exist in the response";//log errornote
    std::cout<< ID<<": "<<logwarn;
    loginstance.logWarnMsg(ID,logwarn);
    //std::cout<<"agent::getrespExpireTime() response don't have exprire_time"<<std::endl;
    return "";  
}

int agent::respCurrentAge(HttpResponse resp,std::string ID){
    std::time_t now_time =time(0);  
    std::string Datestr = resp.getField("Date");//有没有可能Date不存在？
    if(!Datestr.empty()){
		std::size_t Date_end = Datestr.find(" GMT");
        std::string Date;
        if(Date_end!=std::string::npos){
		    Date = Datestr.substr(0,Date_end);
        }else{
            Date = Datestr;
        }
        std::tm dateTm;
        dateTm.tm_isdst = -1;
        if (strptime(Date.c_str(), "%a, %d %b %Y %H:%M:%S", &dateTm) == nullptr) {     
         //   std::cout<<": ERROR Failed to convert tm to time_t" << std::endl;//log errornote
          //  return -1;
        }          
        std::time_t Date_time = std::mktime(&dateTm);
        int current_age = static_cast<int>(now_time +18000 - Date_time);
         std::cout<<"agent::respCurrentAge() current age is" <<current_age<< std::endl;//log errornote
        return current_age;  
    }
    std::cout<<"agent::respCurrentAge() response Date not exist" << std::endl;//log errornote
    return -1;//Date is not exist
}

bool agent::checkMaxStale(HttpResponse resp,int req_max_stale,std::string ID){
    logger& loginstance = logger::getInstance();
    std::string respExpireTime = getrespExpireTime(resp,ID);
    if(respExpireTime.empty()){
        std::string Msg = "agent::checkMaxStale() failed get respExpireTime:" + respExpireTime;
        loginstance.logErrMsg(ID,Msg);
        std::cerr<<Msg<<std::endl;       
        return false;
    }
    std::tm ExpireTm;
    ExpireTm.tm_isdst = -1;
    strptime(respExpireTime.c_str(), "%a %b %d %H:%M:%S %Y", &ExpireTm);
        
    std::time_t Expire_time = std::mktime(&ExpireTm);
    std::time_t now_time =time(0); 
    int Expire_age = static_cast<int>(Expire_time-now_time);
            
    if(Expire_age>req_max_stale){
        std::cout<<"agent::checkMaxStale() :exceed" <<std::endl;
        return false;
    }
     std::cout<<"agent::checkMaxStale() : not exceed" <<std::endl;
    return true;
}

bool agent::checkrevalidation(std::string cacheControl){
    std::cout<<"Start checkrevalidation(): "<<cacheControl<<std::endl;
    if(!cacheControl.empty()){         
        if (cacheControl.find("must-revalidate")!=std::string::npos||cacheControl.find("proxy-revalidate")!=std::string::npos){//need revalid
            std::cout<<"This response need revalidate"<<std::endl;
            return true;
        }
    }
    std::cout<<"This response is valid and don't need revalidate"<<std::endl;
    return false;
}

bool agent::checkNoCache(std::string resp_cacheControl, std::string req_cacheControl){
    //check the exist NoCache in both request and response
        if(!resp_cacheControl.empty()){         
            if (resp_cacheControl.find("no-cache")!=std::string::npos){
                return true;
            }
        }
        else if(!req_cacheControl.empty()){         
            if (req_cacheControl.find("no-cache")!=std::string::npos){
                return true;
            }
        }
        return false;
}

int agent::findage(std::string cacheControl, std::string time_name){
    std::size_t agePos = cacheControl.find(time_name);
   //std::cout<<"findage(): start find: "<<cacheControl<<" .  "<< time_name<<std::endl;
    if (agePos!=std::string::npos){//s-maxage exist
        std:: string ageStr = cacheControl.substr(agePos);
       //std::cout<<"findage(): start find: "<< cacheControl<<std::endl;
        //std::cout<<"findage(): not age value: "<<ageStr[time_name.length()]<<std::endl;
        if(ageStr[time_name.length()]!= '='){
              return 0;
        }
        std::string agevalue;
          agevalue = ageStr.substr(time_name.length()+1);
        int age = atoi(agevalue.c_str());
        //std::cout<<"agent::findage(): "<<time_name<<" value is "<<agevalue<<". age: "<<age<<std::endl;
        return age;
    }
    //std::cout<<"findage(): time name don't exist"<<std::endl;
    return -1;
}


bool agent::checkCachebility(HttpResponse resp,std::string status,std::string ID){
    logger& loginstance = logger::getInstance();
    std::string cacheControl = resp.getField("Cache-Control");
    //Chache not cacheable: no-store/private
    if(!cacheControl.empty()){
        if(cacheControl.find("no-store")!=std::string::npos){
                std::string logline = ID+": not cacheable because no-store\n";//log 
                std::cout<<"agent::checkCachebility "<<logline<< std::endl;
                if(status == "200"){                                      
                    loginstance.logCacheCheckReq(logline);
                }
                return false;
        }
        else if(cacheControl.find("private")!=std::string::npos){
            std::string logline = ID+": not cacheable because private\n";//log    
            std::cout<<"cache::checkCachebility() "<<logline<<std::endl;
            if(status == "200"){                       
                loginstance.logCacheCheckReq(logline);
            }
            return false;
        }
        else if(cacheControl.find("no-cache")!=std::string::npos){
            std::string logline =ID+": cached, but requires re-validation";//log
            std::cout <<"cache::checkCachebility() "<<logline<<std::endl; //log  
            if(status == "200"){      
                loginstance.logCacheCheckReq(logline);
            }
            return true;
        }
    }
    //Cache if the response store: no-store/private
    std::string respExpireTime = getrespExpireTime(resp,ID);
    std::cout <<"respExpireTime: " << respExpireTime << std::endl;
    
    if(respExpireTime.empty()){//Expire time is unknow, need revalidation
        std::string logline = ID+": cached, but requires re-validation";//log 
        std::cout<<"agent::checkCachebility() Expire:"<<logline<< std::endl;
        if(status == "200"){                                 
            loginstance.logCacheCheckReq(logline);
        }
        return true;
    }
    std::tm expireTm;
    expireTm.tm_isdst = -1;
    strptime(respExpireTime.c_str(), "%a %b %d %H:%M:%S %Y", &expireTm);
    std::time_t now_time =time(0)+18000; 
    
    std::time_t Expire_time = std::mktime(&expireTm);//Expire_time may be invalid

    //std::cout <<"agent::checkfreshness()  strptime() Expire_time:"<<ctime(&Expire_time)<<"now: "<<ctime(&now_time)<<std::endl; 
    
    std::cout<<"agent::checkfreshness() in strptime() expireT m:"<<respExpireTime<<std::endl;

    std::cout<<"now_time: "<<ctime(&now_time)<<" Expire_time: "<<ctime(&Expire_time)<<std::endl;
    if(now_time>=Expire_time){ //Response stale     
        std::string logline = ID+": cached, but requires re-validation";//log 
        std::cout<<"agent::checkCachebility() "<<logline<< std::endl;
        if(status == "200"){                                 
            loginstance.logCacheCheckReq(logline);
        }                       
        return true;
    }   
    //Response is fresh 
    std::string logline = ID+": cached, expires at " + respExpireTime;//log 
    std::cout<<"agent::checkCachebility() "<<logline<< std::endl;
    if(status == "200"){                                 
        loginstance.logCacheCheckReq(logline);
    } 
    return true;
}
