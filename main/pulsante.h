#ifndef PULSANTE_H
#define PULSANTE_H

struct button {
    bool held;
    bool pressed;

    void toggle();
    void release();
};

// Variabili globali dei pulsanti
extern button W;
extern button A;
extern button S;
extern button D;
extern button J;

// Funzione di aggiornamento globale
void toggleEv();

#endif // PULSANTE_H