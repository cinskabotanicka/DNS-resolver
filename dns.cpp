/**
 * Projekt:     DNS resolver
 * @file        dns.cpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#define PCKT_LEN 65536 //maximalni velikost UDP paketu
#define RANDOM_NUMBER_FOR_ID 3290

#include <string>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include "struct.hpp"
#include "argument_parser.hpp"
#include "dns_functions.hpp"

int main(int argc, char **argv)
{
	// zpracovani argumentu prikazove radky a ulozeni do promennych
    std::vector<std::string> args(argv, argv + argc);
    std::string server_ip;
    std::string target_address;
	
    bool t_address_loaded = false;
    bool recursion_desired = false;
    bool reverse_query = false;
    bool ipv6_query = false;
    bool got_p = false;
    bool s_loaded = false;
    int server_port = 53;

    parseArguments(argc, argv, server_ip, target_address, t_address_loaded, recursion_desired,
                  reverse_query, ipv6_query, got_p, s_loaded, server_port);

	/*--------------------------------------------------------------------------------
	*Inspirace na strankach https://www.root.cz/man/3/getaddrinfo/
	--------------------------------------------------------------------------------*/
    struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(server_ip.c_str(),NULL , &hints, &result) != 0){
		fprintf(stderr, "Špatná ip adresa nebo špatný název domény serveru\n");
        exit(EXIT_FAILURE);
	}

	// ziskani adresy serveru
    struct sockaddr *addr = result->ai_addr;
    if(!(addr->sa_family==AF_INET || addr->sa_family==AF_INET6)){
        fprintf(stderr, "Špatná ip adresa nebo špatný název domény serveru\n");
        exit(EXIT_FAILURE);
    }

	int pac_len = 0; // promenna pro delku paketu, abychom nasli konec
	char buffer[PCKT_LEN];
	memset(buffer, 0, PCKT_LEN);
	// vytvoreni DNS hlavicky, ma 12 bytu
	struct dns_header *dns_hdr = (struct dns_header *) buffer;
	pac_len += 12;
	dns_hdr->id = htons(RANDOM_NUMBER_FOR_ID);
	
	if(recursion_desired){
		// nastaveni RD flagu na 1
		dns_hdr->flags |= 0b0000000100000000;
	}

	if(reverse_query){
		if(ipv6_query){
			target_address=ipv6(target_address);
			target_address=reverse(target_address,ipv6_query);
		}else{
			uint8_t i_addr[16];
			if(!inet_pton(AF_INET,target_address.c_str(),i_addr)){
				fprintf(stderr,"Neplatna ipv4 adresa\n");
				exit(1);
			}
			target_address=reverse(target_address,ipv6_query); // ipv4
		}
	}

	dns_hdr->qdcount=htons(1); // pocet dotazu
	std::string qname = name_convert(target_address);
	strncpy(buffer+pac_len,qname.c_str(),qname.length());
	pac_len+=qname.length()+1; // +1 kvuli nule na konci
	struct question *q = (struct question *) (buffer+pac_len);
	pac_len+=4; // qtype a qclass jsou 4 byty
	if(reverse_query){
		q->qtype = htons(12); // typ PTR
	}else{
		if(ipv6_query){
			q->qtype = htons(28); // typ AAAA - ipv6
		}else{
			q->qtype = htons(1); // typ A - ipv4
		}
	}
	
	q->qclass = htons(1); // typ IN
	
	dns_hdr->flags=htons(dns_hdr->flags); // zbytek hlavicky je 0
	
	// uziti UDP protokolu
	int sd = socket(addr->sa_family, SOCK_DGRAM, IPPROTO_UDP);
	if(sd < 0){
		fprintf(stderr,"socket() error\n");
		exit(EXIT_FAILURE);
	}
	// nastaveni portu serveru
	struct sockaddr_in *server = (struct sockaddr_in *)addr;
	server->sin_port=htons(server_port);
	socklen_t len;
	// zjisteni delky adresy
	if(addr->sa_family==AF_INET6){
		 len = sizeof(struct sockaddr_in6);
	}else{
		len = sizeof(struct sockaddr_in);
	}
	// zaslani dotazu
	if(sendto(sd,buffer,pac_len,0,(struct sockaddr *)server,len)< 0){
		fprintf(stderr,"sendto() error errno:%i\n%s\n",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}
	// nastaveni timeoutu na 5 sekund pro prijem odpovedi
	alarm(5);
	signal(SIGALRM, timeout); 
	memset(buffer, 0, PCKT_LEN); // vynulovani bufferu
	if(recvfrom(sd,buffer,PCKT_LEN,0,(struct sockaddr *)server,&len)< 0){
		fprintf(stderr,"recvfrom() error errno:%i\n%s\n",errno,strerror(errno));
		exit(EXIT_FAILURE);
	}
	alarm(0); // zruseni timeoutu po prijeti odpovedi
	freeaddrinfo(result); // uvolneni struktury s adresou serveru a jeho portem (getaddrinfo)

	pac_len = 0;
	struct dns_header *dns_hdr_ans = (struct dns_header *) buffer;
	pac_len += 12; // dns header zabira 12 bytu
	uint16_t flags = ntohs(dns_hdr_ans->flags);

	if(flags&(0b0000010000000000)){ // vymaskovani AA flagu
		// pokud je AA flag nastaveny na 1, tak je odpoved autoritativni
		printf("Authoritative: Yes, ");
	}else{
		printf("Authoritative: No, ");
	}
	if(flags&(0b0000000100000000) && flags&(0b0000000010000000)){ // vymaskovani RD a RA flagu
		// pokud je RD i RA flag nastaveny na 1, tak je dotaz rekurzivni
		printf("Recursive: Yes, ");
	}else{
		printf("Recursive: No, ");
	}
	if(flags&(0b0000001000000000)){ // vymaskovani TC flagu
		// pokud je TC flag nastaveny na 1, tak je odpoved zkracena
		printf("Truncated: Yes\n");
	}else{
		printf("Truncated: No\n");
	}

	int count_q = ntohs(dns_hdr_ans->qdcount);
	int count_ans = ntohs(dns_hdr_ans->ancount);
	int count_auth = ntohs(dns_hdr_ans->nscount);
	int count_add = ntohs(dns_hdr_ans->arcount);

	// vypis dotazove casti
	printf("Question section (%d)\n",count_q);
	printf("  ");
	if(count_q==1){ // podle zadani byl vzdy zaslan pouze 1 dotaz
		// vypis jmena v question casti
		int len = 0;
		std::string name_in_question = name(buffer,pac_len,&len);
		printf("%s",name_in_question.c_str());
		pac_len+=len; // posuv za jmeno
		
		struct question *q_in_response = (struct question *) (buffer + pac_len);
		pac_len+=4; // qtype a qclass jsou na 4 bytech
		int q_type_int = ntohs(q_in_response->qtype);
		int q_class_int = ntohs(q_in_response->qclass);
		const char *q_type_str = dns_type(q_type_int);

		// vypis typu v question casti
		if(!q_type_str){ // byl vracen nepodporovany typ
			fprintf(stderr,", Neznamy type: %d",q_type_int);
		}else{
			printf(", %s",q_type_str);
		}

		const char *q_class_str = dns_class(q_class_int);
		if(!q_class_str){ // byl vracen nepodporovany typ
			fprintf(stderr,", Neznama class : %d\n",q_class_int);
			exit(1);
		}else{
			printf(", %s\n",q_class_str);
		}
	}
	
	// vypis casti s odpovedi
	printf("Answer section (%d)\n",count_ans);
	for(int i = 0; i < count_ans; i++){
		printf("  ");
		print_response_data(buffer, &pac_len);
	}
	
	// vypis casti s autoritativnimi odpovedmi
	printf("Authority section (%d)\n",count_auth);
	for(int i = 0; i < count_auth; i++){
		print_response_data(buffer, &pac_len);
	}

	// vypis casti s doplnujicimi informacemi
	printf("Additional section (%d)\n",count_add);
	for(int i = 0; i < count_add; i++)
	{
		print_response_data(buffer, &pac_len);
	}

	return 0;
}