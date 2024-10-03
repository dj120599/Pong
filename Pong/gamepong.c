// Compilar (para gerar um ficheiro objeto 'gamepong.o'):
// gcc -Wall -c gamepong.c


#include "gamepong.h"


GAME* game_init(int width, int height) {
	GAME *game = calloc(1, sizeof(GAME));
	
	// Dimensões do court.
	game->court.width = width;
	game->court.height = height;
	
	// Bola inicia no centro do court, com sentido de movimento para a direita (1 píxel por frame).
	game->ball.position_x = game->court.width>>1;
	game->ball.position_y = game->court.height>>1;
	game->ball.direction_x = 1.0f;
	game->ball.direction_y = 0.0f;
	
	// Posição e dimensão da raquete do player #1.
	game->player[0].position_x = 1.0f;
	game->player[0].position_y = (float)(game->court.height>>1);
	game->player[0].height = 5; // Dimensão da raquete (deve ser ímpar).
	
	// Posição e dimensão da raquete do player #2.
	game->player[1].position_x = (float)(game->court.width - 2);
	game->player[1].position_y = (float)(game->court.height>>1);
	game->player[1].height = 5; // Dimensão da raquete (deve ser ímpar).
	
	// Pontuação que define a condição de vitória.
	game->score_to_win = 5;
	
	// Coloca o jogo em modo de espera.
	game->state = kWaitingForPlayer;
	
	return game;
}


