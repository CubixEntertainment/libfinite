#include "../include/log.h"
#include "../include/auth.h"
#include <finite/jsmn.h>


static FiniteNIPCServer server = {0};


static void send_signal(int fd, FiniteIPCResponse res) {
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;

    char *device = getenv("FINITE_IPC_MAIN_DIR");
    if (!device) {
        FINITE_LOG_FATAL("Attempting to send message to undefined socket");
    }
    
    struct stat dev;
    if (stat(device, &dev) != 0) {
        FINITE_LOG_ERROR("Something went wrong while attempting push a message");
        perror("stat");
    }

    if (!S_ISSOCK(dev.st_mode)) {
        FINITE_LOG_ERROR("Unable to connect to non-socket device %s", device);
    }

    if (!device) {
        FINITE_LOG_WARN("Unable to open the FINITE_IPC_MAIN_DIR socket.");
        close(fd);
        return;
    }

    strcpy(addr.sun_path, device);

    if (send(fd, &res, sizeof(FiniteIPCResponse), 0) != sizeof(res)) {
        perror("send");
    }
}

// simple wrapper that will be called when the api watcher is first started
CURLcode init_api() {
    CURLcode res;
    res = curl_global_init(CURL_GLOBAL_SSL);
    return res;
}


FiniteRequest make_ws_request(char *adr) {
    FiniteRequest req = {0};
    req.link = calloc(1, sizeof(FiniteRequestAddressOption));
    req.link->adr = malloc(strlen(adr) + 1);
    strcpy(req.link->adr, adr);
    req.link->code = CURLOPT_URL;
    req.mode = NULL;
    req.write_callback = NULL;
    req.gen_opts = NULL;
    req._gen_opts = 0;

    return req;
}

CURLcode add_request_body(FiniteRequest *req, char *data) {
    FiniteRequestGenericOption **temp = realloc(req->gen_opts, sizeof(FiniteRequestGenericOption *) * (req->_gen_opts + 1));
    if (temp == NULL) {
        // do nothing
        return CURLE_NOT_BUILT_IN;
    }

    FiniteRequestGenericOption *next = calloc(1, sizeof(FiniteRequestGenericOption));
    next->code = CURLOPT_POSTFIELDS;
    next->param = (void *) data;

    req->gen_opts = temp;
    req->gen_opts[req->_gen_opts] = next;
    req->_gen_opts += 1;

    return CURLE_OK;
}

FiniteIPCResponse WS_MSG(char *data) {
    // decode JSON and send it back to the client
    FiniteIPCResponse res = {0};
    
    jsmn_parser p;
    jsmntok_t t[128];

    jsmn_init(&p);
    int r = jsmn_parse(&p, data, strlen(data), t, 128);
    if (r <= 0) {
        FINITE_LOG("Unable to parse");
    }
    
    int opt = 0; // 1 is status, 2 is msg, 3 is data

    for (int i = 0; i < r; i++) {
        int len = t[i].end - t[i].start;
        char item[len + 1];
        strncpy(item, data + t[i].start, len);
        item[len] = '\0';
        // FINITE_LOG("%s", item);

        if (strcmp(item, "error") == 0 || strcmp(item, "msg") == 0) {
            if (opt != 0 ) {
                // assign the value to the correct opt
                switch (opt) {
                    case 1:
                        res.status = 0;
                        break;
                    case 3:
                        strcpy(res.data, "");
                        break;
                }
            }

            // FINITE_LOG("err bool or msg found.");

            opt = 2;
        } else if (strcmp(item, "status") == 0 || strcmp(item, "code") == 0 ){
            if (opt != 0 ) {
                // assign the value to the correct opt
                switch (opt) {
                    case 2:
                        strcpy(res.msg, "");
                        break;
                    case 3:
                        strcpy(res.data, "");
                        break;
                }
            }

            // FINITE_LOG("status found.");


            opt = 1;
        } else if (strcmp(item, "data") == 0 ){
            if (opt != 0 ) {
                // assign the value to the correct opt
                switch (opt) {
                    case 2:
                        strcpy(res.msg, "");
                        break;
                    case 1:
                        res.status = 0;
                        break;
                }
            }

            // FINITE_LOG("data found.");

            opt = 3;
        } else {
            if (opt != 0 ) {
                // assign the value to the correct opt
                switch (opt) {
                    case 1:
                        res.status = atoi(item);
                        break;
                    case 2:
                        strcpy(res.msg, item);
                        break;
                    case 3:
                        strcpy(res.data, item);
                        break;
                }

                opt = 0;
            }
        }
    }

    FINITE_LOG("Status: %d, Msg: %s, Data: %s", res.status, res.msg, res.data);

    send_signal(server.client_fd, res);

    return res;
}

