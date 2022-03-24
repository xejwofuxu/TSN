#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pcap.h> /* GIMME a libpcap plz! */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "faker_tsn.h"
#include "test.h"

int main(int argc, char **argv) {
    // test_state_pattern();
    // test_logger();
    // test_port_manager();
    // test_reactor();

    // char *dev; /* name of the device to use */
    // char *net; /* dot notation of the network address */
    // char *mask;/* dot notation of the network mask */
    // int ret;   /* return code */
    // char errbuf[PCAP_ERRBUF_SIZE];
    // bpf_u_int32 netp; /* ip */
    // bpf_u_int32 maskp;/* subnet mask */
    // struct in_addr addr;

    // /* ask pcap to find a valid device for use to sniff on */
    // dev = pcap_lookupdev(errbuf);

    // /* error checking */
    // if(dev == NULL)
    // {
    //     printf("%s\n",errbuf);
    //     exit(1);
    // }

    // /* print out device name */
    // printf("DEV: %s\n",dev);

    // /* ask pcap for the network address and mask of the device */
    // ret = pcap_lookupnet(dev,&netp,&maskp,errbuf);

    // if(ret == -1)
    // {
    //     printf("%s\n",errbuf);
    //     exit(1);
    // }

    // /* get the network address in a human readable form */
    // addr.s_addr = netp;
    // net = inet_ntoa(addr);

    // if(net == NULL)/* thanks Scott :-P */
    // {
    //     perror("inet_ntoa");
    //     exit(1);
    // }
    // printf("NET: %s\n",net);

    // /* do the same as above for the device's mask */
    // addr.s_addr = maskp;
    // mask = inet_ntoa(addr);

    // if(mask == NULL)

    // {
    //     perror("inet_ntoa");
    //     exit(1);
    // }

    // printf("MASK: %s\n",mask);

    printf("tsn app");

    return 0;
}
