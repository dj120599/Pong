// Compilar:
// gcc -Wall -o gamepongclient gamepong.o gamepongclient.c

// Executar em um novo terminal:
// ./gamepongclient

// Executar um outro cliente num novo terminal:
// ./gamepongclient


#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include "gamepong.h"


#define BUFSIZE 2048
#define PORT 8080


int kbhit() {
    struct timeval tv;
    fd_set fds;
	
    tv.tv_sec = 0;
    tv.tv_usec = 0;
	
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	
    return FD_ISSET(STDIN_FILENO, &fds);
}


#define NB_ENABLE 1
#define NB_DISABLE 0


void nonblock(int state) {
    struct termios ttystate;
	
    tcgetattr(STDIN_FILENO, &ttystate);
 
    if(state == NB_ENABLE) {
        ttystate.c_lflag &= ~ICANON;
        ttystate.c_cc[VMIN] = 1;
    }
    else if(state == NB_DISABLE) {
        ttystate.c_lflag |= ICANON;
    }
	
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}


void pong_match(int sockfd)
{
    char *buffer;
	
	// Aloca memória para receção de dados.
	buffer = (char *) calloc(BUFSIZE, sizeof(char));
	if(buffer != NULL)
	{
		while(1) {
			// Suspende a execução durante 100ms.
			usleep(100000);
			
			int key = 0;
			nonblock(NB_ENABLE);
			// Se foi detetado um evento de tecla premida.
			if(kbhit() != 0) {
				// Lê a tecla premida.
				key = toupper(getchar_unlocked());
				// 'A' = Move Up
				// 'Z' = Move Down
				// 'Q' = Quit
			}
			nonblock(NB_DISABLE);
			
			if(key == 'Q') break;
			
			bzero(buffer, BUFSIZE);
			buffer[0] = (char)key;
			buffer[1] = '\0';
			write(sockfd, buffer, 2/*sizeof(buffer)*/);
			
			bzero(buffer, BUFSIZE);
			read(sockfd, buffer, BUFSIZE);
			//printf("***** [%s] %d *****\n", buffer, (int)strlen(buffer)); exit(0);
			
			// Limpa o terminal.
			system("clear");
			
			// Imprime a frame no terminal.
			printf("%s", buffer);
		}
		
		// Liberta a memória alocada.
		free(buffer);
	}
}


int main()
{
    int clientfd;
    struct sockaddr_in servaddr;
  
    // Cria o socket (é definida a família de endereços e tipo de comunicação, mas ainda não lhe foi atribuído um endereço).
	// AF_INET     -> Especifica a família de endereços IPv4.
	// SOCK_STREAM -> Especifica um tipo de socket que assegura comunicação bidirecional do tipo TCP.
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        printf("\nGAMECLIENT -> socket() ERROR!\n");
        exit(0);
    }
	
    bzero(&servaddr, sizeof(servaddr));
	
    // Configura os parâmetros do socket (IP e PORT).
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Associa o socket ao dispositivo de rede com endereço "127.0.0.1" (loopback).
    servaddr.sin_port = htons(PORT);
	
	// Conecta o socket client ao socket servidor (que está a aguardar em 'accept()').
    if (connect(clientfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("\nGAMECLIENT -> connect() ERROR!\n");
        exit(0);
    }
    else {
        printf("\nGAMECLIENT -> Connected to the server...\n");
	}
	
    // function for chat
    pong_match(clientfd);
	
    // Fecha o socket.
    close(clientfd);
}
