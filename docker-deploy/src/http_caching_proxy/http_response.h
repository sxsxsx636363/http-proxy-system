#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse(std::vector<char>& resp);
    HttpResponse(const char* resp);
    ~HttpResponse() {}
    bool parse();
    void setRawdata(std::vector<char>& rawData);
    std::vector<char> getRawData() const { return rawData; }
    std::string getRawDataStr() const { return rawDataStr; }
    std::string getField(const std::string& field) const;
    void print() const;
    
private:
    std::vector<char> rawData;
    std::string rawDataStr;
    std::unordered_map<std::string, std::string> fields;
};

#endif