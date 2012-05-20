#include <linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>

#include<linux/file.h>
#include<linux/vmalloc.h>
#include<linux/string.h>
#include<asm/uaccess.h>
#include<linux/namei.h>
#include<linux/fs.h>
#include<linux/sched.h>

#include "printstring.h"
#include "communicate.h"
#include "notification.h"

#define PROC_DIR_NAME "gsfileprotection"
#define PROC_PROTECT_NAME "protect"
#define PROC_CTRL_NAME "ctrl"
#define PROC_NOTI_NAME "noti"

// test code here
char *filePath = "/home/hiro/ProtectFile/text.txt";
char *dirPath = "/home/hiro/ProctectDir";
struct file *testfs = NULL;


void getFs(void) {
    testfs = filp_open(filePath, 0, 1);
    if (IS_ERR(testfs)) {
        conivent_printf("testfs error");
    }
    conivent_printf("testfs is:%d", (int)testfs);
}

// test code end
#define FILEPATH_SIZE 256
#define FILELIST_SIZE 256
#define BUFFER_SIZE (FILEPATH_SIZE * FILELIST_SIZE)

char *data_buf;
char *ctrl_data_buf;
char *noti_data_buf;
char *raw_data_buff;

struct fileList {
    char filePath[FILEPATH_SIZE];
    struct inode *inode;
    struct fileList *next;
    ProtectType type;
} *fileList_root;

int protect_enabeled;


struct proc_dir_entry *proc_dir;
struct proc_dir_entry *proc_protect;
struct proc_dir_entry *proc_ctrl;
struct proc_dir_entry *proc_noti;

/**
 file protection
 */

int sendProtectNotiMsg(struct fileList *thisFile) {
    char *str;
    //char *p;
    char *msghead = "Protected:";
    int flag = 1;
    
    str = (char *)vmalloc(sizeof(char) * 260);
    memset(str, 0, sizeof(char) * 260);
    
    strcat(str, msghead);
    strcat(str, thisFile->filePath);
    
    switch (thisFile->type) {
        case ProtectType_read:
            strcat(str, "\nRead");
            break;
        case ProtectType_write:
            strcat(str, "\nWrite");
            break;
        case ProtectType_hide:
            strcat(str, "\nHide");
            break;
        case ProtectType_open:
            strcat(str, "\nOpen");
            break;
        case ProtectType_del:
            strcat(str, "\nDelete");
            break;
            
        default:
            flag = 0;
            break;
    }
    if (flag) {
        //conivent_printf(str);
        messageWithStr(str);
    }
    vfree(str);
    return 0;
}

int is_fd_protected(unsigned int fd, ProtectType ptype) {
    struct file *this_file;
    struct fileList *p;
    int protected = 0;
    
    if (protect_enabeled) {
        this_file = fget(fd);
        for (p = fileList_root;p; p = p->next) {
            if (p->inode == this_file->f_dentry->d_inode && p->type == ptype) {
                protected = 1;
                sendProtectNotiMsg(p);
                break;
            }
        }
        fput(this_file);
    }
    return protected;
}

int is_ino_protected(unsigned long ino, ProtectType ptype) {
    struct fileList *p;
    
    for (p = fileList_root; p; p = p->next) {
        if (p->inode->i_ino == ino && p->type == ptype) {
            sendProtectNotiMsg(p);
            return 1;
        }
    }
    return 0;
}

int is_path_protected(const char *path, ProtectType ptype) {
    struct fileList *p;
    
    for (p = fileList_root; p; p = p->next) {
        if (strcmp(path, p->filePath) == 0 && p->type == ptype) {
            sendProtectNotiMsg(p);
            return 1;
        }
    }
    return 0;
}

/**
 protect list 
 */

ProtectType protectTypeFromStr(char *str) {
    ProtectType type = ProtectType_none;
    if (!strcmp(str, "r")) {
        type = ProtectType_read;
    }
    else if (!strcmp(str, "w")) {
        type = ProtectType_write;
    }
    else if (!strcmp(str, "h")) {
        type = ProtectType_hide;
    }
    else if (!strcmp(str, "o")) {
        type = ProtectType_open;
    }
    else if (!strcmp(str, "d")) {
        type = ProtectType_del;
    }
    return type;
}

