#include <iostream>
#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include <iostream>
#include <vector>
#include "player.h"
#include "pulsante.h"
#include "camera.h"

using namespace std;
using namespace D2D1;

#pragma region variabili globali
#define BLOCK_SIZE 30
int SCREEN_HEIGHT = 700;
int SCREEN_WIDTH = 1500;
int tempo = 0, score = 0;
HRESULT p = CoInitialize(NULL);
//factory direct 2d
ID2D1Factory* pD2DFactory = NULL;
HRESULT hr = D2D1CreateFactory(
	D2D1_FACTORY_TYPE_SINGLE_THREADED,
	&pD2DFactory
);
//render per direct 2d
ID2D1HwndRenderTarget* pRT = NULL;
//array di beush?
ID2D1SolidColorBrush* terrainBrushes[4];
//bitmap di prova
ID2D1Bitmap* playerBitmapIdle = NULL;
ID2D1Bitmap* terrainBitmap = NULL;
ID2D1Bitmap* numberBitmap = NULL;
ID2D1Bitmap* cuoriBitmap = NULL;
ID2D1Bitmap* skyBitmap = NULL;
//variabili WIC
IWICImagingFactory* wicFactory = NULL;
IWICBitmapDecoder* wicDecoder = NULL;
IWICBitmapFrameDecode* wicFrame = NULL;
IWICFormatConverter* wicConverter = NULL;

int **livello, **initialLiv;
int heightSize, livSize;
vector<entity> enemy, initialArr;
#pragma endregion

//variabili di funzionamento
struct WINDSTUFF {
	bool running = true;
	bool console = true;
	const double MAX_FPS = 60;
}wS;

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

		pRT->BeginDraw();

		//disegna sfondo
		pRT->DrawBitmap(skyBitmap,RectF(
			clientRect.left,
			clientRect.top,
			clientRect.right,
			clientRect.bottom),1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(0, 0, 1920, 885));

		// disegno livello
		for (int i = floor(cam.posX/BLOCK_SIZE); i < floor(cam.posX / BLOCK_SIZE) + 50 && i < livSize; i++) {
			for (int j = 0; j < SCREEN_HEIGHT / BLOCK_SIZE; j++) {
				if (livello[i][j] != 0) {
					pRT->DrawBitmap(terrainBitmap,
						RectF(
							i * BLOCK_SIZE - cam.posX,
							j * BLOCK_SIZE,
							(i + 1) * BLOCK_SIZE - cam.posX,
							(j + 1) * BLOCK_SIZE
						),
						1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, RectF(30 * (livello[i][j] - 1), 0, 30 * livello[i][j], 30)
					);
				}
			}
		}

		//disegno enemy
		for (int i = 0; i < enemy.size(); i++) {
			entity e = enemy[i];
				pRT->DrawRectangle(
					RectF(
						e.r.left - cam.posX,
						e.r.top,
						e.r.right - cam.posX,
						e.r.bottom),
					terrainBrushes[3]);
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
		

		//disegno player
		if(playerBitmapIdle && player.immunity % 2 == 0)
		pRT->DrawBitmap(playerBitmapIdle, RectF(
			player.r.left - cam.posX-2,
			player.r.top,
			player.r.right-cam.posX+2,
			player.r.bottom), 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,(player.state == state::jumping) ? RectF(40, 0, 60, 20): (player.state == state::walking) ? RectF(20, 0, 40, 20) :RectF(0, 0, 20, 20));

		

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
		}
		break;
	}

	return DefWindowProc(hwnd,uInt,wParam,lParam);
}
#pragma endregion 

