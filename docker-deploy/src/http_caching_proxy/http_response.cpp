#include "http_response.h"
#include "http_parser.h"

HttpResponse::HttpResponse(std::vector<char>& resp): rawData(resp), rawDataStr(resp.begin(), resp.end()) {}

HttpResponse::HttpResponse(const char* resp): rawDataStr(resp) {
    rawData.resize(rawDataStr.size());
    for (int i = 0; i < rawDataStr.size(); ++i) {
        rawData[i] = rawDataStr[i];
    }
}

bool HttpResponse::parse() {
    HTTP_Parser parser(HTTP_RESPONSE);
    if (!parser.parse(rawDataStr.c_str(), strlen(rawDataStr.c_str()))) {
        return false;
    }
    fields["Method"] = parser.getMethod();
    fields["Url"] = parser.getUrl();
    fields["Body"] = parser.getBody();
    fields["Status"] = parser.getStatus();
    fields["StatusCode"] = std::to_string(parser.getStatusCode());
    for (size_t i = 0; i < parser.getKvSize(); ++i) {
        fields[parser.getKeyAt(i)] = parser.getValueAt(i);
    }
    return true;
}

void HttpResponse::setRawdata(std::vector<char>& rawData) {
    this->rawData = rawData;
}

std::string HttpResponse::getField(const std::string& field) const {
    if (!fields.count(field)) {
        return "";
    }
    return fields.at(field);
}

void HttpResponse::print() const {
    std::cout << "response rawdata" << std::endl;
    std::cout << rawDataStr << std::endl;
    std::cout << "response fields" << std::endl;
    unordered_map<std::string, std::string>::const_iterator it = fields.begin();
    for (; it != fields.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

