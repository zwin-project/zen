#include <assert.h>
#include <toml.h>

#include "config.c"  // NOLINT(bugprone-suspicious-include)
#include "test-harness.h"
#include "zen-common/util.h"
#include "zen/config.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool test_config_section_destroyed = false;

static void test_config_section_destroy(struct zn_config_section *base);

struct test_config_section {
  struct zn_config_section base;
  bool loaded;
  int64_t number;
};

static bool
test_config_section_load(struct zn_config_section *base, toml_table_t *table)
{
  struct test_config_section *self = zn_container_of(base, self, base);

  self->loaded = true;
  if (table) {
    toml_datum_t result = toml_int_in(table, "number");
    if (result.ok) {
      self->number = result.u.i;
    }
  }

  return self->number >= 0;
}

static struct test_config_section *
test_config_section_create(void)
{
  struct test_config_section *section = zalloc(sizeof *section);

  section->base.load = test_config_section_load;
  section->base.destroy = test_config_section_destroy;
  section->loaded = false;
  section->number = 0;

  return section;
}

static void
test_config_section_destroy(struct zn_config_section *base)
{
  struct test_config_section *self = zn_container_of(base, self, base);
  free(self);

  test_config_section_destroyed = true;
}

/// valid section
TEST(config_section_valid)
{
  struct test_config_section *section = test_config_section_create();

  struct zn_config *config = zn_config_create();

  zn_config_add_section(config, "test", &section->base);

  ASSERT_EQUAL_POINTER(section, zn_config_get_section(config, "test"));

  ASSERT_EQUAL_BOOL(true, section->loaded);

  ASSERT_EQUAL_INT(4, section->number);

  zn_config_destroy(config);

  ASSERT_EQUAL_BOOL(true, test_config_section_destroyed);
}

/// missing section
TEST(config_section_invalid)
{
  struct test_config_section *section = test_config_section_create();

  struct zn_config *config = zn_config_create();

  zn_config_add_section(config, "missing_section", &section->base);

  ASSERT_EQUAL_POINTER(
      section, zn_config_get_section(config, "missing_section"));

  ASSERT_EQUAL_BOOL(true, section->loaded);

  ASSERT_EQUAL_INT(0, section->number);

  zn_config_destroy(config);

  ASSERT_EQUAL_BOOL(true, test_config_section_destroyed);
}

/// duplicated section
TEST(config_section_duplicate)
{
  struct test_config_section *section1 = test_config_section_create();
  struct test_config_section *section2 = test_config_section_create();

  struct zn_config *config = zn_config_create();

  ASSERT_EQUAL_BOOL(
      true, zn_config_add_section(config, "test", &section1->base));
  ASSERT_EQUAL_BOOL(
      false, zn_config_add_section(config, "test", &section2->base));
}

/// load error
TEST(config_section_error)
{
  struct test_config_section *section = test_config_section_create();

  struct zn_config *config = zn_config_create();

  ASSERT_EQUAL_BOOL(
      false, zn_config_add_section(config, "error", &section->base));
}

/// missing config file
TEST(config_section_no_file)
{
  setenv("XDG_CONFIG_HOME", "/tmp/path", 1);

  struct test_config_section *section = test_config_section_create();

  struct zn_config *config = zn_config_create();

  zn_config_add_section(config, "test", &section->base);

  ASSERT_EQUAL_POINTER(section, zn_config_get_section(config, "test"));

  ASSERT_EQUAL_BOOL(true, section->loaded);

  ASSERT_EQUAL_INT(0, section->number);
}

/// unknown section
TEST(config_section_unknown)
{
  struct zn_config *config = zn_config_create();

  ASSERT_EQUAL_POINTER(NULL, zn_config_get_section(config, "unkown"));
}
