#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
#endif
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <time.h>

#ifdef READLINE_LIBRARY
#  include "readline.h"
#  include "history.h"
#else
#  include <readline/readline.h>
#  include <readline/history.h>
#endif

#define NETLINK_TEST 22
#define MAX_PAYLOAD 1024  // maximum payload size

#define PROC_DIR "/proc/gsfileprotection"
#define PROC_PROTECT "/proc/gsfileprotection/protect"
#define PROC_CTRL "/proc/gsfileprotection/ctrl"
#define PROC_NOTI "/proc/gsfileprotection/noti"

#define FILEPATH_SIZE 256
#define FILELIST_SIZE 256
#define BUFFER_SIZE (FILEPATH_SIZE * FILELIST_SIZE)

///home/hiro/ProtectFile/text.txt

char *command_generator PARAMS((const char *, int));
char **fileman_completion PARAMS((const char *, int, int));

void initialize_readline () {
    rl_readline_name = ":";
    
    //rl_attempted_completion_function = fileman_completion;
}


char **fileman_completion (text, start, end)
const char *text;
int start, end;
{
    char **matches;
    
    matches = (char **)NULL;
    
    /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
    if (start == 0)
        matches = rl_completion_matches (text, command_generator);
    
    return (matches);
}

char *
command_generator (text, state)
const char *text;
int state;
{
    static int list_index, len;
    char *name;
    
    /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
    if (!state)
    {
        list_index = 0;
        len = strlen (text);
    }
    
    /* Return the next name which partially matches from the command list. */
    /*while (name = commands[list_index].name)
     {
     list_index++;
     
     if (strncmp (name, text, len) == 0)
     return (dupstr(name));
     }*/
    
    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

int protectfd;
int ctrlfd;
int notifd;

struct fileList {
    char filePath[FILEPATH_SIZE];
    char type;
    struct fileList *next;
} *fileList_root;

struct fileList *tail;

void refreshstdin() {
    char ch;
    while (ch = getchar()) {
        if (ch != '\n');
        break;
    }
}

int notificationReceiver() {
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd, retval;
    
    // Create a socket
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if(sock_fd == -1){
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    
    // To prepare binding
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = PF_NETLINK; 
    src_addr.nl_pid = getpid();  // self pid 
    src_addr.nl_groups = 1; // multi cast
    
    retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    if(retval < 0){
        printf("bind failed: %s\n", strerror(errno));
        close(sock_fd);
        return -1;
    }
    
    // To prepare recvmsg
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nlh){
        printf("malloc nlmsghdr error!\n");
        close(sock_fd);
        return -1;
    }
    
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
    
    memset(&msg, 0, sizeof(msg));
    memset(&dest_addr, 0, sizeof(dest_addr));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    // Read message from kernel
    while(1){
        recvmsg(sock_fd, &msg, 0);
        printf("Received message: %s\n", (char *)NLMSG_DATA(nlh));
    }
    
    close(sock_fd);
    
    return 0;
}

void printFileList() {
    struct fileList *next = fileList_root;
    int number = 0;
    printf("============List============\n");
    while (next) {
        printf("[%d].%s@%c\n", number++, next->filePath, next->type);
        next = next->next;
    }
    printf("============================\n");
    
    if (tail) {
        printf("tail not null\n");
    }
}

void refreshFileList() {
    char *buffer;
    char *p;
    int len = 0;
    struct fileList *next = fileList_root;
    int tmp;
    
    buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer, 0, sizeof(char) * BUFFER_SIZE);
    
    p = buffer;
    while (next != 0) {
        tmp = strlen(next->filePath);
        
        strncpy(p, next->filePath, tmp);
        
        p += tmp;
        *p = '@';
        p++;
        *p = next->type;
        p++;
        *p = '\n';
        p++;
        
        next = next->next;
    }

    printf("buffer is:\n%s----\n", buffer);
    
    write(protectfd, buffer, strlen(buffer));
    
    free(buffer);
}

