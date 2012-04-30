#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/types.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <signal.h>
#include "imp2.h"

struct msg_to_kernel
{
  struct nlmsghdr hdr;
};

struct u_packet_info
{
  struct nlmsghdr hdr;
  struct packet_info icmp_info;
};

static int skfd;

static void sig_int(int signo)
{
  struct sockaddr_nl kpeer;
  struct msg_to_kernel message;

  memset(&kpeer, 0, sizeof(kpeer));
  kpeer.nl_family = AF_NETLINK;
  kpeer.nl_pid    = 0;
  kpeer.nl_groups = 0;

  memset(&message, 0, sizeof(message));
  message.hdr.nlmsg_len = NLMSG_LENGTH(0);
  message.hdr.nlmsg_flags = 0;
  message.hdr.nlmsg_type = IMP2_CLOSE;
  message.hdr.nlmsg_pid = getpid();

  sendto(skfd, &message, message.hdr.nlmsg_len, 0, (struct sockaddr *)(&kpeer),
         sizeof(kpeer));

  close(skfd);
  exit(0);
}

int main(void)
{
  struct sockaddr_nl local;
  struct sockaddr_nl kpeer;
  int kpeerlen;
  struct msg_to_kernel message;
  struct u_packet_info info;
  int sendlen = 0;
  int rcvlen = 0;
  struct in_addr addr;

  skfd = socket(PF_NETLINK, SOCK_RAW, NL_IMP2);
  if(skfd < 0)
    {
      printf("can not create a netlink socket\n");
      exit(0);
    }

  memset(&local, 0, sizeof(local));
  local.nl_family = AF_NETLINK;
  local.nl_pid = getpid();
  local.nl_groups = 0;
  if(bind(skfd, (struct sockaddr*)&local, sizeof(local)) != 0)
    {
      printf("bind() error\n");
      return -1;
    }

  signal(SIGINT, sig_int);

  memset(&kpeer, 0, sizeof(kpeer));
  kpeer.nl_family = AF_NETLINK;
  kpeer.nl_pid = 0;
  kpeer.nl_groups = 0;

  memset(&message, 0, sizeof(message));
  message.hdr.nlmsg_len = NLMSG_LENGTH(0);
  message.hdr.nlmsg_flags = 0;
  message.hdr.nlmsg_type = IMP2_U_PID;
  message.hdr.nlmsg_pid = local.nl_pid;

  sendto(skfd, &message, message.hdr.nlmsg_len, 0,
	 (struct sockaddr*)&kpeer, sizeof(kpeer));

  while(1)
    {
      kpeerlen = sizeof(struct sockaddr_nl);
      rcvlen = recvfrom(skfd, &info, sizeof(struct u_packet_info),
			0, (struct sockaddr*)&kpeer, &kpeerlen);

      addr.s_addr = info.icmp_info.src;
      printf("src: %s, ", inet_ntoa(addr));
      addr.s_addr = info.icmp_info.dest;
      printf("dest: %s\n", inet_ntoa(addr));
    }

  return 0;
}