void game_loop(GAME* game, enum GAME_PLAYER_MOVE player1_move, enum GAME_PLAYER_MOVE player2_move) {
	float step = 1.0f; // Incremento de píxeis por cada frame.
	int offset_y;
	enum GAME_PLAYER_MOVE player_move[2] = {player1_move, player2_move};
	
	if(game->state == kWaitingForPlayer) { // Se aguardava por início do jogo.
		// Se um dos players premiu uma tecla para mover a .
		if(player1_move != kNone || player2_move != kNone)
			game->state = kPlaying;
	}
	else if(game->state == kPlaying) {
		// Atualiza posição do player #1 e player #2, mantendo a raquete dentro do court.
		for(int i=0; i<2; i++) {
			if(player_move[i] == kUp) { // up
				game->player[i].position_y--;
				offset_y = (game->player[i].height - 1)>>1;
				if(game->player[i].position_y - offset_y <= 1)
					game->player[i].position_y = offset_y + 1;
			}
			else if(player_move[i] == kDown) { // down
				game->player[i].position_y++;
				offset_y = (game->player[i].height - 1)>>1;
				if(game->player[i].position_y + offset_y >= game->court.height - 2)
					game->player[i].position_y = -offset_y + game->court.height - 2;
			}
		}
		
		// Atualiza as posições.
		game->ball.position_x += game->ball.direction_x * step;
		game->ball.position_y += game->ball.direction_y * step;
		
		// Verifica se a bola colidiu com a raquete do player #1 e, nesse caso, inverte a direção do movimento da bola.
		offset_y = (game->player[0].height - 1)>>1;
		if( ((int)(game->ball.position_x) <= game->player[0].position_x + 1) && 
			((int)(game->ball.position_y) >= game->player[0].position_y - offset_y) && 
			((int)(game->ball.position_y) <= game->player[0].position_y + offset_y) ) {
			game->ball.direction_x = -game->ball.direction_x;
			game->ball.position_x = game->player[0].position_x + 1;
			// ***************** valor em y deve ser alterado em função da distância ao centro da raquete. *******************
			if((int)(game->ball.position_y) == game->player[0].position_y - 2) game->ball.direction_y = -1.0f;
			else if((int)(game->ball.position_y) == game->player[0].position_y - 1) game->ball.direction_y = -1.0f;
			else if((int)(game->ball.position_y) == game->player[0].position_y) game->ball.direction_y = 0.0f;
			else if((int)(game->ball.position_y) == game->player[0].position_y + 1) game->ball.direction_y = 1.0f;
			else if((int)(game->ball.position_y) == game->player[0].position_y + 2) game->ball.direction_y = 1.0f;
			return;
		}
		// Verifica se a bola colidiu com a raquete do player #2 e, nesse caso, inverte a direção do movimento da bola.
		offset_y = (game->player[1].height - 1)>>1;
		if( ((int)(game->ball.position_x) >= game->player[1].position_x - 1) && 
			((int)(game->ball.position_y) >= game->player[1].position_y - offset_y) && 
			((int)(game->ball.position_y) <= game->player[1].position_y + offset_y) ) {
			game->ball.direction_x = -game->ball.direction_x;
			game->ball.position_x = game->player[1].position_x - 1;
			// ***************** valor em y deve ser alterado em função da distância ao centro da raquete. *******************
			if((int)(game->ball.position_y) == game->player[1].position_y - 2) game->ball.direction_y = -1.0f;
			else if((int)(game->ball.position_y) == game->player[1].position_y - 1) game->ball.direction_y = -1.0f;
			else if((int)(game->ball.position_y) == game->player[1].position_y) game->ball.direction_y = 0.0f;
			else if((int)(game->ball.position_y) == game->player[1].position_y + 1) game->ball.direction_y = 1.0f;
			else if((int)(game->ball.position_y) == game->player[1].position_y + 2) game->ball.direction_y = 1.0f;
			return;
		}
		
		// Verifica se a bola atingiu uma das linhas de fundo do court, sinaliza ponto.
		if(game->ball.position_x <= 0) {
			// Coloca a bola no centro do court.
			game->ball.position_x = game->court.width>>1;
			game->ball.position_y = game->court.height>>1;
			// Bola direcionada para o jogador que perdeu o ponto.
			game->ball.direction_x = 1.0f;
			game->ball.direction_y = 0.0f;
			// Posição da raquete do player #1, ao centro.
			game->player[0].position_x = 1.0f;
			game->player[0].position_y = (float)(game->court.height>>1);
			// Posição da raquete do player #2, ao centro.
			game->player[1].position_x = (float)(game->court.width - 2);
			game->player[1].position_y = (float)(game->court.height>>1);
			// Marca ponto para o player #2.
			game->player[1].score++;
			// Se se alcançou a condição de vitória, altera o estado do jogo.
			if(game->player[1].score >= game->score_to_win)
				game->state = kPlayer2Wins;
			return;
		}
		else if(game->ball.position_x >= game->court.width) {
			// Coloca a bola no centro do court.
			game->ball.position_x = game->court.width>>1;
			game->ball.position_y = game->court.height>>1;
			// Bola direcionada para o jogador que perdeu o ponto.
			game->ball.direction_x = -1.0f;
			game->ball.direction_y = 0.0f;
			// Posição da raquete do player #1, ao centro.
			game->player[0].position_x = 1.0f;
			game->player[0].position_y = (float)(game->court.height>>1);
			// Posição da raquete do player #2, ao centro.
			game->player[1].position_x = (float)(game->court.width - 2);
			game->player[1].position_y = (float)(game->court.height>>1);
			// Marca ponto para o player #1.
			game->player[0].score++;
			// Se se alcançou a condição de vitória, altera o estado do jogo.
			if(game->player[0].score >= game->score_to_win)
				game->state = kPlayer1Wins;
			return;
		}
		
		// Verifica se a bola atingiu os limites laterais do court, e inverte a direção do movimento da bola.
		if((int)game->ball.position_y <= 1) {
			game->ball.position_y = 1.0f;
			game->ball.direction_y = -game->ball.direction_y;
		}
		else if((int)game->ball.position_y >= game->court.height - 2) {
			game->ball.position_y = (float)game->court.height - 2;
			game->ball.direction_y = -game->ball.direction_y;
		}
	}
	else if(game->state == kPlayer1Wins) {
		// Terminou o jogo! Vitória do player #1.
	}
	else { // kPlayer2Wins
		// Terminou o jogo! Vitória do player #2.
	}
}


void game_close(GAME* game) {
	if(game) free(game);
}


