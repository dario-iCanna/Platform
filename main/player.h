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
	bool shooting; // può sparare, powerpups
	unordered_map<short, int> powerUpTime; //tempo per la fine del powerup
}player;

//struttura per le entità
extern struct entity
{
	RECT r;
	double vel;
	double jmpDec;
	double jmpPow;
	int movementX;
	int movementY;
	int state;//usato per collisioniNshit
	short type;//0 per normal (uccisione da sopra) 1 per piattaforme mobili 2 per no collision 3 per collision male ovunque 4 per oggetto raccoglibile, (con effetto in action -1) 5 per uccisione dal basso ma no collision overall
	vector<tuple<short, short, short>> actions; //tipo azione (che usa variabili * 100) , tempo per azione (il set iniziale vale per la prima azione) e tempo usato per resettare l'azione.
	animazione animations;
	int eBlockWidth;
	int eBlockHeight;
	bool facingLeft;
	string animIndex;
	bool differentSideAnimation;
	entity* child; // se voglio spawnare un nemico quando lo stronzo muore
	bool stop; // variabile che serve per fare il movimento in maniera precisa che funziona non so perché cazzo serve ma serve
	//roba per la collisione
	bool turning;
	bool collisionDestroy;
	double intermezzoVel;
};



void printMemoryUsage(const std::string& label);

//aggiungere azione al nemoco
void addActionToEnemy(entity& e, short actionType, short firstAction, short actionTime);

//stato del player e entity
enum state{
	idle = 0,
	jumping,
	walking
};

//metodo movimento del player
void movimentoPlayer(int**& livello, int livSize, vector<tuple<int, int, int>>& changeLiv, vector<entity>& en, vector<entity>& screenEn, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, int SCREEN_HEIGHT_BLOCK, bool& ripristina, int& score);

//funzione per le animazioni n shit
void automaticMovement(int**& livello, int livSize, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, int SCREEN_HEIGHT_BLOCK, int& score);

//collsione laterare 
short sideColl(int m);

//collisione in basso
short bottomColl(int m);

//collsione in alto
short topColl(int m);

// posizione
void ripristino(vector<entity>& screenEn,int& limit, int**& livello, vector<tuple<int, int, int>>& cambiamentiLivello, RECT pos);

//ripristino player
void ripristinoPlayer(RECT pos);

//movimento nemici e piattaforme
void movimentoEntità(int** livello,int livSize, int BLOCK_SIZE, entity& e, vector<entity>& entities, int SCREEN_WIDTH, vector<entity>& uot, bool& elimina, bool& kill, bool& ripristina, int& score);