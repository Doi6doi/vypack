#include "str.h"
#include <stdlib.h>

int main() {
   Strs pool = strsCreate();
   Str s = strCreate( pool, "Hello, world!\n", STR_LEN );
//    strFree( s );
   strsFree( pool );
   return 0;
}


