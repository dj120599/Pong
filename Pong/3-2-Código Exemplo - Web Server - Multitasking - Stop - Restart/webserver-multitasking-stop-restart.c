// Compilar:
// gcc -Wall -o webserver-multitasking webserver-multitasking.c

// Executar com:
// ./webserver-multitasking

// -----------------------------------------------------------------

// Executar num outro terminal:
// curl http://127.0.0.1:8080/
// ou
// lynx http://127.0.0.1:8080
// ou
// wget http://127.0.0.1:8080

// Resposta:
// <html><header><title>IPCA-WEBSERVER</title></header><body>Hello world</body></html>

// *****************************************************************
// Executar 5 pedidos seguidos:
// for i in {1..5}; do (curl http://127.0.0.1:8080 &); done
// *****************************************************************

// Executar 5 pedidos seguidos, com um limite máximo de largura de banda de 10 bytes/segundo:
// for i in {1..5}; do (curl --limit-rate 10 http://127.0.0.1:8080 &); done


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>


#define BUFSIZE 8096
#define SORRY 43


void logerror(int type, int fd) {
	char logbuffer[512] = {0};
	
	if(type == SORRY) {
		strcpy(logbuffer, "<HTML><BODY><H1>IPCA-WEBSERVER Sorry!</H1></BODY></HTML>\r\n");
		write(fd, logbuffer, strlen(logbuffer));
	}
	
	if(type == SORRY)
		exit(3); // Termina o processo, sendo que o 'exit status' será transmitido ao processo pai.
}


// Processo filho, para serviço web.
void web(int fd) {
	struct timeval tv;
	char *buffer;
	long int ret;
	
	// Aloca memória para receção de dados.
	buffer = (char *) calloc(BUFSIZE + 1, sizeof(char));
	if(buffer != NULL) {
		// Timeout de recepção (fecha a ligação se o browser demorar mais de 10 segundos).
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		
		// Atribui o timeout ao socket.
		setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
		
		// Leitura (de uma só vez) do pedido Web feito pelo browser ao nosso servidor (isto é, dos dados enviados pelo cliente).
		ret = read(fd, buffer, BUFSIZE);
		
		// Em caso de erro, fecha o processo que tinha sido gerado em resposta ao pedido.
		if((ret == 0) || (ret < 0)) {
			if(buffer != NULL) free(buffer);
			logerror(SORRY, fd);
		}
		
		// Se recebeu um pedido válido.
		if((ret > 0) && (ret < BUFSIZE))
			buffer[ret] = 0; // Escreve carácter '\0' no fim da mensagem recebida.
		
		if(ret > 0) {printf("\n[socket %d] [%ld] %s", fd, ret, buffer); fflush(stdout);}
		
		// Se recebeu um pedido para encerrar o servidor
		if(strncmp(buffer, "GET /stop", strlen("GET /stop")) == 0) {
			// Envia os dados da página web, em resposta ao pedido efetuado.
			sprintf(buffer, "<html><header><title>IPCA-WEBSERVER</title></header><body>Bye Bye from PID:%d</body></html>\r\n\r\n", getpid());
			write(fd, buffer, strlen(buffer));
			
			char arg[20] = {0};
			sprintf(arg, "%d", getppid());
			execl("/bin/kill", "kill", arg, NULL);
			//execl("/usr/bin/killall", "killall", "webserver-multitasking", NULL);
		}
		// Se recebeu um pedido para encerrar o servidor
		else if(strncmp(buffer, "GET /restart", strlen("GET /restart")) == 0) {
			// Envia os dados da página web, em resposta ao pedido efetuado.
			sprintf(buffer, "<html><header><title>IPCA-WEBSERVER</title></header><body>Restart from PID:%d</body></html>\r\n\r\n", getpid());
			write(fd, buffer, strlen(buffer));
			close(fd);
			
			int ppid = getppid();
			if(fork() == 0) { // Se é o processo filho
				char arg[20] = {0};
				sprintf(arg, "%d", ppid);
				execl("/bin/kill", "kill", arg, NULL);
			}
			else // Se é o processo pai
			{
				int status;
				wait(&status); // Aguarda que o processo filho termine a sua execução
				
				execl("/home/user/webserver-multitasking", "webserver-multitasking", " &", NULL);
			}
		}
		
		// Vamos simular um atraso na resposta, de 5 segundos. Poderia ser provocado, por exemplo, por um acesso à BD.
		sleep(5);
		
		// Envia os dados da página web, em resposta ao pedido efetuado.
		sprintf(buffer, "<html><header><title>IPCA-WEBSERVER</title></header><body>Hello world from PID:%d</body></html>\r\n\r\n", getpid());
		write(fd, buffer, strlen(buffer));
    
		// Liberta a memória alocada.
		free(buffer);
	}
}


// Função que será chamada, no processo pai, para receber o estado de um processo filho que tenha terminado.
// Evita-se, assim, que o processo filho se torne 'zombie'.
void child_cleanup_handler(int signal) {
	while(waitpid((pid_t) -1, NULL, WNOHANG) > 0) {}
}


