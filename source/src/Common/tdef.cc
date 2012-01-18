#include <stdio.h>
#include "globals.h"
int main()
{
#ifdef WITH_DES
printf("glop = %d\n", WITH_DES);
#else
printf("noglop\n");
#endif
return 0;
}
