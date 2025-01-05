#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include "pulsante.h"
#include "player.h"
#include "camera.h"
#include "animation.h"

using namespace std;
using namespace D2D1;

#pragma region variabili globali
#define BLOCK_SIZE 32 // grandezza blocco
int SCREEN_HEIGHT = 775; //altezza schermo
int SCREEN_WIDTH = 1440; //larghezza schermo
int SCREEN_WIDTH_BLOCK = SCREEN_WIDTH / BLOCK_SIZE, SCREEN_HEIGHT_BLOCK = SCREEN_HEIGHT / BLOCK_SIZE; // per vedere la larghezza e altezza in blocks
int tempo = 0, score = 0; //variabili del tempo e dei punti, scritti in alto a sinistra
int prova = 0;
short BLOCK_CODE = 100; // la grandezza in decimali del codice usato per i blocchi, ora 100 per poter usare 100 stili diversi per blocco
// animazione idle del player

animazione* playerAnim;// animazione del player per ogni singola azione

short res = 0;//per resettare l'array dell'animazione quando cambia lo stato

HRESULT p = CoInitialize(NULL);//funzione per tirare in ballo funzioni COM
//factory direct 2d
ID2D1Factory* pD2DFactory = NULL;
HRESULT hr = D2D1CreateFactory(
	D2D1_FACTORY_TYPE_SINGLE_THREADED,
	&pD2DFactory
);
//render per direct 2d
ID2D1HwndRenderTarget* pRT = NULL;
//variabili WIC
IWICImagingFactory* wicFactory = NULL;
IWICBitmapDecoder* wicDecoder = NULL;
IWICBitmapFrameDecode* wicFrame = NULL;
IWICFormatConverter* wicConverter = NULL;
//array di beush? USATI ORA SOLO PER NEMICI
ID2D1SolidColorBrush* terrainBrushes[4];
//bitmap per immagini
ID2D1Bitmap* playerBitmap = NULL;
ID2D1Bitmap* terrainBitmap = NULL;
ID2D1Bitmap* numberBitmap = NULL;
ID2D1Bitmap* cuoriBitmap = NULL;
ID2D1Bitmap* skyBitmap = NULL;
ID2D1Bitmap* enemyBitmap = NULL;


int ***livello, ***initialLiv, numeroLivello = 0, quantitaLivelli = 0;//livelli, livelli salvati per la rigenerazione e il numero del livello da disegnare
int heightSize, *livSize;//altezza livello e lunghezza livello

vector<vector<entity>> enemy,initialArr; // array per i nemici
#pragma endregion

//variabili di funzionamento
struct WINDSTUFF {
	bool running = true;
	bool console = true;
	const double MAX_FPS = 60;
}wS; // variabili per il gameloop

 #pragma region gestione_eventi
