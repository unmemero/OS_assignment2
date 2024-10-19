CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -O3

# Compilation rules
send_udp: send_udp.c
	$(CC) $(CFLAGS) -o send_udp send_udp.c

receive_udp: receive_udp.c
	$(CC) $(CFLAGS) -o receive_udp receive_udp.c

reply_udp: reply_udp.c
	$(CC) $(CFLAGS) -o reply_udp reply_udp.c

send_receive_udp: send_receive_udp.c
	$(CC) $(CFLAGS) -o send_receive_udp send_receive_udp.c

tunnel_udp_over_tcp_client: tunnel_udp_over_tcp_client.c
	$(CC) $(CFLAGS) -o tunnel_udp_over_tcp_client tunnel_udp_over_tcp_client.c

tunnel_udp_over_tcp_server: tunnel_udp_over_tcp_server.c
	$(CC) $(CFLAGS) -o tunnel_udp_over_tcp_server tunnel_udp_over_tcp_server.c

# Build all binaries
binaries: send_udp receive_udp reply_udp send_receive_udp tunnel_udp_over_tcp_client tunnel_udp_over_tcp_server

# Clean rule
clean:
	rm -rf send_udp receive_udp reply_udp send_receive_udp tunnel_udp_over_tcp_client tunnel_udp_over_tcp_server

# Running rules
rsend_udp:
	./send_udp

rreceive_udp:
	./receive_udp

rreply_udp:
	./reply_udp

rsend_receive_udp:
	./send_receive_udp

rtunnel_udp_over_tcp_client:
	./tunnel_udp_over_tcp_client

rtunnel_udp_over_tcp_server:
	./tunnel_udp_over_tcp_server

# General run rule
run:
	./$(target)

# Git push rule
gpush:
	@read -p "Enter commit message: " msg; \
	git status; \
	git add -A; \
	echo "\033[33mStaging changes...\033[0m"; \
	git status; \
	if git diff --cached --exit-code > /dev/null; then \
		echo "No changes to commit."; \
	else \
		git commit -m "$$msg"; \
		echo -e "\033[34mPulling latest changes with rebase...\033[0m"; \
		git pull --rebase origin main; \
		git push origin main; \
	fi

# Taring the project
tar:
	tar -czvf project.tar.gz  *.c Report.pdf

# Convert README.md to Report.pdf
pdf:
	pandoc README.md -o Report.pdf