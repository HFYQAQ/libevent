#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

int main() {
	fd_set readset;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_SET(2, &readset);
	printf("select return %d\nselect error: %s\n", select(3, &readset, NULL, NULL, &tv), strerror(errno));

	return 0;
}
