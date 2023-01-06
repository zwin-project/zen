#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "zen-common.h"

pid_t
launch_command(char *command)
{
  pid_t pid = -1;

  pid = fork();
  if (pid == -1) {
    zn_error("Failed to fork the command process: %s", strerror(errno));
    goto err;
  } else if (pid == 0) {
    execl("/bin/sh", "/bin/sh", "-c", command, NULL);
    fprintf(stderr, "Failed to execute the command (%s): %s\n", command,
        strerror(errno));
    _exit(EXIT_FAILURE);
  }

err:
  return pid;
}
