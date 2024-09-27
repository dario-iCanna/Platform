#include "player.h"
#include "pulsante.h"
#include "camera.h"
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

//variabili del player
struct gameStuff player = {
	player.r = { 20,300,46,330 },
	player.initialPos = player.r,
	player.vel = 0,
	player.acc = 0.1,
	player.initialAcc = 1,
	player.dec = 0.7,
	player.jmpDec = 0.5,
	player.initialJmp = 9,
	player.velMax = 5,
	player.jmpPow = 0,
	player.state = state::idle,
	player.highJump = false,
	player.life = 3,
	player.maxLife = 3,
	player.immunity = 90,
	player.initialImmunity = 90
};
bool discesa ,jump = false;
int movementX = 0, movementY = 0;

//gestione movimento del player 
void movimentoPlayer(int **&livello, int**&initialLiv, int BLOCK_SIZE, vector<entity>& en, int size, int SCREEN_WIDTH, int livSize, vector<entity>& initialArr, int& score, int SCREEN_HEIGTH, int&tempo)
{
	if (jump) {
		player.jmpPow = player.initialJmp;
		player.state = state::jumping;
		player.highJump = true;
	}
	movementX += player.vel; //+ movimento dipendente da entità
	movementY += player.jmpPow; //+ movimento dipendente da entità
	discesa = false;
	
	jump = false;
	
	//controllo per restaare nello schermo
	if (player.r.left + movementX < cam.posX) {
		movementX = cam.posX - player.r.left;
		player.vel = 0;
	}
	else if (player.r.right + movementX >= livSize * BLOCK_SIZE) {
		movementX = (livSize * BLOCK_SIZE) - player.r.right;
		player.vel = 0;
	}

	//azioni dipendenti dallo stato
	if (player.state == state::idle) {
		if (D.pressed) {
			player.vel += player.initialAcc;
		}
		else if (A.pressed) {
			player.vel -= player.initialAcc;
		}
	}
	else if (player.state == state::walking) {
		if (player.vel > 0 && !D.held) { 
			player.vel -= player.dec;
		}
		else if (player.vel < 0 && !A.held) {
			player.vel += player.dec;
		}

		if (abs(player.vel) < player.dec && !A.held && !D.held) {
			player.vel = 0;
		}
	}
	else if (player.state == state::jumping) {
		player.jmpPow -= player.jmpDec;
	}

	//movimento a destra
	if (D.held) {
		//movimento normale
		player.vel += player.acc;
	}
	//movimento a sinitra
	if (A.held) {
		//movimento normale
		player.vel -= player.acc;

	}

	//salto
	if (W.pressed && player.state != state::jumping) {
		player.jmpPow = player.initialJmp;
		player.state = state::jumping;
		player.highJump = true;
	}
	
	//salto più alto
	if (W.held && player.state == state::jumping && player.jmpPow > 0 && player.highJump) {
		player.jmpDec = 0.3;
	}
	else if(player.state == state::jumping){
		player.highJump = false;
		player.jmpDec = 1;
	}

	//controlla la velocità massima
	if (player.vel > player.velMax && D.held) {
		player.vel = player.velMax;
	}
	else if (player.vel < -player.velMax && A.held) {
		player.vel = -player.velMax;
	}

	//collisioni in alto e in basso
	if (movementY != 0) {

		if (player.r.left % BLOCK_SIZE < player.r.right % BLOCK_SIZE || player.r.right % BLOCK_SIZE == 0) {

			//switch collisione
			switch (bottomColl(livello[player.r.left / BLOCK_SIZE][(player.r.bottom - movementY) / BLOCK_SIZE], NULL)) {
			case true:
				if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;
					player.state = state::idle;
					player.jmpPow = 0;
				}
				break;
			case 2:
				livello[player.r.left / BLOCK_SIZE][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}

			switch (topColl(livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], NULL)) {
			case 3:
				livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
			case true:
				if (player.r.top / BLOCK_SIZE > (player.r.top - movementY) / BLOCK_SIZE) {
					movementY = player.r.top - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;
					player.jmpPow = 0;
				}
				break;
			case 2:
				livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}
		}
		else {

			switch (bottomColl(livello[player.r.left / BLOCK_SIZE][(player.r.bottom - movementY) / BLOCK_SIZE], livello[player.r.left / BLOCK_SIZE + 1][(player.r.bottom - movementY) / BLOCK_SIZE])) {
			case true:
				if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;
					player.state = state::idle;
					player.jmpPow = 0;
				}
				break;
			case 2:
				if (livello[player.r.left / BLOCK_SIZE][(player.r.bottom - movementY) / BLOCK_SIZE] == 4) {
					livello[player.r.left / BLOCK_SIZE][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
				}

				if (livello[player.r.left / BLOCK_SIZE + 1][(player.r.bottom - movementY) / BLOCK_SIZE] == 4) {
					livello[player.r.left / BLOCK_SIZE + 1][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
				}
				score++;
				break;
			}
			
			switch (topColl(livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], livello[player.r.left / BLOCK_SIZE + 1][(player.r.top - movementY) / BLOCK_SIZE])) {
			case 3:
				if (livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] == 5) {
					livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}

				if (livello[player.r.left / BLOCK_SIZE + 1][(player.r.top - movementY) / BLOCK_SIZE] == 5) {
					livello[player.r.left / BLOCK_SIZE + 1][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}
			case true:
				if ( player.r.top / BLOCK_SIZE > (player.r.top - movementY) / BLOCK_SIZE) {
					movementY = player.r.top - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;
					player.jmpPow = 0;
				}
				break;
			case 2:
				if (livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] == 4) {
					livello[player.r.left / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}

				if (livello[player.r.left / BLOCK_SIZE + 1][(player.r.top - movementY) / BLOCK_SIZE] == 4) {
					livello[player.r.left / BLOCK_SIZE + 1][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}
				score++;
				break;
			}
			
		}
		
	}
	//collisioni a destra e sinistra
	if (movementX != 0)  {
		if ((player.r.top - movementY) % BLOCK_SIZE < (player.r.bottom - movementY) % BLOCK_SIZE || (player.r.bottom - movementY) % BLOCK_SIZE == 0) {
			switch (sideColl(livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], NULL)) {
			case true:
				if (player.r.left / BLOCK_SIZE > (player.r.left + movementX) / BLOCK_SIZE) {
					movementX = (player.r.left / BLOCK_SIZE) * BLOCK_SIZE - player.r.left;
					player.vel = 0;
				}
				break;
			case 2:
				livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}
			
			switch (sideColl(livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], NULL)) {
			case true:
				if ((player.r.right / BLOCK_SIZE < (player.r.right + movementX) / BLOCK_SIZE || player.r.right == ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementX = ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE - player.r.right;
					player.vel = 0;
				}
				break;
			case 2:
				livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}
			
		}
		else {
			switch (sideColl(livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1])) {
			case true:
				if (player.r.left / BLOCK_SIZE > (player.r.left + movementX) / BLOCK_SIZE) {
					movementX = (player.r.left / BLOCK_SIZE) * BLOCK_SIZE - player.r.left;
					player.vel = 0;
				}
				break;
			case 2:
				if (livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] == 4) {
					livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}

				if (livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1] == 4) {
					livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1] = 0;
				}
				score++;
			}

			switch (sideColl(livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE], livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1])) {
			case true:
				if ((player.r.right / BLOCK_SIZE < (player.r.right + movementX) / BLOCK_SIZE || player.r.right == ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementX = ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE - player.r.right;
					player.vel = 0;
				}
				break;
			case 2:
				if (livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] == 4) {
					livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				}

				if (livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1] == 4) {
					livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + 1] = 0;
				}
				score++;
				break;
			}
			
			
		}
	}

	//controllo stati
	if (player.state != state::jumping) {
		//cambia lo stato se non in jumping
		if (player.vel == 0 && !D.held && !A.held) {
			player.state = state::idle;
		}
		else if (player.vel != 0 && player.state == state::idle) {
			player.state = state::walking;
		}

		if (player.r.left % BLOCK_SIZE < player.r.right % BLOCK_SIZE || player.r.right % BLOCK_SIZE == 0) {
			if (!bottomColl(livello[player.r.left / BLOCK_SIZE][player.r.bottom / BLOCK_SIZE], NULL)) {
				discesa = true;
			}
		}
		else {
			if (!bottomColl(livello[player.r.left / BLOCK_SIZE][player.r.bottom / BLOCK_SIZE], livello[player.r.left / BLOCK_SIZE + 1][player.r.bottom / BLOCK_SIZE])) {
				discesa = true;
			}
		}
	}

	if (discesa) {
		player.state = state::jumping; // si mette lo stto di jumping
	}

	// variabile per il riprisstino e per l'uccisione di un nemico
	bool dead = false, kill = false;
	if (player.r.bottom > 600) {
		dead = true;
	}

	//funzione per i nemici
	if (!dead) {
		vector<entity> uot;
		for (int i = 0; i < en.size(); i++) {
			bool elimina = false;
			entity& e = en[i];
			//si fa il movimento solo se il nemico è all'interno dello schermo
			if (e.r.left < cam.posX + SCREEN_WIDTH) {
				movimentoEntità(livello, BLOCK_SIZE, e, SCREEN_WIDTH, uot, elimina);
				//si controlla se il nemico è fuori dallo schermo
				if (e.type >= 0 && (e.r.right < cam.posX || e.r.left <= 0 || (e.r.left > cam.posX + SCREEN_WIDTH && e.r.left - e.vel < cam.posX + SCREEN_WIDTH))) {
					elimina = true;
				}

				//si controlla se il player uccide il nemico
				if (e.type == 0 && player.state == state::jumping && !kill && player.r.bottom - movementY >= e.r.top && player.r.bottom <= e.r.top && player.r.right > e.r.left && player.r.left < e.r.right && movementY < 0) {
					movementY = player.r.bottom - e.r.top;
					jump = true;
					discesa = false;
					kill = true;
					elimina = true;
				}
				if (elimina) {
					en.erase(en.begin() + i);
					break;
				}

				//si controlla se il player muore
				if (e.type != 2 && !jump && player.r.right + movementX > e.r.left && player.r.left + movementX < e.r.right && player.r.top - movementY < e.r.bottom && player.r.bottom - movementY > e.r.top && player.immunity == player.initialImmunity) {
					player.life--;
					player.immunity--;
				}
			}				
		}
		//si pushano tutti i nemici che sono nell'array
		for (entity i : uot) {
			en.push_back(i);
		}
	}
	
	//movimento player effettivo
	player.r.left += movementX;
	player.r.right += movementX;
	player.r.top -= movementY;
	player.r.bottom -= movementY;
	movementX = 0;
	movementY = 0;

	//spostamento cam.posX
	if (player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX > SCREEN_WIDTH/2 && cam.posX + SCREEN_WIDTH < (livSize)* BLOCK_SIZE) {
		cam.posX += player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX - SCREEN_WIDTH/2;
	}

	//controllo se le vite sono a zero
	if (player.life <= 0) {
		dead = true;
	}

	if (dead) {
		ripristino(en,size, initialArr, livello, initialLiv,SCREEN_HEIGTH,BLOCK_SIZE, livSize);
		score = 0;
		tempo = 0;
	}
}

