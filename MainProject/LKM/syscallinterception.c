#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>

#include "syscallinterception.h"
#include "printstring.h"
#include "communicate.h"
#include "notification.h"

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

extern unsigned long origin_mkdir;
extern unsigned long origin_chdir;
extern unsigned long origin_rmdir;

extern unsigned long origin_getdents64;

extern unsigned long origin_read;
extern unsigned long origin_write;

extern unsigned long origin_open;

extern unsigned long origin_unlinkat;
extern unsigned long origin_unlink;

extern unsigned long origin_rename;
extern unsigned long origin_renameat;


static int lkm_init(void) {
    
    unsigned int orig_cr0;
    printk(KERN_ALERT "lkm_init\n");
    conivent_printf("lkm_init");
    // syscall
    syscall_table = (unsigned int*) get_sys_call_table();
    if(syscall_table == 0)
    {
        printk(KERN_ALERT "can't find syscall_table addr\n");
        return -1;
    }
    origin_mkdir = syscall_table[__NR_mkdir];
    origin_chdir = syscall_table[__NR_chdir];
    origin_rmdir = syscall_table[__NR_rmdir];
    
    origin_getdents64 = syscall_table[__NR_getdents64];
    
    origin_read = syscall_table[__NR_read];
    origin_write = syscall_table[__NR_write];
    
    origin_open = syscall_table[__NR_open];
    
    origin_unlinkat = syscall_table[__NR_unlinkat];
    origin_unlink = syscall_table[__NR_unlink];
    
    origin_rename = syscall_table[__NR_rename];
    origin_renameat = syscall_table[__NR_renameat];
    
    //printk("addr of old_handler %x\n", old_handler);
    orig_cr0 = clear_and_return_cr0();
    syscall_table[__NR_mkdir] = (unsigned long)modified_mkdir;
    syscall_table[__NR_chdir] = (unsigned long)modified_chdir;
    syscall_table[__NR_rmdir] = (unsigned long)modified_rmdir;
    
    syscall_table[__NR_getdents64] = (unsigned long)modified_getdents64;
    
    syscall_table[__NR_read] = (unsigned long)modified_read;
    syscall_table[__NR_write] = (unsigned long)modified_write;
    
    syscall_table[__NR_open] = (unsigned long)modified_open;
    
    syscall_table[__NR_unlinkat] = (unsigned long)modified_unlinkat;
    syscall_table[__NR_unlink] = (unsigned long)modified_unlink;
    
    syscall_table[__NR_rename] = (unsigned long)modified_rename;
    syscall_table[__NR_renameat] = (unsigned long)modified_renameat;
    
    setback_cr0(orig_cr0);
    
    // proc file
    if (init_communicate() != 0) {
        return -1;
    }
    
    // netlink
    if (notification_init() != 0) {
        return -1;
    }
    
    return 0;
}

static void lkm_exit(void)
{
    // syscall
    unsigned int orig_cr0 = clear_and_return_cr0();
    syscall_table[__NR_mkdir] = origin_mkdir;
    syscall_table[__NR_chdir] = origin_chdir;
    syscall_table[__NR_rmdir] = origin_rmdir;
    syscall_table[__NR_getdents64] = origin_getdents64;
    syscall_table[__NR_read] = origin_read;
    syscall_table[__NR_write] = origin_write;
    
    syscall_table[__NR_open] = origin_open;
    
    syscall_table[__NR_unlinkat] = origin_unlinkat;
    syscall_table[__NR_unlink] = origin_unlink;
    
    syscall_table[__NR_rename] = origin_rename;
    syscall_table[__NR_renameat] = origin_renameat;
    
    setback_cr0(orig_cr0);
    
    // proc file
    cleanup_communicate();
    notification_exit();
    printk(KERN_ALERT "lkm_exit\n");
}
module_init(lkm_init);
module_exit(lkm_exit);