int WS_HANDLE(struct lws *wsi, enum lws_callback_reasons reason, void *data, void *in, size_t len) {
    struct per_session_data *session = (struct per_session_data *) data;
    if (!session) {
        FINITE_LOG("session_data not ready yet.");
    }

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            FINITE_LOG("Connected.");
            session->cs.state = CONNECTION_STATE_INITIAL;
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            FINITE_LOG("Msg: %s", (char *)in);
            FiniteIPCResponse res = WS_MSG((char *) in);
            if (strcmp(res.data, "Hello from the server") == 0) {
                session->cs.state = CONNECTION_STATE_CONNECTED;
                FINITE_LOG("Session info: \n\tState: %d\n\tItems: %d", session->cs.state, session->cs.req._gen_opts);

                // send out the followup message for auth
                FiniteRequest opts = session->cs.req;

                size_t len;
                unsigned char *send_buffer;
                for (int i = 0; i < opts._gen_opts; i++) {
                    if (opts.gen_opts[i]->code == CURLOPT_POSTFIELDS) {
                        send_buffer = (unsigned char *) opts.gen_opts[i]->param;
                        len = strlen(opts.gen_opts[i]->param);
                    }
                }

                if (send_buffer == NULL) {
                    FINITE_LOG_ERROR("Unable to send empty request.");
                    break;
                } else {
                    lws_write(wsi, send_buffer, len, LWS_WRITE_TEXT);
                }
            }

            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            FINITE_LOG_ERROR("Something went wrong.");
            perror("lws");
            break;
        case LWS_CALLBACK_CLOSED:
            FINITE_LOG("Closed.");
            session->cs.state = CONNECTION_STATE_DC;
            break;
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            FINITE_LOG("Headers here");
            break;
        default:
            break;
    }

    return 0;
}

void *WS_RUNTIME(void *data) {
    struct per_session_data *session = (struct per_session_data *) data;
    if (!session) {
        FINITE_LOG_ERROR("Unable to maintain runtime focus when no data is provided");
        return NULL;
    } else {
        struct lws_context *context = session->ctx;
        while (session->cs.state != CONNECTION_STATE_DC && server.client_fd != 0) {
            lws_service(context, 1000);
        }

        FINITE_LOG("Running cleanup");

        lws_context_destroy(context);
        return session->buffer;
    }
}

