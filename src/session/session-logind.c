#include <errno.h>
#include <stdlib.h>
#include <systemd/sd-login.h>
#include <unistd.h>
#include <zen-session.h>
#include <zen-util.h>

struct zn_session_logind {
  struct zn_session base;

  char* id;
};

ZN_EXPORT int
zn_session_connect(struct zn_session* parent)
{
  struct zn_session_logind* self = zn_container_of(parent, self, base);
  int ret;

  ret = sd_pid_get_session(getpid(), &self->id);
  if (ret < 0) {
    zn_log("logind: not running in a systemd session\n");
    goto err;
  }

  // TODO: There is more to be done

  return 0;

err:
  errno = -ret;
  return -1;
}

ZN_EXPORT struct zn_session*
zn_session_create()
{
  struct zn_session_logind* self;

  self = zalloc(sizeof *self);
  if (self == NULL) goto err;

  return &self->base;

err:
  return NULL;
}

ZN_EXPORT void
zn_session_destroy(struct zn_session* parent)
{
  struct zn_session_logind* self = zn_container_of(parent, self, base);

  free(self->id);
  free(self);
}
