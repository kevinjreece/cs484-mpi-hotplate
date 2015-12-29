#pragma once

#include <sys/time.h>

/* Return the current time in seconds, using a double precision number. */
double getTime() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return ((double)tp.tv_sec + (double)tp.tv_usec * 1e-6);
}