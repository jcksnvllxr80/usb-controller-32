#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"

extern "C" {
    void APP_Initialize(void);
    void APP_Tasks(void);
}

int main(void)
{
    SYS_Initialize(NULL);
    APP_Initialize();

    while (true)
    {
        APP_Tasks();
    }

    return EXIT_FAILURE;
}
