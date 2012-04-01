#include <linux/dirent.h> 
//#include <dirent.h> 

// Orignal

struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[];
};

// New

asmlinkage int modified_mkdir(const char *path, mode_t mode);
asmlinkage int modified_chdir(const char *path);
asmlinkage int modified_rmdir(const char *pathname);

asmlinkage int modified_getdents64(unsigned int fd, struct linux_dirent *dirp,
                                 unsigned int count);

asmlinkage ssize_t modified_read(unsigned int fd,char* buf,size_t size);
asmlinkage ssize_t modified_write(int fd, const void *buf, size_t count);

asmlinkage int modified_open(const char *pathname, int flags, mode_t mode);