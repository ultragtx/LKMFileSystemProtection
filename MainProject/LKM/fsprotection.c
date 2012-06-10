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

#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/fs_struct.h>

#include "printstring.h"
#include "communicate.h"
#include "fsprotection.h"
#include "utilities.h"
#include "notification.h"

// test code here

typedef enum {
    FILE_PROTECT_READ,
    FILE_PROTECT_WRITE,
    FILE_PROTECT_OPEN,
    FILE_PROTECT_UNLINKAT
}FILE_PROTECT_TYPE;

int isFileProtected(unsigned int fd, FILE_PROTECT_TYPE type) {
    struct file *this_file = NULL;
    //char *path;
    //path = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    
    this_file = fget(fd);
    if (this_file != NULL) {
        if (this_file->f_dentry->d_inode == testfs->f_dentry->d_inode) {
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
    //conivent_printf("isInodeProtected %d, %d", inode, testfs->f_dentry->d_inode->i_ino);
    if (testfs->f_dentry->d_inode->i_ino == inode) {
        return 1;
    }
    return 0;
}

int isPathProtected(const char *path, FILE_PROTECT_TYPE type) {
    if (strcmp(path, filePath) == 0) {
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

asmlinkage int (*origin_unlinkat)(int dirfd, const char *pathname, int flags);
asmlinkage int (*origin_unlink)(const char *pathname);

asmlinkage int (*origin_rename)(const char *old, const char *new);
asmlinkage int (*origin_renameat)(int olddirfd, const char *oldpath, int newdirfd, const char *newpath);

asmlinkage int modified_mkdir(const char *path, mode_t mode) {
    int result;
    char *kernelpath = NULL;
    char *fullpath = NULL;
    
    copyStringFromUser(path, &kernelpath);
    getNewFullPath(kernelpath, &fullpath);

    //printk(KERN_ALERT "mkdir is not allowed now\n");
    conivent_printf("modified_mkdir 2, %s", fullpath);
    
    getParentDirFromFullPath(fullpath);
    
    conivent_printf("modified_mkdir 2, %s", fullpath);
    conivent_printf("modified_mkdir 2, mode %d", mode);
    
    if (is_path_protected(fullpath, ProtectType_mkdir)) {
        conivent_printf("modified_mkdir dir protected");
        result = -EACCES;
    }
    else {
        result = origin_mkdir(path, mode);
    }
    
    
    kfree(kernelpath);
    kfree(fullpath);
    
    return result;
}

asmlinkage int modified_chdir(const char *path) {
    int result;
    char *fullpath = NULL;
    
    copyStringFromUser(path, &fullpath);
    //getNewFullPath(kernelpathname, &fullpath);

    conivent_printf("modified_chdir 1， %s", fullpath);
    
    if (is_path_protected(fullpath, ProtectType_hide)) {
        conivent_printf("modified_chdir dir protected");
        result = -EACCES;
    }
    else {
        result = origin_chdir(path);
    }
    
    kfree(fullpath);
    return result;
}

asmlinkage int modified_rmdir(const char *path) {
    int result = origin_rmdir(path);
    char *fullpath = NULL;
    
    copyStringFromUser(path, &fullpath);
    conivent_printf("modified_rmdir 1, %s", fullpath);
    kfree(fullpath);
    return result;
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
    
    //char hide_file[] = "text.txt";
    
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
        
        //if (isInodeProtected(dirp3->d_ino, FILE_PROTECT_READ)) {
        if (is_ino_protected(dirp3->d_ino, ProtectType_hide)) {    
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
    /*if (isFileProtected(fd, FILE_PROTECT_READ)) {
        conivent_printf("file is protected");
        messageWithStr("file_read is protected");
        return -EACCES;
    }*/
    if (is_fd_protected(fd, ProtectType_read)) {
        conivent_printf("file is protected");
        return -EACCES;
    }
    return origin_read(fd, buf, size);
}

asmlinkage ssize_t modified_write(int fd, const void *buf, size_t count) {
    //conivent_printf("modified_write 3");
    /*if (isFileProtected(fd, FILE_PROTECT_WRITE)) {
        conivent_printf("file write is protected");
        return -EACCES;
    }*/
    if (is_fd_protected(fd, ProtectType_write)) {
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


asmlinkage int modified_unlinkat(int dirfd, const char *pathname, int flags) {
    int result;
    char *fullpath = NULL;
    char *kernelpathname = NULL;
    
    copyStringFromUser(pathname, &kernelpathname);
    getNewFullPath(kernelpathname, &fullpath);
    
    //conivent_printf("modified_unlinkat %d", dirfd);
    //conivent_printf("%s", fullpath);
    
    //if (isPathProtected(fullpath, FILE_PROTECT_UNLINKAT)) {
    if (is_path_protected(fullpath, ProtectType_del)) {
        conivent_printf("modified_unlinkat file protected");
        result = -EACCES;
    }
    else {
        result = origin_unlinkat(dirfd, pathname, flags);
    }
    
    kfree(kernelpathname);
    kfree(fullpath);
    //return -EACCES;
    return result;
}

asmlinkage int modified_unlink(const char *pathname) {
    int result;
    char *fullpath = NULL;
    char *kernelpathname = NULL;
    
    copyStringFromUser(pathname, &kernelpathname);
    getNewFullPath(kernelpathname, &fullpath);
    
    conivent_printf("modified_unlink %s", fullpath);
    
    //if (isPathProtected(fullpath, FILE_PROTECT_UNLINKAT)) {
    if (is_path_protected(fullpath, ProtectType_del)) {
        conivent_printf("modified_unlink file protected");
        result = -EACCES;
    }
    else {
        result = origin_unlink(pathname);
    }
    
    kfree(kernelpathname);
    kfree(fullpath);
    //return -EACCES;
    return result;
}

asmlinkage int modified_rename(const char *old, const char *new) {
    int result;
    char *oldfullpath = NULL;
    char *oldkernelpathname = NULL;
    
    char *newfullpath = NULL;
    char *newkernelpathname = NULL;
    
    copyStringFromUser(old, &oldkernelpathname);
    getNewFullPath(oldkernelpathname, &oldfullpath);
    
    copyStringFromUser(new, &newkernelpathname);
    getNewFullPath(newkernelpathname, &newfullpath);
    
    //conivent_printf("modified_rename %s %s", oldfullpath, newfullpath);
    printk(KERN_ALERT "modified_rename %s %s\n", oldfullpath, newfullpath);
    //if (isPathProtected(oldfullpath, FILE_PROTECT_UNLINKAT)) {// ||
        //isPathProtected(newfullpath, FILE_PROTECT_UNLINKAT)) {
    
    if (is_path_protected(oldfullpath, ProtectType_rename)) {
        conivent_printf("modified_rename file protected");
        //printk(KERN_ALERT "modified_rename file protected %s %s\n", oldfullpath, newfullpath);
        result = -EACCES;
    }
    else {
        result = origin_rename(old, new);
    }
    
    kfree(oldfullpath);
    kfree(oldkernelpathname);
    kfree(newfullpath);
    kfree(newkernelpathname);
    
    return result;
}

asmlinkage int modified_renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath) {
    
    conivent_printf("modified_renameat");
    printk(KERN_ALERT "modified_renameat\n");
    
    return origin_renameat(olddirfd, oldpath, newdirfd, newpath);
}








