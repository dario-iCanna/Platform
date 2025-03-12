#include "pulsante.h"

//inizializzazione tasti
#pragma region tasti
struct button W = { false, false };
struct button A = { false, false };
struct button S = { false, false };
struct button D = { false, false };
struct button J = { false, false };
#pragma endregion

//pressione del pulsante
void button::toggle() {
	if (held == false) {
		pressed = true;
		held = true;
	}
	else if (held == true) {
		pressed = false;
	}
};

//rilascio del pulsante
void button::release() {
	held = false;
	pressed = false;
}

 //aggiornamento del pulsante levando il pressed se è stato premuto per più di un frame
 void toggleEv() {
	 if (W.held) {
		 W.toggle();
	 }
	 if (A.held) {
		 A.toggle();
	 }
	 if (S.held) {
		 S.toggle();
	 }
	 if (D.held) {
		 D.toggle();
	 }
	 if (J.held) {
		 J.toggle();
	 }
 }