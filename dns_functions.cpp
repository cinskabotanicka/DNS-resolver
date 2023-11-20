/**
 * Projekt:     DNS resolver
 * @file        dns_functions.cpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dns_functions.hpp"
#include "struct.hpp"

// (funkcni pouze pro prvnich 16 typu a 28)
// vraci string TYPE odpovedi
const char * dns_type(int type){
	/*--------------------------------------------------------------------------------
	*Ze stranek https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml
	--------------------------------------------------------------------------------*/
	switch (type){
		case 1: return "A";
		case 2: return "NS";
		case 3: return "MD";
		case 4: return "MF";
		case 5: return "CNAME";	
		case 6: return "SOA";	
		case 7: return "MB";	
		case 8: return "MG";	
		case 9: return "MR";	
		case 10: return "NULL";	
		case 11: return "WKS";	
		case 12: return "PTR";	
		case 13: return "HINFO";	
		case 14: return "MINFO";	
		case 15: return "MX";	
		case 16: return "TXT";
		case 28: return "AAAA";
		default: return NULL; //other are printed as uknown
	}
}

// vraci string CLASS odpovedi
const char * dns_class(int type){
	switch (type){
		case 1: return "IN";
		case 2: return "CS";
		case 3: return "CH";	
		case 4: return "HS";
		default: return NULL; //unknown
	}
}

