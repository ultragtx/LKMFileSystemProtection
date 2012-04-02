#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include<linux/vmalloc.h>
#include <linux/namei.h>
#include <linux/string.h>
#include "printstring.h"
#include "communicate.h"
#include "fsprotection.h"

// test code here

typedef enum {
    FILE_PROTECT_READ,
    FILE_PROTECT_WRITE,
    FILE_PROTECT_OPEN
}FILE_PROTECT_TYPE;

int getPathFromFd(unsigned int fd, char *path) {
    struct file *file;
    struct dentry *p_dentry;
    char root_path[80];
    file = fget(fd);
    if (file == NULL) {
        return -1;
    }
    p_dentry = file->f_dentry;
    strcpy(root_path, p_dentry->d_sb->s_root->d_iname);
    sprintf(path, "%s", p_dentry->d_name.name);
    while (strcmp(p_dentry->d_name.name, root_path)) { // not root
        p_dentry = p_dentry->d_parent;
        sprintf(path, "%s/%s", p_dentry->d_name.name, path);
    }
    fput(file);
    return 0;
}

int isFileProtected(unsigned int fd, FILE_PROTECT_TYPE type) {
    struct file *this_file = NULL;
    char path[80];
    
    
    this_file = fget(fd);
    if (this_file != NULL) {
        if (this_file->f_dentry->d_inode == fs->f_dentry->d_inode) {
            //conivent_printf("isProtect yes");
            fput(this_file);
            /*if (getPathFromFd(fd, path) == 0) {
                conivent_printf("%s", path);
            }*/
            return 1;
        }
    }
    fput(this_file);
    return 0;
}

// test code end

asmlinkage int (*origin_mkdir)(const char *path, mode_t mode);
asmlinkage int (*origin_chdir)(const char *path);
asmlinkage int (*origin_rmdir)(const char *pathname);

asmlinkage int (*origin_getdents64)(unsigned int fd, struct linux_dirent64 *dirp,
                                    unsigned int count);

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

asmlinkage int modified_getdents64 (unsigned int fd, struct linux_dirent64 *dirp,
                                    unsigned int count) {
    //conivent_printf("modified_getdents64 3");
    //printk(KERN_ALERT "modified_getdents 3\n");
    return origin_getdents64(fd, dirp, count);
    /*unsigned int tmp, n;
    int t, proc = 0;
    struct inode *dinode;
    struct linux_dirent *dirp2, *dirp3;
    
    char hide[] = "text.txt";
    
    tmp = origin_getdents64(fd, dirp, count);
    
#ifdef __LINUX_DCACHE_H
    dinode = current->files->fd[fd]->f_dentry->d_inode;
#else
    dinode = current->files->fd[fd]->f_inode;
#endif
    
    if (tmp > 0) {
        dirp2 = (struct linux_dirent *)kmalloc(tmp, GFP_KERNEL);
        memcpy_fromfs(dirp2, dirp, tmp);
        dirp3 = dirp2;
        
        t = tmp;
        while (t > 0) {
            n = dirp3->d_reclen;
            t -= n;
            if (strstr((char *)&(dirp3->d_name), (char *)&hide) != NULL) {
                if (t != 0) {
                    memmove(dirp3, (char *)dirp3 + dirp3->d_reclen, t);
                }
                else {
                    dirp3->d_off = 1024;
                }
                tmp -= n;
            }
            if (dirp3->d_reclen == 0) {
                tmp -= t;
                t = 0;
            }
            if (t != 0) {
                dirp3 = (struct linux_dirent *)((char *)dirp3 + dirp3->d_reclen);
            }
        }
        
        
        memcpy_tofs(dirp, dirp2, tmp);
        kfree(dirp2);
    }
    return tmp;*/
    
}

asmlinkage ssize_t modified_read(unsigned int fd,char* buf,size_t size) {
    //conivent_printf("modified read 3");
    if (isFileProtected(fd, FILE_PROTECT_READ)) {
        conivent_printf("file is protected");
        return -EACCES;
    }
    return origin_read(fd, buf, size);
}

asmlinkage ssize_t modified_write(int fd, const void *buf, size_t count) {
    //conivent_printf("modified_write 3");
    if (isFileProtected(fd, FILE_PROTECT_WRITE)) {
        conivent_printf("file write is protected");
        return -EACCES;
    }
    
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