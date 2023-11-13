/**
 * Project:     DNS resolver
 * @file        src/server.c
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Define DNS query structure
struct dns_query {
    char* qname;  // Target address (e.g., "www.example.com")
    uint16_t qtype;  // Query type (A, AAAA, etc.)
    uint16_t qclass;  // Query class (IN for Internet)
};

// Define DNS header structure
struct dns_header {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

// Function to build a DNS query message
void build_dns_query(uint8_t* query, struct dns_query* dns_query) {
    // Initialize the DNS header
    struct dns_header* header = (struct dns_header*)query;
    header->id = htons(1234); // You can use any suitable ID
    header->flags = htons(0x0100); // Flags for a standard query
    header->qdcount = htons(1); // One question in the question section
    header->ancount = 0;  // Answer count (0 for queries)
    header->nscount = 0;   // Name server count (0 for queries)
    header->arcount = 0;   // Additional record count (0 for queries)

    // Encode the QNAME (the target address)
    uint8_t* qname_ptr = query + sizeof(struct dns_header);
    char* token = strtok(dns_query->qname, ".");
    while (token != NULL) {
        int len = strlen(token);
        *qname_ptr = len;
        qname_ptr++;
        memcpy(qname_ptr, token, len);
        qname_ptr += len;
        token = strtok(NULL, ".");
    }
    *qname_ptr = 0; // Terminate the QNAME

    // Set QTYPE and QCLASS
    uint16_t* qtype_ptr = (uint16_t*)(qname_ptr + 1);
    *qtype_ptr = htons(dns_query->qtype);

    uint16_t* qclass_ptr = (uint16_t*)(qtype_ptr + 1);
    *qclass_ptr = htons(dns_query->qclass);

}

// Function to parse a DNS response message
void parse_dns_response(uint8_t* response, size_t response_len) {
    // Check if the response is at least large enough for the DNS header
    if (response_len < sizeof(struct dns_header)) {
        fprintf(stderr, "Received DNS response is too short.\n");
        return;
    }

    // Extract the DNS header
    struct dns_header* header = (struct dns_header*)response;
    uint16_t ancount = ntohs(header->ancount);

    // Check if the response contains answer records
    if (ancount == 0) {
        printf("No answer records in the DNS response.\n");
        return;
    }

    // Move the pointer to the first answer record (right after the header)
    uint8_t* answer_ptr = response + sizeof(struct dns_header);

    // Parse and print answer records
    for (int i = 0; i < ancount; i++) {
        // Extract the name, type, class, TTL, and data from the answer record
        // Here, we assume the answer record format is as follows:
        // - Name (compressed or not)
        // - Type (2 bytes)
        // - Class (2 bytes)
        // - TTL (4 bytes)
        // - Data Length (2 bytes)
        // - Data (variable length)

        char name[256]; // Adjust the buffer size as needed
        uint16_t type, qclass, data_len;
        uint32_t ttl;

        // Parse the name, type, class, TTL, and data length
        int name_len = *answer_ptr;
        memcpy(name, answer_ptr + 1, name_len);
        name[name_len] = '\0';
        answer_ptr += name_len + 1;
        memcpy(&type, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);
        memcpy(&qclass, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);
        memcpy(&ttl, answer_ptr, sizeof(uint32_t));
        answer_ptr += sizeof(uint32_t);
        memcpy(&data_len, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);

        printf("Name: %s, Type: %d, Class: %d, TTL: %u, Data Length: %d\n", name, ntohs(type), ntohs(qclass), ntohl(ttl), ntohs(data_len));

        // Extract and print the data (IP address in this example)
        if (ntohs(type) == 1 && ntohs(data_len) == 4) { // A record with 4-byte data
            char ip_address[16];
            inet_ntop(AF_INET, answer_ptr, ip_address, sizeof(ip_address));
            printf("IP Address: %s\n", ip_address);
        } else if (ntohs(type) == 28 && ntohs(data_len) == 16) { // AAAA record with 16-byte data
            char ip_address[40];
            inet_ntop(AF_INET6, answer_ptr, ip_address, sizeof(ip_address));
            printf("IPv6 Address: %s\n", ip_address);
        } else {
            printf("Data type not supported or unexpected.\n");
        }

        // Move the pointer to the next answer record
        answer_ptr += ntohs(data_len);
    }
}

// Function to print a help message
void print_help() {
    printf("DNS Resolver - Resolve domain names to IP addresses using DNS\n");
    printf("Usage: dns [-r] [-x] [-6] -s server [-p port] address\n");
    printf("Options:\n");
    printf("  -r         Enable recursion (default: disabled)\n");
    printf("  -x         Perform reverse DNS query (default: disabled)\n");
    printf("  -6         Use IPv6 query (default: IPv4 query)\n");
    printf("  -s server  Specify the DNS server (required)\n");
    printf("  -p port    Specify the DNS server port (default: 53)\n");
}

int main(int argc, char *argv[]) {
    int recursion_desired = 0;
    int reverse_query = 0;
    int ipv6_query = 0;
    char* server_ip = NULL;
    int server_port = 53; // Default DNS port
    char* target_address = NULL;

    // Parse command line arguments
    int opt;
    while ((opt = getopt(argc, argv, "hrx6s:p:")) != -1) {
        switch (opt) {
            case 'r':
                recursion_desired = 1;
                break;
            case 'x':
                reverse_query = 1;
                break;
            case '6':
                ipv6_query = 1;
                break;
            case 's':
                server_ip = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'h':
                print_help();
                exit(0);
            default:
                fprintf(stderr, "Usage: dns [-r] [-x] [-6] -s server [-p port] adresa\n");
                exit(1);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Missing target address.\n");
        exit(1);
    }

    target_address = argv[optind];

    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // Define the DNS server's address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(1);
    }

    // Prepare and send the DNS query to the server

    // DNS query structure
    struct dns_query my_query;
    my_query.qname = target_address;
    my_query.qtype = ipv6_query ? 28 : 1; // Use 28 for AAAA, 1 for A
    my_query.qclass = 1; // IN (Internet)

    if (reverse_query) {
        // Reverse query: convert the target address to the appropriate format
        char reverse_query_addr[256]; // Adjust the buffer size as needed
        if (ipv6_query) {
            // Convert IPv6 address to reverse DNS format
            snprintf(reverse_query_addr, sizeof(reverse_query_addr), "%s.in-addr.arpa", target_address);
        } else {
            // Convert IPv4 address to reverse DNS format
            char ip_parts[4][4]; // Maximum of 4 parts (255.255.255.255)
            if (sscanf(target_address, "%3s.%3s.%3s.%3s", ip_parts[0], ip_parts[1], ip_parts[2], ip_parts[3]) == 4) {
                snprintf(reverse_query_addr, sizeof(reverse_query_addr), "%s.%s.%s.%s.in-addr.arpa",
                         ip_parts[3], ip_parts[2], ip_parts[1], ip_parts[0]);
            } else {
                fprintf(stderr, "Invalid IPv4 address format for reverse query.\n");
                close(sock);
                exit(1);
            }
        }

        // Update the query name for the reverse query
        my_query.qname = reverse_query_addr;
    }

    // Allocate a buffer for the DNS query message
    size_t query_len = sizeof(struct dns_header) + strlen(my_query.qname) + 2;
    uint8_t query[query_len];

    // Build the DNS query message
    build_dns_query(query, &my_query);

    // Send the DNS query to the server
    if (sendto(sock, query, query_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(sock);
        exit(1);
    }

    // Receive the DNS response
    uint8_t response[1024]; // Adjust the buffer size as needed
    ssize_t response_len;
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    response_len = recvfrom(sock, response, sizeof(response), 0, (struct sockaddr*)&from_addr, &from_len);
    if (response_len == -1) {
        perror("recvfrom");
        close(sock);
        exit(1);
    }

    // Declare and initialize the header variable in main
    struct dns_header header;
    memset(&header, 0, sizeof(struct dns_header));

    // Parse the DNS response, passing the initialized header as an argument
    parse_dns_response(response, response_len);

    // Print the received DNS response (in hexadecimal)
    printf("Received DNS Response (Hexadecimal):\n");
    for (size_t i = 0; i < response_len; i++) {
        printf("%02x ", response[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Print the response length
    printf("Received DNS Response Length: %zu\n", response_len);

    // Get DNS response flags
    uint16_t flags = ntohs(header.flags);
    int authoritative = (flags >> 10) & 1;  // Bit 10 indicates authoritative answer
    int recursive = recursion_desired;     
    int truncated = (flags >> 9) & 1;     // Bit 9 indicates truncated message

    // Print the results as specified in the output format
    printf("Authoritative: %s, Recursive: %s, Truncated: %s\n", authoritative ? "Yes" : "No", recursive ? "Yes" : "No", truncated ? "Yes" : "No");

    // Print question section
    printf("Question section (1)\n");
    printf("  %s, %s, %s\n", my_query.qname, ipv6_query ? "AAAA" : "A", "IN");

    // Print answer section
    printf("Answer section (%d)\n", ntohs(header.ancount));
    uint8_t* answer_ptr = response + sizeof(struct dns_header);
    for (int i = 0; i < ntohs(header.ancount); i++) {
        // Extract and print the name, type, class, TTL, and data
        int name_len = *answer_ptr;
        char name[256];
        memcpy(name, answer_ptr + 1, name_len);
        name[name_len] = '\0';
        answer_ptr += name_len + 1;
        uint16_t type, qclass, data_len;
        uint32_t ttl;
        memcpy(&type, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);
        memcpy(&qclass, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);
        memcpy(&ttl, answer_ptr, sizeof(uint32_t));
        answer_ptr += sizeof(uint32_t);
        memcpy(&data_len, answer_ptr, sizeof(uint16_t));
        answer_ptr += sizeof(uint16_t);

        printf("  %s, %s, %s, %u, ", name, ntohs(type) == 1 ? "A" : "CNAME", "IN", ntohl(ttl));

        // Extract and print the data (IP address or CNAME)
        if (ntohs(type) == 1) { // A record with 4-byte data
            char ip_address[16];
            inet_ntop(AF_INET, answer_ptr, ip_address, sizeof(ip_address));
            printf("%s\n", ip_address);
        } else if (ntohs(type) == 5) { // CNAME record
            int cname_len = *answer_ptr;
            char cname[256];
            memcpy(cname, answer_ptr + 1, cname_len);
            cname[cname_len] = '\0';
            printf("%s\n", cname);
        } else {
            printf("Unsupported record type\n");
        }

        // Move to the next answer record
        answer_ptr += ntohs(data_len);
    }

    // Print authority section
    printf("Authority section (%d)\n", ntohs(header.nscount));
    uint8_t* authority_ptr = answer_ptr;
    for (int i = 0; i < ntohs(header.nscount); i++) {
        // Extract and print the name, type, class, TTL, and data
        int name_len = *authority_ptr;
        char name[256];
        memcpy(name, authority_ptr + 1, name_len);
        name[name_len] = '\0';
        authority_ptr += name_len + 1;
        uint16_t type, qclass, data_len;
        uint32_t ttl;
        memcpy(&type, authority_ptr, sizeof(uint16_t));
        authority_ptr += sizeof(uint16_t);
        memcpy(&qclass, authority_ptr, sizeof(uint16_t));
        authority_ptr += sizeof(uint16_t);
        memcpy(&ttl, authority_ptr, sizeof(uint32_t));
        authority_ptr += sizeof(uint32_t);
        memcpy(&data_len, authority_ptr, sizeof(uint16_t));
        authority_ptr += sizeof(uint16_t);

        printf("  %s, %s, %s, %u, ", name, ntohs(type) == 1 ? "A" : "CNAME", "IN", ntohl(ttl));

        // Extract and print the data (IP address or CNAME)
        if (ntohs(type) == 1) { // A record with 4-byte data
            char ip_address[16];
            inet_ntop(AF_INET, authority_ptr, ip_address, sizeof(ip_address));
            printf("%s\n", ip_address);
        } else if (ntohs(type) == 5) { // CNAME record
            int cname_len = *authority_ptr;
            char cname[256];
            memcpy(cname, authority_ptr + 1, cname_len);
            cname[cname_len] = '\0';
            printf("%s\n", cname);
        } else {
            printf("Unsupported record type\n");
        }

        authority_ptr += ntohs(data_len);
    }

    // Print additional section
    printf("Additional section (%d)\n", ntohs(header.arcount));
    // uint8_t* additional_ptr = authority_ptr; // Adjust the pointer accordingly
    for (int i = 0; i < ntohs(header.arcount); i++) {
        // Extract and print additional section as needed
        // The format can vary based on the specific resource records.
        // You may need to handle different record types, like MX, TXT, etc.
        // Parse and print data similar to previous sections.
    }

    // Close the socket and clean up
    close(sock);

    return 0;
}