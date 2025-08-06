#include "player.h"
#include "pulsante.h"
#include "camera.h"
#include "audio.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <cmath>
#include <typeinfo>
#include <vector>
#include <psapi.h>
using namespace std;

//variabili del player
struct gameStuff player = {
	player.r = { 0,448,24,480 },
	player.vel = 0,
	player.acc = 0.14,
	player.initialAcc = 1,
	player.dec = 0.6,
	player.jmpDec = 1,
	player.jmpHigh = 0.3,
	player.initialJmp = 9.2,
	player.velMax = 5,
	player.jmpPow = 0,
	player.state = state::idle,
	player.widthBlock = 0,
	player.heightBlock = 0,
	player.widthPl = false,
	player.heightPl = false,
	player.highJump = false,
	player.life = 3,
	player.maxLife = 3,
	player.immunity = 30,
	player.initialImmunity = 30,
	player.facingLeft = true,
	player.shooting = false
};
bool discesa;
int movementX = 0, movementY = 0, shootingCooldown = 0;

void printMemoryUsage(const std::string& label) {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		SIZE_T memKB = pmc.WorkingSetSize / 1024;
		std::cout << label << " - RAM: " << memKB << " KB" << std::endl;
	}
	else {
		std::cerr << "Errore nella lettura della memoria\n";
	}
}

void addActionToEnemy(entity& e, short actionType, short firstAction, short actionTime) {
	e.actions.push_back({ actionType,firstAction, actionTime });
}