short sideColl(int m, int f) {
	if (f) {
		switch (f) {
		case 1: return true;
		case 3: return true;
		case 4: return 2;
		case 5: return true;
		}
	}

	switch (m) {
	case 1: return true;
	case 3: return true;
	case 4: return 2;
	case 5: return true;
	default:
		return false;
	}
}

short bottomColl(int m, int f) {
	if (f) {
		switch (f) {
		case 1: return true;
		case 2: return true;
		case 3: return true;
		case 4: return 2;
		case 5: return true;
		}
	}

	switch (m) {
	case 1: return true;
	case 2: return true;
	case 3: return true;
	case 4: return 2;
	case 5: return true;
	default:
		return false;
	}
}

short topColl(int m, int f) {
	if (f) {
		switch (f) {
		case 1: return true;
		case 3: return true;
		case 4: return 2;
		case 5: return 3;
		}
	}

	switch (m) {
	case 1: return true;
	case 3: return true;
	case 4: return 2;
	case 5: return 3;
	default:
		return false;
	}
}

void ripristino(vector<entity>& enemies, int size, vector<entity>& initialArr, int**& livello, int**& initialLiv, int SCREEN_HEIGHT, int BLOCK_SIZE, int livSize) {
	player.r = player.initialPos;
	player.vel = 0;
	player.jmpPow = 0;
	player.state = state::idle;
	cam.posX = 0;
	enemies = initialArr;
	for (int i = 0; i < livSize; i++) {
		for (int j = 0; j < SCREEN_HEIGHT / BLOCK_SIZE; j++) {
			livello[i][j] = initialLiv[i][j];
		}
	}
	player.life = player.maxLife;
	player.immunity = player.initialImmunity;
	//entità.r = entità.initialPos;
}

