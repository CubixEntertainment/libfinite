#define JSMN_HEADER
#include "user.h"
#include "log.h"
#include "json.h"
#include <stdio.h>
#include <string.h>

static void send_signal(int fd, FiniteIPCRequest req) {
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    char *path = getenv("FINITE_IPC_MAIN_DIR");
    if (!path) {
        FINITE_LOG_WARN("Unable to open the FINITE_IPC_MAIN_DIR socket.");
        close(fd);
        return;
    }

    strcpy(addr.sun_path, path);
    
    if (connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == 0) {
        send(fd, &req, sizeof(FiniteIPCRequest), 0);
    } else {
        perror("connect");
    }
}

FiniteUser *finite_user_get_by_id_debug(const char *file, const char *func, int line, char *user_id, char* token, char *dev) {
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);

    FiniteIPCRequest req = {
        .write_mode = 1, // use USERDATA write mode to get data formatted correctly
        .data_type = 0,
        .cmd = "POST",
        .adr = "https://api.cubixdev.org/user"
    };

    strncpy(req.token, token, 1024);
    strncpy(req.user_id, user_id, 36);
    strncpy(req.device_id, dev, 36);
    snprintf(req.data, 4096, "{\n\"user_id\":\"%s\",\n\"token\":\"%s\",\n\"device_id\":\"%s\"}", user_id, token, dev);

    send_signal(fd, req);

    FiniteIPCResponse response;
    ssize_t n = recv(fd, &response, sizeof(FiniteIPCResponse), 0);
    if (n == 0) {
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Buffer is 0. (Request closed?).");
        close(fd);
        return NULL;
    } else if (n < 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong.");
        perror("recv");
        close(fd);
        return NULL;
    } else {
        FINITE_LOG("Data %s", response.data);
    }

    FiniteUser *usr = calloc(1, sizeof(FiniteUser));
        
    FiniteJSONValue *json = finite_json_parse(response.data);
    if (!json) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong while parsing data.");
        finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
        close(fd);
        return NULL;
    }

    char *name = finite_json_get_value_from_key(json, "username");
    char *id = finite_json_get_value_from_key(json, "id");
    char *display = finite_json_get_value_from_key(json, "display_name");
    char *bio = finite_json_get_value_from_key(json, "bio");
    char *banner = finite_json_get_value_from_key(json, "bio");
    char *is_online = finite_json_get_value_from_key(json, "is_online");
    char *is_mod = finite_json_get_value_from_key(json, "is_moderator");
    int last_online = atoi(finite_json_get_value_from_key(json, "last_online"));

    // TODO: If any values are empty strings set them to NULL


    strncpy(usr->user, name, sizeof(usr->user) - 1);
    usr->user[sizeof(usr->user)-1] = '\0';
    strncpy(usr->user_id, id, sizeof(usr->user_id) - 1);
    usr->user_id[sizeof(usr->user_id)-1] = '\0';
    strncpy(usr->display, display, sizeof(usr->display) - 1);
    usr->display[sizeof(usr->display)-1] = '\0';

    if (bio) {
        usr->bio = malloc(strlen(bio) + 1);
        strcpy(usr->bio, bio);
    } else {
        usr->bio = NULL;
    }

    if (banner) {
        usr->banner_link = malloc(strlen(banner) + 1);
        strcpy(usr->banner_link, banner);
    } else {
        usr->banner_link = NULL;
    }

    usr->isOnline = strcmp(is_online, "true") == 0 ? true : false;
    usr->isMod = strcmp(is_mod, "true") == 0 ? true : false;

    usr->last_online = last_online;

    FINITE_LOG("Name: %s", usr->user);

    return usr;
}