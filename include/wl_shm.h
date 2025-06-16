#ifndef __WL_SHM_H__
#define __WL_SHM_H__
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "window.h"

int finite_shm_allocate_shm_file(size_t size);
void finite_shm_alloc(FiniteShell *shell);

#endif