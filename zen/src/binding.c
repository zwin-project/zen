#include "zen/binding.h"

#include <assert.h>
#include <linux/input.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/server.h"

#define CONFIG_SECTION_KEY "binding"

#define TOKEN_SHIFT "shift"
#define TOKEN_CTRL "ctrl"
#define TOKEN_ALT "alt"
#define TOKEN_LOGO "logo"
#define TOKEN_RIGHT "right"
#define TOKEN_LEFT "left"
#define TOKEN_UP "up"
#define TOKEN_DOWN "down"
#define TOKEN_SEPARATOR '+'

static const uint32_t ascii_to_keycode[] = {
    ['a'] = KEY_A,
    ['b'] = KEY_B,
    ['c'] = KEY_C,
    ['d'] = KEY_D,
    ['e'] = KEY_E,
    ['f'] = KEY_F,
    ['g'] = KEY_G,
    ['h'] = KEY_H,
    ['i'] = KEY_I,
    ['j'] = KEY_J,
    ['k'] = KEY_K,
    ['l'] = KEY_L,
    ['m'] = KEY_M,
    ['n'] = KEY_N,
    ['o'] = KEY_O,
    ['p'] = KEY_P,
    ['q'] = KEY_Q,
    ['r'] = KEY_R,
    ['s'] = KEY_S,
    ['t'] = KEY_T,
    ['u'] = KEY_U,
    ['v'] = KEY_V,
    ['w'] = KEY_W,
    ['x'] = KEY_X,
    ['y'] = KEY_Y,
    ['z'] = KEY_Z,
    ['0'] = KEY_0,
    ['1'] = KEY_1,
    ['2'] = KEY_2,
    ['3'] = KEY_3,
    ['4'] = KEY_4,
    ['5'] = KEY_5,
    ['6'] = KEY_6,
    ['7'] = KEY_7,
    ['8'] = KEY_8,
    ['9'] = KEY_9,
};

void
zn_binding_add(struct zn_binding *self, const char *name,
    zn_binding_handler_t handler, void *user_data)
{
  struct zn_binding_item *item = NULL;

  wl_array_for_each (item, &self->items) {
    if (strcmp(name, item->name) == 0) {
      zn_warn("Binding name is duplicated (%s). Skipping.", name);
      return;
    }
  }

  item = wl_array_add(&self->items, sizeof *item);
  item->name = name;
  item->handler = handler;
  item->key = 0;
  item->modifiers = 0;
  item->user_data = user_data;
}

bool
zn_binding_handle_key(struct zn_binding *self, uint32_t key, uint32_t modifiers)
{
  struct zn_binding_item *item = NULL;

  wl_array_for_each (item, &self->items) {
    bool matches = item->key == key && item->modifiers == modifiers;

    if (!matches) {
      continue;
    }

    if (item->handler(item->name, key, modifiers, item->user_data)) {
      return true;
    }
  }

  return false;
}

static int
find_next_separator(const char *string, char separator)
{
  for (int i = 0;; i++) {
    if (string[i] == separator || string[i] == '\0') {
      return i;
    }
  }

  assert(false && "Unreachable");
}

static void
zn_binding_item_reconfigure(struct zn_binding_item *item, char *description)
{
  char *current = description;
  while (true) {
    int length = find_next_separator(current, TOKEN_SEPARATOR);
    if (length <= 0) {
      break;
    }

    if (strncmp(current, TOKEN_SHIFT, length) == 0) {
      item->modifiers |= WLR_MODIFIER_SHIFT;
    } else if (strncmp(current, TOKEN_CTRL, length) == 0) {
      item->modifiers |= WLR_MODIFIER_CTRL;
    } else if (strncmp(current, TOKEN_ALT, length) == 0) {
      item->modifiers |= WLR_MODIFIER_ALT;
    } else if (strncmp(current, TOKEN_LOGO, length) == 0) {
      item->modifiers |= WLR_MODIFIER_LOGO;
    } else if (strncmp(current, TOKEN_RIGHT, length) == 0) {
      item->key = KEY_RIGHT;
    } else if (strncmp(current, TOKEN_LEFT, length) == 0) {
      item->key = KEY_LEFT;
    } else if (strncmp(current, TOKEN_UP, length) == 0) {
      item->key = KEY_UP;
    } else if (strncmp(current, TOKEN_DOWN, length) == 0) {
      item->key = KEY_DOWN;
    } else if (length == 1) {
      char key = current[0];
      if (('0' <= key && key <= '9') || ('a' <= key && key <= 'z')) {
        item->key = ascii_to_keycode[(int)key];
      }
    }

    if (current[length] == '\0') {
      break;
    }

    current += length + 1;
  }
}

void
zn_binding_remap(struct zn_binding *self)
{
  struct zn_server *server = zn_server_get_singleton();

  toml_table_t *table = NULL;
  if (server->config->root_table) {
    table = toml_table_in(server->config->root_table, CONFIG_SECTION_KEY);
  }

  if (table == NULL) {
    return;
  }

  struct zn_binding_item *item = NULL;

  // TODO(@Aki-7): Refactor implementing lexer for more flexibility
  // TODO(@Aki-7): Enable to add multiple key bindings to one command
  wl_array_for_each (item, &self->items) {
    toml_datum_t data = toml_string_in(table, item->name);
    if (!data.ok) {
      continue;
    }

    zn_binding_item_reconfigure(item, data.u.s);
  }
}

struct zn_binding *
zn_binding_create(void)
{
  struct zn_server *server = zn_server_get_singleton();

  struct zn_binding *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_array_init(&self->items);

  zn_config_reserve_section(server->config, CONFIG_SECTION_KEY);

  return self;

err:
  return NULL;
}

void
zn_binding_destroy(struct zn_binding *self)
{
  wl_array_release(&self->items);
  free(self);
}