void movimentoEntità(int** livello,int BLOCK_SIZE,entity& e,int SCREEN_WIDTH, vector<entity>& uot, bool& elimina) {
	bool stop = false;
	//collisioni a destra e sinistra
	if (e.type != 2) {
		if (e.state != state::jumping && e.type != 3) {
			if (e.r.left % BLOCK_SIZE < e.r.right % BLOCK_SIZE || e.r.right % BLOCK_SIZE == 0) {
				if (!bottomColl(livello[e.r.left / BLOCK_SIZE][e.r.bottom / BLOCK_SIZE], NULL)) {
					e.state = state::jumping;
				}
			}
			else {
				if (!bottomColl(livello[e.r.left / BLOCK_SIZE][e.r.bottom / BLOCK_SIZE], livello[e.r.left / BLOCK_SIZE + 1][e.r.bottom / BLOCK_SIZE])) {
					e.state = state::jumping;
				}
			}
		}
		else {
			if (e.r.left % BLOCK_SIZE < e.r.right % BLOCK_SIZE || e.r.right % BLOCK_SIZE == 0) {
				if (bottomColl(livello[e.r.left / BLOCK_SIZE][(int)(e.r.bottom - e.jmpPow) / BLOCK_SIZE], NULL) && e.r.bottom <= ((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE) {

						e.jmpPow = e.r.bottom - floor((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE;
						stop = true;

				}
			}
			else {
				if (bottomColl(livello[e.r.left / BLOCK_SIZE][(int)(e.r.bottom - e.jmpPow) / BLOCK_SIZE], livello[e.r.left / BLOCK_SIZE + 1][(int)(e.r.bottom - e.jmpPow) / BLOCK_SIZE]) && e.r.bottom <= ((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE) {
						e.jmpPow = e.r.bottom - floor((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE;
						stop = true;

				}
			}
		}
		//controllo collisioni si elimina la palla di cannone se tocca un muro
		if (e.r.right + e.vel > cam.posX && e.r.left + e.vel < cam.posX + SCREEN_WIDTH) {
			if ((int)(e.r.top - e.jmpPow) % BLOCK_SIZE < (int)(e.r.bottom - e.jmpPow) % BLOCK_SIZE || (int)(e.r.bottom - e.jmpPow) % BLOCK_SIZE == 0) {
				if (e.vel < 0 && sideColl(livello[(e.r.left + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE], NULL)) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
				}
				if (e.vel > 0 && sideColl(livello[(e.r.right + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE], NULL)) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
				}
			}
			else {
				if (sideColl(livello[(e.r.left + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE], livello[(e.r.left + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE + 1])) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
				}
				if (sideColl(livello[(e.r.right + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE], livello[(e.r.right + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE + 1])) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
				}
			}
		}
		//movimento nemico
		e.r.right += e.vel;
		e.r.left += e.vel;
		//salto nemico
		if (state::jumping == e.state) {
			e.r.bottom -= e.jmpPow;
			e.r.top -= e.jmpPow;
			e.jmpPow -= e.jmpDec;
		}
		//fermata salto nemico
		if (stop) {
			e.state = state::walking;
			e.jmpPow = 0;
		}
	}
	else if(e.type == 2){
		if (player.r.right < e.r.left) {
			e.fpa--;
			if (e.fpa == 0) {
				uot.push_back({
						{e.r.left - BLOCK_SIZE, e.r.top + 7, e.r.left, e.r.bottom - 7},  // r
						-12,                  // vel
						0.0,                   // jmpDec
						0.0,                   // jmpPow (default value, change if needed)
						 state::walking,        // state
						3,
						0,
						0
					});
				e.fpa = e.iniFpa;
			}
				
		}
		else if (player.r.left > e.r.right) {
			e.fpa--;
			if (e.fpa == 0) {
				uot.push_back({
						{e.r.right , e.r.top + 7, e.r.right + BLOCK_SIZE, e.r.bottom - 7},  // r
						 12,                  // vel
						0.0,                   // jmpDec
						0.0,                   // jmpPow (default value, change if needed)
						 state::walking,        // state
						3,
						0,
						0
					});
				e.fpa = e.iniFpa;
			}
		}
	}
}