#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include<linux/fs.h>
#include <linux/dirent.h>
#include "printstring.h"

asmlinkage int (*origin_mkdir)(const char *path, mode_t mode);
asmlinkage int (*origin_chdir)(const char *path);
asmlinkage int (*origin_rmdir)(const char *pathname);

asmlinkage ssize_t (*origin_read)(unsigned int,char*,size_t);
asmlinkage ssize_t (*origin_write)(int fd, const void *buf, size_t count);

asmlinkage int (*origin_open)(const char *pathname, int flags, mode_t mode);

asmlinkage int modified_mkdir(const char *path, mode_t mode) {
    //printk(KERN_ALERT "mkdir is not allowed now\n");
    conivent_printf("modified_mkdir 2");
    
    return origin_mkdir(path, mode);
}

asmlinkage int modified_chdir(const char *path) {
    conivent_printf("modified_chdir 1");
    return origin_chdir(path);
}

asmlinkage int modified_rmdir(const char *path) {
    conivent_printf("modified_rmdir 1");
    return origin_rmdir(path);
}

/*asmlinkage struct dirent *readdir(DIR *dirp) {
    return NULL;
}*/

asmlinkage ssize_t modified_read(unsigned int fd,char* buf,size_t size) {
    //conivent_printf("modified read 3");
    
    return origin_read(fd, buf, size);
}

asmlinkage ssize_t modified_write(int fd, const void *buf, size_t count) {
    //conivent_printf("modified_write 3");
    return origin_write(fd, buf, count);
}

asmlinkage int modified_open(const char *pathname, int flags, mode_t mode) {
    //conivent_printf("modified_open 3");
    return origin_open(pathname, flags, mode);
}

/*asmlinkage long modified_close(void)
{
    conivent_printf("close is not allowed now");
    return 0;
}*/