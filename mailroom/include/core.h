#include <stdlib.h>
#include <curl/curl.h>

typedef struct Request Request;

typedef struct {
    pid_t owner; // pid of the device that has input focus (only one client at a time)
    int server_fd;
    int client_fd;
} FiniteIPCServer;

struct MemoryStruct{
  char *memory;
  size_t size;
};

struct Request{
    char* jsonObj;
    CURLcode resCode;
    struct MemoryStruct res;
};

static size_t mem_cb(char *data, size_t size, size_t nsize, void *clientp);

// #define finite_user_request_auth(request) finite_user_request_auth_debug(__FILE__, __func__, __LINE__, request)
// int finite_user_request_auth_debug(const char *file, const char *func, int line, Request request);