#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <iostream>
#include <fstream>
#include <string> 
#include <iostream>
#include <unordered_map>
#include <vector>
#include "audio.h"
#include "pulsante.h"
#include "player.h"
#include "animazione.h"
#include "camera.h"

using namespace std;
using namespace D2D1;

#pragma region variabili globali
#define BLOCK_SIZE 32 // grandezza blocco
int SCREEN_HEIGHT = 775; //altezza schermo
int SCREEN_WIDTH = 1440; //larghezza schermo
int SCREEN_WIDTH_BLOCK = SCREEN_WIDTH / BLOCK_SIZE, SCREEN_HEIGHT_BLOCK = SCREEN_HEIGHT / BLOCK_SIZE; // per vedere la larghezza e altezza in blocks
int tempo = 0, score = 0; //variabili del tempo e dei punti, scritti in alto a sinistra
int prova = 0;
bool ripristina = false;
bool changeLiv = false; // per aumentare il livello con il ripristina

audio music;//musica per il livello

//variabile per dire se il gioco è over oppure no (MENU')
bool gameOver = true;

//tempo per dire al gaem di non runnare usato per animazioni n shit
int notRunning = 0;

//variabile per il wait-time
int waitTime = 0;

short BLOCK_CODE = 100; // la grandezza in decimali del codice usato per i blocchi, ora 100 per poter usare 100 stili diversi per blocco
// animazione idle del player

short res = 0;//per resettare l'array dell'animazione quando cambia lo stato

HRESULT p = CoInitializeEx(nullptr, COINIT_MULTITHREADED);//funzione per tirare in ballo funzioni COM
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
ID2D1Bitmap* fontBitmap = NULL;
ID2D1Bitmap* cuoriBitmap = NULL;
ID2D1Bitmap* skyBitmap = NULL;
ID2D1Bitmap* enemyBitmap = NULL;
ID2D1Bitmap* buttonsBitmap = NULL;


int ***livello, *** initialLiv, numeroLivello = 0, quantitaLivelli = 0;//livelli, livelli salvati per la rigenerazione e il numero del livello da disegnare
RECT* playerStartPos;
int heightSize, *livSize;//altezza livello e lunghezza livello

