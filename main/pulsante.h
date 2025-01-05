#ifndef PULSANTE_H
#define PULSANTE_H

//struct per ogni pulsante
extern struct button {

	bool held;
	bool pressed;

	void toggle();

	void release();


}W, A, S, D, J;

//metodo toggle
void toggleEv();

#endif // PULSANTE_H