#include "player.h"
#include "pulsante.h"
#include "camera.h"
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

//variabili del player
struct gameStuff player = {
	player.r = { 24,300,48,332 },
	player.initialPos = player.r,
	player.vel = 0,
	player.acc = 0.1,
	player.initialAcc = 1,
	player.dec = 0.5,
	player.jmpDec = 1,
	player.jmpHigh = 0.3,
	player.initialJmp = 9.2,
	player.velMax = 5,
	player.jmpPow = 0,
	player.state = state::idle,
	player.highJump = false,
	player.life = 3,
	player.maxLife = 3,
	player.immunity = 90,
	player.initialImmunity = 90,
	player.facingLeft = true
};
bool discesa ,jump = false;
int movementX = 0, movementY = 0;

//gestione movimento del player 
void movimentoPlayer(int**& livello, int**& initialLiv, int BLOCK_SIZE, vector<entity>& en, int size, int SCREEN_WIDTH, int livSize, vector<entity>& initialArr, int& score, int SCREEN_HEIGTH, int& tempo, int pWidthBlock, int pHeightBlock)
{
	if (jump) {
		player.jmpPow = player.initialJmp;
		player.state = state::jumping;
		player.highJump = true;
	}
	movementX += player.vel; //+ movimento dipendente da entità
	movementY += player.jmpPow; //+ movimento dipendente da entità
	discesa = true;
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

	if (D.held && !A.held && player.vel >= 0 && player.vel < 1) {
		player.vel += player.initialAcc;
		if(player.state == state::idle || player.state == state::walking)
			player.facingLeft = false;
	}
	else if (A.held && !D.held && player.vel <= 0 && player.vel > -1) {
		player.vel -= player.initialAcc;
		if (player.state == state::idle || player.state == state::walking)
			player.facingLeft = true;
	}
	else if (player.state == state::walking) {
		if (player.vel > 0 && (!D.held || A.held)) {
			player.vel -= player.dec;
		}
		else if (player.vel < 0 && (!A.held || D.held)) {
			player.vel += player.dec;
		}

		if (abs(player.vel) < player.dec) {
			player.vel = 0;
		}
	}
	//si diminuisce la forza salto
	if (player.state == state::jumping) {
		if (player.jmpPow <= 0) {
			player.highJump = false;
		}
		if (player.highJump)
			player.jmpPow -= player.jmpHigh;
		else
			player.jmpPow -= player.jmpDec;
	}

	//movimento a destra
	if (D.held && !A.held) {
		//movimento normale
		player.vel += player.acc;
		if (player.facingLeft)
			player.facingLeft = false;
	}
	//movimento a sinitra
	if (A.held && !D.held) {
		//movimento normale
		player.vel -= player.acc;
		if (!player.facingLeft)
			player.facingLeft = true;
	}

	//salto
	if (W.pressed && player.state != state::jumping) {
		player.jmpPow = player.initialJmp;
		player.state = state::jumping;
		player.highJump = true;
	}

	//salto più alto
	if (player.state == state::jumping && W.held == false) {
		player.highJump = false;
	}

	//controlla la velocità massima
	if (player.vel > player.velMax && D.held) {
		player.vel = player.velMax;
	}
	else if (player.vel < -player.velMax && A.held) {
		player.vel = -player.velMax;
	}

	// variabile per il riprisstino e per l'uccisione di un nemico
	bool dead = false, kill = false;
	if (player.r.bottom > 600) {
		dead = true;
	}


	//funzione per i nemici
	if (!dead) {
		vector<entity> uot;//riferimento per aggiungere nemici nella funzione EnemyMovement
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
					jump = true;//serve per evitare di fare collisione una volta che si è ucciso
					discesa = false;//mette la discesa quindi mette il jumping
					kill = true;// non so a cosa serve
					elimina = true;// serve per rimuovere il nemico in modo ordinato
				}
				if (elimina) {
					en.erase(en.begin() + i);//funzione per eliminare in maniera ordinata
					break;
				}

				if (kill)
					kill = false;

				//si controlla se il player fa collisione
				if (e.type != 2 && !jump && player.r.right + movementX >= e.r.left && player.r.left + movementX <= e.r.right && player.r.top - movementY <= e.r.bottom && player.r.bottom - movementY >= e.r.top) {
					switch (e.type) {
					case 1:
						//piattaforme mobili:
						//si controlla che si possa fare una collisione
						if (player.r.bottom <= e.r.top && player.jmpPow <= 0) {
							//si cambia per snappare con la entità
							movementY = player.r.bottom - e.r.top;
							//se lo stato è jumping lo cambio
							if (player.state == state::jumping)
								player.state = state::idle;
							//cambio le variabili e nullifico la discesa
							player.jmpPow = 0;
							movementX += e.vel;
							discesa = false;
						}


						break;
					case 4:
						if (player.life < 3)
							player.life++;
						en.erase(en.begin() + i);
						break;
					case 5:
						player.velMax = 10;
						if (player.powerUpTime[e.type] < e.fpa) player.powerUpTime[e.type] = e.fpa;
						//rimuovere l'entità dal gruppo di entità
						en.erase(en.begin() + i);
						break;
					default:
						if (player.immunity == player.initialImmunity && (player.r.bottom - movementY > e.r.top && player.r.top - movementY < e.r.bottom)) {
							player.life--;
							player.immunity--;
							// si fa saltare indietro il player
							player.state = state::jumping;
							player.jmpPow = 2;
							movementY += player.jmpPow;
							kill = true; // variabile che serve per non killare il nemico se subisci danni
							if (player.r.left + movementX < e.r.right && player.r.left + movementX >= e.r.left)
								player.vel = 3;
							else
								player.vel = -3;
						}

						break;
					}

				}
			}
		}
		//si pushano tutti i nemici che sono nell'array
		for (entity i : uot) {
			en.push_back(i);
		}
	}

	//collisioni in alto e in basso

	if (player.r.left / BLOCK_SIZE + pWidthBlock == player.r.right / BLOCK_SIZE && player.r.right % BLOCK_SIZE != 0) {
		pWidthBlock++;
	}

	//si prende per quanti blocchi bisogna fare la collisione
	for (int i = 0; i < pWidthBlock; i++) {
			if (player.jmpPow <= 0)
			switch (bottomColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE])) {
			case true:
				if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;
					player.state = state::idle;
					player.jmpPow = 0;
				}
				discesa = false;
				break;
			case 2:
				livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}

			if (player.jmpPow > 0)
			switch (topColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE])) {
			case 3:
				livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
			case true:
					movementY = ((player.r.top - movementY) / BLOCK_SIZE + 1)*BLOCK_SIZE - player.r.top;

					player.jmpPow = 0;
				
				break;
			case 2:
				livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				score++;
				break;
			}
		}
	//collisioni a destra e sinistra da sistemare nelle piattaforme mobili
		
		//si prende per quanti blocchi bisogna fare la collisione
		if (movementX != 0) {
			if (player.r.top / BLOCK_SIZE + pHeightBlock == player.r.bottom / BLOCK_SIZE && (player.r.bottom - movementY) % BLOCK_SIZE != 0) {
				pHeightBlock++;
			}
			for (int i = 0; i < pHeightBlock; i++) {
				switch (sideColl(livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
				case true:
					if (player.r.left / BLOCK_SIZE > (player.r.left + movementX) / BLOCK_SIZE) {
						movementX = (player.r.left / BLOCK_SIZE) * BLOCK_SIZE - player.r.left;
						player.vel = 0.99;
					}
					break;
				case 2:
					livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
					score++;
				}

				switch (sideColl(livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
				case true:
					if ((player.r.right / BLOCK_SIZE < (player.r.right + movementX) / BLOCK_SIZE || player.r.right == ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementX = ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE - player.r.right;
						player.vel = -0.99;
					}
					break;
				case 2:
					livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
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
		}

		//movimento player effettivo
		player.r.left += movementX;
		player.r.right += movementX;
		player.r.top -= movementY;
		player.r.bottom -= movementY;
		movementX = 0;
		movementY = 0;

		//spostamento cam.posX
		if (player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX > SCREEN_WIDTH / 2 && cam.posX + SCREEN_WIDTH < (livSize)*BLOCK_SIZE) {
			cam.posX += player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX - SCREEN_WIDTH / 2;
		}

		//controllo se le vite sono a zero
		if (player.life <= 0) {
			dead = true;
		}

		//si riparte dal 
		if (dead) {
			ripristino(en, size, initialArr, livello, initialLiv, SCREEN_HEIGTH, BLOCK_SIZE, livSize);
			score = 0;
			tempo = 0;
		}

		if (discesa) {
			player.state = state::jumping; // si mette lo stto di jumping
		}
}

short sideColl(int m) {
	switch (m/100) {
	case 1: return true;
	case 3: return 2;
	case 4: return true;
	default:
		return false;
	}
}

short bottomColl(int m) {
	switch (m/100) {
	case 1: return true;
	case 2: return true;
	case 3: return 2;
	case 4: return true;
	default:
		return false;
	}
}

short topColl(int m) {
	switch (m/100) {
	case 1: return true;
	case 3: return 2;
	case 4: return 3;
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
	//collisioni a destra e sinistra si cambia direzione se non è un proiettile
	switch (e.type) {
	case 2:
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
		break;
	default:
		int limit = e.eBlockWidth;
		if (e.r.left / BLOCK_SIZE + e.eBlockWidth == e.r.right / BLOCK_SIZE && e.r.right % BLOCK_SIZE != 0) {
			limit++;
		}
		if (e.state != state::jumping && e.type != 3) {
			
			int count = 0;
			for (int i = 0; i < limit; i++) {
				if (bottomColl(livello[e.r.left / BLOCK_SIZE + i][e.r.bottom / BLOCK_SIZE]) != true) {
					count++;
				}
			}
			if (count == limit) {
				e.state = state::jumping;
			}
		}
		else {
				for (int i = 0; i < limit; i++) {
					if (bottomColl(livello[e.r.left / BLOCK_SIZE + i][(int)(e.r.bottom - e.jmpPow) / BLOCK_SIZE]) == true && e.r.bottom <= ((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE) {
						e.jmpPow = e.r.bottom - floor((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE;
						stop = true;
						break;

					}
				}
		}
		//controllo collisioni si elimina la palla di cannone se tocca un muro
		limit = e.eBlockHeight;
		if (e.r.top / BLOCK_SIZE + e.eBlockHeight == e.r.bottom / BLOCK_SIZE && (int)(e.r.bottom - e.jmpPow) % BLOCK_SIZE != 0) {
			limit++;
		}
		if (e.r.right + e.vel > cam.posX && e.r.left + e.vel < cam.posX + SCREEN_WIDTH) {
			
			for (int i = 0; i < limit; i++) {
				if (e.vel < 0 && sideColl(livello[(e.r.left + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE + i]) == true) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
					break;
				}
				if (e.vel > 0 && sideColl(livello[(e.r.right + (int)e.vel) / BLOCK_SIZE][e.r.top / BLOCK_SIZE + i]) == true) {
					e.vel = -e.vel;
					if (e.type == 3) {
						elimina = true;
					}
					break;
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
		break;
	}
}