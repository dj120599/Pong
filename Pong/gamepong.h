#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>


enum GAME_STATE {kWaitingForPlayer, kPlaying, kPlayer1Wins, kPlayer2Wins};
enum GAME_PLAYER_MOVE {kNone, kUp, kDown, kInvalidKey};


typedef struct {
	enum GAME_STATE state;
	int score_to_win;
	
	struct {
		int width, height;
	} court;
	
	struct {
		float position_x, position_y;
		float direction_x, direction_y; // Define o vetor de deslocamento da bola. Só altera o seu valor aquando de uma colisão.
	} ball;
	
	struct {
		int score;
		int position_x, position_y;
		int height; // Deve ter valor ímpar.
	} player[2]; // 2 players.
} GAME;


GAME* game_init(int width, int height);
void game_loop(GAME* game, enum GAME_PLAYER_MOVE player1_move, enum GAME_PLAYER_MOVE player2_move);
enum GAME_PLAYER_MOVE game_ai(GAME* game);
char* game_draw_frame(GAME *game, int is_server_mode /* 1=NO DRAW 0=DRAW */);
void game_free_frame(char *framebuffer);
void game_close(GAME* game);
