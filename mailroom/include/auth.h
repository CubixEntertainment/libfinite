#include <finite/user.h>
// #include <finite/jsmn.h>
#include <curl/curl.h>
#include <libwebsockets.h>

typedef struct {
    pid_t owner; // pid of the device that has input focus (only one client at a time)
    int server_fd;
    int client_fd;
} FiniteNIPCServer;

enum FiniteConnectionState {
    CONNECTION_STATE_PENDING,
    CONNECTION_STATE_CONNECTED,
    CONNECTION_STATE_INITIAL,
    CONNECTION_STATE_DC,
};

typedef enum {
    FINITE_TYPE_PTR,
    FINITE_TYPE_LONG,
    FINITE_TYPE_VOID,
} FiniteOptType;


typedef struct {
    CURLoption code;
    FiniteOptType type;
    void *param;
} FiniteRequestGenericOption; // any param that can safely be void or casted automatically (like char params)

typedef struct {
    CURLoption code;
    size_t (*func)(char *, size_t, size_t, void *);
} FiniteRequestWriteFuncOption; // write callback

typedef struct {
    CURLoption code;
    long mode;
} FiniteRequestMethodOption; // mode

typedef struct {
    CURLoption code;
    char *adr;
} FiniteRequestAddressOption; // link

typedef struct {
    FiniteRequestAddressOption *link;
    FiniteRequestMethodOption *mode;
    FiniteRequestWriteFuncOption *write_callback;
    FiniteRequestGenericOption **gen_opts;
    int _gen_opts;
} FiniteRequest;

typedef struct {
    enum FiniteConnectionState state;
    int code;

    FiniteRequest req;
} FiniteConnection;


struct per_session_data {
    struct lws_context *ctx;
    char buffer[FILENAME_MAX]; 
    FiniteConnection cs;
};

int initializAPISocket();
void *handle_api();