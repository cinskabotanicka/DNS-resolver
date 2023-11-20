/**
 * Projekt:     DNS resolver
 * @file        dns_functions.hpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include <string>

const char * dns_type(int type);
const char * dns_class(int type);
std::string ipv6(std::string name);
std::string reverse(std::string name, bool ipv6);
std::string name_convert(std::string name);
std::string name(char *buffer, int start_offset, int *len);
void print_response_data(char *buffer, int *pac_len);
void timeout(int sig);