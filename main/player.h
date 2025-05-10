#pragma once
#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "animazione.h"
#include "audio.h"

using namespace std;

//variabili del player
extern struct gameStuff {
	RECT r ;
	double vel;
	double acc;
	double initialAcc;
	double dec;
	double jmpDec;
	double jmpHigh;
	double initialJmp;
	int velMax;
	double jmpPow;
	int state;
	int widthBlock;
	int heightBlock;
	bool widthPl;
	bool heightPl;
	bool highJump;
	short life;
	short maxLife;
	short immunity;
	short initialImmunity;
	bool facingLeft; // variabile per disegnare
	unordered_map<short, int> powerUpTime; //tempo per la fine del powerup
}player;

//struttura per le entità
extern struct entity
{
	RECT r;
	double vel;
	double jmpDec;
	double jmpPow;
	int state;//usato per collisioniNshit
	short type;//-1 per scritte 0 per normal 1 per piattaforme mobili 2 per cannone 3 per proiettile 4 per cuore 5 power up
	vector<tuple<short, short, short>> actions;
	animazione animations;
	int eBlockWidth;
	int eBlockHeight;
	bool facingLeft;
	string animIndex;
	bool differentSideAnimation;
	int timeAlive;// tempo per far stare vivo un personaggio, usato solo per i nemici a -1 (le scritte mobili che non fanno un cazzo)
};

//aggiungere azione al nemoco
void addActionToEnemy(entity& e, short actionType, short firstAction, short actionTime);

//stato del player
enum state{
	idle = 0,
	jumping,
	walking
};

//metodo movimento del player
void movimentoPlayer(int**& livello, int livSize, vector<entity>& en, vector<entity>& screenEn, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, bool& ripristina, int& score, audioBuffer ab);

//collsione laterare 
short sideColl(int m);

//collisione in basso
short bottomColl(int m);

//collsione in alto
short topColl(int m);

// posizione
void ripristino(vector<entity>& screenEn, int& size, int**& livello, int**& initialLiv, int SCREEN_HEIGHT, int BLOCK_SIZE, int livSize, RECT pos);

//ripristino player
void ripristinoPlayer(RECT pos);

//movimento nemici e piattaforme
void movimentoEntità(int** livello, int BLOCK_SIZE, entity& e, int SCREEN_WIDTH, vector<entity>& uot, bool& elimina, bool& kill, bool top, bool& ripristina, int& score, audioBuffer ab);