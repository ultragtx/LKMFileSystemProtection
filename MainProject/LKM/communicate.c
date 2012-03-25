#include <linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>

#include<linux/file.h>
#include<linux/vmalloc.h>
#include<linux/string.h>
#include<asm/uaccess.h>
#include<linux/namei.h>
#include<linux/fs.h>

#include "printstring.h"

#define PROC_DIR_NAME "gsfileprotection"
#define PROC_PROTECT_NAME "protect"
#define PROC_COMMUNICATE_NAME "communicate"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_protect;
static struct proc_dir_entry *proc_communicate;

int proc_communicate_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
    conivent_printf("proc_communicate read");
    return 0;
}

int proc_communicate_write(struct file *file, const char *buffer, unsigned long count, void *data) {
    conivent_printf("proc_communicate write");
    return 0;
}

int proc_protect_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
    conivent_printf("proc_protect read");
    return 0;
}

int proc_protect_write(struct file *file, const char *buffer, unsigned long count, void *data) {
    conivent_printf("proc_protect write");
    return 2;
}

int init_communicate(void) {
    
    // create proc file
    conivent_printf("start create proc");
    proc_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (proc_dir == NULL) {
        remove_proc_entry(PROC_DIR_NAME, NULL);
        conivent_printf("can't create proc_dir");
        return -1;
    }
    else {
        proc_protect = create_proc_entry(PROC_PROTECT_NAME, 0666, proc_dir);
        proc_communicate = create_proc_entry(PROC_COMMUNICATE_NAME, 0666, proc_dir);
        if (proc_protect == NULL || proc_communicate == NULL) {
            conivent_printf("can't create proc file");
            remove_proc_entry(PROC_PROTECT_NAME, proc_dir);
            remove_proc_entry(PROC_COMMUNICATE_NAME, proc_dir);
            return -1;
        }
    }
    
    proc_protect->read_proc = proc_protect_read;
    proc_protect->write_proc = proc_protect_write;
    //proc_protect->owner = THIS_MODULE;
    
    proc_communicate->read_proc = proc_communicate_read;
    proc_communicate->write_proc = proc_communicate_write;
    //proc_communicate->owner = THIS_MODULE;
    
    return 0;
}

void cleanup_communicate(void) {
    conivent_printf("start remove proc");
    remove_proc_entry(PROC_DIR_NAME, NULL);
    remove_proc_entry(PROC_PROTECT_NAME, proc_dir);
    remove_proc_entry(PROC_COMMUNICATE_NAME, proc_dir);
}