LRESULT Wndproc(HWND hwnd,UINT uInt,WPARAM wParam,LPARAM lParam)
{

	switch (uInt) {
	case WM_CREATE:
		// Creazione del Render Target
	{
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);

		HRESULT hr = pD2DFactory->CreateHwndRenderTarget(
			RenderTargetProperties(),
			HwndRenderTargetProperties(
				hwnd,
				SizeU(
					clientRect.right - clientRect.left,
					clientRect.bottom - clientRect.top)
			),
			&pRT
		);
		SCREEN_WIDTH = clientRect.right - clientRect.left;
		SCREEN_HEIGHT = clientRect.bottom - clientRect.top;

		if (FAILED(hr)) {
			// Gestione errore
			return -1;
		}

		// Creazione dei pennelli 
		pRT->CreateSolidColorBrush(ColorF(ColorF::LightBlue), &terrainBrushes[0]);
		pRT->CreateSolidColorBrush(ColorF(ColorF::Brown), &terrainBrushes[1]);
		pRT->CreateSolidColorBrush(ColorF(ColorF::Green), &terrainBrushes[2]);
		pRT->CreateSolidColorBrush(ColorF(ColorF::Red), &terrainBrushes[3]);
	}
	break;
	case WM_PAINT:
	{
		
		//si prende l'area del disegno
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);

		pRT->BeginDraw();//inizia il disegno

		//disegna sfondo PROVVISORIO
		pRT->DrawBitmap(skyBitmap,RectF(
			clientRect.left,
			clientRect.top,
			clientRect.right,
			clientRect.bottom),1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 1920, 885));

		// disegno livello FINITO
		for (int i = floor(cam.posX/BLOCK_SIZE); i <= floor(cam.posX / BLOCK_SIZE) + SCREEN_WIDTH_BLOCK + 1 && i < livSize[numeroLivello]; i++) {
			for (int j = 0; j < SCREEN_HEIGHT_BLOCK; j++) {
				int val = livello[numeroLivello][i][j];//prende il valore totale
				int cod = (int)floor(val / BLOCK_CODE);//prende il gruppo del blocco
				int off = val % BLOCK_CODE; // da un offset * 16

				pRT->DrawBitmap(terrainBitmap,
					RectF(
						i * BLOCK_SIZE - cam.posX,
						j * BLOCK_SIZE,
						(i + 1) * BLOCK_SIZE - cam.posX,
						(j + 1) * BLOCK_SIZE
					),
					1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * off, 16*(cod), 16 * (off + 1), 16*(cod+1)) // disegna a seconda dei valori dati il blocco giusto
				);
			}
		}
		prova = 0;

		//disegno enemy
		for (int i = 0; i < enemy[numeroLivello].size(); i++) {
			entity e = enemy[numeroLivello][i];
			//hotbox
				pRT->DrawRectangle(
					RectF(
						e.r.left - cam.posX,
						e.r.top,
						e.r.right - cam.posX,
						e.r.bottom),
					terrainBrushes[3]);
				// da sistemare
					switch (e.type) {
					case 2:
						break;
					case 4://disegno il cuore nel powerup
						pRT->DrawBitmap(cuoriBitmap, RectF(
							e.r.left - cam.posX,
							e.r.top,
							e.r.right - cam.posX,
							e.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(30, 0, 60, 30));
						break;
					default:
						pRT->DrawBitmap(enemyBitmap, RectF(
							e.r.left - cam.posX,
							e.r.top,
							e.r.right - cam.posX,
							e.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 30, 30));
						break;
					}
				
		}

		//disegno tempo
		int cifre = (int)(floor(log10(tempo) + 1));
		if (tempo == 0) {
			pRT->DrawBitmap(numberBitmap, RectF(
				0,
				0,
				10,
				10), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 10, 10));
		}
		else {
			for (int i = 0; i < cifre; i++) {
				int f = floor((tempo % (int)(pow(10, cifre- i))) / (pow(10, cifre - 1 - i)));
				pRT->DrawBitmap(numberBitmap, RectF(
					10 * i,
					0,
					10 * (i + 1),
					10), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(10 * f, 0, 10*(f+1), 10));
			}
		}
		
		//disegno punteggio
		pRT->DrawBitmap(terrainBitmap, RectF(
			0,
			20,
			20,
			40), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(90, 0, 120, 30));
		cifre = (int)(floor(log10(score) + 1));
		if (score == 0) {
			pRT->DrawBitmap(numberBitmap, RectF(
				20,
				26,
				30,
				36), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 10, 10));
		}
		else {
			for (int i = 0; i < cifre; i++) {
				int f = floor((score % (int)(pow(10, cifre - i))) / (pow(10, cifre - 1 - i)));
				pRT->DrawBitmap(numberBitmap, RectF(
					10 * (i+2),
					26,
					10 * (i + 3),
					36), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(10 * f, 0, 10 * (f + 1), 10));
			}
		}

		//disgno cuori
		for (int i = 1; i <= player.maxLife; i++) {
			if (player.life >= i) {
				pRT->DrawBitmap(cuoriBitmap, RectF(
					30*(i-1),
					40,
					30*i,
					70), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(30, 0, 60, 30));
			}
			else {
				pRT->DrawBitmap(cuoriBitmap, RectF(
					30 * (i - 1),
					40,
					30*i,
					70), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 30, 30));
			}
			
		}
		
		//disegno velocità player
		if (player.vel != 0)
			cifre = (int)(floor(log10(abs(player.vel * 100)) + 1));
		else
			cifre = 0;
		for (int i = 0; i < cifre; i++) {
			int f = floor(((int)(abs(player.vel * 100)) % (int)(pow(10, cifre - i))) / (pow(10, cifre - 1 - i)));
			pRT->DrawBitmap(numberBitmap, RectF(
				player.r.left - cam.posX - 2 + 10 * i,
				player.r.top + 50,
				player.r.left - cam.posX - 2 + 10 * (i + 1),
				player.r.top + 40), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(10 * f, 0, 10 * (f + 1), 10));
		}


		//disegno player + hitbox
		if(playerBitmap && player.immunity % 2 == 0)
		pRT->DrawBitmap(playerBitmap, RectF(
			player.r.left - cam.posX - 4,
			player.r.top,
			player.r.right-cam.posX + 4,
			player.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,RectF(0 + playerAnim[player.state].width * playerAnim[player.state].ind, 16 * player.state + 48 * player.facingLeft, 16 + playerAnim[player.state].width * playerAnim[player.state].ind, 16 + 16  * player.state + 48 * player.facingLeft));
		pRT->DrawRectangle(
			RectF(
				player.r.left - cam.posX,
				player.r.top,
				player.r.right - cam.posX,
				player.r.bottom),
			terrainBrushes[3]);
		

		pRT->EndDraw();

		break;
	};
	case WM_DESTROY:
	case WM_CLOSE:
		pD2DFactory->Release();		
		wS.running = false;
		break;
		//gestione tasti tastiera
	case WM_KEYDOWN:
		switch (wParam) {
		case 'W':
			W.toggle();
			break;
		case 'A':
			A.toggle();
			break;
		case 'S':
			S.toggle();
			break;
		case 'D':
			D.toggle();
			break;
		case 'J':
			J.toggle();
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam) {
		case 'W':
			W.release();
			break;
		case 'A':
			A.release();
			break;
		case 'S':
			S.release();
			break;
		case 'D':
			D.release();
			break;
		case 'J':
			J.release();
			break;
		}
		break;
	}

	return DefWindowProc(hwnd,uInt,wParam,lParam);
}
#pragma endregion 

