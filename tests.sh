 # Projekt:     DNS resolver
 # @file        argument_parser.cpp
 # 
 # @author Martina Hromádková <xhroma15>
#!/bin/sh

# Spatne zadane argumenty programu (chybove hlasky)
test1="Invalid option;./dns -z"
test2="Missing value for -s option;./dns -s"
test3="Invalid port number;./dns -p abc www.example.com"
test4="Invalid IP address;./dns -s invalid_ip www.example.com"
test5="Invalid IPv6 address;./dns -6 -s invalid_ipv6 www.example.com"
test6="Missing target address;./dns -s kazi.fit.vutbr.cz"
test7="Too long target address;./dns -s kazi.fit.vutbr.cz www.example.com.invalidaddress"
test8="Missing server IP address;./dns -r -s"
test9="Missing target address for reverse query;./dns -x -s kazi.fit.vutbr.cz"
test10="Missing target address for AAAA query;./dns -6 -s kazi.fit.vutbr.cz"
test11="Invalid combination of options;./dns -x -6 -s kazi.fit.vutbr.cz www.example.com"
test12="Invalid port number range;./dns -p 0 www.example.com"
test13="Invalid port number format;./dns -p abc www.example.com"
test14="Invalid IP address format;./dns -s invalid_ip_address www.example.com"
test15="Invalid IPv6 address format;./dns -6 -s invalid_ipv6_address www.example.com"
test16="Missing recursion option for reverse query;./dns -x -s kazi.fit.vutbr.cz www.example.com"

# Spravne vypisovani odpovedi pomoci porovnani s aktualnim dig
test17="PTR 147.229.9.26 recursive; dig @kazi.fit.vutbr.cz -x 147.229.9.26 +all;./dns -r -x -s kazi.fit.vutbr.cz 147.229.9.26"
test18="A www.github.com recursive; dig @kazi.fit.vutbr.cz www.github.com +all;./dns -r -s kazi.fit.vutbr.cz www.github.com"
test19="A nes.fit.vutbr.cz recursive; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz +all;./dns -r -s kazi.fit.vutbr.cz nes.fit.vutbr.cz"
test20="A nes.fit.vutbr.cz arg order 1; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz +all;./dns -s kazi.fit.vutbr.cz nes.fit.vutbr.cz -r"
test21="A www.fit.vut.cz recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz +all;./dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz"
test22="A www.fit.vut.cz non-recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz +all;./dns -s kazi.fit.vutbr.cz www.fit.vut.cz"
test23="PTR 142.251.37.100 recursive;dig @kazi.fit.vutbr.cz -x 142.251.37.100 +all;./dns -s kazi.fit.vutbr.cz 142.251.37.100 -r -x"
test24="AAAA www.github.com recursive; dig @kazi.fit.vutbr.cz www.github.com AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz www.github.com"
test25="PTR 147.229.8.16 recursive; dig @kazi.fit.vutbr.cz -x 147.229.8.16 +all;./dns -r -x -s kazi.fit.vutbr.cz 147.229.8.16"
test26="AAAA nes.fit.vutbr.cz recursive; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz nes.fit.vutbr.cz"
test27="PTR 147.229.8.16 non-recursive; dig @kazi.fit.vutbr.cz -x 147.229.8.16 +norecurse +all;./dns -x -s kazi.fit.vutbr.cz 147.229.8.16"
test28="AAAA www.fit.vut.cz recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz www.fit.vut.cz"

# Spravne zachazeni se zadanymi argumenty
test29="Reverse query for IPv4 address; dig @kazi.fit.vutbr.cz -x 147.229.9.26 +all;./dns -x -s kazi.fit.vutbr.cz 147.229.9.26"
test30="IPv6 query with recursion; dig @kazi.fit.vutbr.cz www.example.com AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz www.example.com"
test31="Recursive query with custom server; dig @8.8.8.8 www.example.com +all;./dns -r -s 8.8.8.8 www.example.com"