//funzione main
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	//roba per la bitmap di directD2
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&wicFactory); // factory WIC
	wicFactory->CreateDecoderFromFilename(L"sprites/idle.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	livSize = 1000;

	// Allocazione dinamica della matrice
	livello = new int* [livSize];
	initialLiv = new int* [livSize];
	for (int i = 0; i < livSize; ++i) {
		livello[i] = new int[SCREEN_HEIGHT / BLOCK_SIZE];
		initialLiv[i] = new int[SCREEN_HEIGHT / BLOCK_SIZE];
	}

	enemy.push_back({
		{200, 300, 230, 330},  // r
		-1,                  // vel
		0.0,                   // jmpDec
		0,                    //jmpPow
		 state::walking,        // state
		0,					//type
		0,					//frame per azione
		0					//variabile per la inizializzazione dei frame
		});

	enemy.push_back({
		{240, 300, 270, 330},  // r
		-1,                  // vel
		1.0,                   // jmpDec
		0,                    //jmpPow
		 state::walking,        // state
		0,					//type
		0,					//frame per azione
		0					//variabile per la inizializzazione dei frame
		});

	enemy.push_back({
		{9*BLOCK_SIZE, 13*BLOCK_SIZE, 10*BLOCK_SIZE, 14*BLOCK_SIZE},  // r
		0,                  // vel
		0.0,                   // jmpDec
		0,                    //jmpPow
		 state::walking,        // state
		2,					//type
		120,					//frame per azione
		120					//variabile per la inizializzazione dei frame
		});

	enemy.push_back({
		{1600, 300, 1630, 330},  // r
		-1,                  // vel
		1.0,                   // jmpDec
		0,                    //jmpPow
		 state::walking,        // state
		0,					//type
		0,					//frame per azione
		0					//variabile per la inizializzazione dei frame
		});

	initialArr = enemy;
	// inizializzazione livello 
	for (int i = 0; i < livSize; i++) {
		for (int j = 0; j < SCREEN_HEIGHT / BLOCK_SIZE; j++) {
			if (j < 16) {
				livello[i][j] = 0;
			}
			else {
				livello[i][j] = 1;
			}
		}
	}

	livello[5][16] = 0;
	livello[6][16] = 0;
	livello[7][16] = 0;
	livello[5][17] = 1;
	livello[6][17] = 1;
	livello[7][17] = 1;
	livello[0][15] = 1;
	livello[7][12] = 1;
	livello[9][12] = 2;
	livello[9][13] = 3;
	livello[26][9] = 1;
	for (int i = 7; i <= SCREEN_HEIGHT / BLOCK_SIZE; i++) {
		livello[20][i] = 1;
	}
	livello[19][12] = 1;
	livello[14][9] = 1;
	livello[13][8] = 1;
	for (int i = 25; i < livSize; i++) {
		livello[i][7] = 2;
		livello[i][6] = 4;
	}
	livello[10][10] = 5;
	livello[11][11] = 5;
	for (int i = 25; i < 30; i++) {
		for (int j = 16; j <= SCREEN_HEIGHT / BLOCK_SIZE; j++) {
			livello[i][j] = 0;
		}
	}
	livello[24][15] = 4;
	livello[30][15] = 4;
	livello[25][14] = 4;
	livello[29][14] = 4;
	livello[26][13] = 4;
	livello[26][12] = 4;
	livello[28][13] = 4;
	livello[28][12] = 4;
	livello[27][12] = 4;


	for (int i = 0; i < livSize; i++) {
		for (int j = 0; j < SCREEN_HEIGHT / BLOCK_SIZE; j++) {
			initialLiv[i][j] = livello[i][j];
		}
	}

	//calcolo tempo per un frame
	double fps = 1000000 / wS.MAX_FPS;

	//creo la console
	if (wS.console) {
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		ios_base::sync_with_stdio;
	}

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
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, &playerBitmapIdle);// crea la bitmap direct2d

	wicFactory->CreateDecoderFromFilename(L"sprites/terreno.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, &terrainBitmap);// crea la bitmap direct2d

	wicFactory->CreateDecoderFromFilename(L"sprites/sky.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, &skyBitmap);// crea la bitmap direct2d

	wicFactory->CreateDecoderFromFilename(L"sprites/numeri.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, &numberBitmap);// crea la bitmap direct2d

	wicFactory->CreateDecoderFromFilename(L"sprites/cuori.png", NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &wicDecoder); // Crea Decoder
	wicFactory->CreateFormatConverter(&wicConverter); // crea Converter
	wicDecoder->GetFrame(0, &wicFrame); // prende l'immagine
	wicConverter->Initialize(wicFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom); // inizializza il converter
	pRT->CreateBitmapFromWicBitmap(wicConverter, NULL, &cuoriBitmap);// crea la bitmap direct2d
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
			movimentoPlayer(livello, initialLiv, BLOCK_SIZE, enemy, enemy.size(), SCREEN_WIDTH, livSize, initialArr, score, SCREEN_HEIGHT, tempo);
			toggleEv();
			//RedrawWindow(hW, NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW | RDW_INVALIDATE);
			InvalidateRect(hW, NULL, TRUE);
			UpdateWindow(hW);
			QueryPerformanceCounter(&start); // riinizializzo il tempo iniziale
			ent++;
		}
		if (ent >= wS.MAX_FPS) {
			tempo++;
			ent = 0;
		}
		Sleep(10);// sleeppo per non usare tutta la cpu
	}
	return 0;
}