int main(int argc, char **argv) {
	int pid, listenfd, socketfd;
	static struct sockaddr_in cli_addr;		// static = inicializado a zeros
	static struct sockaddr_in serv_addr;	// static = inicializado a zeros
	socklen_t length;
	
	// Torna o processo num daemon.
	// A função daemon() irá separar o programa do terminal onde foi executado, passando a executar este processo em background como um daemon do sistema.
	// Em Linux/Unix o processo pai de um daemon é, por norma, o processo 'init'.
	daemon(0, 0);
	// Para evitar processos zombies passa por:
	// Registar uma função de callback, neste caso a função 'child_cleanup_handler()', em resposta a um sinal 'SIGCHLD'.
	// Essa função de callback será chamada se o processo pai receber um sinal 'SIGCHLD', que indica o encerramento de um processo filho.
	signal(SIGCHLD, child_cleanup_handler); // or better - sigaction*/
	// Uma outra forma de tornar o processo num daemon: realizar um fork() e depois matar o processo pai.
	// Deste modo, o processo filho irá executar em background, e ficará dependente do processo 'init'. Não encerrará com o fecho do terminal.
	/*if(fork() != 0) exit(0);
	// Para que não sejam gerados zombies, deve ainda ser realizada a dissociação do processo de qualquer terminal (tty) de controlo.
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN ,SIG_IGN);*/
	
	// Cria o socket (é definida a família de endereços e tipo de comunicação, mas ainda não lhe foi atribuído um endereço).
	// AF_INET     -> Especifica a família de endereços IPv4.
	// SOCK_STREAM -> Especifica um tipo de socket que assegura comunicação bidirecional do tipo TCP.
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0) {
		printf("\nGAMESERVER -> socket() ERROR!\n");
		return 0;
	}
	
	// Cria o socket (é definida a família de endereços e tipo de comunicação, mas ainda não lhe foi atribuído um endereço).
	// AF_INET     -> Especifica a família de endereços IPv4.
	// SOCK_STREAM -> Especifica um tipo de socket que assegura comunicação bidirecional do tipo TCP.
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0) {
		printf("\nGAMESERVER -> socket() ERROR!\n");
		return 0;
	}
	
	// Permitir a reutilização do porto, sem ter que aguardar que este seja libertado (aprox. 1 minuto).
	int yes=1;
	//char yes='1'; // Em Solaris
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		printf("\nWEBSERVER -> setsockopt() ERROR!\n");
		exit(1);
	}
	
	// Configura os parâmetros do socket
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY associa o socket a todos interfaces de rede existentes no sistema (e não a um IP específico).
	serv_addr.sin_port = htons(8080); // Associa o socket à porta 8080.
	
	// Associa o endereço especificado em 'serv_addr' ao socket 'listenfd'.
	if(bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
	    printf("\nWEBSERVER -> bind() ERROR!\n");
		return 0;
	}
	// Torna o socket num socket passivo, isto é, será utilizado aceitar pedidos de ligação.
	// O segundo parâmetro define o número máximo de pedidos de conexão suportados pelo socket.
	if(listen(listenfd, 64) < 0) {
	    printf("\nWEBSERVER -> listen() ERROR!\n");
		return 0;
	}
	
	while(1) {
		length = sizeof(cli_addr);
		// Aceita o pedido de ligação, recebido pelo socket 'listenfd'.
		// Cria um novo socket (neste caso 'socketfd').
		// Note que este novo socket não se encontra em modo de "escuta".
		// cli_addr -> Endereço do socket que efetuou o pedido de conexão.
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
			//printf("\nWEBSERVER -> accept() ERROR!\n");
			return 0;
		}
		
		if((pid = fork()) < 0) {
		    printf("\nWEBSERVER -> fork() ERROR!\n");
			return 0;
		}
		else if(pid == 0) { // Processo filho
			// Quando se efetua o 'fork' de um processo, os 'file descriptors' são duplicados para o processo filho.
			// Estes 'file descriptors' são, no entanto, independentes dos que existem processo pai. Eles são uma cópia.
			// Ao fechar ('close()') um 'file descriptor' no processo filho, estamos apenas a fechar o 'file descriptor' desse processo, e não o do pai.
			// Podemos fazer uma analogia com um caso em que dois processos possuem 'file descriptors' para um mesmo ficheiro.
			// Fechar o 'file descriptor' de um processo não irá afetar o estado do 'file descriptor' do outro processo.
			close(listenfd);
			
			web(socketfd);
			
			// Encerra este processo.
			return 0;
		}
		else { // Processo pai (pid > 0)
			// Aguarda que o processo filho termine.
			//int status;
			//wait(&status);
			
			// Fecha o socket de comunicação.
			close(socketfd);
			
			// NOTA:
			// Este processo (pai) irá ser sempre mantido em execução.
		}
	}
	
	return 0;
}