
#include "args-parser.h"

int imap_parser_arg(struct ImapParser_t *parser);
int imap_parser_read_atom(struct ImapParser_t *parser);
int imap_parser_read_string(struct ImapParser_t *parser);
int imap_parser_read_list(struct ImapParser_t *parser);
int imap_parser_read_literal(struct ImapParser_t *parser);
int imap_parser_save_arg(struct ImapParser_t *parser, int type, char *data, int len);

struct ImapParser_t *imap_parser_create(char *data)                            
{
    if(data == NULL) {
        return NULL;
    }

    struct ImapParser_t *parser = malloc(sizeof(struct ImapParser_t));
    if(parser == NULL) {
        return NULL;
    }

    parser->pos = parser->data;
    strncpy(parser->data, data, sizeof(parser->data)-1);

    int retval = imap_parser_arg(parser);
    if(retval == -1) {
        imap_parser_destroy(parser);
        return NULL;
    }

    return parser;
}

void imap_parser_destroy(struct ImapParser_t *parser) 
{
    if(parser) {
        imap_parser_reset(parser);        
        free(parser);
    }

    return;
}

void imap_parser_reset(struct ImapParser_t *parser)                            
{
    if(parser && parser->args) {
        struct ImapArg_t *arg;
        while(parser->args) {
            arg=parser->args->next;
            free(parser->args);
            parser->args = arg;
        } 
    }

    return;
}

int imap_parser_arg(struct ImapParser_t *parser)    
{
    while(*parser->pos) {
        while(isspace(*parser->pos)) parser->pos++;
        if(!(*parser->pos)) break;

        switch(*parser->pos) {
            case '"':
                imap_parser_read_string(parser);
                break;
            case '(':
                imap_parser_read_list(parser);
                break;
            case '{':
                imap_parser_read_literal(parser);
                break;
            default:
                imap_parser_read_atom(parser);
                break;
        }
    }

    return 0;
}

int imap_parser_read_atom(struct ImapParser_t *parser)
{
    int in_bracket = 0; 
    char *data = parser->pos;

    while(*data++) {
        if(*data == '[') { 
            in_bracket++;
            continue;
        }
        if(in_bracket > 1) {
            return -1;
        }
        if(*data == ']') {
            in_bracket--;
            continue;
        }
        if(in_bracket < 0) {
            return -1;
        }
        if(*data == ' ' && in_bracket == 0) {
            break;
        }
    }
    
    imap_parser_save_arg(parser, IMAP_ARG_ATOM, parser->pos, data-parser->pos);

    if(data) parser->pos = data;

    return 0;
}

int imap_parser_read_string(struct ImapParser_t *parser)
{
    char *data = strchr(parser->pos+1, '"');
    if(data == NULL) {
        return -1;
    }

    imap_parser_save_arg(parser, IMAP_ARG_STRING, parser->pos, data-parser->pos);

    parser->pos = data;
    
    return 0;
}

int imap_parser_read_list(struct ImapParser_t *parser) 
{
    int in_bracket = 0; 
    char *data = parser->pos;

    for(; *data; data++) {
        if(*data == '(') { 
            in_bracket++;
            continue;
        }
        if(in_bracket > 2) {
            return -1;
        }
        if(*data == ')') {
            in_bracket--;
            continue;
        }
        if(in_bracket < 0) {
            return -1;
        }
        if(*data == ' ' && in_bracket == 0) {
            break;
        }
    }

    imap_parser_save_arg(parser, IMAP_ARG_LIST, parser->pos, data-parser->pos); 
    if(data) parser->pos = data;
    
    return 0;
}

int imap_parser_read_literal(struct ImapParser_t *parser) 
{
    int in_bracket = 0; 
    char *data = parser->pos;

    while(*data++) {
        if(*data == '{') { 
            in_bracket++;
            continue;
        }
        if(in_bracket > 1) {
            return -1;
        }
        if(*data == '}') {
            in_bracket--;
            continue;
        }
        if(in_bracket < 0) {
            return -1;
        }
        if(*data < '0' || *data > '9') {
            return -1;
        }
        if(*data == ' ' && in_bracket == 0) {
            break;
        }
    }

    imap_parser_save_arg(parser, IMAP_ARG_LITERAL, parser->pos, data-parser->pos);
    if(data) parser->pos = data;

    return 0;
}

int imap_parser_save_arg(struct ImapParser_t *parser, int type, char *data, int len)
{
    if(parser == NULL || data == NULL) {
        return -1;
    }

    struct ImapArg_t *arg = malloc(sizeof(struct ImapArg_t));
    if(arg == NULL) {
        return -1;
    }
    if(sizeof(arg->data)-1 < len) return -1;

    if(parser->args == NULL) {
        parser->args = arg;
    } else {
        struct ImapArg_t *tmparg = parser->args;
        while(tmparg->next) {
            tmparg = tmparg->next;
        }
        tmparg->next = arg;
    }
    arg->type = type;
    strncpy(arg->data, data, len);

    return 0;
}

char *imap_parser_get_arg(struct ImapParser_t *parser, int idx)
{
    int i;
    struct ImapArg_t *args;

    if(parser == NULL || parser->args == NULL) {
        return NULL;
    }

    args = parser->args;
    for(i=0; args; i++) {
        if(i == (idx-1)) {
            return args->data;
        }

        args = args->next;
    }
    
    return NULL;
}

void imap_parser_state(struct ImapParser_t *parser)
{

    if(parser) {
        printf("string:%s\n", parser->data);

        struct ImapArg_t *arg = parser->args;
        while(arg) {
            printf("type: %d, string:%s\n", arg->type, arg->data);
            arg=arg->next;
        }
    }

    return;
}

////////////////////////////////////////////////

#if 0

int main(void)
{
    // UID FETCH 604,605,606,607,608,609,610,611,612,613 (UID FLAGS INTERNALDATE RFC822.SIZE BODY.PEEK[HEADER.FIELDS (date subject from content-type to cc message-id)])
    char *data = "610,611,612,613 (UID FLAGS INTERNALDATE RFC822.SIZE BODY.PEEK[HEADER.FIELDS (date subject from content-type to cc message-id)])";

    struct ImapParser_t *parser = imap_parser_create(data);     
    if(parser == NULL) {
        printf("imap_parser_create() error\n");
        return;
    }

    imap_parser_state(parser);

    printf("args 1 : %s\n", imap_parser_get_arg(parser, 1));

    imap_parser_destroy(parser);

    return;
}

#endif 

// end

