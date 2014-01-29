
#include "addr-parser.h"

int addr_parser_create(struct mail_addr_t *addr_parser, char *data)
{
    char *b, *e, *p;
    if(addr_parser == NULL || data == NULL) {
        return -1;
    }

    if(strlen(data) > sizeof(addr_parser->data)-1) {
        return -1;
    }

    bzero(addr_parser, sizeof(struct mail_addr_t));
    strcpy(addr_parser->data, data);

    b = addr_parser->data;
    p = strchr(b, '<');
    if(p) {
        strncpy(addr_parser->name, b, p-b);
        b=p;
        b++;

        p = strchr(b, ':');
        if(p) {
            strncpy(addr_parser->route, b, p-b);
            b = p;
            b++;
        }

        p = strchr(b, '@');
        if(p == NULL) {
            return -1;
        }
        strncpy(addr_parser->email, b, p-b);
        b = p;
        b++;

        p = strchr(b, '>');
        if(p == NULL) {
            return -1;
        }
        strncpy(addr_parser->domain, b, p-b);
    } else {
        p = strchr(b, '@');
        if(p == NULL) {
            return -1;
        }
        strncpy(addr_parser->email, b, p-b);
        b = p++;
        
        strcpy(addr_parser->domain, b);
    }

    return 0;
}

int addr_check(struct mail_addr_t *addr_parser)
{
    if(strlen(addr_parser->name)) {
    }

    return 0;
}

void addr_parser_state(struct mail_addr_t *addr_parser)
{
    if(addr_parser) {
        printf("data : %s\n", addr_parser->data);
        printf("name : %s\n", addr_parser->name);
        printf("route : %s\n", addr_parser->route);
        printf("email : %s\n", addr_parser->email);
        printf("domain : %s\n", addr_parser->domain);
    }

    return;
}

////////////////////////////////////////////////

#if 1

int main(void)
{
    char *email = "\"na me\" < route : \"ema il\"@sina.com > ";
    char *email2 = "<\"ema il\"@sina.com >";
    char *email3 = "<email@sina.com>";
    char *email4 = "\"na me\" <email@sina.com>";
    char *email5 = "aa bb < cc @ dd @ ee . dd oo>";
    int retval;

    struct mail_addr_t addr_parser;

    retval = addr_parser_create(&addr_parser, email);
    if(retval == -1) {
        printf("parse error\n");
    }
    addr_parser_state(&addr_parser);

    retval = addr_parser_create(&addr_parser, email2);
    if(retval == -1) {
        printf("parse error\n");
    }
    addr_parser_state(&addr_parser);

    retval = addr_parser_create(&addr_parser, email3);
    if(retval == -1) {
        printf("parse error\n");
    }
    addr_parser_state(&addr_parser);

    retval = addr_parser_create(&addr_parser, email4);
    if(retval == -1) {
        printf("parse error\n");
    }
    addr_parser_state(&addr_parser);

    retval = addr_parser_create(&addr_parser, email5);
    if(retval == -1) {
        printf("parse error\n");
    }
    addr_parser_state(&addr_parser);

    return 0;
}

#endif 

// end
