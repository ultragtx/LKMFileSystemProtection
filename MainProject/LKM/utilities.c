#include "utilities.h"

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
#include <linux/dirent.h> 

#include "printstring.h"

int getPathFromFd(unsigned int fd, char *path) {
    struct file *file;
    struct dentry *p_dentry;
    char *root_path;
    char *temp_path;
    root_path = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    temp_path = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    
    file = fget(fd);
    if (file == NULL) {
        fput(file);
        return -1;
    }
    p_dentry = file->f_dentry;
    strcpy(root_path, p_dentry->d_sb->s_root->d_iname);
    //conivent_printf("%s", root_path);
    //conivent_printf("%d", strlen(root_path));
    sprintf(path, "%s", p_dentry->d_name.name);
    //conivent_printf("%s", path);
    //conivent_printf("%s", p_dentry->d_name.name);
    p_dentry = p_dentry->d_parent;
    while (strcmp(p_dentry->d_name.name, root_path)) { // not root
        //conivent_printf("%s", p_dentry->d_name.name);
        sprintf(temp_path, "%s/%s", p_dentry->d_name.name, path);
        sprintf(path, "%s", temp_path);
        p_dentry = p_dentry->d_parent;
        //sprintf(path, "%s/%s", p_dentry->d_name.name, path);
    }
    //if (strcmp("/", root_path)) {
    if (strlen(root_path) == 1) {
        //conivent_printf("equal");
        sprintf(temp_path, "/%s", path);
    }
    else {
        sprintf(temp_path, "%s/%s", root_path, path);
    }
    sprintf(path, "%s", temp_path);
    fput(file);
    kfree(root_path);
    kfree(temp_path);
    return 0;
}

char *getfullPath(const char *pathname, char *fullpath) {
    //char *fullpath = NULL;
    char *path = NULL;
    char *start = NULL;
    //struct dentry *pwd;
    //struct vfsmount *vfsmount;
    
    struct fs_struct *fs = current->fs;
    
    struct path pwd;
    /*fullpath = kmalloc(PATH_MAX, GFP_KERNEL);
     if (!fullpath) {
     // kmalloc error
     return fullpath;
     }
     memset(fullpath, 0, PATH_MAX);*/
    
    path = kmalloc(PATH_MAX, GFP_KERNEL);
    if (!path) {
        return NULL;
    }
    // 2.4
    // get dentry and vfsmnt
    //read_lock(&(fs->lock));
    //pwd = dget(fs->pwd);
    //vfsmount = mntget(fs->pwdmnt);
    //read_unlock(&(fs->lock));
    
    // get path
    //start = d_path(pwd, vfsmount, path, PATH_MAX);
    //strcat(fullpath, start);
    
    // 2.6.32
    read_lock(&fs->lock);
    pwd = fs->pwd;
    path_get(&pwd);
    read_unlock(&fs->lock);
    //set_fs_pwd(fs, &pwd);
    start = d_path(&pwd, path, PATH_MAX);

    strcat(fullpath, start);
    strcat(fullpath, "/");
    strcat(fullpath, pathname);
    
    // 2.6.35
    // use spinlock
    
    kfree(path);
    return fullpath;
}

char *getstringfromuser(const char *userstring, char *kernelstring) {
    //int index = 0;
    const char *src = userstring;
    char *tar = kernelstring;
    do {
        //kernelstring[index++] = __get_user(tmp++, 1);
        copy_from_user(tar++, src++, 1);
    } while (*(tar - 1) != '\0');
    //conivent_printf("%s", kernelstring);
    return kernelstring;
}

//// call the following without alloc

// return 0 success others failed
int getNewPathFromFd(unsigned int fd, char **path) {
    *path = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    
    if (!path) {
        conivent_printf("getNewPathFromFd kmalloc error");
        return -1;
    }
    
    return getPathFromFd(fd, *path);
}

int getNewFullPath(const char *pathname, char **fullpath) {
    char *temp = NULL;
    *fullpath = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    if (!*fullpath) {
        conivent_printf("getNewFullPath kmalloc error");
        return -1;
    }
    temp = getfullPath(pathname, *fullpath);
    if (temp == NULL) {
        conivent_printf("getNewFullPath kmalloc error");
        return -1;
    }
    return 0;
}

int copyStringFromUser(const char *userstring, char **kernelstring) {
    *kernelstring = (char *)kmalloc(PATH_MAX, GFP_KERNEL);
    if (!*kernelstring) {
        conivent_printf("copyStringFromUser kmalloc error");
        return -1;
    }
    getstringfromuser(userstring, *kernelstring);
    return 0;
}

int getParentDirFromFullPath(char *path) {
    int len = strlen(path);
    int slashCount = 0;
    int i;
    if (path[len - 1] == '/') {
        slashCount = 2;
    }
    else {
        slashCount = 1;
    }
    
    for (i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            slashCount--;
            if (slashCount == 0) {
                path[i + 1] = '\0';
                break;
            }
        }
    }
    return 0;
}
