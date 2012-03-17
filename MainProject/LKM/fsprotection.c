#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include "printstring.h"


asmlinkage long modified_mkdir(const char *path)
{
    //printk(KERN_ALERT "mkdir is not allowed now\n");
    conivent_printf("mkdir is not allowed now");
    return 0;
}

asmlinkage long modified_read(void)
{
    conivent_printf("read is not allowed now");
    return 0;
}

asmlinkage long modified_write(void)
{
    conivent_printf("write is not allowed now");
    return 0;
}

asmlinkage long modified_open(void)
{
    conivent_printf("open is not allowed now");
    return 0;
}

asmlinkage long modified_close(void)
{
    conivent_printf("close is not allowed now");
    return 0;
}