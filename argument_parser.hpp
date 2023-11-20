/**
 * Projekt:     DNS resolver
 * @file        argument_parser.hpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include <vector>
#include <string>

void parseArguments(int argc, char* argv[], std::string& server_ip, std::string& target_address, bool& t_address_loaded,
                    bool& recursion_desired, bool& reverse_query, bool& ipv6_query,
                    bool& got_p, bool& s_loaded, int& server_port);