void *WS_INIT(FiniteRequest options) {
    struct lws_context_creation_info info = {0};
    struct lws_client_connect_info i = {0};
    struct lws_context *context;
    struct lws *wsi;

    // create a connection state to keepalive the socket
    FiniteConnection cs = {
        .state = CONNECTION_STATE_PENDING,
        .req = options
    };

    FINITE_LOG("items: %d", cs.req._gen_opts);

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.protocols = (struct lws_protocols[]) {
        {
            .name = "cls",
            .callback = WS_HANDLE,
            .per_session_data_size = sizeof(struct per_session_data)
        },
        { NULL, NULL, 0 }
    };

    context = lws_create_context(&info);
    if (context == NULL) {
        FINITE_LOG_ERROR("Unable to create context");
        return NULL;
    }

    i.context = context;
    i.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED;
    i.path = "/console/request_auth";
    i.port = 443;
    i.address = options.link->adr;
    i.host = options.link->adr;
    i.origin = options.link->adr;
    i.protocol = "cls";
    
    FINITE_LOG("Route: %s%s (Host: %s, Origin: %s)", i.address, i.path, i.host, i.origin);

    wsi = lws_client_connect_via_info(&i);
    if (!wsi) {
        FINITE_LOG_ERROR("Unable to connect");
        lws_context_destroy(context);
        return NULL;
    }

    // set session data here
    struct per_session_data *session = lws_wsi_user(wsi);
    session->cs = cs;
    session->ctx = context;

    for (int i = 0; i < 30; i++) {
        FINITE_LOG("Ping");
        lws_service(context, 1000);
        sleep(1);
        if (session->cs.state != CONNECTION_STATE_PENDING) {
            break;
        }
    }

    if (session->cs.state == CONNECTION_STATE_PENDING) {
        FINITE_LOG("Handshake Timeout reached.");
        FiniteIPCResponse res = {
            .status = 404,
            .msg = "Unable to preform initial handshake (timeout)",
        };

        send_signal(server.client_fd, res);
        close(server.client_fd);
        lws_context_destroy(context);

    } else {
        pthread_t ln;
        pthread_create(&ln, NULL, WS_RUNTIME, session);
    }

    return options.link;
}

int initializAPISocket() {
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    if (!getenv("FINITE_IPC_MAIN_DIR")) {
        // test environment: set it locally
        setenv("FINITE_IPC_MAIN_DIR", "/run/user/1000/api.sock", 1);
    }

    const char *path = getenv("FINITE_IPC_MAIN_DIR");

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    // if we had an old socket destroy it
    unlink(path);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        close(fd);
        FINITE_LOG_ERROR("Unable to bind to socket");
        return -1;
    }


    chmod(path, 0666);

    if (listen(fd, 8) < 0) {
        perror("listen");
        close(fd);
        FINITE_LOG_ERROR("Unable to listen to socket");
        return -1;
    }

    server.server_fd = fd;
    server.owner = 0; // self

    init_api(); // initialize api
    FINITE_LOG("Socket at %s was made.", path);
    return fd;
}

void *handle_api() {
    int client;
    while ((client = accept(server.server_fd, NULL, NULL)) >= 0) {
        FiniteIPCRequest req;

        // n is the size of the buffer
        ssize_t n;
        while((n = recv(client, &req, sizeof(FiniteIPCRequest), 0)) > 0) {
            if (strcmp(req.cmd, "AUTH") == 0) {
                if (server.client_fd == 0 || server.client_fd == client) { 
                    // Do not grant focus if the link is NULL
                    if (strcmp(req.adr, "") == 0) {
                        FiniteIPCResponse res = {
                            .status = 0,
                            .msg = "Link invalid"
                        };
                        FINITE_LOG("Replying with signal (denying request for bad url)");  
                        send_signal(client, res);
                    } else {
                        server.client_fd = client;

                        FiniteRequest areq;

                        areq = make_ws_request("api.cubixdev.org");
                        char body[4096] = {0};
                        snprintf(body, 4096, "{\n\"token\":\"%s\",\n\"device_id\":\"05b47eea-0149-4daf-8b3a-efe7532cdecb\",\n\"game_id\":\"26\"\n}", req.token);
                        FINITE_LOG("Data: \n%s", body);
                        add_request_body(&areq, body);
                        WS_INIT(areq);
                    }
                } else {
                    FiniteIPCResponse res = {
                        .status = 0,
                        .msg = "Request busy"
                    };

                    FINITE_LOG("Replying with signal (denying request)");
                    send_signal(client, res);
                }
            }
        }

        FINITE_LOG("Recieved client disconnected (%d)", client);
        if (client == server.client_fd) {
            server.client_fd = 0;
            server.owner = 0;
        }
    }    

    FINITE_LOG("Exited out");
    
    return NULL;
}