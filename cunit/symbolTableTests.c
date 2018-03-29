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

// Add an entry to a parent scope
// Add a child to that scope
// The entry should be scope-visible from the child scope
void test02() { 

  struct scope * s = new_scope(NULL);
  struct scope * s2 = new_scope(s);

  struct symtab * expected = add_entry(s, 0, "testFunction", NULL);

  CU_ASSERT_PTR_EQUAL(expected, find_in_scope(s2, "testFunction"));
}

// Create a scope
// Add a child to that scope
// Add entry to child scope
// The entry should NOT be scope-visible from the parent scope
void test03() {
  struct scope * s = new_scope(NULL);
  struct scope * s2 = new_scope(s);

  struct symtab * expected = add_entry(s2, 0, "testFunction", NULL);

  CU_ASSERT_PTR_NULL(find_in_scope(s, "testFunction"));
}

// Create a scope
// Add a child to that scope
// Add entry to child scope
// The entry should be children-visible from the parent scope
void test04() {
  struct scope * s = new_scope(NULL);
  struct scope * s2 = new_scope(s);

  struct symtab * expected = add_entry(s2, 0, "testFunction", NULL);

  CU_ASSERT_PTR_EQUAL(expected, find_in_children(s, "testFunction"));
}

// Create a scope
// Add a child to that scope
// Add entry to parent scope
// The entry should be NOT children-visible from the child scope
void test05() {
  struct scope * s = new_scope(NULL);
  struct scope * s2 = new_scope(s);

  struct symtab * expected = add_entry(s, 0, "testFunction", NULL);

  CU_ASSERT_PTR_NULL(find_in_children(s2, "testFunction"));
}



int addSymbolTableTests() {
    CU_pSuite Suite = CU_add_suite("Symbol Table Tests", NULL, NULL);

  if (NULL == Suite) {
    return 0;
  }
  if ( (    
      NULL == CU_add_test(Suite, "test00", test00) 
      || NULL == CU_add_test(Suite, "test01", test01) 
      || NULL == CU_add_test(Suite, "test02", test02) 
      || NULL == CU_add_test(Suite, "test03", test03) 
      || NULL == CU_add_test(Suite, "test03", test04) 
      || NULL == CU_add_test(Suite, "test03", test05) 
	) ) {
    return 0;
  }

  return 1;
}
