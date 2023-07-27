#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include "./lib/llhttp.h"

// Reference: https://github.com/theanarkh/llhttp-demo/blob/main/example/HTTP_Parser.h
#define MAX_LEN 2048

using namespace std;
class HTTP_Parser
{
public:
    HTTP_Parser(llhttp_type type) {
        llhttp_init(&parser, type, &HTTP_Parser::settings);
        // set data after llhttp_init, because llhttp_init will call memset to fill zero to memory
        parser.data = this;
    }

    int on_message_begin(llhttp_t *parser)
    {
        major_version = 0;
        minor_version = 0;
        upgrade = 0;
        keepalive = 0;
        parse_start_time = 0;
        header_end_time = 0;
        message_end_time = 0;
        url.clear();
        status.clear();
        keys.clear();
        values.clear();
        body.clear();

        parse_start_time = time(NULL);
        return 0;
    }

    int on_status(llhttp_t *parser, const char *at, size_t length)
    {
        status.append(at, length);
        return 0;
    }

    int on_method(llhttp_t *parser, const char *at, size_t length)
    {
        method.append(at, length);
        return 0;
    }

    int on_url(llhttp_t *parser, const char *at, size_t length)
    {
        url.append(at, length);
        return 0;
    }

    int on_header_field(llhttp_t *parser, const char *at, size_t length)
    {
        keys.push_back(string(at, length));
        return 0;
    }

    int on_header_value(llhttp_t *parser, const char *at, size_t length)
    {
        values.push_back(string(at, length));
        return 0;
    }

    int on_body(llhttp_t *parser, const char *at, size_t length)
    {
        body.append(at, length);
        return 0;
    }

    bool parse(const char *data, int len);

    string getMethod()
    {
        return method;
    }
    string getUrl()
    {
        return url;
    }
    string getStatus()
    {
        return status;
    }
    int getStatusCode() {
        return llhttp_get_status_code(&parser);
    }
    string getBody()
    {
        return body;
    }
    string getKeyAt(size_t at)
    {
        return keys[at];
    }
    string getValueAt(size_t at)
    {
        return values[at];
    }
    size_t getKvSize()
    {
        return min(keys.size(), values.size());
    }
    void printKeys() {
        for (int i = 0; i < keys.size(); ++i) {
            cout << keys[i] << endl;
        }
    }
    void printValues() {
        for (int i = 0; i < values.size(); ++i) {
            cout << values[i] << endl;
        }
    }

private:
    unsigned char major_version;
    unsigned char minor_version;
    unsigned char upgrade;
    unsigned char keepalive;
    time_t parse_start_time;
    time_t header_end_time;
    time_t message_end_time;
    string method;
    string url;
    string status;
    vector<string> keys;
    vector<string> values;
    string body;
    llhttp_t parser;
    static llhttp_settings_t settings;
};


#endif