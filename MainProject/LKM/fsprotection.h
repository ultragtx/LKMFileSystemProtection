

// Orignal



// New

asmlinkage long modified_mkdir(const char *path);
asmlinkage int modified_chdir(const char *path);
asmlinkage int modified_rmdir(const char *pathname);

asmlinkage ssize_t modified_read(unsigned int fd,char* buf,size_t size);
asmlinkage ssize_t modified_write(int fd, const void *buf, size_t count);

asmlinkage int modified_open(const char *pathname, int flags, mode_t mode);