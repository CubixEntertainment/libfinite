#include "../include/wl_shm.h"

static void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

static int create_shm_file(void) {
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		randname(name + sizeof(name) - 7);
		--retries;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}


/*
    # finite_shm_allocate_shm_file

    A poweruser function.
    
    Allocates a shared memory file to a specific size. Normally you would call finite_shm_alloc() to handle buffer management.
*/
int finite_shm_allocate_shm_file(size_t size) {
	int fd = create_shm_file();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

void finte_shm_alloc(FiniteShell *shell) {
    if (!shell) {
        // if no shell throw an error
        printf("[Finite] - Unable to manage shared memory on NULL. \n"); // TODO create a finite_log function
        return;
    }

    FiniteWindowInfo *det = shell->details;
    const int width = det->width, height = det->height;

    const int stride = width * 4; // stride must be four to comply with the spec

    int pool_size = height * stride;

    shell->shm_fd = finite_shm_allocate_shm_file(pool_size);

    shell->pool_data = mmap(NULL, pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, shell->shm_fd, 0);

    shell->pool = wl_shm_create_pool(shell->shm, shell->shm_fd, pool_size);

    shell->pool_size = pool_size;
     
    if (shell->pool_data == MAP_FAILED || shell->shm_fd < 0) {
        printf("[Home] - Unable to allocate needed memory.\n");
        wl_display_disconnect(shell->display);
        return 1;
    }
}
