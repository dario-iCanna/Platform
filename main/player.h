#pragma once
#include <windows.h>
#include <vector>

using namespace std;

//variabili del player
extern struct gameStuff {
	RECT r ;
	RECT initialPos;
	double vel;
	double acc;
	double initialAcc;
	double dec;
	double jmpDec;
	double initialJmp;
	int velMax;
	double jmpPow;
	int state;
	bool highJump;
	short life;
	short maxLife;
	short immunity;
	short initialImmunity;
}player;

//struttura per le entit�
extern struct entity
{
	RECT r;
	double vel;
	double jmpDec;
	double jmpPow;
	int state;
	short type;
	short fpa;
	short iniFpa;
};
//stato del player
enum state{
	idle = 0,
	jumping,
	walking
};

//metodo movimento del player
void movimentoPlayer(int **&livello, int**& initialLiv, int BLOCK_SIZE, vector<entity>& en, int size, int SCREEN_WIDTH, int livSize, vector<entity>& initialArr, int& score, int SCREEN_HEIGHT,int&tempo);

//collsione laterare 
short sideColl(int m, int f);

//collisione in basso
short bottomColl(int m, int f);

//collsione in alto
short topColl(int m, int f);

// ripristino posizione
void ripristino(vector<entity>& enemies, int size, vector<entity>& initialArr, int**& livello, int**& initialLiv, int SCREEN_HEIGHT, int BLOCK_SIZE, int livSize);

//movimento nemici e piattaforme
void movimentoEntit�(int** livello, int BLOCK_SIZE, entity& e, int SCREEN_WIDTH, vector<entity>& uot, bool& elimina);