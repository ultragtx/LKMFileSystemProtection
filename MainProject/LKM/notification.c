#include "notification.h"
#include "printstring.h"

#include <linux/timer.h>
#include <linux/types.h>
#include <net/sock.h>
#include <net/netlink.h>

#define NETLINK_TEST 22
#define MAX_MSGSIZE 1024

struct sock *nl_sk = NULL;

int stringlength(char *s) {
	int slen = 0;
    
	for(; *s; s++){
        slen++;
    }
    
	return slen;
}

void sendnlmsg(char *message) {
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int len = NLMSG_SPACE(MAX_MSGSIZE);
	int slen = 0;
	
	if(!message || !nl_sk){
        return;
    }
    
	// Allocate a new sk_buffer
	skb = alloc_skb(len, GFP_KERNEL);
	if(!skb){
		printk(KERN_ERR "my_net_link: alloc_skb Error.\n");
		return;
	}
    
	slen = stringlength(message);
    
    //Initialize the header of netlink message
	nlh = nlmsg_put(skb, 0, 0, 0, MAX_MSGSIZE, 0);
    
	NETLINK_CB(skb).pid = 0; // from kernel
    NETLINK_CB(skb).dst_group = 1; // multi cast
    
	message[slen] = '\0';
	memcpy(NLMSG_DATA(nlh), message, slen+1);
    printk("my_net_link: send message '%s'.\n", (char *)NLMSG_DATA(nlh));
    
	//send message by multi cast
    netlink_broadcast(nl_sk, skb, 0, 1, GFP_KERNEL); 
    return;
}

int messageWithStr(char *str) {
    struct completion cmpl;
    conivent_printf("ready to send");
    init_completion(&cmpl);
    wait_for_completion_timeout(&cmpl, 3 * HZ);
    sendnlmsg(str);
    conivent_printf("send done");
    return 0;
}

int notification_init(void) {
    //struct completion cmpl;
    
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, 1,   
                                  NULL, NULL, THIS_MODULE);
    
    if (!nl_sk) {
        conivent_printf("netlink create socket error");
        return -1;
    }
    
    return 0;
}

int notification_exit(void) {
    if (nl_sk != NULL) {
        sock_release(nl_sk->sk_socket);
    }
    return 0;
}