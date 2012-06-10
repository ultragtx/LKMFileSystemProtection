
typedef enum {
    ProtectType_none,
    ProtectType_read,
    ProtectType_write,
    ProtectType_hide,
    ProtectType_rename,
    ProtectType_open,
    ProtectType_del,
    ProtectType_mkdir
}ProtectType;

int init_communicate(void);
void cleanup_communicate(void); 

// test code

extern char *filePath;
extern struct file *testfs;

// test end


int is_fd_protected(unsigned int fd, ProtectType ptype);
int is_ino_protected(unsigned long ino, ProtectType ptype);
int is_path_protected(const char *path, ProtectType ptype);