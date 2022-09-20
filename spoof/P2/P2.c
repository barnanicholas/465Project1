#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>



// checksum provided by profs tutorial
unsigned short csum (unsigned short *buf, int nwords)
{
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
      sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}


int main(int argc, char **argv)
{
    int sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    struct sockaddr_in sin;
    int datagramsize = 1024;
    char datagram[datagramsize];

    if(sd < 0)
    {
        perror("socket() error"); exit(-1);
    }

    sin.sin_family = AF_INET;

    memset (datagram, 0, datagramsize); // zero out the datagram
    sizeof(datagram);
    // - construct the IP header
    struct ip * iph = (struct ip*) datagram;
    struct icmphdr *icmph = (struct icmphdr *) (datagram + sizeof(struct ip));

    iph->ip_hl = 5; // there will be no options added to the ip packet
    iph->ip_v = 4; // version 4
    iph->ip_tos = 0; // leave 0
    iph->ip_len = ((short)(unsigned short)(sizeof (struct ip) + sizeof (struct icmphdr)));	// we are sending one ip and one udp packet
    iph->ip_id = htons((short)1234);	// does not matter unless we are sending multiple packets
    iph->ip_off = 0; // offset is zero bc we only send one packet
    iph->ip_ttl = 255; // make it sufficiently large so it doesnt go away
    iph->ip_p = 1; // set to 17 so we know we are sending udp packet
    iph->ip_sum = 0; // this will be calculated later
    iph->ip_src.s_addr = inet_addr ("192.168.1.183"); // where we want the packet to come fram
    iph->ip_dst.s_addr = inet_addr ("192.168.1.223"); // where we want packet to go

    // - construct ICMP header with contents

    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->checksum = 0; // change later 
    icmph->un.echo.id = 10;
    icmph->un.echo.sequence = 0;

    char * message = (char *)(datagram + sizeof(struct ip) + sizeof(struct icmphdr));
    char contents[] = "Wow, spoofing is great!";
    memcpy(message, contents, sizeof(contents));

    //udph->len = htons(ntohs(udph->len) + sizeof(contents));
    iph->ip_len = iph->ip_len + sizeof(contents);
    

    // - fill in the data part if needed ...
    // Note: you should pay attention to the network/host byte order.
    /* Send out the IP packet.
    * ip_len is the actual size of the packet. */

    iph->ip_sum = htons(csum ((unsigned short *) datagram, iph->ip_len >> 1));
    
    icmph->checksum = csum ((unsigned short *) icmph , (sizeof(struct icmphdr) + sizeof(contents)) >> 1);

    // printf("\n%d\n", iph->ip_len);

    {				/* lets do it the ugly way.. */
    int one = 1;
    const int *val = &one;
    if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
      printf ("Warning: Cannot set HDRINCL!\n");
    }

    // for (int i = 0; i < datagramsize; i++)
    // {
    //   printf("%d ", datagram[i]);
    //   if(i % 16 == 0)
    //     printf("\n");
    // }
    

    iph->ip_len = htons(iph->ip_len);

    if(sendto(sd, datagram, ntohs(iph->ip_len), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        perror("sendto() error"); exit(-1);
    }
    printf("ran\n");



}