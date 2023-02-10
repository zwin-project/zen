#include "zen-common/signal.h"

#include "zen-common/util.h"

static void
handle_noop(struct wl_listener *listener UNUSED, void *data UNUSED)
{
  /* Do nothing */
}

void
zn_signal_emit_mutable(struct wl_signal *signal, void *data)
{
  struct wl_listener cursor;
  struct wl_listener end;

  wl_list_insert(&signal->listener_list, &cursor.link);
  cursor.notify = handle_noop;
  wl_list_insert(signal->listener_list.prev, &end.link);
  end.notify = handle_noop;

  while (cursor.link.next != &end.link) {
    struct wl_list *pos = cursor.link.next;
    struct wl_listener *l = wl_container_of(pos, l, link);

    wl_list_remove(&cursor.link);
    wl_list_insert(pos, &cursor.link);

    l->notify(l, data);
  }

  wl_list_remove(&cursor.link);
  wl_list_remove(&end.link);
}
