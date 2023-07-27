#include "http_parser.h"

bool HTTP_Parser::parse(const char *data, int len)
{
    llhttp_errno err = llhttp_execute(&parser, data, len);
    if (err != HPE_OK)
    {
        if (err == HPE_PAUSED_UPGRADE) {
            return true;
        }
        // fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err), parser.reason);
        return false;
    }
    return true;
}

llhttp_settings_t HTTP_Parser::settings = {
            .on_url = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_url(parser, data, len);
            },
            .on_status = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_status(parser, data, len);
            },
            .on_method = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_method(parser, data, len);
            },
            .on_header_field = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_header_field(parser, data, len);
            },
            .on_header_value = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_header_value(parser, data, len);
            },
            .on_body = [](llhttp_t *parser, const char *data, size_t len)
            {
                return ((HTTP_Parser *)parser->data)->on_body(parser, data, len);
            },
};