
#include "memory_adapter.h"
#include <stdlib.h>
void *MEM_Malloc(uint32 size)
{
    if (size == 0) {
        return NULL;
    }
    return malloc(size);
}

void MEM_Free(void *buffer)
{
    if (buffer == NULL) {
        return;
    }
    (void)free(buffer);
}