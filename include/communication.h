#include <stdbool.h>

bool ping();
void connect(int port);
void sendcmd(const char* cmd, ...);
void ioctl(...)