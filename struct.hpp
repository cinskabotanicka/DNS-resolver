/**
 * Projekt:     DNS resolver
 * @file        struct.hpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include <unistd.h>

// struktura dns hlavičky
struct dns_header{
	uint16_t id;
	/*počet bitů	flagy:
	* 1 			QR
	* 4 			OPCODE
	* 1				AA
	* 1 			TC
	* 1				RD
	* 1 			RA
	* 3 			zero
	* 3 			rCode
	*/
	uint16_t flags; //flagy
	uint16_t qdcount; //count pro question section
	uint16_t ancount; //count pro answer section
	uint16_t nscount; //count pro authority section
	uint16_t arcount; //counter pro addional section
};

// resource record data
struct rr_data{
	uint16_t type;
	uint16_t dclass; //1 pro internet
	uint32_t ttl; //time to live
	uint16_t rdlength;
};

// Start of Authority data
struct SOA_data{
	uint32_t serial; //serial number
	uint32_t refresh; //refresh interval
	uint32_t retry; //retry interval
	uint32_t expire; //expire limit
	uint32_t minimum; //minimum TTL
};

// struktura pro dotaz
struct question{
	uint16_t qtype;
	uint16_t qclass;
};