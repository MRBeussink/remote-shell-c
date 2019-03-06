// Make the necessary includes and set up the variables:
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>

int main (int argc, char* argv[])
{
	int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	char ch = 'A';
	char message[300];
	int message_len;
	char* server_address;
	int server_port;

	char output[4096];
	char input[512];

	pid_t pid;
	int nread;

	const char* const EXIT = "exit\n";

	if (argc != 3) {
		perror("Supply ip address and port\n");
		exit(1);
	}

	server_address = argv[1];
	server_port = atoi(argv[2]);

// Create a socket for the client:

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

// Name socket as agreed with the server:

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_address);
	// address.sin_port = 9734;
	address.sin_port = htons(server_port);
	len = sizeof(address);

// Connect your socket to the server's socket:
	result = connect(sockfd, (struct sockaddr *)&address, len);

	if(result == -1) {
		perror("oops: client1");
		exit(1);
	}

	printf("Now connected to remote shell.\n");

// You can now read and write via sockfd:

	
	// write(sockfd, &ch, 1);
	// read(sockfd, &ch, 1);
	// printf("char from server = %c\n", ch);

	if((pid = fork()) == 0) {
		// child loop

		while (1) {
			if((nread = read(STDIN_FILENO, input, 511)) != -1){
				input[nread] = '\0';

				if ((write(sockfd, input, nread)) == -1){
					perror("Child could not write to server socket\n");
					close(sockfd);
					exit(1);
				}
			}
			else {
				perror("Child process reading from client\n");
				close(sockfd);
				exit(1);
			}
		}
	}
	else if(pid > 0) {
		// parent loop

		while((nread = read(sockfd, output, 4096)) > 0){
			output[nread] = '\0';
			// kill child
			if((strcmp(output, EXIT) == 0) || nread == -1) {
				kill(0, SIGKILL);
				close(sockfd);
				exit(1);
			}

			// Read from socket and send to client
			if ((write(STDOUT_FILENO, output, nread)) == -1) {
				kill(0, SIGKILL);
				close(sockfd);
				exit(1);
			}
		}

		kill(0, SIGKILL);
		exit(1);
	}
	
	close(sockfd);
	exit(0);
}