char* game_draw_frame(GAME *game, int is_server_mode /* 1=NO DRAW 0=DRAW */) {
	int x, y, offset_y;
	int framebuffer_width = game->court.width + 1; // Carácter extra para o '\n' no final de cada linha.
	char *framebuffer;
	
	// Aloca memória para o conteúdo da frame.
	framebuffer = calloc(framebuffer_width * game->court.height + 1 /* Carácter extra para marcar fim da string '\0' */, sizeof(char));
	
	// Se não foi possível alocar memória para a frame.
	if(framebuffer == NULL) {
		printf("******* ERROR! ********\n");
		printf(" Unable to draw frame. \n");
		printf("***********************\n");
		return NULL;
	}
	
	// Preenche todo o 'framebuffer' com o carácter 'espaço'.
	memset(framebuffer, ' ', framebuffer_width * game->court.height);
	
	// Insere o carácter de quebra de linha, isto é '\n', no final de cada linha.
	for(y=0; y<game->court.height; y++) {
		framebuffer[y * framebuffer_width + game->court.width] = '\n';
	}
	
	if(is_server_mode == 0) {
		// Limpa o terminal.
		system("clear");
	}
	
	// Se aguarda por início do jogo.
	if(game->state == kWaitingForPlayer) {
		char str[30] = "PRESS ANY KEY TO START!";
		memcpy(&framebuffer[(game->court.height>>1) * framebuffer_width + (game->court.width>>2) - strlen(str) / 2], str, strlen(str));
	}
	// Se vitória do player #1.
	else if(game->state == kPlayer1Wins) {
		char str[30] = "PLAYER #1 WINS!";
		memcpy(&framebuffer[(game->court.height>>1) * framebuffer_width + (game->court.width>>2) - strlen(str) / 2], str, strlen(str));
	}
	// Se vitória do player #2.
	else if(game->state == kPlayer2Wins) {
		char str[30] = "PLAYER #2 WINS!";
		memcpy(&framebuffer[(game->court.height>>1) * framebuffer_width + (game->court.width>>2) - strlen(str) / 2], str, strlen(str));
	}
	else {
		// Desenha as linhas superior e inferior do court.
		for(x=0; x<game->court.width; x++) {
			framebuffer[x] = '-';
			framebuffer[(game->court.height - 1) * framebuffer_width + x] = '-';
		}
		// Desenha as linhas à esquerda e direita do court.
		for(y=0; y<game->court.height; y++) {
			framebuffer[y * framebuffer_width] = '|';
			framebuffer[y * framebuffer_width + game->court.width - 1] = '|';
		}
		
		// Desenha a bola.
		int ball_x = (int)(game->ball.position_x);
		if(ball_x < 1) ball_x = 1;
		else if(ball_x > game->court.width - 2) ball_x = game->court.width - 2;
		int ball_y = (int)(game->ball.position_y);
		if(ball_y < 1) ball_y = 1;
		else if(ball_y > game->court.height - 2) ball_y = game->court.height - 2;
		framebuffer[ball_y * framebuffer_width + ball_x] = 'O';
		
		// Desenha a pontuação do player #1.
		framebuffer[2 * framebuffer_width + 3] = game->player[0].score + 48;
		// Desenha a pontuação do player #2.
		framebuffer[2 * framebuffer_width + game->court.width - 1 - 3] = game->player[1].score + 48;
		
		// Desenha raquete do player #1.
		offset_y = (game->player[0].height - 1)>>1;
		for(y=game->player[0].position_y - offset_y; y<=game->player[0].position_y + offset_y; y++)
			framebuffer[y * framebuffer_width + game->player[0].position_x] = '#';
		
		// Desenha raquete do player #2.
		offset_y = (game->player[1].height - 1)>>1;
		for(y=game->player[1].position_y - offset_y; y<=game->player[1].position_y + offset_y; y++)
			framebuffer[y * framebuffer_width + game->player[1].position_x] = '#';
	}
	
	if(is_server_mode == 0) {
		// Imprime a frame no terminal.
		printf("%s", framebuffer);
	}
	
	return framebuffer;
}


void game_free_frame(char *framebuffer) {
	if(framebuffer != NULL)
		free(framebuffer);
}


enum GAME_PLAYER_MOVE game_ai(GAME* game) {
	enum GAME_PLAYER_MOVE move_ai = kNone;
	
	// Se a bola se move na direção do player #2
	if(game->ball.direction_x > 0.0f) {
		if(game->ball.position_y > game->player[1].position_y)
			move_ai = kDown;
		
		if(game->ball.position_y < game->player[1].position_y)
			move_ai = kUp;
	}
	
	return move_ai;
}
