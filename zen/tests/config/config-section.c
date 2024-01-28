#include <toml.h>

#include "test-harness.h"
#include "zen/config.h"

TEST(reserve)
{
  struct zn_config *config = zn_config_create();

  ASSERT_EQUAL_BOOL(true, zn_config_reserve_section(config, "section1"));

  ASSERT_EQUAL_BOOL(false, zn_config_reserve_section(config, "section1"));

  zn_config_destroy(config);
}

TEST(get_section)
{
  struct zn_config *config = zn_config_create();

  toml_table_t *table = toml_table_in(config->root_table, "test");
  toml_datum_t result = toml_int_in(table, "number");

  ASSERT_NOT_EQUAL_POINTER(NULL, table);
  ASSERT_EQUAL_INT(1, result.ok);
  ASSERT_EQUAL_INT(4, result.u.i);

  table = toml_table_in(config->root_table, "test2");

  ASSERT_EQUAL_POINTER(NULL, table);

  zn_config_destroy(config);
}
