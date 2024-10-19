#!/bin/bash

git_config="y"

#Complete
instructions() {
    echo -e "\033[33m-----------------------------------------------------------\nSetup complete. You can now start working on the assignment\n-----------------------------------------------------------\n\n\033[0m"
    echo -e "\033[34mTo compile the files, type \`make <filename>\` into the terminal \033[31m(Don't include the .c at the end)\n\033[0m"
    echo -e "\033[34mTo run the files, type \`make \033[31mr\033[34m<filename>\` into the terminal\033[31m(Remember, no .c at the end)\n\n\033[0m"
    echo -e "\033[34mFor the report, I've created README.md. You can write your file on markdown for a cleaner look (includes built in latex and code snippets). You can use programs such as Obsidian (BEST NOTE APP EVER) to edit it and convert it to pdf: 033[4;34mhttps://obsidian.md/\033[0m\n"

    if [ "$git_config" == "y" ]; then
        echo -e "\033[33m  --------------------------------------------\n----------------GIT INFO-----------------------\n  --------------------------------------------\n\033[0m"
        echo -e "\033[34mTo clean the files, type \`make clean\` into the terminal\033[0m\n"
        echo -e "\033[34mTo add files to the staging area, type \`git add -A\` into the terminal\033[0m\n"
        echo -e "\033[34mTo commit the files, type \`git commit -m \"Commit message\"\` into the terminal\033[0m\n"
        echo -e "\033[34mTo push the files to the remote repository, type \`git push origin main\` into the terminal\033[0m\n"
        echo -e "\033[34mTo pull the files from the remote repository, type \`git pull origin main\` into the terminal\033[0m\n"
        echo -e "\033[34mTo check the status of the repository, type \`git status\` into the terminal\033[0m\n"
        echo -e "\033[34mTo check the logs of the repository, type \`git log\` into the terminal\033[0m\n"
    fi

    echo -e "\n\n\033[34mFor any questions, message me on discord \033[33m@psmva\033[0m\nIf you can, please give me a star in any github repo :)\n\n"

    echo -e "\033[34m--------------------------------------------\n--------------------------------------------\n--------------------------------------------\n\n\033[0m"
}

read -r -p $'\033[34mJump to instructions? If this is your first time type n \033[32m(Default y) \033[34m(\033[32my\033[34m/\033[31mn\033[34m): \033[0m' jump
if [ "$jump" != "n" ]; then
    echo -e "\033[34mJumping to instructions...\033[0m\\n"
    instructions
    exit 0
fi

# Check for package manager
if command -v brew &> /dev/null; then
    PACKAGE_MANAGER="brew"
elif command -v pacman &> /dev/null; then
    PACKAGE_MANAGER="pacman"
elif command -v apt-get &> /dev/null; then
    PACKAGE_MANAGER="apt"
elif command -v dnf &> /dev/null; then
    PACKAGE_MANAGER="dnf"
else
    echo -e "\033[34mMissing required package manager. Installing Hombrew\033[0m"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    if [ $? -eq 0 ]; then
        echo -e "\033[32mHomebrew installed successfully\033[0m"
    else
        echo -e '\033[31mFailed to install Homebrew.\nInstall using command /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" to debug properly\n and rerun this script again\033[0m'
        exit 1
    fi
fi

# Create folder and initialize git repository
if [ "$(basename "$PWD")" == "assignment2" ]; then
    echo -e "\033[32mYou are in the assignment2 directory\033[0m\n"
else
    if [ -d "./assignment2" ]; then
        echo -e "\033[34mThe assignment2 directory already exists in the current working directory. \nMoving into it...\033[0m"
        cd ./assignment2
    else
        echo -e "\033[34mCreating the assignment2 directory\033[0m"
        mkdir ./assignment2
        cd ./assignment2
        echo -e "\033[32mDirectory created successfully\033[0m"
    fi
fi

#Config git user
read -r -p $'\033[34mWant to configure git repository? \033[32m(Default y) \033[34m(\033[32my\033[34m/\033[31mn\033[34m): \033[0m' git_config
echo -e "\n"
git_ignore="n"

if [ "$git_config" != "n" ]; then
    if git --version &> /dev/null; then
        read -r -p $'\033[34mEnter your git username (Can be your full name): \033[0m' git_username
        read -r -p $'\033[34mEnter your git email: \033[0m' git_email
        read -r -p $'\033[34mWant to add a .gitignore file? \033[32m(Default y) \033[34m(\033[32my\033[34m/\033[31mn\033[34m): \033[0m' git_ignore
        git config --global user.name "$git_username"
        git config --global user.email "$git_email"
        git init
        read -r -p $'\033[34mWant to set repo to the remote repository? \033[32m(Default y) \033[34m(\033[32my\033[34m/\033[31mn\033[34m): \033[0m' git_remote
        if [ "$git_remote" == "y" ]; then
            read -r -p $'\033[34mEnter the remote repository URL: \033[0m' remote_url
            git remote add origin "$remote_url"
            if [ $? -eq 0 ]; then
                echo -e "\033[32mRemote repository added successfully\033[0m\n"
            else
                echo -e "\033[31mFailed to add remote repository\033[0m\n"
            fi
        else
            echo -e "\033[34mSkipping remote repository configuration...\033[0m\n"
        fi
    else
        echo -e "\033[31mGit is not installed. \nInstalling git\033[0m\n"
        sudo apt-get install git
    fi