//posizione nel menù
int pos;
animazione playerAnim; // animazione player
string animIndex; // indice per l'animazione del player
vector<vector<entity>> entities; // array per i nemici
int limit = 0;
vector<entity> screenEn; // array tmporaneo
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
		pRT->CreateSolidColorBrush(ColorF(ColorF::Black), &terrainBrushes[0]);
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

		// si decide cosa disegnare se il gioco si muove oppure se c'è da aspettare
		if (waitTime == 0) {
			if (gameOver) {
				/*ID2D1SolidColorBrush* white;
				pRT->CreateSolidColorBrush(ColorF(ColorF::White), &white);
				pRT->FillRectangle(RectF(clientRect.left, clientRect.top,clientRect.right, clientRect.bottom), white);*/
				pRT->DrawBitmap(skyBitmap, RectF(
					clientRect.left,
					clientRect.top,
					clientRect.right,
					clientRect.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 1920, 885));

				if (numeroLivello == quantitaLivelli) {
					string title = "HAI FINITO NIG";
					for (int i = 0; i < title.size(); i++) {
						pRT->DrawBitmap(fontBitmap, RectF(
							128 * (i),
							128,
							128 * (i + 1),
							256), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (title[i] - 48), 0, 16 * (title[i] - 48 + 1), 16));
					}
				}
				else {
					string title = "AN ASS GAME";
					for (int i = 0; i < title.size(); i++) {
						pRT->DrawBitmap(fontBitmap, RectF(
							128 * (i),
							128,
							128 * (i + 1),
							256), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (title[i] - 48), 0, 16 * (title[i] - 48 + 1), 16));
					}

					int x = 620;
					int y = 400;

					for (int i = 0; i < 1; i++) {
						if (pos != i) {
							for (int j = 0; j <= 5; j++) {
								if (j == 0) {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 16, 16, 32));
								}
								else if (j == 5) {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(32, 16, 48, 32));
								}
								else {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16, 16, 32, 32));
								}
							}
							string title = "GIOCA";
							for (int j = 0; j < title.size(); j++) {
								pRT->DrawBitmap(fontBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (title[j] - 48), 0, 16 * (title[j] - 48 + 1), 16));
							}
						}
						else {
							for (int j = 0; j <= 5; j++) {
								if (j == 0) {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 16, 16));
								}
								else if (j == 5) {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(32, 0, 48, 16));
								}
								else {
									pRT->DrawBitmap(buttonsBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16, 0, 32, 16));
								}
							}

							string title = "GIOCA";
							for (int j = 0; j < title.size(); j++) {
								pRT->DrawBitmap(fontBitmap, RectF(x + 32 * j, y + 32 * i, x + 32 * (j + 1), y + 32 * (i + 1)), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (title[j] - 48), 0, 16 * (title[j] - 48 + 1), 16));
							}
						}
					}
				}
			}
			else {
				//disegna sfondo PROVVISORIO
				pRT->DrawBitmap(skyBitmap, RectF(
					clientRect.left,
					clientRect.top,
					clientRect.right,
					clientRect.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 1920, 885));

				// disegno livello FINITO
				for (int i = floor(cam.posX / BLOCK_SIZE); i <= floor(cam.posX / BLOCK_SIZE) + SCREEN_WIDTH_BLOCK + 1 && i < livSize[numeroLivello]; i++) {
					for (int j = 0; j < SCREEN_HEIGHT_BLOCK; j++) {
						int val = livello[numeroLivello][i][j];//prende il valore totale
						int cod = (int)floor(val / BLOCK_CODE);//prende il gruppo del blocco
						int off = val % BLOCK_CODE; // da un offset * 16

						pRT->DrawBitmap(terrainBitmap, RectF(i * BLOCK_SIZE - cam.posX, j * BLOCK_SIZE, (i + 1) * BLOCK_SIZE - cam.posX, (j + 1) * BLOCK_SIZE),
							1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * off, 16 * (cod), 16 * (off + 1), 16 * (cod + 1)) // disegna a seconda dei valori dati il blocco giusto
						);
					}
				}
				prova = 0;

				//disegno enttity
				for (entity e : screenEn) {
					/*hitbox
					pRT->DrawRectangle(
						RectF(
							e.r.left - cam.posX,
							e.r.top,
							e.r.right - cam.posX,
							e.r.bottom),
						terrainBrushes[0]);*/
					if (e.eBlockWidth > 1)
						for (int i = 0; i < e.eBlockWidth; i++) {
							pRT->DrawBitmap(enemyBitmap, RectF(
								e.r.left - cam.posX + 32 * i,
								e.r.top,
								e.r.left - cam.posX + 32 * (i + 1),
								e.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(getAnimX(e.animations, e.animIndex) + getAnimWidthByFrame(e.animations, e.animIndex) + e.differentSideAnimation * e.facingLeft * (getAnimSize(e.animations, e.animIndex) * getAnimWidth(e.animations, e.animIndex)), getAnimY(e.animations, e.animIndex), getAnimX(e.animations, e.animIndex) + getAnimWidth(e.animations, e.animIndex) + getAnimWidthByFrame(e.animations, e.animIndex) + e.differentSideAnimation * e.facingLeft * (getAnimSize(e.animations, e.animIndex) * getAnimWidth(e.animations, e.animIndex)), getAnimY(e.animations, e.animIndex) + getAnimHeight(e.animations, e.animIndex)));
						}
					else {
						if (existsAnim(e.animations, e.animIndex)) {
							int height = e.r.bottom - e.r.top;
							int width = e.r.right - e.r.left;
							pRT->DrawBitmap(enemyBitmap, RectF(
								e.r.left - cam.posX - (BLOCK_SIZE - width) / 2,
								e.r.top - (BLOCK_SIZE - height) / 2,
								e.r.right - cam.posX + (BLOCK_SIZE - width) / 2,
								e.r.bottom + (BLOCK_SIZE - height) / 2), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(getAnimX(e.animations, e.animIndex) + getAnimWidthByFrame(e.animations, e.animIndex) + e.differentSideAnimation * e.facingLeft * (getAnimSize(e.animations, e.animIndex) * getAnimWidth(e.animations, e.animIndex)), getAnimY(e.animations, e.animIndex), getAnimX(e.animations, e.animIndex) + getAnimWidth(e.animations, e.animIndex) + getAnimWidthByFrame(e.animations, e.animIndex) + e.differentSideAnimation * e.facingLeft * (getAnimSize(e.animations, e.animIndex) * getAnimWidth(e.animations, e.animIndex)), getAnimY(e.animations, e.animIndex) + getAnimHeight(e.animations, e.animIndex)));

						}
					}


				}

				//disegno tempo (fatto il to string/ da stampare)
				string timeS = "TIME:" + to_string(tempo);

				for (int i = 0; i < timeS.size(); i++) {
					pRT->DrawBitmap(fontBitmap, RectF(
						16 * (i),
						16,
						16 * (i + 1),
						32), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (timeS[i] - 48), 0, 16 * (timeS[i] - 48 + 1), 16));
				}

				//disegno punteggio
				string scoreS = "SCORE:" + to_string(score);

				for (int i = 0; i < scoreS.size(); i++) {
					pRT->DrawBitmap(fontBitmap, RectF(
						16 * (i),
						0,
						16 * (i + 1),
						16), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (scoreS[i] - 48), 0, 16 * (scoreS[i] - 48 + 1), 16));
				}

				//disgno cuori
				for (int i = 1; i <= player.maxLife; i++) {
					if (player.life >= i) {
						pRT->DrawBitmap(cuoriBitmap, RectF(
							32 * (i - 1),
							40,
							32 * i,
							72), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16, 0, 32, 16));
					}
					else {
						pRT->DrawBitmap(cuoriBitmap, RectF(
							32 * (i - 1),
							40,
							32 * i,
							72), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 16, 16));
					}

				}

				//disegno player + hitbox
				{
					if (playerBitmap && player.immunity % 2 == 0)
						pRT->DrawBitmap(playerBitmap, RectF(
							player.r.left - cam.posX - 4,
							player.r.top,
							player.r.right - cam.posX + 4,
							player.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(getAnimX(playerAnim, animIndex) + getAnimWidthByFrame(playerAnim, animIndex) + player.facingLeft * (getAnimSize(playerAnim, animIndex) * getAnimWidth(playerAnim, animIndex)), getAnimY(playerAnim, animIndex), getAnimX(playerAnim, animIndex) + getAnimWidth(playerAnim, animIndex) + getAnimWidthByFrame(playerAnim, animIndex) + player.facingLeft * (getAnimSize(playerAnim, animIndex) * getAnimWidth(playerAnim, animIndex)), getAnimY(playerAnim, animIndex) + getAnimHeight(playerAnim, animIndex)));

					/*pRT->DrawRectangle(
						RectF(
							player.r.left - cam.posX,
							player.r.top,
							player.r.right - cam.posX,
							player.r.bottom),
						terrainBrushes[3]);*/
				}

			}

		}
		else {
			pRT->FillRectangle(RectF(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom),terrainBrushes[0]);
			//disegno numero livello
			string livelloS = "LIVELLO:" + to_string(numeroLivello + 1);

			for (int i = 0; i < livelloS.size(); i++) {
				pRT->DrawBitmap(fontBitmap, RectF(
					200+32 * (i),
					230,
					200+ 32 * (i + 1),
					262), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (livelloS[i] - 48), 16, 16 * (livelloS[i] - 48 + 1), 32));
			}


			string startS = "START";

			for (int i = 0; i < startS.size(); i++) {
				pRT->DrawBitmap(fontBitmap, RectF(
					200 + 32 * (i),
					290,
					200 + 32 * (i + 1),
					322), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(16 * (startS[i] - 48), 16, 16 * (startS[i] - 48 + 1), 32));
			}
		}

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

