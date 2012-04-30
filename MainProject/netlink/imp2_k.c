#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include "imp2.h"

DECLARE_MUTEX(receive_sem);

static struct sock *nlfd;

struct {
    __u32 pid;
    rwlock_t lock;
}user_proc;

static void kernel_receive(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    //printk("%s: skb->user: %d\n", __func__, atomic_read(&skb->users)); return;
    
    
    
    if(down_trylock(&receive_sem))
        return;
    
    if(skb->len < sizeof(struct nlmsghdr))
        goto out;
    
    nlh = nlmsg_hdr(skb);
    
    if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
       && (skb->len >= nlh->nlmsg_len)) {
        if(nlh->nlmsg_type == IMP2_U_PID) {
            write_lock_bh(&user_proc.lock);
            user_proc.pid = nlh->nlmsg_pid;
            write_unlock_bh(&user_proc.lock);
        } else if(nlh->nlmsg_type == IMP2_CLOSE) {
            write_lock_bh(&user_proc.lock);
            if(nlh->nlmsg_pid == user_proc.pid)
                user_proc.pid = 0;
            write_unlock_bh(&user_proc.lock);
        }
    }
    
    //kfree_skb(skb);
    
    
out:
    up(&receive_sem);
}

static int send_to_user(struct packet_info *info)
{
    int ret;
    int size;
    unsigned char *old_tail;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    struct packet_info *packet;
    
    size = NLMSG_SPACE(sizeof(*info));
    
    skb = alloc_skb(size, GFP_ATOMIC);
    old_tail = skb->tail;
    
    nlh = NLMSG_PUT(skb, 0, 0, IMP2_K_MSG, size-sizeof(*nlh));
    packet = NLMSG_DATA(nlh);
    memset(packet, 0, sizeof(struct packet_info));
    
    packet->src = info->src;
    packet->dest = info->dest;
    
    nlh->nlmsg_len = skb->tail - old_tail;
    NETLINK_CB(skb).dst_group = 0;
    
    read_lock_bh(&user_proc.lock);
    ret = netlink_unicast(nlfd, skb, user_proc.pid, MSG_DONTWAIT);
    read_unlock_bh(&user_proc.lock);
    
    return ret;
    
nlmsg_failure:
    if(skb)
        kfree_skb(skb);
    return -1;
}

static unsigned int get_icmp(unsigned int hook,
                             struct sk_buff *pskb,
                             const struct net_device *in,
                             const struct net_device *out,
                             int (*okfn)(struct sk_buff *))
{
    printk("%s\n", __func__);
    struct iphdr *iph = (struct iphdr *)skb_network_header(pskb);
    struct packet_info info;
    
    if(iph->protocol == IPPROTO_ICMP) {
        read_lock_bh(&user_proc.lock);
        if(user_proc.pid != 0) {
            read_unlock_bh(&user_proc.lock);
            info.src = iph->saddr;
            info.dest = iph->daddr;
            send_to_user(&info);
        }
        else
            read_unlock_bh(&user_proc.lock);
    }
    
    return NF_STOLEN;
}

static struct nf_hook_ops imp2_ops =
{
    .hook = get_icmp,
    .pf = PF_INET,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

static int __init init(void)
{
    rwlock_init(&user_proc.lock);
    
    //nlfd = netlink_kernel_create(NL_IMP2, kernel_receive);
    
    
    nlfd = netlink_kernel_create(&init_net, NL_IMP2, 0, kernel_receive, NULL, THIS_MODULE);
    
    if(!nlfd) {
        printk("can not create a netlink socket\n");
        return -1;
    }
    return nf_register_hook(&imp2_ops);
}

static void __exit fini(void)
{
    if(nlfd) {
        sock_release(nlfd->sk_socket);
    }
    nf_unregister_hook(&imp2_ops);
}

module_init(init);
module_exit(fini);
