#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <string>
#include <memory>

#include "http_client.h"

socketPtr HttpClient::init(HttpRequestPtr req) {
    std::string addr = req->getField("Host");
    std::string port = "80";
    int colon = addr.find(':');
    if (colon != std::string::npos) {
        port = addr.substr(colon + 1);
        addr = addr.substr(0, colon);
    }
    struct addrinfo hints, *serverinfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(addr.c_str(), port.c_str(), &hints, &serverinfo) != 0) {
        std::cout << "HttpClient::init() getaddrinfo() error addr " << addr << std::endl;
        return std::shared_ptr<SocketWrapper>(nullptr);
    }

    for (p = serverinfo; p != NULL; p = p->ai_next) {
        sockFd = std::make_shared<SocketWrapper>(socket(p->ai_family, p->ai_socktype, p->ai_protocol));
        if (getSockVal(sockFd) == -1) {
            std::cout << "HttpClient::init() socket() error" << std::endl;
            continue;
        }
        if (connect(getSockVal(sockFd), p->ai_addr, p->ai_addrlen) == -1) {
            close(getSockVal(sockFd));
            std::cout << "HttpClient::init() connect() error" << std::endl;
            continue;
        }
        break;
    }

    if (!p) {
        std::cout << "HttpClient: init() error" << std::endl;
        return std::shared_ptr<SocketWrapper>(nullptr);
    }

    freeaddrinfo(serverinfo);
    return sockFd;
}

bool HttpClient::sendRequest(HttpRequestPtr req) {
    int total = 0;
    int numBytes = 0;
    std::vector<char> buffer = req->getRawData();
    int len = buffer.size();
    while (total < len) {
        numBytes = send(getSockVal(sockFd), &buffer.data()[total], len - total, 0);
        if (numBytes < 0) {
            perror("send");
            std::cout << "HttpClient::sendRequest send() error" << std::endl;
            return false;
        }
        total += numBytes;
    }
    return true;
}

HttpResponsePtr HttpClient::recvResponse() {
    std::vector<char> buffer(65536, 0);
    int total = 0;
    int numbytes = 0;
    numbytes = recv(getSockVal(sockFd), &(buffer.data()[total]), buffer.size(), 0);
    total += numbytes;
    if (numbytes < 0) {
        std::cerr << "HttpClient::recvRequest() recv error" << std::endl;
        return std::shared_ptr<HttpResponse>(nullptr);
    }
    HttpResponsePtr resp = std::make_shared<HttpResponse>(buffer);
    resp->parse();
    // std::cout << "Content-Encoding " << resp->getField("Content-Encoding") << std::endl; 
    if (!resp->getField("Content-Encoding").empty()) {
        return resp;
    }
    if (!resp->getField("Content-Length").empty()) {
        int content_length = stoi(resp->getField("Content-Length"));
        while (resp->getField("Body").size() < content_length) {
            numbytes = recv(getSockVal(sockFd), &(buffer.data()[total]), buffer.size(), 0);
            if (numbytes == 0) {
                break;
            }
            total += numbytes;
            resp = std::make_shared<HttpResponse>(buffer);
            resp->parse();
        }
    }
    buffer.resize(total);
    return std::make_shared<HttpResponse>(buffer);
}