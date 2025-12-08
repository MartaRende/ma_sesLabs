#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>        /* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

static uint32_t print_pkt(struct nfq_data *tb);
static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
                    struct nfq_data *nfa, void *data);

int main(int argc, char **argv)
{
struct nfq_handle *h;
struct nfq_q_handle *qh;
int fd, rv;
char buf[4096] __attribute__ ((aligned(4)));

    h = nfq_open();                    //opening library handle
    nfq_unbind_pf(h, AF_INET);         //unbinding existing nf_queue handler for AF_INET

    nfq_bind_pf(h, AF_INET);           // binding nfnetlink_queue as nf_queue handler for AF_INET
    qh = nfq_create_queue(h,  0, &callback, NULL); // binding this socket to queue '0'

    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff); // setting copy_packet mode
    fd = nfq_fd(h);
    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
       printf("pkt received\n");
       nfq_handle_packet(h, buf, rv);
    }
    nfq_destroy_queue(qh);
    nfq_close(h);
    exit(0);
}

static u_int32_t print_pkt (struct nfq_data *tb)
{
    int id = 0;
    struct nfqnl_msg_packet_hdr *ph;
    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) {
       id = ntohl(ph->packet_id);
       printf("hw_protocol=0x%04x hook=%u id=%u \n ",ntohs(ph->hw_protocol), ph->hook, id);
    }
    return id;
}

static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
   u_int32_t id = print_pkt(nfa);
   printf("entering callback\n");
   return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}
