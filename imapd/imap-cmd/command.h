
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "../imapd/client.h"

typedef int(*cmd_func)(struct client_t *client);

cmd_func command_find(char *name);

// Non-Authenticated State.
int cmd_authenticate(struct client_t *client);
int cmd_login(struct client_t *client);
int cmd_logout(struct client_t *client);

int cmd_capability(struct client_t *client);
int cmd_noop(struct client_t *client);
int cmd_id(struct client_t *client);

// Authenticateed State.
int cmd_select(struct client_t *client);
int cmd_examine(struct client_t *client);

int cmd_create(struct client_t *client);
int cmd_delete(struct client_t *client);
int cmd_rename(struct client_t *client);

int cmd_subscribe(struct client_t *client);
int cmd_unsubscribe(struct client_t *client);

int cmd_list(struct client_t *client);
int cmd_lsub(struct client_t *client);

int cmd_status(struct client_t *client);
int cmd_append(struct client_t *client);

// Selected State.
int cmd_check(struct client_t *client);
int cmd_close(struct client_t *client);
int cmd_expunge(struct client_t *client);
int cmd_fetch(struct client_t *client);
int cmd_search(struct client_t *client);
int cmd_store(struct client_t *client);
int cmd_copy(struct client_t *client);
int cmd_uid(struct client_t *client);

#endif // __COMMAND_H__

