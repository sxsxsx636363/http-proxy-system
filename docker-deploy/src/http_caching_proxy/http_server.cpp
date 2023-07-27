#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <pthread.h>
#include "http_server.h"

#define PORT "12345"
#define BACKLOG 10

#define OK_200 "HTTP/1.1 200 OK\r\n\r\n"
#define BAD_REQUEST_400 "HTTP/1.1 400 Bad Request\r\n\r\n"
#define BAD_GATEWAY_502 "HTTP/1.1 502 Bad Gateway\r\n\r\n"

HttpServer::HttpServer() {
    init();
}

HttpServer::~HttpServer() {}

void HttpServer::init() {
    struct addrinfo hints, *serverinfo, *p;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int yes = 1;

    int rv = 0;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0) {
        perror("getaddrinfo");
        std::cerr << "HttpServer::init() getaddrinfo() error: " << gai_strerror(rv) << std ::endl;
    }

    for (p = serverinfo; p != NULL; p = p->ai_next) {
        sockFd = std::make_shared<SocketWrapper>(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (getSockVal(sockFd) == -1) {
            perror("socket");
            std::cerr << "HttpServer::init() socket() error" << std::endl;
            continue;
        }
        if (setsockopt(getSockVal(sockFd), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("setsockopt");
            std::cerr << "HttpServer::init() setsockopt() error" << std::endl;
            continue;
        }
        if (bind(getSockVal(sockFd), p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind");
            std::cerr << "HttpServer::init() bind() error" << std::endl;
            continue;
        }
        break;
    }
    freeaddrinfo(serverinfo);
    if (!p) {
        std::cout << "HttpServer::init() error" << std::endl;
    }
}

void HttpServer::startServer() {
    if (listen(getSockVal(sockFd), BACKLOG) == -1) {
        perror("listen");
        std::cerr << "HttpServer::startServer listen() error" << std::endl;
        return;
    }
    struct sockaddr_storage remote_addr;
    socklen_t sin_size = sizeof(remote_addr);
    while (true) {
        socketPtr newSockFd = std::make_shared<SocketWrapper>(accept(getSockVal(sockFd), (struct sockaddr*)&remote_addr, &sin_size));
        if (getSockVal(newSockFd) == -1) {
            std::cerr << "HttpServer::startServer() accept() error" << std::endl;
            continue;
        }
        std::thread t(&HttpServer::process, this, newSockFd);
        t.detach();
    }
}

void HttpServer::process(socketPtr newSockFd) {  
    HttpRequestPtr req = recvRequest(newSockFd);
    logger& loginstance = logger::getInstance();
    if (!req) {
        std::cerr << "HttpServer::process() recvRequest" << std::endl;
        return;
    }
    if (!req->parse()) {
        std::cerr << "HttpServer::process() HttpRequest parse error" << std::endl;
        HttpResponsePtr resp_err = std::make_shared<HttpResponse>(BAD_REQUEST_400);
        if (!sendResponse(resp_err, newSockFd)) {
            std::cerr << "HttpServer::process() sendResponse() error" << std::endl;
        }
        return;
    }
    
    // req->print();
    if (req->getField("Method") == "CONNECT") {
        process_connect(req, newSockFd);
        return;
    }
    else {
        //loginstance.logServRecvReq(*req);
        std::cout << "start::getCorrespondResp()"<<endl;
        agent& agentinstance = agent::getInstance();
        
        HttpClient client;    
        std::string ID = req->getField("ID");
        if(!req->getRawDataStr().empty()){
            loginstance.logServRecvReq(*req); 
        }
         
        HttpResponsePtr resp = agentinstance.getCorrespondResp(req,client);
        if(resp==NULL){
            std::string logline = "HttpServer::process() HttpClient error within getCorrespondResp()";//log errornote
            std::cout <<logline<<std::endl; //log 
            loginstance.logErrMsg(ID,logline); 
            //std::cerr << "HttpServer::process() HttpClient error within getCorrespondResp()" << std::endl;
            return;
        }
        /*
        HttpClient client;
        if (!client.init(req)) {
            std::cerr << "HttpServer::process() HttpClient init() error" << std::endl;
            return;
        }        
        if (!client.sendRequest(req)) {
            std::cerr << "HttpServer::process() HttpClient sendRequest() error" << std::endl;
            return;
        }
        HttpResponsePtr resp = client.recvResponse();
        */
        if (!resp) {
            std::string logline = "HttpServer::process() client.recvResponse error";//log errornote
            std::cout <<logline<<std::endl; //log 
            loginstance.logErrMsg(ID,logline); 
            //std::cerr << "HttpServer::process() client.recvResponse error" << std::endl;
            return;
        }
        if (!resp->parse()) {
            std::string logline = "HttpServer::process() HttpResponse parse error";//log errornote
            std::cout <<logline<<std::endl; //log 
            loginstance.logErrMsg(ID,logline); 
            //std::cerr << "HttpServer::process() HttpResponse parse error" << std::endl;
            HttpResponsePtr resp_err = std::make_shared<HttpResponse>(BAD_GATEWAY_502);
            if (!sendResponse(resp_err, newSockFd)) {
                std::cerr << "HttpServer::process() sendResponse() error" << std::endl;
                std::string logline = "HttpServer::process() sendResponse() error";//log errornote
                std::cout <<logline<<std::endl; //log 
                loginstance.logErrMsg(ID,logline); 
            }
            loginstance.logServSendResp(*resp_err,ID);
        }
        // resp->print();
        if (resp->getField("Transfer-Encoding") == "chunked") {
            process_chunked(client, resp, newSockFd);
            return;
        }
        if (!sendResponse(resp, newSockFd)) {
            std::cerr << "HttpServer::process() sendResponse() error" << std::endl;
            std::string logline = "HttpServer::process() sendResponse() error";//log errornote
            std::cout <<logline<<std::endl; //log 
            //std::cerr << "HttpServer::process() sendResponse() error" << std::endl;
        }
        loginstance.logServSendResp(*resp,ID);
    }
}

bool HttpServer::sendResponse(HttpResponsePtr resp, socketPtr newSockFd) {
    int total = 0;
    int numBytes = 0;
    std::vector<char> buffer = resp->getRawData();
    int len = buffer.size();
    while (total < len) {
        numBytes = send(getSockVal(newSockFd), &buffer.data()[total], len - total, 0);
        if (numBytes < 0) {
            return false;
        }
        total += numBytes;
    }
    return true;
}

HttpRequestPtr HttpServer::recvRequest(socketPtr newSockFd) {
    std::vector<char> buffer(65536, 0);
    int numbytes = recv(getSockVal(newSockFd), &(buffer.data()[0]), buffer.size(), 0);
    if (numbytes < 0) {
        std::cerr << "HttpServer::recvRequest() recv error" << std::endl;
        return std::shared_ptr<HttpRequest>(nullptr);
    }
    buffer.resize(numbytes);
    return std::make_shared<HttpRequest>(buffer);
}

void HttpServer::process_connect(HttpRequestPtr req, socketPtr newSockFd) {
    HttpClient client;
    socketPtr clientSockFd = client.init(req);
    if (!clientSockFd) {
        return;
    }
    fd_set listened;
    fd_set readfds;
    FD_ZERO(&listened);
    FD_ZERO(&readfds);
    FD_SET(getSockVal(newSockFd), &listened);
    FD_SET(getSockVal(clientSockFd), &listened);
    int fdmax = std::max(getSockVal(newSockFd), getSockVal(clientSockFd));
    HttpResponsePtr okResp = std::make_shared<HttpResponse>(OK_200);
    if (!sendResponse(okResp, newSockFd)) {
        std::cerr << "HttpServer::connect() sendResponse error" << std::endl;
        return;
    }
    struct timeval tv;
    // timeout: 2s
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    while (true) {
        readfds = listened;
        if (select(fdmax + 1, &readfds, NULL, NULL, &tv) == -1) {
            perror("select");
            std::cout << "select() error" << std::endl;
            return;
        }
        bool timeout = true;
        if (FD_ISSET(getSockVal(newSockFd), &readfds)) {
            HttpRequestPtr request = recvRequest(newSockFd);
            if (!request) {
                std::cerr << "HttpServer:connect() recvRequest error" << std::endl;
                return;
            }           
            if (!client.sendRequest(request)) {
                std::cerr << "HttpServer::connect() client.sendRequest error" << std::endl;
                return;
            }
            timeout = false;
        }
        else if (FD_ISSET(getSockVal(clientSockFd), &readfds)) {
            HttpResponsePtr resp = client.recvResponse();
            if (!resp) {
                std::cerr << "HttpServer::connect() client.recvResponse error" << std::endl;
                return;
            }
            if (!sendResponse(resp, newSockFd)) {
                std::cerr << "HttpServer::connect() sendResponse error" << std::endl;
                return;
            }
            timeout = false;
        }
        if (timeout) {
            logger& loginstance = logger::getInstance();
            loginstance.logTunnelClose(req->getField("ID"));
            return;
        }
    }
}

void HttpServer::process_chunked(HttpClient& client, HttpResponsePtr resp, socketPtr newSockFd) {
    while (true) {
        if(resp==NULL){
            std::cerr << "HttpServer::process() HttpClient error within getCorrespondResp()" << std::endl;
            return;
        }
        if (!sendResponse(resp, newSockFd)) {
            std::cerr << "HttpServer::chunked() sendResponse() error" << std::endl;
            return;
        }
        resp = client.recvResponse();
        if (!resp) {
            std::cerr << "HttpServer::chunked() client.recvResponse error" << std::endl;
            return;
        }
        if (resp->getRawData().size() == 0) {
            break;
        }
        
    }
}
