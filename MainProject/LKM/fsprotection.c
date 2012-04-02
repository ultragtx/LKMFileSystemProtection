#include <linux/sched.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include<linux/vmalloc.h>

#include <asm/processor.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/slab.h>

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

int isInodeProtected(unsigned long inode, FILE_PROTECT_TYPE type) {
    //conivent_printf("isInodeProtected %d, %d", inode, fs->f_dentry->d_inode->i_ino);
    if (fs->f_dentry->d_inode->i_ino == inode) {
        return 1;
    }
    return 0;
}

// test code end

asmlinkage int (*origin_mkdir)(const char *path, mode_t mode);
asmlinkage int (*origin_chdir)(const char *path);
asmlinkage int (*origin_rmdir)(const char *pathname);

asmlinkage long (*origin_getdents64)(unsigned int fd, struct linux_dirent64 *dirp,
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

asmlinkage long modified_getdents64 (unsigned int fd, struct linux_dirent64 *dirp,
                                    unsigned int count) {
    //conivent_printf("modified_getdents64 3");
    //printk(KERN_ALERT "modified_getdents 3\n");
    //return origin_getdents64(fd, dirp, count);
    
    unsigned int buf_length, record_length, modified_buf_length;
    
    // head points to the first in the list
    // prev points to the previous
    struct linux_dirent64 *dirp2, *dirp3, *head = NULL, *prev = NULL;
    
    char hide_file[] = "text.txt";
    
    buf_length = origin_getdents64(fd, dirp, count);
    
    if (buf_length <= 0) {
        // return when error
        return buf_length;
    }
    
    dirp2 = (struct linux_dirent64 *)kmalloc(buf_length, GFP_KERNEL);
    if (!dirp2) {
        // kmalloc error
        return buf_length;
    }
    
    if (copy_from_user(dirp2, dirp, buf_length)) {
        conivent_printf("copy_from_user(dirp2, dirp, buf_length) failed");
        return buf_length;
    }
    
    head = dirp2;
    dirp3 = dirp2;
    modified_buf_length = buf_length;
    
    while (((unsigned long) dirp3) < (((unsigned long) dirp2) + buf_length)) {
        record_length = dirp3->d_reclen;
        
        if (record_length == 0) {
            // origin_getdents64 failed 
            break;
        }
        
        //if (strncmp(dirp3->d_name, hide_file, strlen(hide_file)) == 0) {
        if (isInodeProtected(dirp3->d_ino, FILE_PROTECT_READ)) {
            if (!prev) {
                // head is our file
                head = (struct linux_dirent64*)((char *)dirp3 + record_length);
                modified_buf_length -= record_length;
            }
            else {
                prev->d_reclen += record_length;
                memset(dirp3, 0, record_length);
            }
        }
        else {
            prev = dirp3;
        }
        
        // next elem in list
        dirp3 = (struct linux_dirent64 *)((char *)dirp3 + record_length);
    }
    
    copy_to_user(dirp, head, modified_buf_length);
    kfree(dirp2);
    
    return modified_buf_length;
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