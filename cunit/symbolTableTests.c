#include "symbolTableTests.h"
#include "../symbolTable.h"

// Add an entry to a scope and try to find it
void test00() { 

  struct scope * s = new_scope(NULL);

  struct symtab * expected = add_entry(s, 0, "testFunction", NULL);

  CU_ASSERT_PTR_EQUAL(expected, find_in_scope(s, "testFunction"));
}

// Look for an entry that doesn't exist
// Should be NULL
void test01() { 

  struct scope * s = new_scope(NULL);

  CU_ASSERT_PTR_NULL(find_in_scope(s, "testFunction"));
}


int addSymbolTableTests() {
    CU_pSuite Suite = CU_add_suite("Symbol Table Tests", NULL, NULL);

  if (NULL == Suite) {
    return 0;
  }
  if ( (    
      NULL == CU_add_test(Suite, "test00", test00) 
      || NULL == CU_add_test(Suite, "test01", test01) 
	) ) {
    return 0;
  }

  return 1;
}
