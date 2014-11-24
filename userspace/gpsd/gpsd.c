#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "gpsd.h"

#define GPSD_DEBUG

#ifdef GPSD_DEBUG
#define log	printf
#else
#define log(...)
#endif

static void daemonize(void)
{
	pid_t pid = 0;
	int fd = 0;

	/* Authentication */
	if (!getuid()) {
		log("Not root\n");
		exit(EXIT_FAILURE);
	}

	/* Fork to create new session */
	pid = fork();
	if (pid < 0) {
		log("Failed to new session\n");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		log("1st parent to terminate\n");
		exit(EXIT_SUCCESS);
	}

	if (setsid() < 0) {
		log("Failed to set sid\n");
		exit(EXIT_FAILURE);
	}

	/* Fork to get rid of TTY */
	pid = fork();
	if (pid < 0) {
		log("Failed to fork\n");
		exit(EXIT_FAILURE);
	} else if (pid > 0) {
		log("nst parent to terminate\n");
		exit(EXIT_SUCCESS);
	}

	/* Set up directory */
	if (chdir("/") < 0) {
		log("Failed to change dir\n");
		exit(EXIT_FAILURE);
	}

	umask(0);

	/* Turn of std io */
	fd = open("/dev/null", O_RDWR);
	if (fd < 0) {
		log("Failed to open null dev\n");
		exit(EXIT_FAILURE);
	}
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
	close(fd);
}

int main(int argc, char *argv[])
{
	daemonize();

	return 0;
}

