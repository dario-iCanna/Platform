#include "pulsante.h"

//inizializzazione tasti
#pragma region tasti
button W = { false, false };
 button A = { false, false };
 button S = { false, false };
 button D = { false, false };
 button J = { false, false };
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