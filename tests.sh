#!/bin/sh

test1="PTR 147.229.9.26 recursive; dig @kazi.fit.vutbr.cz -x 147.229.9.26 +all;./dns -r -x -s kazi.fit.vutbr.cz 147.229.9.26"
test2="A www.github.com recursive; dig @kazi.fit.vutbr.cz www.github.com +all;./dns -r -s kazi.fit.vutbr.cz www.github.com"
test3="A nes.fit.vutbr.cz recursive; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz +all;./dns -r -s kazi.fit.vutbr.cz nes.fit.vutbr.cz"
test4="A nes.fit.vutbr.cz arg order 1; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz +all;./dns -s kazi.fit.vutbr.cz nes.fit.vutbr.cz -r"
test5="A www.fit.vut.cz recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz +all;./dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz"
test6="A www.fit.vut.cz non-recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz +all;./dns -s kazi.fit.vutbr.cz www.fit.vut.cz"
test7="PTR 142.251.37.100 recursive;dig @kazi.fit.vutbr.cz -x 142.251.37.100 +all;./dns -s kazi.fit.vutbr.cz 142.251.37.100 -r -x"
test8="AAAA www.github.com recursive; dig @kazi.fit.vutbr.cz www.github.com AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz www.github.com"
test9="PTR 147.229.8.16 recursive; dig @kazi.fit.vutbr.cz -x 147.229.8.16 +all;./dns -r -x -s kazi.fit.vutbr.cz 147.229.8.16"
test10="AAAA nes.fit.vutbr.cz recursive; dig @kazi.fit.vutbr.cz nes.fit.vutbr.cz AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz nes.fit.vutbr.cz"
test11="PTR 147.229.8.16 non-recursive; dig @kazi.fit.vutbr.cz -x 147.229.8.16 +norecurse +all;./dns -x -s kazi.fit.vutbr.cz 147.229.8.16"
test12="AAAA www.fit.vut.cz recursive; dig @kazi.fit.vutbr.cz www.fit.vut.cz AAAA +all;./dns -6 -r -s kazi.fit.vutbr.cz www.fit.vut.cz"

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
    if [ "$dig_domain" != "$dns_domain" ]; then
        return 1
    fi
    if [ "$dig_type" != "$dns_type" ]; then
        return 1
    fi
    if [ "$dig_ip" != "$dns_ip" ]; then
        return 1
    fi
    if [ "$dig_class" != "$dns_class" ]; then
        return 1
    fi
    return 0
}

# funkce pro rozdeleni vystupu dig na pole
split_to_array() {
    local prefix=$1
    local i=0
    while IFS= read -r line; do
        eval "$prefix$i='$line'"
        i=$((i + 1))
    done <<EOF
EOF
}


green='\033[0;32m'
red='\033[0;31m'
index=1
failed=0
success=0
while eval "test=\$test$index" ; do
    [ -z "$test" ] && break

    # rozdeleni testu na nazev a prikazy
    IFS=';' read -r test_name dig_cmd dns_cmd <<EOF
$test
EOF

    # ulozeni vystupu prikazu dig do promenne
    dig_output=$(eval "$dig_cmd")

    dig_answer_count=$(echo "$dig_output" |  awk 'match($0, /ANSWER: ([0-9]+)/) {print substr($0, RSTART + 8, RLENGTH - 8)}')
    dig_authority_count=$(echo "$dig_output" |  awk 'match($0, /AUTHORITY: ([0-9]+)/) {print substr($0, RSTART + 11, RLENGTH - 11)}')
    dig_additional_count=$(echo "$dig_output" |  awk 'match($0, /ADDITIONAL: ([0-9]+)/) {print substr($0, RSTART + 12, RLENGTH - 12)}')

    dig_answer=$(echo "$dig_output" | awk '/^;; ANSWER SECTION:/,/^$/ { if (!/;; ANSWER SECTION:/ && !/^$/) print }')

    # ulozeni vystupu prikazu dns do promenne
    dns_output=$(eval "$dns_cmd")

    # validni vystup
    dns_question_count=$(echo "$dns_output" | awk -F '[()]' '/Questions/{print $2}')
    dns_answer_count=$(echo "$dns_output" | awk -F '[()]' '/Answer section/{print $2}')
    dns_authority_count=$(echo "$dns_output" | awk -F '[()]' '/Authority section/{print $2}')
    dns_additional_count=$(echo "$dns_output" | awk -F '[()]' '/Additional section/{print $2}')

    if [ "$dns_question_count" == "" ]; then
        echo -e "$index. $test_name $red FAILED"
        failed=$((failed + 1))
        index=$((index + 1))
        continue
    fi

    dns_answer=$(echo "$dns_output" | awk '/^Answer section \(/ {flag=1; next} /Authority section \(/ {flag=0} flag')

    # konverze vystupu dns do pole
    split_to_array "dig_answer_record_" "$dig_answer"
    split_to_array "dns_answer_record_" "$dns_answer"

    # po
    if [ "$dig_answer_count" != "$dns_answer_count" ]; then
        echo -e "$index. $test_name $red FAILED "
        failed=$((failed + 1))
        index=$((index + 1))
        continue
    fi

    result=true

    # porovnani kazdeho zaznamu
    i=0
    while eval "dig_record=\$dig_answer_record_$i" && [ -n "$dig_record" ]; do
        eval "dns_record=\$dns_answer_record_$i"
        if ! compare_records "$dns_record" "$dig_record"; then
            result=false
        fi
        i=$((i + 1))
    done

    if [ "$result" = "true" ]; then
         echo -e "$index. $test_name $green OK "
         success=$((success + 1))
     else
         echo -e "$index. $test_name $red FAILED "
         failed=$((failed + 1))
     fi
     index=$((index + 1))
done

echo -e "$green""Tests passed: $success "
echo -e "$red""Tests failed: $failed "