#include "symbolTableTests.h"
#include "../symbolTable.h"

void test00() { }


int addSymbolTableTests() {
    CU_pSuite Suite = CU_add_suite("Symbol Table Tests", NULL, NULL);

  if (NULL == Suite) {
    return 0;
  }
  if ( (    
      NULL == CU_add_test(Suite, "test00", test00) 
	) ) {
    return 0;
  }

  return 1;
}
