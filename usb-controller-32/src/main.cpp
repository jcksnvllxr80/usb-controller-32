#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include "definitions.h"

int main(void)
{
    /* Initialize all modules (including APP_Initialize and USB) */
    SYS_Initialize(NULL);

    /* Create RTOS tasks (USB + App) and start the scheduler — never returns */
    SYS_Tasks();

    return EXIT_FAILURE;
}
