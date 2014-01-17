
#ifndef __ARGS_PARSER_H__
#define __ARGS_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IMAP_ARG_NIL  0
#define IMAP_ARG_ATOM 1
#define IMAP_ARG_STRING 2
#define IMAP_ARG_LITERAL 3
#define IMAP_ARG_LIST  4

struct ImapArgList_t;
struct ImapArg_t;

struct ImapArg_t {
    struct ImapArg_t *next;

    int type;
    char arg[1024];
};

struct ImapParser_t {
    char *pos;
    char argstring[1024];
    struct ImapArg_t args;
};

struct ImapParser_t *imap_parser_create(char *argstring);
void imap_parser_destroy(struct ImapParser_t *parser);
void imap_parser_reset(struct ImapParser_t *parser);


#endif // __ARGS_PARSER_H__