void log_list(void) {
    struct fileList *next = fileList_root;
    conivent_printf("---[kernel begin]--------");
    while (next) {
        
        conivent_printf("filePath:%s", next->filePath);
        conivent_printf("inode:%d", next->inode);
        conivent_printf("type:%d", next->type);
        
        next = next->next;
    }
    conivent_printf("---[kernel end]--------");
}

int create_list(struct fileList **top, char *rdt, size_t sz) {
    int cnt;
    char *tmp_data;
    char **pp_str1;
    char *p_str1;
    char **pp_substr1;
    char *p_substr1;
    char nl[] = "\n";
    char at[] = "@";
    int i;
    struct fileList *p_fl = NULL;
    struct fileList *lastvalid_fl = NULL;
    struct file *fs;
    
    for (lastvalid_fl = *top; lastvalid_fl;) {
        if (lastvalid_fl->next) {
            lastvalid_fl = lastvalid_fl->next;
        }
        else
            break;
    }
    
    tmp_data = (char *)vmalloc(sz);
    memset(tmp_data, 0, sz);
    strcpy(tmp_data, rdt);
    pp_str1 = &tmp_data;
    
    i = 0;
    cnt = 0;
    do {
        i++;
        p_str1 = (char *)strsep(pp_str1, nl);
        //conivent_printf("p_str1 %s", p_str1);
        pp_substr1 = &p_str1;
        p_substr1 = (char *)strsep(pp_substr1, at);
        //conivent_printf("p_substr1 %s", p_substr1);
        if (p_substr1) {
            if (!strcmp(p_substr1, "")) {
                fs = 0;
            }
            else {
                fs = filp_open(p_substr1, 0, 1);
            }
            
            if (!IS_ERR(fs) && fs) {
                cnt++;
                
                p_fl = (struct fileList *)vmalloc(sizeof(struct fileList));
                strncpy(p_fl->filePath, p_substr1, FILEPATH_SIZE);
                // protect mode
                //p_substr1 = (char *)strsep(pp_substr1, at);
                //conivent_printf("pp_substr1 %s", *pp_substr1);
                p_fl->type = protectTypeFromStr(*pp_substr1);
                // protect mode
                p_fl->inode = fs->f_dentry->d_inode;
                p_fl->next = NULL;
                if (NULL == lastvalid_fl) {
                    //conivent_printf("root null");
                    *top = p_fl;
                }
                else {
                    //conivent_printf("has tail");
                    p_fl->next = lastvalid_fl->next;
                    lastvalid_fl->next = p_fl;
                }
                lastvalid_fl = p_fl;
                filp_close(fs, current->files);
            }
        }
    } while (p_str1);
    
    vfree(tmp_data);
    
    return cnt;
}

void destroy_list(struct fileList **top) {
    struct fileList *p1, *p2;
    p1 = *top;
    while (p1) {
        p2 = p1->next;
        p1->inode = NULL;
        vfree((void *)p1);
        p1 = p2;
    }
    *top = NULL;
}

void refresh(void) {
    destroy_list(&fileList_root);
    create_list(&fileList_root, data_buf, sizeof data_buf);
}

void clear(void) {
    destroy_list(&fileList_root);
    memset(data_buf, 0, sizeof data_buf);
    create_list(&fileList_root, data_buf, sizeof data_buf);
}

void disable(void) {
    protect_enabeled = 0;
}

void enable(void) {
    protect_enabeled = 1;
}

/**
 Costom read write
 */
int proc_ctrl_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
    //conivent_printf("proc_ctrl read");
    return 0;
}

