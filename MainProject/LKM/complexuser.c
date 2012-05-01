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

#define NETLINK_TEST 22
#define MAX_PAYLOAD 1024  // maximum payload size

#define PROC_DIR "/proc/gsfileprotection"
#define PROC_PROTECT "/proc/gsfileprotection/protect"
#define PROC_CTRL "/proc/gsfileprotection/ctrl"
#define PROC_NOTI "/proc/gsfileprotection/noti"

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
        printf("bind failed: %s", strerror(errno));
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

void printmenu() {
    printf("1. Protect list\n");
    printf("2. Add\n");
    printf("3. Remove\n");
    printf("9. Quit\n");
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
        printmenu();
        while (flag) {
            scanf("%d", &command);
            switch (command) {
                case 1:
                    
                    break;
                case 2:
                    
                    break;
                case 3:
                    
                    break;
                case 9:
                    if (kill(pid, SIGTERM) < 0) {
                        printf("kill error\n");
                    }
                    else {
                        printf("all terminate\n");
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