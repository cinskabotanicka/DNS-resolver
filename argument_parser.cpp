/**
 * Projekt:     DNS resolver
 * @file        argument_parser.cpp
 * 
 * @author Martina Hromádková <xhroma15>
 */

#include "argument_parser.hpp"
#include <iostream>

void invalid_params() {
    std::cerr << "Špatně zadané argumenty. Použij -h pro zobrazení nápovědy.\n";
    exit(EXIT_FAILURE);
}

void print_help() {
	std::cout   << "Použití: dns [-r] [-x] [-6] -s server [-p port] adresa\n"
				<< " Možnosti:\n"
				<< " -h Zobrazí tuto nápovědu\n"
				<< " -r: Požadována rekurze (Recursion Desired = 1), jinak bez rekurze.\n"
				<< " -x: Reverzní dotaz místo přímého.\n"
				<< " -6: Dotaz typu AAAA místo výchozího A.\n"
				<< " -s: IP adresa nebo doménové jméno serveru, kam se má zaslat dotaz.\n"
				<< " -p port: Číslo portu, na který se má poslat dotaz, výchozí 53.\n"
				<< " adresa: Dotazovaná adresa.\n";
}

// Parser argumentu, kontrola jejich spravnosti a ulozeni do promennych
void parseArguments(int argc, char* argv[], std::string& server_ip, std::string& target_address, bool& t_address_loaded,
                    bool& recursion_desired, bool& reverse_query, bool& ipv6_query,
                    bool& got_p, bool& s_loaded, int& server_port) {
    std::vector<std::string> args(argv, argv + argc);

    for (std::size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "-h") {
            print_help();
            exit(EXIT_SUCCESS);
        } else if (args[i] == "-r") {
            if (recursion_desired) {
                invalid_params();
            }
            recursion_desired = true;
        } else if (args[i] == "-x") {
            if (reverse_query) {
                invalid_params();
            }
            reverse_query = true;
        } else if (args[i] == "-6") {
            if (ipv6_query) {
                invalid_params();
            }
            ipv6_query = true;
        } else if (args[i] == "-s") {
            if (s_loaded) {
                invalid_params();
            }
            if ((i + 1) < args.size()) {
                server_ip = args[i + 1];
                s_loaded = true;
                ++i;
            } else {
                invalid_params();
            }
        } else if (args[i] == "-p") {
            if (got_p) {
                invalid_params();
            }
            if ((i + 1) < args.size()) {
                try {
                    server_port = std::stoi(args[i + 1]);
                } catch (const std::exception& e) {
                    invalid_params();
                }
                if (server_port > 65535 || server_port < 1) {
                    invalid_params();
                }
                got_p = true;
                ++i;
            } else {
                invalid_params();
            }
        } else {
            if (t_address_loaded) {
                invalid_params();
            }
            t_address_loaded = true;
            target_address = args[i];
            if (target_address.length() > 253) {
                invalid_params();
            }
        }
    }

    if (!(t_address_loaded || s_loaded)) {
        invalid_params();
    }
}