else
    echo -e "\033[34mSkipping git configuration...\033[0m\n"
fi

#Download Instructions
echo -e "\033[34mDownloading the assignment instructions...\033[0m"
#wget -O https://www.christoph-lauter.org/utep-os/hw2.pdf
if wget --version &> /dev/null; then
    wget -O hw2.pdf https://www.christoph-lauter.org/utep-os/hw2.pdf
else
    echo -e "\033[31mWget is not installed. Installing wget\033[0m\n"
    
    if [ "$PACKAGE_MANAGER" == "brew" ]; then
        sudo brew install wget -y
        sudo brew update -y
        sudo brew upgrade -y
        sudo brew cleanup -y
    elif [ "$PACKAGE_MANAGER" == "apt" ]; then
        sudo apt install wget
        sudo apt update -y
        sudo apt upgrade -y
        sudo apt autoremove -y
        sudo apt autoclean -y
    elif [ "$PACKAGE_MANAGER" == "dnf" ]; then
        sudo dnf install wget
        sudo dnf update -y
        sudo dnf autoremove -y
        sudo dnf clean all -y
    elif [ "$PACKAGE_MANAGER" == "pacman" ]; then
        sudo pacman -S wget
        sudo pacman -Syu
    else
        echo -e "\033[31mFailed to install wget\033[0m\n"
        exit 1
    fi
    wget -O hw2.pdf https://www.christoph-lauter.org/utep-os/hw2.pdf
    echo -e "\033[32mDownload complete\033[0m\n"
fi

#Create files
# Array of filenames
files=("send_udp.c" "receive_udp.c" "reply_udp.c" "send_receive_udp.c" "tunnel_udp_over_tcp_client.c" "tunnel_udp_over_tcp_server.c")

config_files=("README.md" "Makefile")

# Create files
echo -e "\033[32mCreating files...\033[0m\n"
if [ "$git_ignore" == "y" ]; then
    if [ ! -f .gitignore ]; then
        touch .gitignore
    else
        echo -e "\033[32m.gitignore already exists\033[0m\n"
    fi
fi

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo -e "\033[32m$file already exists\033[0m"
    else
        touch "$file"
        echo -e "#include <stdio.h>\n#include <stdlib.h>\n#include <unistd.h>\n#include <string.h>\n#include <errno.h>\nint main(int argc, char *argv[]){\n\treturn 0;\n}" > "$file"
    fi
done

for file in "${config_files[@]}"; do
    if [ -f "$file" ]; then
        echo -e "\033[32m$file already exists\033[0m\n"
    else
        touch "$file"
    fi
done

echo -e "\n\033[34mWriting into Makefile\033[0m"

# Create Makefile
echo -e "CC=gcc\n

CFLAGS=-Wall -Wextra -Werror -g -O3\n

send_udp: send_udp.c
    \$(CC) \$(CFLAGS) -o send_udp send_udp.c

receive_udp: receive_udp.c
    \$(CC) \$(CFLAGS) -o receive_udp receive_udp.c

reply_udp: reply_udp.c
    \$(CC) \$(CFLAGS) -o reply_udp reply_udp.c

send_receive_udp: send_receive_udp.c
    \$(CC) \$(CFLAGS) -o send_receive_udp send_receive_udp.c

tunnel_udp_over_tcp_client: tunnel_udp_over_tcp_client.c
    \$(CC) \$(CFLAGS) -o tunnel_udp_over_tcp_client tunnel_udp_over_tcp_client.c

tunnel_udp_over_tcp_server: tunnel_udp_over_tcp_server.c
    \$(CC) \$(CFLAGS) -o tunnel_udp_over_tcp_server tunnel_udp_over_tcp_server.c

clean:
    \trm -rf send_udp receive_udp reply_udp send_receive_udp tunnel_udp_over_tcp_client tunnel_udp_over_tcp_server\n
    
rsend_udp:
    \t./send_udp
rreceive_udp:
    \t./receive_udp
rreply_udp:
    \t./reply_udp\n
rsend_receive_udp:
    \t./send_receive_udp
rtunnel_udp_over_tcp_client:
    \t./tunnel_udp_over_tcp_client
rtunnel_udp_over_tcp_server:https://github.com/unmemero/OS_assignment2.git
    \t./tunnel_udp_over_tcp_server" > Makefile

instructions