void addEntity(int levelNum, int x, int y,int width, int height, double vel, double jmpDec, double jmpPow,int baseState, short type, bool differentSideAnim) {

	vector<tuple<short, short, short>> actions;
	actions = {};
	entities[levelNum].push_back({
		{x, y, x + width, y + height},  // r
		vel,                  // vel
		jmpDec,                   // jmpDec
		jmpPow,                    //jmpPow
		 baseState,        // state
		type,					//type
		actions,//azioniù
		{},
		(width % BLOCK_SIZE == 0) ? width / BLOCK_SIZE : width / BLOCK_SIZE + 1,
		(height % BLOCK_SIZE == 0) ? height / BLOCK_SIZE : height / BLOCK_SIZE + 1,
		true,
		"",
		differentSideAnim
		});
}

//funzione main
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)  {
	
	//inizializzazione audiox2
	InizializzaAudio();

	//prendi il suono
	audioBuffer suonoBuffer = { 0 };

	//animazione idle
	newAnimation(playerAnim,0, 0, 16, 16, "idle");
	addFrame(playerAnim, 60, "idle");
	addFrame(playerAnim, 10, "idle");
	animIndex = "idle";
	newAnimation(playerAnim, 0, 32, 16, 16, "walking");
	addFrame(playerAnim, 30, "walking");
	addFrame(playerAnim, 30, "walking");
	addFrame(playerAnim, 30, "walking");
	newAnimation(playerAnim, 0, 16, 16, 16, "ascending");
	addFrame(playerAnim, 30, "ascending");
	newAnimation(playerAnim, 32, 16, 16, 16, "descending");
	addFrame(playerAnim, 30, "descending");

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
	playerStartPos = new RECT[quantitaLivelli];
	for (int i = 0; i < quantitaLivelli; i++) {
		file >> livSize[i];
	}

	//creo la grandezza giusta dell'array dei nemici
	entities.resize(quantitaLivelli);

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


	//aggiunta di tutti i nemici, rigorosamente in ordine crescente della posizione

	addEntity(0, 100, 300, 16, 32, 1, 1, 0, state::jumping, 4, false);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], -1, 0, 0);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 400, 0, 0);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "idle";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 64, 16, 16, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 100, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");

	addEntity(0, 230, 300, 16, 32, 0, 0, 0, state::walking, 4, false);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], -1, 600, 1);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "idle";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 80, 16, 16, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 100, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "idle");


	addEntity(0, 760, 300, 20, 32, -1, 1, 0, state::walking, 0, true);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 400, 1, 0);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 500, 500, 100);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 601, 525, 100);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 0, 16, 16, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "idle");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 32, 0, 16, 16, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 160, 0, 16, 16, "descending");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "descending");


	addEntity(0, 260, 180,64,32, 0, 0, -1, state::walking, 1, false);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 100, 70, 70);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 16, 16, 16, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "walking");


	addEntity(0, 288, 416, 32, 32, 0, 0, 0, state::walking, 2, false);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 212, 120, 120);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 700, 0, 0);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 32, 16, 16, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "walking");



	addEntity(0, 1500, 100, 20, 32, -1, 1, 0, state::walking, 0, true);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 320, 160, 300);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 0, 16, 16, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "idle");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 32, 0, 16, 16, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 128, 0, 16, 16, "ascending");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "ascending");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 160, 0, 16, 16, "descending");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "descending");


	addEntity(0, 1600, 300, 20, 32, -1, 1, 0, state::walking, 0, true);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 205, 160, 300);
	addActionToEnemy(entities[numeroLivello][entities[numeroLivello].size() - 1], 700, 0, 0);
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	entities[numeroLivello][entities[numeroLivello].size() - 1].animIndex = "walking";
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 0, 0, 16, 16, "idle");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "idle");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 32, 0, 16, 16, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 10, "walking");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 128, 0, 16, 16, "ascending");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "ascending");
	newAnimation(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 160, 0, 16, 16, "descending");
	addFrame(entities[numeroLivello][entities[numeroLivello].size() - 1].animations, 1, "descending");

	
	// mettiamo nel livello i numeri dei blocchi
	for (int f = 0; f < quantitaLivelli; f++) {
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

	//posizioni del player per ogni livello
	playerStartPos[0] = { 0,448,24,480 };
	playerStartPos[1] = { 35,672,59,704 };
	//playerStartPos[2] = { 0,448,24,480 };



	//calcolo tempo per un frame
	double fps = 1000000 / wS.MAX_FPS;

	//calcolo misure player

	int PLAYER_WIDTH = player.r.right - player.r.left;
	int PLAYER_HEIGHT = player.r.bottom - player.r.top;
	player.widthBlock = (PLAYER_WIDTH % BLOCK_SIZE == 0) ? PLAYER_WIDTH / BLOCK_SIZE : PLAYER_WIDTH / BLOCK_SIZE +1;
	player.heightBlock = (PLAYER_HEIGHT % BLOCK_SIZE == 0) ? PLAYER_HEIGHT / BLOCK_SIZE : PLAYER_HEIGHT / BLOCK_SIZE+ 1;

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
	createBitmap(L"sprites/font.png", &fontBitmap);
	createBitmap(L"sprites/cuori.png", &cuoriBitmap);
	createBitmap(L"sprites/enemy.png", &enemyBitmap);
	createBitmap(L"sprites/buttons.png", &buttonsBitmap);

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
			if (waitTime == 0) {
				if (gameOver) {
					if (S.pressed && pos < 0) {
						pos++;
					}
					if (W.pressed && pos > 0) {
						pos--;
					}
					if (J.pressed && numeroLivello != quantitaLivelli) {
						gameOver = false;
						tempo = 0;
						toggleEv();
						waitTime = 60;
						music = PlayAudio(L"./sfx/music.wav", suonoBuffer, XAUDIO2_LOOP_INFINITE, 0.04);
					}
				}
				else {
					if (notRunning == 0 && !ripristina) {
						if (numeroLivello == quantitaLivelli) {
							gameOver = true;
						}
						else {
							if (J.pressed) {
								gameOver = true;
								ripristino(screenEn, limit, livello[numeroLivello], initialLiv[numeroLivello], SCREEN_HEIGHT, BLOCK_SIZE, livSize[numeroLivello], playerStartPos[numeroLivello]);
							}
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
									case 1:
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

							movimentoPlayer(livello[numeroLivello], livSize[numeroLivello], entities[numeroLivello], screenEn, limit, BLOCK_SIZE, SCREEN_WIDTH, ripristina, score, suonoBuffer);
							toggleEv();
							if (player.r.left >= (livSize[numeroLivello] - 2) * BLOCK_SIZE) {
								changeLiv = true;
								player.state = state::walking;
								player.vel = 5;
								notRunning = 120;
								ripristina = true;
							}

						}
					}
					else {
						notRunning--;

						//animazione di molto provvisoria per l'uscita dalla fine del livello
						player.r.left++; 
						player.r.right++;

						//funzione da fare quando finisce il tempo nel quale il gioco è fermo
						if (notRunning <= 0) {
							if (changeLiv) {
								numeroLivello++;
								changeLiv = false;
							}
							if (ripristina) {
								ripristino(screenEn, limit, livello[numeroLivello], initialLiv[numeroLivello], SCREEN_HEIGHT, BLOCK_SIZE, livSize[numeroLivello], playerStartPos[numeroLivello]);
								score = 0;
								tempo = 0;
								ripristina = false;
								StopAudio(music);
								music = PlayAudio(L"./sfx/music.wav", suonoBuffer, XAUDIO2_LOOP_INFINITE, 0.04);

							}
							notRunning = 0;
							waitTime = 60;
						}
					}
					//animazioni player che si possono fare anche quando il gioco è fermo 
					switch (player.state) {
					case state::walking:
						if (animIndex != "walking") {
							reset(playerAnim, animIndex);
							animIndex = "walking";
						}
						break;
					case state::idle:
						if (animIndex != "idle") {
							reset(playerAnim, animIndex);
							animIndex = "idle";
						}
						break;
					case state::jumping:
						if (player.jmpPow > 0) {
							if (animIndex != "ascending") {
								reset(playerAnim, animIndex);
								animIndex = "ascending";
							}
						}
						else {
							if (animIndex != "descending") {
								reset(playerAnim, animIndex);
								animIndex = "descending";
							}
						}

						break;
					}

					if (player.state == state::walking) {
						reduceFrames(playerAnim, animIndex, abs(player.vel));
					}
					else {
						reduceFrames(playerAnim, animIndex, 1);
					}
					if (getFrames(playerAnim, animIndex) <= 0)
						goForward(playerAnim, animIndex);
				}
			}
			else {
				waitTime--;
			}

			//si ridisegna tutto
			//RedrawWindow(hW, NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW | RDW_INVALIDATE);
			InvalidateRect(hW, NULL, TRUE);
			UpdateWindow(hW);
			QueryPerformanceCounter(&start); // riinizializzo il tempo iniziale
			ent++;
		}

		// per aumentare il contatore del tempo
		if (ent >= wS.MAX_FPS) {			
			tempo++;
			ent = 0;
		}
	}
	return 0;
}