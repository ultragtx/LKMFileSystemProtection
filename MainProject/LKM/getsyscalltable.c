#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include "getsyscalltable.h"

MODULE_LICENSE("Dual BSD/GPL");

struct
{
    unsigned short limit;
    unsigned int base;
}__attribute__((packed))idtr;

struct
{
    unsigned short off1;
    unsigned short sel;
    unsigned char none, flags;
    unsigned short off2;
}__attribute__((packed))idt;

unsigned int get_sys_call_table(void)
{
    unsigned int sys_call_off;
    unsigned int sys_call_table;
    char *p;
    int i;
    //获得中断描述符表寄存器地址
    
    asm("sidt %0":"=m"(idtr)); 
    printk("add of idtr %x\n", idtr.base);
    //获取0x80中断处理程序的地址
    
    memcpy((char* )&idt,(char*)(idtr.base+8*0x80),sizeof(idt));
    sys_call_off = ((idt.off2 << 16) | idt.off1 );
    printk("addr of idt 0x80 %x\n", sys_call_off);
    //从0x80中断处理程序的二进制代码中搜索sys_call_table地址    
    
    p=(char*)sys_call_off;
    for(i = 0; i < 100; i++)
    {
        //0xff 0x14 0x85暂时不明白是什么特殊的汇编代码
        
        if(p[i] == '\xff' && p[i+1] == '\x14' && p[i+2] == '\x85')
        {
            sys_call_table = *(unsigned int*)(p + i + 3);
            printk("addr of sys_call_table %x\n", sys_call_table);
            return sys_call_table;
        }
    }
    return 0;
}
