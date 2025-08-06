#include "pulsante.h"

// Inizializzazione delle variabili globali
button W = { false, false };
button A = { false, false };
button S = { false, false };
button D = { false, false };
button J = { false, false };

// Implementazione dei metodi
void button::toggle() {
    if (!held) {
        pressed = true;
        held = true;
    }
    else {
        pressed = false;
    }
}

void button::release() {
    held = false;
    pressed = false;
}

// Aggiornamento dei pulsanti
void toggleEv() {
    if (W.held) W.toggle();
    if (A.held) A.toggle();
    if (S.held) S.toggle();
    if (D.held) D.toggle();
    if (J.held) J.toggle();
}