int proc_ctrl_write(struct file *file, const char *buffer, unsigned long count, void *data) {
    int cnt;
    char *raw_data = raw_data_buff;
    
    conivent_printf("proc_ctrl write");
    
    if (count > BUFFER_SIZE) {
        cnt = BUFFER_SIZE;
    }
    else if (count == 0) {
        return 0;
    }
    else {
        cnt = count;
    }
    
    if (copy_from_user(raw_data, buffer, cnt)) {
        return -EFAULT;
    }
    
    raw_data[cnt] = 0;
    
    if (!strncmp(raw_data, "refresh", 7)) {
        conivent_printf("refresh");
        refresh();
    }
    else if (!strncmp(raw_data, "clear", 5)) {
        conivent_printf("clear");
        clear();
    }
    else if (!strncmp(raw_data, "disable", 7)) {
        conivent_printf("disable");
        disable();
    }
    else if (!strncmp(raw_data, "enable", 6)) {
        conivent_printf("enable");
        enable();
    }
    
    return cnt;
}

int proc_noti_read(char * buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
    return 0;
}

int proc_noti_write(struct file *file, const char *buffer, unsigned long count, void *data) {
    return 0;
}

int proc_protect_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
    conivent_printf("proc_protect read");
    return 0;
}

int proc_protect_write(struct file *file, const char *buffer, unsigned long count, void *data) {
    int cnt;
    int list_cnt;
    char *raw_data = raw_data_buff;
    
    conivent_printf("proc_protect write");
    
    if (count > BUFFER_SIZE) {
        cnt = BUFFER_SIZE;
    }
    else if (count == 0) {
        return 0;
    }
    else {
        cnt = count;
    }
    
    if (copy_from_user(raw_data, buffer, cnt)) {
        return -EFAULT;
    }
    
    raw_data[cnt] = 0;
    //conivent_printf("raw_data %s", raw_data);
    destroy_list(&fileList_root);
    list_cnt = create_list(&fileList_root, raw_data, sizeof raw_data);
    log_list();
    return cnt;
}

int init_communicate(void) {
    // test code
    getFs();
    fileList_root = NULL;
    // test end
    data_buf = (char *)vmalloc(BUFFER_SIZE);
    ctrl_data_buf = (char *)vmalloc(BUFFER_SIZE);
    noti_data_buf = (char *)vmalloc(BUFFER_SIZE);
    raw_data_buff = (char *)vmalloc(BUFFER_SIZE);
    
    if (data_buf == NULL || ctrl_data_buf == NULL || noti_data_buf == NULL) {
        conivent_printf("create buf failed");
        return -ENOMEM;
    }
    else {
        memset(data_buf, 0, BUFFER_SIZE);
        memset(ctrl_data_buf, 0, BUFFER_SIZE);
        memset(noti_data_buf, 0, BUFFER_SIZE);
    }
    
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
        proc_ctrl = create_proc_entry(PROC_CTRL_NAME, 0666, proc_dir);
        proc_noti = create_proc_entry(PROC_NOTI_NAME, 0666, proc_dir);
        if (proc_protect == NULL || proc_ctrl == NULL || proc_noti == NULL) {
            conivent_printf("can't create proc file");
            remove_proc_entry(PROC_PROTECT_NAME, proc_dir);
            remove_proc_entry(PROC_CTRL_NAME, proc_dir);
            remove_proc_entry(PROC_NOTI_NAME, proc_dir);
            return -1;
        }
    }
    
    proc_protect->read_proc = proc_protect_read;
    proc_protect->write_proc = proc_protect_write;
    //proc_protect->owner = THIS_MODULE;
    
    proc_ctrl->read_proc = proc_ctrl_read;
    proc_ctrl->write_proc = proc_ctrl_write;
    //proc_ctrl->owner = THIS_MODULE;
    
    protect_enabeled = 1;
    
    return 0;
}

void cleanup_communicate(void) {
    conivent_printf("start remove proc");
    destroy_list(&fileList_root);
    vfree(data_buf);
    vfree(ctrl_data_buf);
    vfree(noti_data_buf);
    vfree(raw_data_buff);
    remove_proc_entry(PROC_PROTECT_NAME, proc_dir);
    remove_proc_entry(PROC_CTRL_NAME, proc_dir);
    remove_proc_entry(PROC_NOTI_NAME, proc_dir);
    remove_proc_entry(PROC_DIR_NAME, NULL);
}