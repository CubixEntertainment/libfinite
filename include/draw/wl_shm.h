#ifndef __WL_SHM_H__
#define __WL_SHM_H__
#define _POSIX_C_SOURCE 200112L
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "window.h"

int finite_shm_allocate_shm_file(size_t size);

#define finite_shm_alloc(shell, withAlpha) finite_shm_alloc_debug(__FILE__, __func__, __LINE__, shell, withAlpha)
void finite_shm_alloc_debug(const char *file, const char *func, int line, FiniteShell *shell, bool withAlpha);

#endif