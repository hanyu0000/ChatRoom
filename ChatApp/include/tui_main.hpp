#ifndef T_MAIN_H_
#define T_MAIN_H_
typedef struct User_Information
{
    char name[30];
    char pwd[30];
} UI;
UI tui_main();
int content();
int thread_pool();
#endif