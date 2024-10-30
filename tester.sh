#!/bin/bash
echo -e "\033[1;94m1. Run send_udp\n2. Run receive_udp\n3. Run reply_udp\n4. Run send_receive_udp\n5. Run tunnel_udp_over_tcp_client\n6. Run tunnel_udp_over_tcp_server\n7. exit\n\033[0m"

export dandelion='129.108.156.68'
export udp_port='8080'
export tcp_port='9999'


read -p "\033[1;93mWhat would you like to do Master Wayne (1-6)? \033[32m" choice

while [ $choice -ne 7 ]; do
    if [ $choice -eq 1 ]
    then
        read -p "\033[1;94mVery well sir, I'll run send_udp for you\n1. localhost\n 2. dandelion\nOn  which address? \033[32m" address
        if [ $address -eq 1 ]
        then
            ./send_udp localhost $udp_port
        elif [ $address -eq 2 ]
        then
            ./send_udp $dandelion $udp_port
        else
            echo -e "\033[1;91mInvalid choice\033[0m"
        fi
        echo -e "\033[0m\n"

    elif [ $choice -eq 2 ]
    then
        echo -e "\033[1;94mVery well sir, I'll run receive_udp for you\033[0m"
        ./receive_udp $udp_port

    elif [ $choice -eq 3 ]
    then
        echo -e "\033[1;94mVery well sir, I'll run reply_udp for you\033[0m"
        ./reply_udp $udp_port

    elif [ $choice -eq 4 ]
    then
        read -p "\033[1;94mVery well sir, I'll run send_receive_udp for you\n1. dandelion\n 2. localhost\nOn  which address? \033[32m" address
        if [ $address -eq 1 ]
        then
            ./send_receive_udp localhost $udp_port
        elif [ $address -eq 2 ]
        then
            ./send_receive_udp $dandelion $udp_port
        else
            echo -e "\033[1;91mInvalid choice\033[0m"
        fi
        echo -e "\033[0m\n"

    elif [ $choice -eq 5 ]
    then
        read -p "\033[1;94mVery well sir, I'll run tunnel_udp_over_tcp_client for you\n1. dandelion\n 2. localhost\nOn  which address? \033[32m" address
        if [ $address -eq 1 ]
        then
            ./tunnel_udp_over_tcp_client $tcp_port localhost $udp_port
        elif [ $address -eq 2 ]
        then
            ./tunnel_udp_over_tcp_client $tcp_port $dandelion $udp_port
        else
            echo -e "\033[1;91mInvalid choice\033[0m"
        fi
        echo -e "\033[0m\n"

    elif [ $choice -eq 6 ]
    then
        echo -e "\033[1;94mVery well sir, I'll run tunnel_udp_over_tcp_server for you\033[0m"
        ./tunnel_udp_over_tcp_server $tcp_port

    elif [ $choice -eq 7 ]
    then
        echo -e "\033[1;94mVery well sir, Let me know if you need anything else\033[0m"
        exit 0
        
    else
        echo -e "\033[1;91mInvalid choice\033[0m"
    fi
    echo -e "\033[1;94m1. Run send_udp\n2. Run receive_udp\n3. Run reply_udp\n4. Run send_receive_udp\n5. Run tunnel_udp_over_tcp_client\n6. Run tunnel_udp_over_tcp_server\n7. exit\n\033[0m"

    read -p "\033[1;93mWhat would you like to do Master Wayne (1-6)? \033[32m" choice
done