#include "App.h"

void main_task(void) _task_ 0
{
    if (APP_System_Init() == APP_OK) {
        os_create_task(TASK_ROBOT_MOTION);
        os_create_task(TASK_COMMAND);
        os_create_task(TASK_SAFETY);
    }
    os_delete_task(0);
}
