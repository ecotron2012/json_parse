#include "../parser.h"
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void test_valid_json(void) {
  CU_ASSERT(parse("./valid/test1.json") == 0);
  CU_ASSERT(parse("./valid/test2.json") == 0);
  CU_ASSERT(parse("./valid/test3.json") == 0);
  CU_ASSERT(parse("./valid/test4.json") == 0);
  CU_ASSERT(parse("./valid/test4.json") == 1);
}

void test_invalid_json(void) {
  CU_ASSERT(parse("./invalid/test1.json") == 1);
  CU_ASSERT(parse("./invalid/test2.json") == 1);
  CU_ASSERT(parse("./invalid/test3.json") == 1);
  CU_ASSERT(parse("./invalid/test4.json") == 1);
  CU_ASSERT(parse("./invalid/test5.json") == 1);
  CU_ASSERT(parse("./invalid/test6.json") == 1);
}

int main() {
  CU_initialize_registry();
  CU_pSuite suite = CU_add_suite("JSONParseTestSuite", 0, 0);
  CU_add_test(suite, "test valid json files()", test_valid_json);
  CU_add_test(suite, "test invalid json files()", test_invalid_json);
  CU_basic_run_tests();
  // CU_pFailureRecord pFailure = CU_get_failure_list();
  // CU_basic_show_failures(pFailure);
  CU_cleanup_registry();
  return 0;
}
