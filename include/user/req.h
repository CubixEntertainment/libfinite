#include "draw.h"
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct __attribute__((packed)) {
    int write_mode; // int reference to FiniteWriteModeFiniteRequest
    int data_type;
    char cmd[8];
    char adr[2048]; // ! required
    char token[1024]; // * optional (required for infinite API requests)
    char user_id[36]; // * optional (required for infinite API requests)
    char device_id[36]; // * optional (required for infinite API requests)
    char data[4096]; // ? optional
} FiniteIPCRequest;

typedef struct __attribute__((packed)) {
    int status;
    char msg[4096];
    char data[4096];
} FiniteIPCResponse;

enum FiniteAuthState {
    AUTH_STATE_PENDING,
    AUTH_STATE_SUCCESS,
    AUTH_STATE_CODE_EXPIRED,
    AUTH_STATE_ERROR
};

typedef struct {
    int id; // request id
    int game_id;
    int iat;
    enum FiniteAuthState auth_state;
    char *state;
    char *user_id;
    char *device_id;
    char *verify_code;
} FiniteAuthRequest;

typedef struct {
    char *device;
    char *code;
    char *state;
    enum FiniteAuthState auth_state;
    int game_id;
    int timestamp;
} FiniteAuthCode;

#define finite_user_request_auth(shell, id, token, callback) finite_user_request_auth_debug(__FILE__, __func__, __LINE__, shell, id, token, callback)
FiniteAuthRequest *finite_user_request_auth_debug(const char *file, const char *func, int line, FiniteShell *shell, char id[36], char token[64], void (*callback)(char *code));
