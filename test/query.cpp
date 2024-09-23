#include <stdio.h>

#include "aiowdm.h"

int main (int argc, char **argv)
{
    int NumCards = AIOWDM::GetNumCards();
    long unsigned int device_id;
    long unsigned int base;
    unsigned char name[512];
    long unsigned int name_size = sizeof(name);
    int status = 0;

    printf("NumCards = %d\n", NumCards);

    status = AIOWDM::QueryCardInfo(0, &device_id, &base, &name_size, name);

    printf("device_id = %X, base = %p, name_size = %d, name = %s\n", device_id, base, name_size, name);

    return 0;
}
