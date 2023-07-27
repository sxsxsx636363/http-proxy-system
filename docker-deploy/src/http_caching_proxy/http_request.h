#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <atomic>
#include <boost/asio.hpp>

class HttpRequest {
public:
    HttpRequest(std::vector<char> req);
    ~HttpRequest() {}
    bool parse();
    std::vector<char> getRawData() const { return rawData; }
    std::string getRawDataStr() const { return rawDataStr; }
    std::string getField(const std::string& field) const;
    void print() const;

    static std::atomic<unsigned int> unique_id;

    void setRawdata(std::vector<char>& rawData);

private:
    std::vector<char> rawData;
    std::string rawDataStr;
    std::unordered_map<std::string, std::string> fields;
};
#endif