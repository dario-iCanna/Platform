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
	player.immunity = 90,
	player.initialImmunity = 90,
	player.facingLeft = true,
	player.shooting = false
};
bool discesa;
int movementX = 0, movementY = 0;

void addActionToEnemy(entity& e, short actionType, short firstAction, short actionTime) {
	e.actions.push_back({ actionType,firstAction, actionTime });
}

//gestione movimento del player 
void movimentoPlayer(int**& livello, int livSize, vector<entity>& en, vector<entity>& screenEn, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, bool& ripristina, int& score, audioBuffer ab)
{
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
			PlayAudio(L"./sfx/jump.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio

			
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
			PlayAudio(L"./sfx/jump.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio

		}

		break;
	}

	//azione che dipende dal powerUP
	if (S.pressed) {
		double vel = 7;


		if (!player.facingLeft && player.shooting) {
			PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones

			screenEn.push_back({
					{player.r.right+ 7, player.r.top + 7, player.r.right + BLOCK_SIZE +7, player.r.bottom - 7},  // r
					vel,                  // vel
					0.0,                   // jmpDec
					0.0,                   // jmpPow (default value, change if needed)
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
			addActionToEnemy(screenEn[screenEn.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
			newAnimation(screenEn[screenEn.size() - 1].animations, 0, 48, 16, 16, "walking");
			addFrame(screenEn[screenEn.size() - 1].animations, 1, "walking");
			cout << screenEn.size() << endl;

		}
		else if(player.shooting){
			PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones

			screenEn.push_back({
					{player.r.left - BLOCK_SIZE - 7, player.r.top + 7, player.r.left - 7, player.r.bottom - 7},  // r
					-vel,                  // vel
					0.0,                   // jmpDec
					0.0,                   // jmpPow (default value, change if needed)
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
			addActionToEnemy(screenEn[screenEn.size() - 1], 800, 0, 0);//ultimo enemigo si aggiunge Esplosione quando tocca muro ASs
			newAnimation(screenEn[screenEn.size() - 1].animations, 0, 48, 16, 16, "walking");
			addFrame(screenEn[screenEn.size() - 1].animations, 1, "walking");

			cout << screenEn.size() << endl;

		}
	}

	movementX += player.vel; //+ movimento dipendente da entità
	movementY += player.jmpPow; //+ movimento dipendente da entità

	// variabile per il riprisstino e per l'uccisione di un nemico
	bool kill = false, elimina, top = false; //top collision per vedere se si muore nel caso dell piattaforma mobile;
	if (player.r.bottom >= 24* BLOCK_SIZE) {
		ripristina = true;
		return;
	}


	
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
			if (movementY <= 0)
				switch (bottomColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE])) {
					//danno
				case 4:
					if (player.immunity == player.initialImmunity) {
						player.life--;
						PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio
						player.immunity--;
						player.shooting = false;
					}
				case true:
					if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;

						if(player.state == state::jumping)
							PlayAudio(L"./sfx/step.wav", ab, 0, 0.05); // ogni volta che si atterra si fa l'audio
						
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
					livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008); // si fa l'audio ogni volta che prendo una money
					score++;
					break;
				}

			if (movementY >= 0 && player.r.bottom != 0)
				switch (topColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE])) {
					//distruzione
				case 3:
					PlayAudio(L"./sfx/bricks.wav", ab, 0, 0.5);
					livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				case true:
					if(movementY > 0)
					movementY = ((player.r.top - movementY) / BLOCK_SIZE + 1) * BLOCK_SIZE - player.r.top;

					player.jmpPow = 0;

					top = true;

					break;
					//distruzione con aggiunta di score
				case 2:
					livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

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
				livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
				PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);
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
				livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
				PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

				score++;
				break;
			}
		}
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
			elimina = false;
			entity& e = screenEn[i];
			movimentoEntità(livello,livSize, BLOCK_SIZE, e, SCREEN_WIDTH, uot, elimina, kill, top, ripristina, score, ab);

			if (elimina) {
				screenEn.erase(screenEn.begin() + i);//funzione per eliminare in maniera ordinata
				break;
			}
		}
		//si pushano tutti i nemici che sono nell'array
		for (entity i : uot) {
			screenEn.push_back(i);
		}
	}
	if (ripristina)
		return;

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

	

	if (discesa) {
		player.state = state::jumping; // si mette lo stto di jumping
	}
}

