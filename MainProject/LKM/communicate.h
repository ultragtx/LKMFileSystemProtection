
typedef enum {
    ProtectType_none,
    ProtectType_read,
    ProtectType_write,
    ProtectType_hide,
    ProtectType_open,
    ProtectType_del
}ProtectType;

int init_communicate(void);
void cleanup_communicate(void); 

// test code

extern char *filePath;
extern struct file *testfs;

// test end