//gestione movimento del player 
void movimentoPlayer(int**& livello, int livSize, int heightSize, vector<tuple<int, int, int>>& changeLiv, vector<entity>& en, vector<entity>& screenEn, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, int SCREEN_HEIGHT, bool& ripristina, int& score, string& animIndex, animazione& playerAnim)
{
	int x, y;
	
	discesa = true;

	//azioni dipendnti dallo stato (movimento)
	switch (player.state) {
	case state::jumping:
		//salto più alto
		if (W.held == false || player.jmpPow <= 0) {
			player.highJump = false;
		}

		// si salta secondo la forza da diminuire
		if (player.highJump)
			player.jmpPow -= player.jmpHigh;
		else
			player.jmpPow -= player.jmpDec;

		//movimento a destra
		if (D.held && !A.held) {
			//movimento normale
			player.vel += player.acc;

			//si controlla la velocità massima
			if (player.vel > player.velMax) {
				player.vel = player.velMax;
			}
		}
		//movimento a sinitra
		if (A.held && !D.held) {
			//movimento normale
			player.vel -= player.acc;

			//si controlla la velocità massima
			if (player.vel < -player.velMax) {
				player.vel = -player.velMax;
			}
		}
		break;
	
	case state::walking:


		if (A.pressed) {
			player.vel -= player.initialAcc;
			player.facingLeft = true;
		}
		else if (D.pressed) {
			player.vel += player.initialAcc;
			player.facingLeft = false;
		}
		else {
			//movimento a destra
			if (D.held && !A.held) {
				//movimento normale
				player.vel += player.acc;
				if (player.facingLeft)
					player.facingLeft = false;

				//si controlla la velocità massima
				if (player.vel > player.velMax) {
					player.vel = player.velMax;
				}
			}
			else if (player.vel > 0) {
				player.vel -= player.dec;
				if (abs(player.vel) < player.dec) {
					player.vel = 0;
				}
			}
			//movimento a sinitra
			if (A.held && !D.held) {
				//movimento normale
				player.vel -= player.acc;
				if (!player.facingLeft)
					player.facingLeft = true;

				//si controlla la velocità massima
				if (player.vel < -player.velMax) {
					player.vel = -player.velMax;
				}
			}
			else if (player.vel < 0) {
				player.vel += player.dec;
				if (abs(player.vel) < player.dec) {
					player.vel = 0;
				}
			}

			//cambia lo stato se non in jumping
			if (player.vel == 0 &&((!A.held && !D.held) ||( A.held && D.held))) {
				player.state = state::idle;
			}
		}

		//salto
		if (W.pressed) {
			player.jmpPow = player.initialJmp;
			player.state = state::jumping;
			player.highJump = true;
			//PlayAudio(L"./sfx/jump.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio

			
		}

		break;
	case state::idle:
		//movimento iniziale
		if (A.pressed) {
			player.vel -= player.initialAcc;
			player.facingLeft = true;
		}
		else if (D.pressed) {
			player.vel += player.initialAcc;
			player.facingLeft = false;
		}

		// si cambia lo stato in walking
		if (player.vel > 0) {
			player.state = state::walking;
		}
		else if (player.vel < 0) {
			player.state = state::walking;
		}

		//salto
		if (W.pressed) {
			player.jmpPow = player.initialJmp;
			player.state = state::jumping;
			player.highJump = true;
			//PlayAudio(L"./sfx/jump.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio

		}

		break;
	}

	//azione che dipende dal powerUP
	if (S.pressed) {
		double vel = 12;
		

		if (!player.facingLeft && player.shooting && shootingCooldown == 0) {
			//PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones

			if (sideColl(livello[player.r.right / BLOCK_SIZE][(player.r.top + 7)/BLOCK_SIZE]) != true && sideColl(livello[(player.r.right + 10) / BLOCK_SIZE][(player.r.top + 7) / BLOCK_SIZE]) != true) {
				screenEn.push_back({
					{player.r.right, player.r.top + 7, player.r.right + 10, player.r.bottom - 7},  // r
					vel,                  // vel
					0.0,                   // jmpDec
					0.0,                   // jmpPow (default value, change if needed)
					0,0,					//movements sempre a 0
					 state::walking,        // state
					-1,
					{},
					{},
					1,
					1,
					false,
					"walking",
					true,
					5, nullptr, 0,0,0,0,0
					});
				addActionToEnemy(screenEn[screenEn.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
				newAnimation(screenEn[screenEn.size() - 1].animations, 16, 16, 16, 16, "walking");
				addFrame(screenEn[screenEn.size() - 1].animations, 1, "walking");
			}
			
			
			animIndex = "shooting";
			reset(playerAnim, animIndex);
			shootingCooldown = 60;
		}
		else if(player.shooting && shootingCooldown == 0){
			//PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones
			if (sideColl(livello[player.r.left / BLOCK_SIZE][(player.r.top + 7) / BLOCK_SIZE]) != true && sideColl(livello[(player.r.left - 10) / BLOCK_SIZE][(player.r.top + 7) / BLOCK_SIZE]) != true) {

				screenEn.push_back({
						{player.r.left - 10, player.r.top + 7, player.r.left, player.r.bottom - 7},  // r
						-vel,                  // vel
						0.0,                   // jmpDec
						0.0,                   // jmpPow (default value, change if needed)
						0,0,
						 state::walking,        // state
						-1,
						{},
						{},
						1,
						1,
						true,
						"walking",
						true,
						5, nullptr, 0,0,0,0,0
					});
				addActionToEnemy(screenEn[screenEn.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
				newAnimation(screenEn[screenEn.size() - 1].animations, 16, 16, 16, 16, "walking");
				addFrame(screenEn[screenEn.size() - 1].animations, 1, "walking");
			}

			animIndex = "shooting";
			reset(playerAnim, animIndex);
			shootingCooldown = 60;
		}
	}

	movementX += player.vel; //+ movimento dipendente da entità
	movementY += player.jmpPow; //+ movimento dipendente da entità

	// variabile per il riprisstino e per l'uccisione di un nemico
	bool kill = false; //top collision per vedere se si muore nel caso dell piattaforma mobile;
	if (player.r.bottom / BLOCK_SIZE >= heightSize) {
		ripristina = true;
		return;
	}

	//funzione per i nemici
	{
		vector<entity> uot;//riferimento per aggiungere nemici nella funzione EnemyMovement
		// si aggiunge nemici per la loro posizione
		while (size < en.size() && en[size].r.left < cam.posX + SCREEN_WIDTH) {
			screenEn.push_back(en[size]);
			size++;
		}
		for (int i = 0; i < screenEn.size(); i++) {
			entity& e = screenEn[i];
			
			e.elimina = false;
			movimentoEntità(livello,livSize, heightSize, BLOCK_SIZE, e,screenEn, SCREEN_WIDTH, uot, kill, ripristina, score);
			if (e.elimina) {
				if (e.child) {
					uot.push_back(*e.child);
					//si usa la r.right per la width e la r left per offset che dipende dalla direzione cioè facing Left
					int enemyWidth = e.r.right - e.r.left;
					int childWidth = e.child->r.right - e.child->r.left;
					uot[uot.size() - 1].r.left = e.r.left + (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);
					uot[uot.size() - 1].r.right = e.r.right - (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);


					//si usa la r bottom per la height e la r top per offset NEGATIVO
					int enemyHeight = e.r.bottom - e.r.top;
					int childHeight = e.child->r.bottom - e.child->r.top;
					uot[uot.size() - 1].r.top = e.r.top + (enemyHeight - childHeight) / 2 - e.child->r.top;
					uot[uot.size() - 1].r.bottom = e.r.bottom - (enemyHeight - childHeight) / 2 - e.child->r.top;
				}
				screenEn.erase(screenEn.begin() + i);//funzione per eliminare in maniera ordinata
				i--;
				break;
			}
		}

		//movimento effettivo (si fa un altro for perché così si riescono a fare tutte le collisioni del caso
		for (int i = 0; i < screenEn.size(); i++) {
			entity& e = screenEn[i];

			//collisioni a destra e sinistra si cambia direzione se non è un proiettile
			if (e.r.right < BLOCK_SIZE * (livSize - 2)) {
				switch (e.type) {
				case 2:
					break;
				case 6:
					//si controlla se il player salta sul trampoligga
					if (player.state == state::jumping && !kill && player.r.bottom - movementY >= e.r.top && player.r.bottom <= e.r.top && player.r.right > e.r.left && player.r.left < e.r.right && movementY < 0) {
						player.jmpPow = 13.2;
						player.state = state::jumping;
						player.highJump = true;
						movementY = player.r.bottom - e.r.top;
						discesa = false;//mette la discesa quindi mette il jumping
						kill = true; // variabile che serve per non killare il nemico se subisci danni
						break;

					}
					break;
				case 0:
				case 5:
				{
					//si controlla se il player uccide il nemico
					if (e.type == 0 && player.state == state::jumping && !kill && player.r.bottom - movementY >= e.r.top && player.r.bottom <= e.r.top && player.r.right > e.r.left && player.r.left < e.r.right && movementY < 0) {
						player.jmpPow = player.initialJmp;
						player.state = state::jumping;
						player.highJump = true;
						movementY = player.r.bottom - e.r.top;
						discesa = false;//mette la discesa quindi mette il jumping
						kill = true; // variabile che serve per non killare il nemico se subisci danni
						e.elimina = true;// serve per rimuovere il nemico in modo ordinato

						score += 10;
						break;

					}
					else if (e.type == 5) {
						//cout << e.r.bottom << endl;
						if (player.state == state::jumping && !kill && player.r.top - movementY <= e.r.bottom && player.r.top >= e.r.bottom && player.r.right > e.r.left && player.r.left < e.r.right && movementY >= 0) {
							player.jmpPow = 0;
							movementY = player.r.top - e.r.bottom;
							discesa = true;//mette la discesa true
							kill = true; // variabile che serve per non killare il nemico se subisci danni
							e.elimina = true;// serve per rimuovere il nemico in modo ordinato


							score += 10;
						}
						break;
					}
				}
				default:
				{
					int limit = e.eBlockWidth;
					if (e.r.left / BLOCK_SIZE + e.eBlockWidth == e.r.right / BLOCK_SIZE && e.r.right % BLOCK_SIZE != 0) {
						limit++;
					}
					if (e.state != state::jumping) {

						int count = 0;
						for (int i = 0; i < limit; i++) {
							if (e.r.left / BLOCK_SIZE + i < livSize && bottomColl(livello[e.r.left / BLOCK_SIZE + i][e.r.bottom / BLOCK_SIZE]) != true) {
								count++;
							}
						}
						if (count == limit) {
							e.state = state::jumping;

						}
						else if (count > 0 && e.turning) {
							e.vel = -e.vel;
							e.movementX = e.vel;
						}
					}
					else if (e.type != 1) {
						for (int i = 0; i < limit; i++) {
							if (e.r.left / BLOCK_SIZE + i < livSize && bottomColl(livello[e.r.left / BLOCK_SIZE + i][(int)(e.r.bottom - e.movementY) / BLOCK_SIZE]) == true && e.r.bottom <= ((e.r.bottom - e.movementY) / BLOCK_SIZE) * BLOCK_SIZE) {
								e.movementY = e.r.bottom - floor((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE;
								e.stop = true;
								break;

							}
						}
					}
					//controllo collisioni si elimina se ha collision death
					limit = e.eBlockHeight;
					if (e.r.top / BLOCK_SIZE + e.eBlockHeight == e.r.bottom / BLOCK_SIZE && (int)(e.r.bottom - e.movementY) % BLOCK_SIZE != 0) {
						limit++;
					}
					if (e.r.right < BLOCK_SIZE *(livSize - 2)) {

						for (int i = 0; i < limit; i++) {
							if (e.movementX < 0 && sideColl(livello[(e.r.left + (int)e.movementX) / BLOCK_SIZE][(int)(e.r.top - e.movementY) / BLOCK_SIZE + i]) == true) {
								e.vel = -e.vel;
								e.movementX = e.vel;
								if (e.collisionDestroy) {
									e.elimina = true;
								}
								break;
							}
							if (e.movementX > 0 && sideColl(livello[(e.r.right + (int)e.movementX) / BLOCK_SIZE][(int)(e.r.top - e.movementY) / BLOCK_SIZE + i]) == true) {
								e.vel = -e.vel;
								e.movementX = e.vel;
								if (e.collisionDestroy) {
									e.elimina = true;
								}
								break;
							}
						}
					}
					break;
				}
				}
			}

			if (kill)
				kill = false;

			//si controlla se il player fa collisione
			if (e.type != 2 && player.r.right + movementX >= e.r.left && player.r.left + movementX <= e.r.right && player.r.top - movementY <= e.r.bottom && player.r.bottom - movementY >= e.r.top) {
				switch (e.type) {
				case -1:
					if (e.vel == 0) {
						if (player.r.left - e.r.left < e.r.right - player.r.right) {
							e.vel = 10;
						}
						else {
							e.vel = -10;
						}
					}
					break; // si levano i proiettili del player
				case 6:break; // si leva il trampolino jmbr
				case 7:
				case 1:
					//piattaforme mobili:
					//si controlla che si possa fare una collisione
					if (player.r.bottom <= e.r.top && discesa) {
						//si cambia per snappare con la entità
						movementY = player.r.bottom - e.r.top;
						//se lo stato è jumping lo cambio
						if (player.state == state::jumping)
							player.state = state::idle;
						//cambio le variabili e nullifico la discesa
						player.jmpPow = 0;
						movementY += e.movementY;
						movementX += e.movementX;
						discesa = false;

						if (e.type == 7) {
							//si controllano le collisioni del player per non fala cadere prematuramente
							bool contr = false;
							for (int i = 0; i < (player.widthBlock + player.widthPl); i++) {
								if (movementY <= 0)
									switch (bottomColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE])) {
									case true:
										contr = true;
										break;
									}
							}
							if(!contr)
							e.jmpDec = 0.4;
						}
					}
					break;
				case 4: // powerup Spiaccicato in 1 type solo, si usa il get<2> per l'effetto sull'azione -1
					for (auto i : e.actions) {
						if (get<0>(i) == -1) {
							switch (get<2>(i)) {
							case 0:
								if (player.life < 3)
									player.life++;
								e.elimina = true;
								break;
							case 1:
								//si aumenta la velocità massima player
								player.velMax = 10;
								if (player.powerUpTime[get<2>(i)] < get<1>(i))
									player.powerUpTime[get<2>(i)] = get<1>(i);
								//rimuovere l'entità dal gruppo di entità
								e.elimina = true;

								break;
							case 2:
								//si da la possibilità di shootingare
								player.shooting = true;
								player.powerUpTime[get<2>(i)] = get<1>(i);
								e.elimina = true;

								break;
							}
						}

					}
					break;
				default:
					if (player.immunity == player.initialImmunity && (player.r.bottom - movementY > e.r.top && player.r.top - movementY < e.r.bottom)) {
						player.life--;
						player.shooting = false;
						//PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si viene colpiti si fa l'audio
						player.immunity--;
						// si fa saltare indietro il player
						player.state = state::jumping;
						player.jmpPow = 2;
						movementY += player.jmpPow;
						kill = true; // variabile che serve per non killare il nemico se subisci danni
						if (player.r.right > e.r.right)
							player.vel = 3;
						else
							player.vel = -3;
					}
					break;
				case 5:
					break;
				}
			}

			if (e.vel > 0 && e.facingLeft) {
				e.facingLeft = false;
			}
			else if (e.vel < 0 && !e.facingLeft) {
				e.facingLeft = true;
			}

			if (e.elimina) {
				if (e.child) {
					uot.push_back(*e.child);
					//si usa la r.right per la width e la r left per offset che dipende dalla direzione cioè facing Left
					int enemyWidth = e.r.right - e.r.left;
					int childWidth = e.child->r.right - e.child->r.left;
					uot[uot.size() - 1].r.left = e.r.left + (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);
					uot[uot.size() - 1].r.right = e.r.right - (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);


					//si usa la r bottom per la height e la r top per offset NEGATIVO
					int enemyHeight = e.r.bottom - e.r.top;
					int childHeight = e.child->r.bottom - e.child->r.top;
					uot[uot.size() - 1].r.top = e.r.top + (enemyHeight - childHeight) / 2 - e.child->r.top;
					uot[uot.size() - 1].r.bottom = e.r.bottom - (enemyHeight - childHeight) / 2 - e.child->r.top;
				}
				screenEn.erase(screenEn.begin() + i);//funzione per eliminare in maniera ordinata
				i--;
				break;
			}

			//movimento nemico
			e.r.right += e.movementX;
			e.r.left += e.movementX;
			
			e.r.bottom -= e.movementY;
			e.r.top -= e.movementY;

			if (state::jumping == e.state) {
				e.jmpPow -= e.jmpDec;
			}

			//fermata salto nemico
			if (e.stop) {
				e.state = state::walking;
				e.jmpPow = 0;
				e.movementY = e.jmpPow;
			}
		}

		//si pushano tutti i nemici che sono nell'array
		for (entity i : uot) {
			screenEn.push_back(i);
		}

		//uot va pulito
		uot.clear();
	}
	if (ripristina)
		return;

	//collisioni in alto e in basso
	{
		//si prende per quanti blocchi bisogna fare la collisione
		if (player.r.left / BLOCK_SIZE + player.widthBlock == player.r.right / BLOCK_SIZE && player.r.right % BLOCK_SIZE != 0) {
			player.widthPl = true;
		}
		else {
			player.widthPl = false;
		}

		for (int i = 0; i < (player.widthBlock + player.widthPl); i++) {
			if (movementY >= 0 && player.r.bottom != 0 && state::jumping == player.state) {
				switch (topColl(livello[player.r.left / BLOCK_SIZE + i][(int)(player.r.top - movementY - 0.1) / BLOCK_SIZE])) { // si fa con -0.1 per aumentare la precisione e fare le collisioni anche se il valore è precisamente uguale al fondo
					//distruzione
				case 3:
					x = player.r.left / BLOCK_SIZE + i;
					y = (int)(player.r.top - movementY - 0.1) / BLOCK_SIZE;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < heightSize) {
						changeLiv.push_back({ x, y, livello[x][y] }); // ci si salva il cambio che viene fatto nel livello
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/bricks.wav", ab, 0, 0.5);
				case true:
					if (movementY > 0)
						movementY = player.r.top - ((int)(player.r.top - movementY - (0.1 * movementY)) / BLOCK_SIZE + 1) * BLOCK_SIZE;

					player.jmpPow = 0;
					break;
					//distruzione con aggiunta di score
				case 2:
					x = player.r.left / BLOCK_SIZE + i;
					y = (int)(player.r.top - movementY - 0.1) / BLOCK_SIZE;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < heightSize) {
						changeLiv.push_back({ x, y, livello[x][y] }); // ci si salva il cambio che viene fatto nel livello
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

					score++;
					break;
				}
			}
		}

		for (int i = 0; i < (player.widthBlock + player.widthPl); i++) {
			if (movementY <= 0)
				switch (bottomColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE])) {
					//danno
				case 4:
					if (player.immunity == player.initialImmunity) {
						player.life--;
						//PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio
						player.immunity--;
						player.shooting = false;
					}
				case true:
					if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;

						if (player.state == state::jumping)
							////PlayAudio(L"./sfx/step.wav", ab, 0, 0.05); // ogni volta che si atterra si fa l'audio

							if (player.vel == 0 && !A.held && !D.held)
								player.state = state::idle;
							else
								player.state = state::walking;
						player.jmpPow = 0;
					}
					discesa = false;
					break;
					//distruzione con aggiunta di score
				case 2:
					x = player.r.left / BLOCK_SIZE + i;
					y = (player.r.bottom - movementY) / BLOCK_SIZE;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < heightSize) {
						changeLiv.push_back({ x, y, livello[x][y] }); // ci si salva il cambio che viene fatto nel livello
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008); // si fa l'audio ogni volta che prendo una money
					score++;
					break;
				}
		}
	}

	//collisioni a destra e sinistra
	if (movementX != 0) {

		//controllo per restaare nello schermo
		if (player.r.left + movementX < cam.posX) {
			movementX = cam.posX - player.r.left;
			player.vel = 0;
		}
		else if (player.r.right + movementX >= livSize * BLOCK_SIZE) {
			movementX = (livSize * BLOCK_SIZE) - player.r.right;
			player.vel = 0;
		}

		//aumentare l'altezza di un blocco
		if (player.r.top / BLOCK_SIZE + player.heightBlock == player.r.bottom / BLOCK_SIZE && (player.r.bottom - movementY) % BLOCK_SIZE != 0) {
			player.heightPl = true;
		}
		else {
			player.heightPl = false;
		}
		for (int i = 0; i < (player.heightBlock + player.heightPl); i++) {
			switch (sideColl(livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
			case true:
				if (player.r.left / BLOCK_SIZE > (player.r.left + movementX) / BLOCK_SIZE) {
					movementX = (player.r.left / BLOCK_SIZE) * BLOCK_SIZE - player.r.left;
					player.vel = 0.99;
				}
				break;
				//distruzione con aggiunta di score
			case 2:
				x = (player.r.left + movementX) / BLOCK_SIZE;
				y = (player.r.top - movementY) / BLOCK_SIZE + i;

				if (x >= 0 && x < livSize &&
					y >= 0 && y < heightSize) {
					changeLiv.push_back({ x, y, livello[x][y] }); // ci si salva il cambio che viene fatto nel livello
					livello[x][y] = 0;
				}
				//PlayAudioNoReturn(L"./sfx/coin.wav", ab, 0, 0.008);
				score++;
			}

			switch (sideColl(livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
			case true:
				if ((player.r.right / BLOCK_SIZE < (player.r.right + movementX) / BLOCK_SIZE || player.r.right == ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE)) {
					movementX = ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE - player.r.right;
					player.vel = -0.99;
				}
				break;
				//distruzione con aggiunta di score
			case 2:
				x = (player.r.right + movementX) / BLOCK_SIZE;
				y = (player.r.top - movementY) / BLOCK_SIZE + i;

				if (x >= 0 && x < livSize &&
					y >= 0 && y < heightSize) {
					changeLiv.push_back({ x, y, livello[x][y] }); // ci si salva il cambio che viene fatto nel livello
					livello[x][y] = 0;
				}
				//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

				score++;
				break;
			}
		}
	}

	//movimento player effettivo
	{
		player.r.left += movementX;
		player.r.right += movementX;
		player.r.top -= movementY;
		player.r.bottom -= movementY;
		movementX = 0;
		movementY = 0;
	}
	
	//spostamento cam.posX
	if (player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX > SCREEN_WIDTH / 2 && cam.posX + SCREEN_WIDTH < (livSize)*BLOCK_SIZE) {
		cam.posX += player.r.left + ((player.r.right - player.r.left) / 2) - cam.posX - SCREEN_WIDTH / 2;
	}

	//spostamento cam.posY verso Basso
	if (player.r.top + ((player.r.bottom - player.r.top) / 2) - cam.posY > SCREEN_HEIGHT / 2 && cam.posY + SCREEN_HEIGHT < (heightSize)*BLOCK_SIZE) {
		cam.posY += player.r.top + ((player.r.bottom - player.r.top) / 2) - cam.posY - SCREEN_HEIGHT / 2;
		if (cam.posY + SCREEN_HEIGHT > heightSize * BLOCK_SIZE) {
			cam.posY = heightSize * BLOCK_SIZE - SCREEN_HEIGHT;
		}
	}
	// Spostamento verso l'alto
	else if (player.r.top + ((player.r.bottom - player.r.top) / 2) - cam.posY < SCREEN_HEIGHT / 2 &&
		cam.posY > 0) {

		cam.posY += player.r.top + ((player.r.bottom - player.r.top) / 2) - cam.posY - SCREEN_HEIGHT / 2;

		if (cam.posY < 0) {
			cam.posY = 0;
		}
	}

	//si mette jumping
	if (discesa) {
		player.state = state::jumping; // si mette lo stto di jumping
	}

	//si abbassa il cooldown
	if (shootingCooldown > 0) {
		shootingCooldown--;
	}
}

//funzione per le animazioni del cazzo, gli do dei valori delle diverse cose e lui me le fa
void automaticMovement(int**& livello, int livSize, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, int SCREEN_HEIGHT_BLOCK, int& score) {
	discesa = true;

	if (player.state == state::jumping) {
		player.jmpPow -= player.jmpDec;
	
	}
	else if(player.life <= 0){
		//si mette la decelerazione per il pg SOLO SE é MORT, SEnno animazioni fine liv non funzionano
		if (player.vel > 0) {
			player.vel -= player.dec;
			if (abs(player.vel) < player.dec) {
				player.vel = 0;
			}
		}

		if (player.vel < 0) {
			player.vel += player.dec;
			if (abs(player.vel) < player.dec) {
				player.vel = 0;
			}
		}
	}

	

	movementX += player.vel; //+ movimento dipendente da entità
	movementY += player.jmpPow; //+ movimento dipendente da entità

	//collisioni in alto e in basso
	{
		//si prende per quanti blocchi bisogna fare la collisione
		if (player.r.left / BLOCK_SIZE + player.widthBlock == player.r.right / BLOCK_SIZE && player.r.right % BLOCK_SIZE != 0) {
			player.widthPl = true;
		}
		else {
			player.widthPl = false;
		}

		for (int i = 0; i < (player.widthBlock + player.widthPl); i++) {
			if (player.r.left / BLOCK_SIZE + i < livSize &&  movementY <= 0)
				switch ( bottomColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE])) {
					//danno
				case 4:
					if (player.immunity == player.initialImmunity) {
						player.life--;
						//PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio
						player.immunity--;
						player.shooting = false;
					}
				case true:
					if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;

						if (player.state == state::jumping)
							//PlayAudio(L"./sfx/step.wav", ab, 0, 0.05); // ogni volta che si atterra si fa l'audio

						if (player.vel == 0 && !A.held && !D.held)
							player.state = state::idle;
						else
							player.state = state::walking;
						player.jmpPow = 0;
					}
					discesa = false;
					break;
					//distruzione con aggiunta di score
				case 2:
					int x = player.r.left / BLOCK_SIZE + i;
					int y = (player.r.bottom - movementY) / BLOCK_SIZE;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < SCREEN_HEIGHT_BLOCK) {
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008); // si fa l'audio ogni volta che prendo una money
					score++;
					break;
				}

			if (player.r.left / BLOCK_SIZE + i < livSize && movementY >= 0 && player.r.bottom != 0)
				switch ( topColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE])) {
					//distruzione
				case 3:
					//PlayAudio(L"./sfx/bricks.wav", ab, 0, 0.5);
					livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				case true:
					if (movementY > 0)
						movementY = player.r.top - ((player.r.top - movementY) / BLOCK_SIZE + 1) * BLOCK_SIZE ;

					player.jmpPow = 0;

					break;
					//distruzione con aggiunta di score
				case 2:
					int x = player.r.left / BLOCK_SIZE + i;
					int y = (player.r.top - movementY) / BLOCK_SIZE;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < SCREEN_HEIGHT_BLOCK) {
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

					score++;
					break;
				}
		}
	}

	//collisioni a destra e sinistra
	if (movementX != 0) {

		//aumentare l'altezza di un blocco
		if (player.r.top / BLOCK_SIZE + player.heightBlock == player.r.bottom / BLOCK_SIZE && (player.r.bottom - movementY) % BLOCK_SIZE != 0) {
			player.heightPl = true;
		}
		else {
			player.heightPl = false;
		}
		for (int i = 0; i < (player.heightBlock + player.heightPl); i++) {
			if ((player.r.left + movementX) / BLOCK_SIZE < livSize) {
				switch (sideColl(livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
				case true:
					if (player.r.left / BLOCK_SIZE > (player.r.left + movementX) / BLOCK_SIZE) {
						movementX = (player.r.left / BLOCK_SIZE) * BLOCK_SIZE - player.r.left;
						player.vel = 0.99;
					}
					break;
					//distruzione con aggiunta di score
				case 2:
					int x = (player.r.left + movementX) / BLOCK_SIZE;
					int y = (player.r.top - movementY) / BLOCK_SIZE + i;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < SCREEN_HEIGHT_BLOCK) {
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);
					score++;
				}
			}
			if ((player.r.right + movementX) / BLOCK_SIZE < livSize) {
				switch (sideColl(livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i])) {
				case true:
					if ((player.r.right / BLOCK_SIZE < (player.r.right + movementX) / BLOCK_SIZE || player.r.right == ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementX = ((player.r.right + movementX) / BLOCK_SIZE) * BLOCK_SIZE - player.r.right;
						player.vel = -0.99;
					}
					break;
					//distruzione con aggiunta di score
				case 2:
					int x = (player.r.right + movementX) / BLOCK_SIZE;
					int y = (player.r.top - movementY) / BLOCK_SIZE + i;

					if (x >= 0 && x < livSize &&
						y >= 0 && y < SCREEN_HEIGHT_BLOCK) {
						livello[x][y] = 0;
					}
					//PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

					score++;
					break;
				}
			}
		}
	}

	//movimento player effettivo
	{
		player.r.left += movementX;
		player.r.right += movementX;
		player.r.top -= movementY;
		player.r.bottom -= movementY;
		movementX = 0;
		movementY = 0;
	}

	if (discesa) {
		player.state = state::jumping; // si mette lo stto di jumping
	}
}