//funzione per le animazioni del cazzo, gli do dei valori delle diverse cose e lui me le fa
void automaticMovement(int**& livello, int livSize, int& size, int BLOCK_SIZE, int SCREEN_WIDTH, int& score, audioBuffer ab) {
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
						PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si atterra si fa l'audio
						player.immunity--;
						player.shooting = false;
					}
				case true:
					if ((player.r.bottom / BLOCK_SIZE < (player.r.bottom - movementY) / BLOCK_SIZE || player.r.bottom == ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE)) {
						movementY = player.r.bottom - ((player.r.bottom - movementY) / BLOCK_SIZE) * BLOCK_SIZE;

						if (player.state == state::jumping)
							PlayAudio(L"./sfx/step.wav", ab, 0, 0.05); // ogni volta che si atterra si fa l'audio

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
					livello[player.r.left / BLOCK_SIZE + i][(player.r.bottom - movementY) / BLOCK_SIZE] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008); // si fa l'audio ogni volta che prendo una money
					score++;
					break;
				}

			if (player.r.left / BLOCK_SIZE + i < livSize && movementY >= 0 && player.r.bottom != 0)
				switch ( topColl(livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE])) {
					//distruzione
				case 3:
					PlayAudio(L"./sfx/bricks.wav", ab, 0, 0.5);
					livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
				case true:
					if (movementY > 0)
						movementY = ((player.r.top - movementY) / BLOCK_SIZE + 1) * BLOCK_SIZE - player.r.top;

					player.jmpPow = 0;

					break;
					//distruzione con aggiunta di score
				case 2:
					livello[player.r.left / BLOCK_SIZE + i][(player.r.top - movementY) / BLOCK_SIZE] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

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
					livello[(player.r.left + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);
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
					livello[(player.r.right + movementX) / BLOCK_SIZE][(player.r.top - movementY) / BLOCK_SIZE + i] = 0;
					PlayAudio(L"./sfx/coin.wav", ab, 0, 0.008);

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

void ripristino(vector<entity>& screenEn, int & size, int**& livello, int**& initialLiv, int SCREEN_HEIGHT, int BLOCK_SIZE, int livSize, RECT pos) {
	ripristinoPlayer(pos);

	size = 0;
	cam.posX = 0;
	screenEn.resize(0);
	for (int i = 0; i < livSize; i++) {
		for (int j = 0; j < SCREEN_HEIGHT / BLOCK_SIZE; j++) {
			livello[i][j] = initialLiv[i][j];
		}
	}
	
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
void movimentoEntità(int** livello,int livSize, int BLOCK_SIZE,entity& e,int SCREEN_WIDTH, vector<entity>& uot, bool& elimina, bool& kill, bool top, bool& ripristina, int& score, audioBuffer ab) {
	bool stop = false;
	bool collisionDestroy = false;

	//si controlla se il nemico è fuori dallo schermo
	if (e.r.right < cam.posX || e.r.left <= 0) {
		elimina = true;
		return;
	}

	if (e.r.bottom >= 24 * BLOCK_SIZE) {
		elimina = true;
		return;
	}

	bool turning = false;

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
			if(get<1>(i) > 0) // si scorre l'azione diminuendo il tempo per cui accada
				get<1>(i)--;
			
			if (get<1>(i) == 0) {
				//azione per get 0
				switch (get<0>(i)/100) {
				case 0:  //girarsi
					e.vel = -e.vel;
					get<1>(i) = get<2>(i);
					break;
				case 1: //cambiare la direzione verticale
					e.jmpPow = -e.jmpPow;
					get<1>(i) = get<2>(i);
					break;
				case 2: // cannones METODO ROTTISSIMO
				{
					
					double vel = get<0>(i) % 100;


					if (player.r.right < e.r.left) {
						PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones


						if (!(e.r.left < cam.posX)) {
							uot.push_back({
									{e.r.left - BLOCK_SIZE, e.r.top + 7, e.r.left, e.r.bottom - 7},  // r
									-vel,                  // vel
									0.0,                   // jmpDec
									0.0,                   // jmpPow (default value, change if needed)
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
						PlayAudio(L"./sfx/shoot.wav", ab, 0, 0.5); // sparo Cannones

						if (!(e.r.left < cam.posX)) {
							uot.push_back({
								{e.r.right , e.r.top + 7, e.r.right + BLOCK_SIZE, e.r.bottom - 7},  // r
									 vel,                  // vel
									0.0,                   // jmpDec
									0.0,                   // jmpPow (default value, change if needed)
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
				

					break;
				case 3: // salto
					if (e.jmpPow == 0) {
						e.jmpPow = + get<0>(i)%100;
						e.state = state::jumping;
						get<1>(i) = get<2>(i);
					}
					else {
						get<1>(i)++;
					}
					break;
				case 4: // girare quando c'è un blocco vuoto
					turning = true;
					get<1>(i)++;
					break;
				case 5://si ferma
					e.vel = 0;
					get<1>(i) = get<2>(i);
					break;
				case 6://velocità da valore
					e.vel = -(get<0>(i) % 100) + (!e.facingLeft * (get<0>(i) % 100) *2);
					get<1>(i) = get<2>(i);
					break;
				case 7: // gira quando il player si sposta
					if (player.r.right < e.r.left && e.vel > 0) {
						e.facingLeft = true;
						e.vel = -e.vel;
					}
					else if (player.r.right > e.r.right && e.vel < 0) {
						e.facingLeft = false;
						e.vel = -e.vel;
					}

					break;
				case 8://distruggi in collision
					collisionDestroy = true;
					get<1>(i)++;
					break;
				}
					
			}
		}
		
	}

	//collisioni a destra e sinistra si cambia direzione se non è un proiettile
	if (e.r.right < cam.posX + SCREEN_WIDTH) {
		switch (e.type) {
		case 2:
			break;
		case 0:
			//si controlla se il player uccide il nemico
			if (player.state == state::jumping && !kill && player.r.bottom - movementY >= e.r.top && player.r.bottom <= e.r.top && player.r.right > e.r.left && player.r.left < e.r.right && movementY < 0) {
				player.jmpPow = player.initialJmp;
				player.state = state::jumping;
				player.highJump = true;
				movementY = player.r.bottom - e.r.top;
				discesa = false;//mette la discesa quindi mette il jumping
				kill = true; // variabile che serve per non killare il nemico se subisci danni
				elimina = true;// serve per rimuovere il nemico in modo ordinato

				score += 10;
				return;
			}
		default:
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
				else if (count > 0 && turning) {
					e.vel = -e.vel;
				}
			}
			else if (e.type != 1) {
				for (int i = 0; i < limit; i++) {
					if (e.r.left / BLOCK_SIZE + i < livSize && bottomColl(livello[e.r.left / BLOCK_SIZE + i][(int)(e.r.bottom - e.jmpPow) / BLOCK_SIZE]) == true && e.r.bottom <= ((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE) {
						e.jmpPow = e.r.bottom - floor((e.r.bottom - e.jmpPow) / BLOCK_SIZE) * BLOCK_SIZE;
						stop = true;
						break;

					}
				}
			}
			//controllo collisioni si elimina se ha collision death
			limit = e.eBlockHeight;
			if (e.r.top / BLOCK_SIZE + e.eBlockHeight == e.r.bottom / BLOCK_SIZE && (int)(e.r.bottom - e.jmpPow) % BLOCK_SIZE != 0) {
				limit++;
			}
			if (e.r.right + e.vel > cam.posX && e.r.left + e.vel < cam.posX + SCREEN_WIDTH && e.r.right + movementX < cam.posX + SCREEN_WIDTH - BLOCK_SIZE) {

				for (int i = 0; i < limit; i++) {
					if (e.vel < 0 && sideColl(livello[(e.r.left + (int)e.vel) / BLOCK_SIZE][(int)(e.r.top - e.jmpPow) / BLOCK_SIZE + i]) == true) {
						e.vel = -e.vel;
						if (collisionDestroy) {
							elimina = true;
						}
						break;
					}
					if (e.vel > 0 && sideColl(livello[(e.r.right + (int)e.vel) / BLOCK_SIZE][(int)(e.r.top - e.jmpPow) / BLOCK_SIZE + i]) == true) {
						e.vel = -e.vel;
						if (collisionDestroy) {
							elimina = true;
						}
						break;
					}
				}
			}
			break;
		}
	}

	if (kill)
		kill = false;

	//si controlla se il player fa collisione
	if (e.type != 2 && player.r.right + movementX >= e.r.left && player.r.left + movementX <= e.r.right && player.r.top - movementY <= e.r.bottom && player.r.bottom - movementY >= e.r.top) {
		switch (e.type) {
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
				movementY += e.jmpPow;
				movementX += e.vel;
				discesa = false;
				if (top) {
					ripristina = true;
					return;
				}
			}
			break;
		case 4: // powerup Spiaccicato in 1 type solo, si usa il get<2> per l'effetto


			for (auto i : e.actions) {
				if (get<0>(i)) {
					switch (get<2>(i)) {
					case 0:
						if (player.life < 3)
							player.life++;
						elimina = true;
						break;
					case 1:
						//si aumenta la velocità massima player
						player.velMax = 10;
						if (player.powerUpTime[get<2>(i)] < get<1>(i))
							player.powerUpTime[get<2>(i)] = get<1>(i);
						//rimuovere l'entità dal gruppo di entità
						elimina = true;

						break;
					case 2:
						//si da la possibilità di shootingare
						player.shooting = true;
						player.powerUpTime[get<2>(i)] = get<1>(i);
						elimina = true;

						break;
					}
				}
				
			}
			

			

			
			break;
		default:
			if (player.immunity == player.initialImmunity && (player.r.bottom - movementY > e.r.top && player.r.top - movementY < e.r.bottom)) {
				player.life--;
				player.shooting = false;
				PlayAudio(L"./sfx/hit.wav", ab, 0, 0.5); // ogni volta che si viene colpiti si fa l'audio
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

	if (e.vel > 0 && e.facingLeft) {
		e.facingLeft = false;
	}
	else if (e.vel < 0 && !e.facingLeft) {
		e.facingLeft = true;
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


	//animazione da non toccare che sono buggate a bestia
	if (e.animIndex != "" && existsAnim(e.animations, e.animIndex)) {
		if (e.state == state::walking) {
			reduceFrames(e.animations, e.animIndex, abs(e.vel));
		}
		else {
			reduceFrames(e.animations, e.animIndex, 1);

		}
		if (getFrames(e.animations, e.animIndex) <= 0)
			goForward(e.animations, e.animIndex);
	}
}