#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "CUnit.h"
#include "Basic.h"
#include "symbolTableTests.h"

int main() {
    if (CUE_SUCCESS != CU_initialize_registry()) { return CU_get_error(); }

    if(!addSymbolTableTests()) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