void addFile() {
    struct fileList *pfile = NULL;
    char *line;
    int len;
    int i;
    
    pfile = (struct fileList *)malloc(sizeof(struct fileList));
    memset(pfile, 0, sizeof(struct fileList));
    
    refreshstdin();
    printf("Input file path\n");
    line = readline(":");
    len = strlen(line);
    for (i = 0; i < len; i++) {
        pfile->filePath[i] = line[i];
    }
    if (pfile->filePath[len - 1] == ' ') {
        pfile->filePath[len - 1] = '\0'; // remove the slash at the end
        len--;
    }
    if (pfile->filePath[len - 1] == '/') {
        pfile->filePath[len - 1] = '\0'; // remove the slash at the end
        len--;
    }
    if (line) {
        free(line);
    }
    //scanf("%[^\n]", pfile->filePath);
    
    printf("Input protect type: r(read), w(write), h(hide), d(delete), n(rename)\n:");
    //refreshstdin();
    scanf("%c", &pfile->type);
    
    pfile->next = NULL;
    
    //printf("%s@%c\n----\n", pfile->filePath, pfile->type);
    
    if (tail == NULL) {
        fileList_root = pfile;
        tail = fileList_root;
    }
    else {
        tail->next = pfile;
        tail = tail->next;
    }
    
    refreshFileList();
}

void removeFile() {
    struct fileList *pfile = fileList_root;
    struct fileList *pprevous = NULL;
    int number;
    int currentNum = 0;
    
    if (pfile == NULL) {
        printf("No files being protected!\n");
        return;
    }
    printFileList();
    refreshstdin();
    printf("Input the number you want to remove:\n");
    
    scanf("%d", &number);
    
    if (number == 0) {
        //printf("current:%d root:%d tail:%d\n", pfile, fileList_root, tail);
        fileList_root = pfile->next;

        if (pfile == tail) {
            tail = NULL;
        }
        free(pfile);
    }
    else if (number >= 0) {
        while (pfile) {
            if (currentNum == number) {
                pprevous->next = pfile->next;
                
                if (pfile == tail) {
                    tail = pprevous;
                }
                
                free(pfile);
                break;
            }
            pprevous = pfile;
            pfile = pfile->next;
            currentNum++;
        }
    }
    
    refreshFileList();
}

void printmenu() {
    printf("1. Protect list\n");
    printf("2. Add\n");
    printf("3. Remove\n");
    printf("9. Quit\n");
}

void exitall() {
    printf("all terminate\n");
    close(protectfd);
    close(ctrlfd);
    close(notifd);
}

int main(int argc, char *argv[]) {
    pid_t pid;
    int command;
    int flag = 1;
        
    pid = fork();
    
    if (pid < 0) {
        printf("fork error\n");
        return -1;
    }
    else if (pid == 0) {
        // child
        notificationReceiver();
    }
    else {
        // father
        initialize_readline();
        fileList_root = NULL;
        tail = fileList_root;
        
        protectfd = open(PROC_PROTECT, O_RDWR);
        ctrlfd = open(PROC_NOTI, O_RDWR);
        notifd = open(PROC_NOTI, O_RDWR);
        
        if (protectfd < 0 || ctrlfd < 0 || notifd < 0) {
            printf("open file error!\n");
            exitall();
            return -1;
        }

        printmenu();
        while (flag) {
            scanf("%d", &command);
            switch (command) {
                case 1:
                    printFileList();
                    break;
                case 2:
                    addFile();
                    break;
                case 3:
                    removeFile();
                    break;
                case 9:
                    if (kill(pid, SIGTERM) < 0) {
                        printf("kill error\n");
                    }
                    else {
                        exitall();
                        flag = 0;
                    }
                    break;
                case 4:
                    
                    break;
                default:
                    printmenu();
                    break;
            }
        }
    }
}