# funkce pro porovnani zaznamu
compare_records() {
    local dns_record="$1"
    local dig_record="$2"

    # parsovani dns_record
    local dns_domain=$(echo "$dns_record" | awk -F', ' '{print $1}' | tr -d '[:space:]')
    local dns_type=$(echo "$dns_record" | awk -F', ' '{print $2}' | tr -d '[:space:]')
    local dns_class=$(echo "$dns_record" | awk -F', ' '{print $3}' | tr -d '[:space:]')
    local dns_ip=$(echo "$dns_record" | awk -F', ' '{print $5}' | tr -d '[:space:]')

    # parsovani dig_record
    local dig_domain=$(echo "$dig_record" | awk '{print $1}' | tr -d '[:space:]')
    local dig_class=$(echo "$dig_record" | awk '{print $3}' | tr -d '[:space:]')
    local dig_type=$(echo "$dig_record" | awk '{print $4}' | tr -d '[:space:]')
    local dig_ip=$(echo "$dig_record" | awk '{print $5}' | tr -d '[:space:]')

    # porovnani hodnot
    if [ "$dig_domain" != "$dns_domain" ] || [ "$dig_type" != "$dns_type" ] || [ "$dig_ip" != "$dns_ip" ] || [ "$dig_class" != "$dns_class" ]; then
        return 1
    fi
    return 0
}

# funkce pro rozdeleni vystupu dig na pole
split_to_array() {
    local pre=$1
    local i=0
    while IFS= read -r line; do
        eval "$pre$i='$line'"
        i=$((i + 1))
    done <<EOF
EOF
}

green='\033[0;32m'
red='\033[0;31m'
reset='\033[0m'
index=1
failed=0
success=0
# pole testu
while eval "test=\$test$index" ; do
    [ -z "$test" ] && break

    # rozdeleni testu na nazev a prikazy
    IFS=';' read -r name cmd_dig cmd_dns <<EOF
$test
EOF

    # ulozeni vystupu prikazu dig do promenne
    dig_out=$(eval "$cmd_dig")

    dig_ans_cnt=$(echo "$dig_out" |  awk 'match($0, /ANSWER: ([0-9]+)/) {print substr($0, RSTART + 8, RLENGTH - 8)}')
    dig_auth_cnt=$(echo "$dig_out" |  awk 'match($0, /AUTHORITY: ([0-9]+)/) {print substr($0, RSTART + 11, RLENGTH - 11)}')
    dig_add_cnt=$(echo "$dig_out" |  awk 'match($0, /ADDITIONAL: ([0-9]+)/) {print substr($0, RSTART + 12, RLENGTH - 12)}')

    dig_ans=$(echo "$dig_out" | awk '/^;; ANSWER SECTION:/,/^$/ { if (!/;; ANSWER SECTION:/ && !/^$/) print }')

    # ulozeni vystupu prikazu dns do promenne
    dns_out=$(eval "$cmd_dns")

    # validni vystup
    dns_que_cnt=$(echo "$dns_out" | awk -F '[()]' '/Questions/{print $2}')
    dns_ans_cnt=$(echo "$dns_out" | awk -F '[()]' '/Answer section/{print $2}')
    dns_auth_cnt=$(echo "$dns_out" | awk -F '[()]' '/Authority section/{print $2}')
    dns_add_cnt=$(echo "$dns_out" | awk -F '[()]' '/Additional section/{print $2}')
    dns_ans=$(echo "$dns_out" | awk '/^Answer section \(/ {flag=1; next} /Authority section \(/ {flag=0} flag')

    # konverze vystupu dns do pole
    split_to_array "dig_ans_record_" "$dig_ans"
    split_to_array "dns_ans_record_" "$dns_ans"

    # porovnani poctu zaznamu
    if [ "$dig_ans_cnt" != "$dns_ans_cnt" ]; then
        echo -e "$index. $name $red FAILED "
        failed=$((failed + 1))
        index=$((index + 1))
        continue
    fi

    result=true

    # porovnani kazdeho zaznamu
    i=0
    while eval "dig_record=\$dig_ans_record_$i" && [ -n "$dig_record" ]; do
        eval "dns_record=\$dns_ans_record_$i"
        if ! compare_records "$dns_record" "$dig_record"; then
            result=false
        fi
        i=$((i + 1))
    done

    # vypis vysledku
    if [ "$result" = "false" ]; then
        echo -e "$index. $name ${red}FAILED${reset}"
        failed=$((failed + 1))
    else
        echo -e "$index. $name ${green}PASSED${reset}"
        success=$((success + 1))
    fi
    index=$((index + 1))

done

# vypis kompletnich vysledku
echo -e "${red}Tests failed: $failed${reset}"
echo -e "${green}Tests passed: $success${reset}"