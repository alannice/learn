
#ifndef __ADDR_PARSER_H__
#define __ADDR_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>

struct mail_addr_t {
    char data[128];
    char name[128];
    char route[128];
    char email[128];
    char domain[128];
};

int addr_parser_create(struct mail_addr_t *addr_parser, char *data);
void addr_parser_state(struct mail_addr_t *addr_parser);

#endif // __ADDR_PARSER_H__
