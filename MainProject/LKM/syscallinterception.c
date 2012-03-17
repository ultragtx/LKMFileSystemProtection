#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>

#include "syscallinterception.h"
#include "printstring.h"

unsigned int clear_and_return_cr0(void)
{
    unsigned int cr0 = 0;
    unsigned int ret;
    asm("movl %%cr0 ,%%eax":"=a"(cr0));
    ret = cr0;
    //清除cr0的WP(Write Protect)标志位
    
    cr0 &= 0xfffeffff;//第16位为WP
    
    asm("movl %%eax, %%cr0"::"a"(cr0));
    return ret;
}

void setback_cr0(unsigned int val)
{
    asm("movl %%eax, %%cr0"::"a"(val));
}

unsigned int* syscall_table;
unsigned int origin_mkdir;
unsigned int origin_read;
unsigned int origin_write;
unsigned int origin_open;
unsigned int origin_close;

static int lkm_init(void) {
    
    unsigned int orig_cr0;
    printk(KERN_ALERT "lkm_init\n");
    conivent_printf("lkm_init");
    syscall_table = (unsigned int*) get_sys_call_table();
    if(syscall_table == 0)
    {
        printk(KERN_ALERT "can't find syscall_table addr\n");
        return -1;
    }
    origin_mkdir = syscall_table[__NR_mkdir];
    //printk("addr of old_handler %x\n", old_handler);
    orig_cr0 = clear_and_return_cr0();
    syscall_table[__NR_mkdir] = (unsigned long) modified_mkdir;
    setback_cr0(orig_cr0);
    return 0;
}

static void lkm_exit(void)
{
    unsigned int orig_cr0 = clear_and_return_cr0();
    syscall_table[__NR_mkdir] = (unsigned long) origin_mkdir;
    setback_cr0(orig_cr0);
    printk(KERN_ALERT "lkm_exit\n");
}
module_init(lkm_init);
module_exit(lkm_exit);