// konvertuje ipv6 adresu ze zkraceneho formatu na plny format (všech 32 cisel)
std::string ipv6(std::string name){
	std::string out;
	uint8_t i6_addr[16];
    if(inet_pton(AF_INET6,name.c_str(),i6_addr)){ 
		// prevod na hex
		char const hex_to_char[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		for(int i = 0 ; i < 16 ; i ++){ 
			out+=hex_to_char[i6_addr[i]>>4];
			out+='.';
			out+=hex_to_char[i6_addr[i]&0b00001111];
			out+='.';
		}
		out.pop_back(); // - posledni tecka
	}else{
		fprintf(stderr,"Ipv6 adresa neni platna.\n");
		exit(1);
	}
    return out;
}

// prevede ipv4 adresu na adresu pro reverzni dotaz
std::string reverse(std::string name, bool ipv6){
	std::string out;
	std::vector<std::string> tmp_v;
	std::string tmp_s;
	for(long unsigned int i = 0; i < name.length(); i ++){
		if(name[i]=='.'){ 
			// rozdeleni na jednotliva cisla
			tmp_v.push_back(tmp_s);
			tmp_s="";
			continue;
		}
		tmp_s+=name[i];
	}
	tmp_v.push_back(tmp_s);

	for(std::vector<std::string>::iterator i = tmp_v.end(); i-- != tmp_v.begin(); ){
		// reverzni poradi
		out+=*i;
		out+='.';
	}

	if(ipv6){
		out+="ip6.arpa";
	}else{
		out+="in-addr.arpa";
	}
	return out;
}

// prevod jmena na delka+label
std::string name_convert(std::string name){
	std::string out;
	int cnt = 0;
	for(long unsigned int i = 0; i < name.length(); i ++){
		if(name[i] == '.'){
			// pokud najdeme tecku, tak zapiseme delku labelu pred zacatek toho labelu
			out.insert(i-cnt,1,(char)cnt);
            cnt =0;
		}else{
			cnt++;
			out+=name[i];
		}
	}
	// pokud jsme skoncili na konci jmena (bez tecky) tak zapiseme delku labelu doted precteneho, pred zacatek toho labelu
    out.insert(name.length()-cnt,1,(char)cnt);
	return out;
}

// transformuje zapis jmena v paketu na citelnou podobu (vcetne pointeru)
std::string name(char *buffer,int start_offset, int *len){
	//buffer pro cteni celeho paketu (zacina hlavickou)
	//len vrati delku jmena co se precetla z paketu
	if(buffer[start_offset]==0){
		*len=1;
		return std::string("root");
	}else{
		*len = 0;
	}
	std::string name;
	int i = start_offset; // zacatek jmena
	unsigned int offset = 0;
	while (buffer[i]!=0){
		if(i > start_offset){
			*len = i-start_offset;
		}
		if((buffer[i]&0b11000000) ==  0b11000000){ // pointer (zacina dvema jednickami)
			offset = ((buffer[i]&(0b00111111)) * 256)+(buffer[i+1]&(0b11111111)); // zbyvajicich 14 bitu
			i = offset;
		}else{
			name+=(buffer[i]);
			i++;
		}
	}

	// pointer ma 2xchar
	// string musi mit +1char za zacatek (delka) a +1char za konec (ukoncovaci 0)
	// takze obe moznosti +2
	*len +=2;  

	//prevod z adresy formatu delka+znaky na citelnou finalni adresu
	std::string addr;
	int j = name[0];
	for(long unsigned int i = 1; i < name.size();i++){
		if(j>0){
			addr+=name[i];
			j--;
		}else{
			j=name[i];
			addr+='.';
		}
	}
	return addr;	
}

// vypise info z jednotlivych sekci (answer, additional, authoritative)
void print_response_data(char *buffer,int *pac_len){
	int len;
	std::string name_in_answer = name(buffer,*pac_len,&len);
	printf("%s",name_in_answer.c_str());
	*pac_len+=len;
	struct rr_data *rr = (struct rr_data *) (buffer+*pac_len);
	*pac_len+=10; // posun o 80 bitu, ktere jsou mezi mezi name a rdata

	int rr_type_int = ntohs(rr->type);
	int rr_rlen_int = ntohs(rr->rdlength);
	const char *rr_type_str = dns_type(rr_type_int);
	int rr_class_int = ntohs(rr->dclass);
	int rr_ttl_int = ntohl(rr->ttl);

	// vypis typu
	if(!rr_type_str){
		// pokud byl vracen nepodporovany typ
		fprintf(stderr,", Neznámý typ v DNS odpovědi: %d",rr_type_int);
	}else{
		printf(", %s",rr_type_str);//vypis typu v opdovedi
	}

	// vypis classy
	const char *rr_class_str = dns_class(rr_class_int);
	if(!rr_class_str){
		// pokud byl vracen nepodporovany typ
		fprintf(stderr,", Neznámá třída v DNS odpovědi: %d\n",rr_class_int);
		exit(1);
	}else{
		printf(", %s",rr_class_str); // vypis classy odpovedi
	}
	printf(", %d, ", rr_ttl_int); // vypis ttl 

	if(rr_type_int == 1){
		// typ A, tudiz rdata obsahuji ipv4 adresu
		struct in_addr adress;
		adress.s_addr = *(uint32_t*)(buffer+*pac_len);
		printf("%s\n",inet_ntoa(adress)); // vypis ipv4 adresy
	}else if (rr_type_int == 28){
		// typ AAAA, tudiz rdata obsahuji ipv6 adresu
		char src[16];
		memcpy(src,buffer+(*pac_len),rr_rlen_int);
		char tmp[INET6_ADDRSTRLEN]; // docasny buffer pro inet_ntop funkci
		printf("%s\n",inet_ntop(AF_INET6,src,tmp,INET6_ADDRSTRLEN)); // vypis ipv6 adresy
	}else if(rr_type_int == 6){
		// rdata obsahuji SOA
		/*--------------------------------------------------------------------------------
		*Inspirace https://datatracker.ietf.org/doc/html/rfc1035#section-3.3.13
		--------------------------------------------------------------------------------*/
		int tmp = *pac_len;
		std::string mrdata_name = name(buffer,tmp,&len);
		printf("\nmname: %s\n",mrdata_name.c_str()); // vypis m jmena v odpovedi
		tmp+=len; // posuv za jmeno

		std::string rrdata_name = name(buffer,tmp,&len);
		printf("rname: %s\n",rrdata_name.c_str()); // vypis r jmena v odpovedi
		tmp+=len; // posuv za jmeno

		struct SOA_data *soa = (struct SOA_data*) (buffer+tmp);

		printf("Serial number: %d\n",ntohl(soa->serial));
		printf("Refresh interval: %d s\n",ntohl(soa->refresh));
		printf("Retry interval: %d s\n",ntohl(soa->retry));
		printf("Expire limit: %d s\n",ntohl(soa->expire));
		printf("Minimum TTL: %d s\n",ntohl(soa->minimum));
	}else if(rr_type_int == 5 || rr_type_int == 2 || rr_type_int == 12){
		// rdata obsahuji jmeno (bud CNAME, NS nebo PTR)
		std::string rdata_name = name(buffer,*pac_len,&len);
		printf("%s\n",rdata_name.c_str()); // vypis jmena
	}else{
		for(int i=*pac_len;i<*pac_len+rr_rlen_int;i++)
		{
			printf("%x",buffer[i]); // vypis hex znaku	
		}
		printf("\n");
	}
	*pac_len+=rr_rlen_int; // posun o delku rdata
}

// funkce, ktera se spusti, pokud neprijde odpoved do 5 sekund
void timeout(int sig){
	printf("Dotaz byl prilis dlouho bez odpovedi.\n");
	exit(1);
}