short sideColl(int m) {
	switch (m/100) {
	case 2:
	case 0: return false;
	case 3: return 2;
	default:
		return true;
	}
}

short bottomColl(int m) {
	switch (m/100) {
	case 0: return false;
	case 3: return 2;
	case 5: if (player.immunity == player.initialImmunity)return 4;
	default:
		return true;
	}
}

short topColl(int m) {
	switch (m/100) {
	case 2:
	case 0: return false;
	case 3: return 2;
	case 4: return 3;
	default:
		return true;
	}
}

void ripristino(vector<entity>& screenEn, int& limit, int**& livello, vector<tuple<int, int, int>>& cambiamentiLivello, RECT pos){
	ripristinoPlayer(pos);

	//si mette la life a 1, !NUOVA SCELTA!
	player.life = 1;

	limit = 0;
	cam.posX = 0;
	cam.posY = 0;
	screenEn.clear();  // svuota i contenuti
	screenEn.shrink_to_fit();  // forza il rilascio della memoria

	for (auto c : cambiamentiLivello) {
		livello[get<0>(c)][get<1>(c)] = get<2>(c);
	}

	cambiamentiLivello.clear();
	cambiamentiLivello.shrink_to_fit();
	//entità.r = entità.initialPos;
	//entità.r = entità.initialPos;
}

