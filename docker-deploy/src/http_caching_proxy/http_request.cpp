#include "http_request.h"
#include "http_parser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

std::atomic<unsigned int> HttpRequest::unique_id(0);
void HttpRequest::setRawdata(std::vector<char>& rawData) {
    this->rawData = rawData;
}
std::string getIpFromHost(const std::string& host) {
    std::string addr = host;
    std::string port = "80";
    int colon = addr.find(':');
    if (colon != std::string::npos) {
        port = addr.substr(colon + 1);
        addr = addr.substr(0, colon);
    }
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(addr, port);
    boost::asio::ip::tcp::resolver::iterator iter;
    try {
        iter = resolver.resolve(query);
    }
    catch (boost::wrapexcept<boost::system::system_error> e) {
        return "";
    }
    // std::cout << "ip: " << iter->endpoint().address().to_string() << std::endl;
    return iter->endpoint().address().to_string();
}

HttpRequest::HttpRequest(std::vector<char> req): rawData(req), rawDataStr(req.begin(), req.end()) {}

bool HttpRequest::parse() {
    HTTP_Parser parser(HTTP_REQUEST);
    if (!parser.parse(rawDataStr.c_str(), strlen(rawDataStr.c_str()))) {
        return false;
    }
    HttpRequest::unique_id++;
    fields["ID"] = std::to_string(HttpRequest::unique_id);
    fields["Method"] = parser.getMethod();
    fields["Url"] = parser.getUrl();
    fields["Body"] = parser.getBody();
    for (size_t i = 0; i < parser.getKvSize(); ++i) {
        fields[parser.getKeyAt(i)] = parser.getValueAt(i);
    }
    fields["IP"] = getIpFromHost(fields["Host"]);
    return true;
}

std::string HttpRequest::getField(const std::string& field) const {
    if (!fields.count(field)) {
        return "";
    }
    return fields.at(field);
}

void HttpRequest::print() const {
    std::cout << "request rawdata" << std::endl;
    std::cout << rawDataStr << std::endl;
    std::cout << "request fields" << std::endl;
    unordered_map<std::string, std::string>::const_iterator it = fields.begin();
    for (; it != fields.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}
