#include <stdbool.h>

// actual player structs
typedef struct {
    char user[20];
    char display[32];
    char user_id[36]; // user's uuid not the index id
    char *bio; // forgor the limit on bio
    char *banner_link;
    int last_online;
    bool isOnline;
    bool isMod;
} FiniteUser;

#define finite_user_get_by_id(user_id, token, dev) finite_user_get_by_id_debug(__FILE__, __func__, __LINE__, user_id, token, dev)
FiniteUser *finite_user_get_by_id_debug(const char *file, const char *func, int line, char *user_id, char* token, char *dev);