//ripristino player 
void ripristinoPlayer(RECT pos) {
	player.r = pos;
	player.vel = 0;
	player.jmpPow = 0;
	player.state = state::idle;
	player.life = player.maxLife;
	player.immunity = player.initialImmunity;
	//serve per eliminare e reverting gli effetti dei powerUP
	for (auto& i : player.powerUpTime) {
		// si diminuice il tempo del power up, se è -1 è infinito
			// si guarda di che tipo è il power up e si reversa la sua azione
		switch (i.first) {
		case 1:
			player.velMax = 5;
			break;
		case 2:
			player.shooting = false;
			break;
		}

	}
	player.powerUpTime.empty();
}

//movimento entità da rifare
void movimentoEntità(int** livello, int livSize,int heightSize, int BLOCK_SIZE, entity& e, vector<entity>& entities, int SCREEN_WIDTH, vector<entity>& uot, bool& kill, bool& ripristina, int& score) {

	if (e.timeAlive != -1) {
		e.timeAlive--;
		if (e.timeAlive == 0) {
			e.elimina = true;
			return;
		}
	}

	e.stop = false;
	e.collisionDestroy = false;
	int increase = 0;
	e.movementX = e.vel;

	if (fmod(e.vel, 1) != 0) {
		e.intermezzoVel += e.vel;
		if (abs(e.intermezzoVel) >= 1) {
			e.movementX = e.intermezzoVel;
			increase = e.intermezzoVel;
			e.intermezzoVel = fmod(e.intermezzoVel, 1);
		}
	}

	e.movementY = e.jmpPow;

	//si controlla se il nemico è fuori dallo schermo
	if (e.r.right < cam.posX || e.r.left <= 0) {
		e.elimina = true;
		return;
	}

	if (e.r.bottom >= heightSize * BLOCK_SIZE) {
		e.elimina = true;
		return;
	}

	e.turning = false;

	//cambio animazione in base allo stato e poi in base all'azione ANIMAZIONI STANDARD
	switch (e.state) {
	case state::walking:
		if (e.vel == 0) {
			if (existsAnim(e.animations, "idle") && e.animIndex != "idle") {
				reset(e.animations, e.animIndex);
				e.animIndex = "idle";
				e.animations.index = 0;
			}
		}
		else if (e.vel != 0) {
			if (existsAnim(e.animations, "walking") && e.animIndex != "walking") {
				reset(e.animations, e.animIndex);
				e.animIndex = "walking";
				e.animations.index = 0;
			}
		}
		break;
	case state::jumping:
		if (e.jmpPow > 0) {
			if (existsAnim(e.animations, "ascending") && e.animIndex != "ascending") {
				reset(e.animations, e.animIndex);
				e.animIndex = "ascending";
			}
		}
		else {
			if (existsAnim(e.animations, "descending") && e.animIndex != "descending") {
				reset(e.animations, e.animIndex);
				e.animIndex = "descending";
			}
		}

		break;
	}

	// si diminuisce il numero delle azioni 
	for (tuple<short, short, short>& i : e.actions) {
		if (get<0>(i) >= 0) {// si guardano le azioni attive, non quelle power up
				// si scorre l'azione diminuendo il tempo per cui accada
			if (get<1>(i) > 0) {
				get<1>(i)--;
			}

			if (get<1>(i) == 0) {
				//azione per get 0
				switch (get<0>(i) / 100) {
				case 0:  //girarsi
					e.vel = -e.vel;
					e.movementX = e.vel;
					if (e.facingLeft)
						e.facingLeft = false;
					else
						e.facingLeft = true;
					get<1>(i) = get<2>(i);
					break;
				case 1: //cambiare la direzione verticale
					e.jmpPow = -e.jmpPow;
					e.movementY = e.jmpPow;

					get<1>(i) = get<2>(i);
					break;
				case 2: // cannones METODO ROTTISSIMO
				{
					double vel = (double)(get<0>(i) % 100)/10;
					if (e.child) {

						uot.push_back(*e.child);
						//si usa la r.right per la width e la r left per offset che dipende dalla direzione cioè facing Left
						int enemyWidth = e.r.right - e.r.left;
						int childWidth = e.child->r.right - e.child->r.left;
						uot[uot.size() - 1].r.left = e.r.left + (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);
						uot[uot.size() - 1].r.right = e.r.right - (enemyWidth - childWidth) / 2 + e.child->r.left * (1 - 2 * e.facingLeft);


						//si usa la r bottom per la height e la r top per offset NEGATIVO
						int enemyHeight = e.r.bottom - e.r.top;
						int childHeight = e.child->r.bottom - e.child->r.top;
						uot[uot.size() - 1].r.top = e.r.top + (enemyHeight - childHeight) / 2 - e.child->r.top;
						uot[uot.size() - 1].r.bottom = e.r.bottom - (enemyHeight - childHeight) / 2 - e.child->r.top;
						uot[uot.size() - 1].vel = vel * (1 - 2 * e.facingLeft);
						get<1>(i) = get<2>(i);
					}
					else {
						if (player.r.right < e.r.left) {
							//PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones


							if (!(e.r.left < cam.posX)) {
								//enemigo standard
								uot.push_back({
									{e.r.left - BLOCK_SIZE, e.r.top + 7, e.r.left, e.r.bottom - 7},  // r
									-vel,                  // vel
									0.0,                   // jmpDec
									0.0,                   // jmpPow (default value, change if needed)
									0,0,
									 state::walking,        // state
									3,
									{},
									{},
									1,
									1,
									true,
									"walking",
									true
									});
								addActionToEnemy(uot[uot.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
								newAnimation(uot[uot.size() - 1].animations, 0, 48, 16, 16, "walking");
								addFrame(uot[uot.size() - 1].animations, 1, "walking");
							}
							get<1>(i) = get<2>(i);
						}
						else if (player.r.left > e.r.right) {
							//PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones
							if (!(e.r.left < cam.posX)) {
								//enemigo standard
								uot.push_back({
								{e.r.right , e.r.top + 7, e.r.right + BLOCK_SIZE, e.r.bottom - 7},  // r
									 vel,                  // vel
									0.0,                   // jmpDec
									0.0,                   // jmpPow (default value, change if needed)
									0,0,
									 state::walking,        // state
									3,
									{},
									{},
									1,
									1,
									true,
									"walking",
									true
									});
								addActionToEnemy(uot[uot.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
								newAnimation(uot[uot.size() - 1].animations, 0, 48, 16, 16, "walking");
								addFrame(uot[uot.size() - 1].animations, 1, "walking");
							}
							get<1>(i) = get<2>(i);
						}
						else {
							get<1>(i)++;
						}
					}
				}
				break;
				case 3: // salto
					if (e.jmpPow == 0) {
						e.jmpPow = +get<0>(i) % 100;
						e.movementY = e.jmpPow;

						e.state = state::jumping;
						get<1>(i) = get<2>(i);
					}
					else {
						get<1>(i)++;
					}
					break;
				case 4: // girare quando c'è un blocco vuoto
					e.turning = true;
					get<1>(i)++;
					break;
				case 5://si ferma
					e.vel = 0;
					e.movementX = e.vel;

					get<1>(i) = get<2>(i);
					break;
				case 6://velocità da valore
					e.vel = -(double)(get<0>(i) % 100) / 10 + (!e.facingLeft * (double)(get<0>(i) % 100) / 10 * 2);
					e.movementX = e.vel;

					get<1>(i) = get<2>(i);
					break;
				case 7: // gira quando il player si sposta
					if (player.r.right < e.r.left && e.vel >= 0) {
						e.facingLeft = true;
						e.vel = -e.vel;
						e.movementX = e.vel;

					}
					else if (player.r.right > e.r.right && e.vel <= 0) {
						e.facingLeft = false;
						e.vel = -e.vel;
						e.movementX = e.vel;

					}

					break;
				case 8://distruggi in collision
					e.collisionDestroy = true;
					get<1>(i)++;
					break;
				}

			}
		}

	}

	//collisioni IntraNemici
	for (entity& c : entities) {
		if (e.type == c.type && c.r.top == e.r.top && c.r.bottom == e.r.bottom && c.r.left == e.r.left && c.r.right == e.r.right) {
			//se è uguale si smette, perché bisogna fare le collisioni solo con i nemici che hanno il movement fatto bene
			break;
		}

		//funziona ma non so se gasa
		if (e.r.top <= c.r.bottom - c.movementY && e.r.bottom >= c.r.top - c.movementY && e.r.left + e.movementX <= c.r.right + c.movementX && e.r.right + e.movementX >= c.r.left + c.movementX) {

			if (e.type == -1 && e.vel != 0) {
				switch (c.type) {
				case 3:
				case 0:
					e.elimina = true;
					c.elimina = true;
					score += 10;
				}
				continue;
				
			}
			else if (c.type == -1 && c.vel != 0) {
				switch (e.type) {
				case 3:
				case 0:
					e.elimina = true;
					c.elimina = true;
					score += 10;
					continue;
					break;
				}
			}

			//collsione in basso
			if (e.r.bottom <= c.r.top - c.movementY) {
				if (c.type != 4) { //si leva il 4 che non deve fare collisioni
					e.movementY = e.r.bottom - (c.r.top - c.movementY);
					e.stop = true;
					e.movementX += c.movementX;
				}
			}

			//collisione in alto
			if (c.r.bottom <= e.r.top) {
				if (e.type != 4) { //si leva il 4 che non deve fare collisioni
					c.movementY = c.r.bottom - (e.r.top - e.movementY);
					c.stop = true;
					c.movementX += e.movementX;
				}
			}

			//collisione a destra (I POWERUPS non fanno la collisione aa lato)
			if (e.type != 4 && c.type != 4) {
				if (e.r.right < c.r.left && e.r.bottom != c.r.top && c.r.bottom != e.r.top) {
					//si trova la metà del punto di incontro
					int val = ((e.r.right + e.movementX) - (c.r.left + c.movementX)) / 2;

					//si aggiunge al movementX
					e.movementX -= val;
					c.movementX += val;

					//si cambia la velocità se non sta andando in quella direzione
					if (e.vel > 0)
						e.vel *= -1;
					if (c.vel < 0)
						c.vel *= -1;
				}

				//collisione a sinistra
				if (e.r.left >= c.r.right && e.r.bottom != c.r.top && c.r.bottom != e.r.top) {
					//si trova la metà del punto di incontro
					int val = ((c.r.right + c.movementX) - (e.r.left + e.movementX)) / 2;

					//si aggiunge al movementX
					c.movementX -= val;
					e.movementX += val;

					//si cambia la velocità se non sta andando in quella direzione
					if (c.vel > 0)
						c.vel *= -1;
					if (e.vel < 0)
						e.vel *= -1;
				}
			}
			
		}
	}

	//animazione da non toccare che sono buggate a bestia
	if (e.animIndex != "" && existsAnim(e.animations, e.animIndex)) {

		if (e.state == state::walking && e.animIndex == "walking") {
			reduceFrames(e.animations, e.animIndex, abs(e.vel));
			reduceFrames(e.animations, e.animIndex, abs(increase));
		}
		else {
			reduceFrames(e.animations, e.animIndex, 1);

		}
		if (getFrames(e.animations, e.animIndex) <= 0)
			goForward(e.animations, e.animIndex);
	}
}