

int getPathFromFd(unsigned int fd, char *path);
char *getfullPath(const char *pathname, char *fullpath);
char *getstringfromuser(const char *userstring, char *kernelstring);

//// call the following without kmalloc but should kfree them yourself

int getNewPathFromFd(unsigned int fd, char **path);
int getNewFullPath(const char *pathname, char **fullpath);
int copyStringFromUser(const char *userstring, char **kernelstring);