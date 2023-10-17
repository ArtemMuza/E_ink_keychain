#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "screen.h"

void app_main(void)
{
	DEV_Module_Init();
    while (true) {
        printf("Hello from app_main!\n");
        sleep(1);
    }
}