void createBitmap(const wchar_t* file, ID2D1Bitmap** bitmap) {
	wicFactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, bitmap);// crea la bitmap direct2d
}

void addEnemy(int levelNum, int x, int y,int width, int height, double vel, double jmpDec, double jmpPow,int baseState, short type, short framePerAction) {
	enemy[levelNum].push_back({
		{x, y, x+width, y+height},  // r
		vel,                  // vel
		jmpDec,                   // jmpDec
		jmpPow,                    //jmpPow
		 baseState,        // state
		type,					//type
		framePerAction,					//frame per azione
		framePerAction,					//variabile per la inizializzazione dei frame
		(width % BLOCK_SIZE == 0) ? width / BLOCK_SIZE : width / BLOCK_SIZE + 1,
		(height % BLOCK_SIZE == 0) ? height / BLOCK_SIZE : height / BLOCK_SIZE + 1
		});
}

//funzione main
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)  {

	playerAnim = new animazione[3];

	//animazione idle
	newAnimation(0, 0, 16, 16, 3, playerAnim[state::idle]);
	frameSprite(0, 20, playerAnim[state::idle]);
	frameSprite(1, 10, playerAnim[state::idle]);
	frameSprite(2, 20, playerAnim[state::idle]);

	//animazione jumping
	newAnimation(0, 0, 16, 16, 3, playerAnim[state::jumping]);
	frameSprite(0, 1, playerAnim[state::jumping]);
	frameSprite(1, 1, playerAnim[state::jumping]);

	//animazione walking
	newAnimation(0, 0, 16, 16, 3, playerAnim[state::walking]);
	frameSprite(0, 20, playerAnim[state::walking]);
	frameSprite(1, 20, playerAnim[state::walking]);
	frameSprite(2, 20, playerAnim[state::walking]);

	//creo la console
	if (wS.console) {
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		ios_base::sync_with_stdio;
	}
	//mi prendo il file da dove leggere i livelli
	ifstream file;
	ofstream ofile;
	ofile.open("Output.txt");
	file.open("Livelli.txt");

	//roba per la bitmap di directD2
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&wicFactory); // factory WIC
	file >> quantitaLivelli; // si fa cin con il file del numero dei livelli
	//creo array della lunghezza dei livelli
	livSize = new int[quantitaLivelli];
	for (int i = 0; i < quantitaLivelli; i++) {
		file >> livSize[i];
	}

	//creo la grandezza giusta dell'array dei nemici
	enemy.resize(quantitaLivelli);
	initialArr.resize(quantitaLivelli);

	// Allocazione dinamica della matrice si creano 2 livelli
	livello = new int** [quantitaLivelli];
	initialLiv = new int** [quantitaLivelli];
	for (int i = 0; i < quantitaLivelli; i++) {
		livello[i] = new int* [livSize[i]];//inizializzazione livello
		initialLiv[i] = new int* [livSize[i]];//inizializzazione inizio livello
		for (int j = 0; j < livSize[i]; j++) {
			livello[i][j] = new int[SCREEN_HEIGHT_BLOCK];
			initialLiv[i][j] = new int[SCREEN_HEIGHT_BLOCK];
		}
	}

	addEnemy(0, 260, 180,64,32, 1, 0, 0, state::walking, 1, 0);
	addEnemy(0, 200, 300, 32, 32, 0, 0, 0, state::walking, 4, 0);
	addEnemy(0, 230, 300, 32, 32, 0, 0, 0, state::walking, 5, 600);
	addEnemy(0, 240, 300, 32, 32, -1, 1, 0, state::walking, 0, 0);
	addEnemy(0, 9 * BLOCK_SIZE, 13 * BLOCK_SIZE, 32, 32, 0, 0, 0, state::walking, 2, 120);
	addEnemy(0, 1600, 300, 32, 32, -1, 1, 0, state::walking, 0, 0);

	for (int f = 0; f < quantitaLivelli; f++) {
		initialArr[f] = enemy[f];
	}

	
	// mettiamo nel livello i numeri dei blocchi
	for (int f = 0; f < 2; f++) {
		for (int j = 0; j < SCREEN_HEIGHT_BLOCK; j++) {
			for (int i = 0; i < livSize[f]; i++) {
				file >> livello[f][i][j];
			}
		}
	}
	
	

	//trascrivo su intial liv
	for (int f = 0; f < quantitaLivelli; f++) {
		for (int j = 0; j < SCREEN_HEIGHT_BLOCK; j++) {
			for (int i = 0; i < livSize[f]; i++) {
				initialLiv[f][i][j] = livello[f][i][j];
				//ofile << livello[f][i][j] +14 << "\t";
			}
			//ofile << endl;
		}
		//ofile << endl;
	}

	//calcolo tempo per un frame
	double fps = 1000000 / wS.MAX_FPS;

	//calcolo misure player

	int PLAYER_WIDTH = player.r.right - player.r.left;
	int PLAYER_HEIGHT = player.r.bottom - player.r.top;
	int pWidthBlock = (PLAYER_WIDTH % BLOCK_SIZE == 0) ? PLAYER_WIDTH / BLOCK_SIZE : PLAYER_WIDTH / BLOCK_SIZE +1;
	int pHeightBlock = (PLAYER_HEIGHT % BLOCK_SIZE == 0) ? PLAYER_HEIGHT / BLOCK_SIZE : PLAYER_HEIGHT / BLOCK_SIZE+ 1;

	//registro la finestra

	WNDCLASS wcl = {};
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpfnWndProc = Wndproc;
	wcl.lpszClassName = L"game";
	wcl.hInstance = hInstance;

	RegisterClass(&wcl);

	HWND hW = CreateWindowExW(0, wcl.lpszClassName, L"Platform", WS_OVERLAPPEDWINDOW, 40, 40, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, hInstance, NULL);
	//SetWindowPos(hW,NULL, 0, 0, SCREEN_WIDTH * 1.5, SCREEN_HEIGHT * 1.5,SWP_NOMOVE);
	ShowWindow(hW, SW_NORMAL);
	// creazione bitmaps
	createBitmap(L"sprites/player.png", &playerBitmap);
	createBitmap(L"sprites/terreno.png", &terrainBitmap);
	createBitmap(L"sprites/sky.png", &skyBitmap);
	createBitmap(L"sprites/numeri.png", &numberBitmap);
	createBitmap(L"sprites/cuori.png", &cuoriBitmap);
	createBitmap(L"sprites/enemy.png", &enemyBitmap);

	//rilascia risorse inutili
	wicFactory->Release();
	wicDecoder->Release();
	wicConverter->Release();
	wicFrame->Release();
	LARGE_INTEGER start, end;
	long long deltaTime;
	QueryPerformanceCounter(&start); // prendo il tempo iniziale
	MSG msg;
	int ent = 0;
	// Game loop 
	while (wS.running){
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		QueryPerformanceCounter(&end); // prendo il tempo finale e calcolo il delta time
		deltaTime =  end.QuadPart - start.QuadPart;
		//controllo se il tempo è maggiore del frame per secondo
		if (deltaTime >= fps) {
			if (player.immunity < player.initialImmunity && player.immunity != 0)
				player.immunity--;
			else if (player.immunity == 0)
				player.immunity = player.initialImmunity;

			//serve per eliminare gli effetti dei powerUP
			vector<int> erase;

			//scorro i powerup Attivi
			for (auto& i : player.powerUpTime) {
				// si diminuice il tempo del power up, se è -1 è infinito
				if (i.second > 0)
					i.second--;
				else if (i.second == 0) {
					// si guarda di che tipo è il power up e si reversa la sua azione
					switch (i.first) {
					case 5:
						player.velMax = 5;
						break;
					}
					erase.push_back(i.first);
				}	
			}

			//elimino l'effetto dei powerup Finiti
			for (int i : erase) {
				player.powerUpTime.erase(i);
			}
			

			movimentoPlayer(livello[numeroLivello], initialLiv[numeroLivello], BLOCK_SIZE, enemy[numeroLivello], enemy[numeroLivello].size(), SCREEN_WIDTH, livSize[numeroLivello], initialArr[numeroLivello], score, SCREEN_HEIGHT, tempo, pWidthBlock, pHeightBlock);
			toggleEv();
			if (player.r.left >= (livSize[numeroLivello] - 2) * BLOCK_SIZE) {
				numeroLivello++;
				ripristino(enemy[numeroLivello], enemy[numeroLivello].size(), initialArr[numeroLivello], livello[numeroLivello], initialLiv[numeroLivello], SCREEN_HEIGHT, BLOCK_SIZE, livSize[numeroLivello]);
				score = 0;
				tempo = 0;
			}
			//RedrawWindow(hW, NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW | RDW_INVALIDATE);
			InvalidateRect(hW, NULL, TRUE);
			UpdateWindow(hW);
			QueryPerformanceCounter(&start); // riinizializzo il tempo iniziale
			ent++;

			// variabile del reset


			//animazioni player
			{
				if (player.state != res) {
					playerAnim[res].fpF[playerAnim[res].ind] = playerAnim[res].inifpF[playerAnim[res].ind];
					playerAnim[res].ind = 0;
					res = player.state;
				}

				switch (player.state)
				{
				case state::walking:
					playerAnim[player.state].fpF[playerAnim[player.state].ind] -= abs(player.vel / 2);
					break;
				case state::jumping:
					if (playerAnim[player.state].ind == 0 && player.jmpPow <= 0)
						playerAnim[player.state].fpF[playerAnim[player.state].ind]--;
					break;
				default:
					playerAnim[player.state].fpF[playerAnim[player.state].ind]--;
					break;
				}

				//controllo per cambiare frame
				if (playerAnim[player.state].fpF[playerAnim[player.state].ind] <= 0) {
					playerAnim[player.state].fpF[playerAnim[player.state].ind] = playerAnim[player.state].inifpF[playerAnim[player.state].ind];
					if (playerAnim[player.state].ind != playerAnim[player.state].size - 1) {
						playerAnim[player.state].ind++;
					}
					else {
						playerAnim[player.state].ind = 0;
					}
				}
			}
		}

		// per aumentare il contatore del tempo
		if (ent >= wS.MAX_FPS) {
			tempo++;
			ent = 0;
		}
		Sleep(10);// sleeppo per non usare tutta la cpu
	}
	return 0;
}

