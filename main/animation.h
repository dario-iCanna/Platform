#pragma once

//struct per le animazioni
extern struct animazione{
	int x, y, width, height, size, ind; // variabili per descrivere la grandezza dello sprite
	int* fpF;
	int* inifpF;// variabili per dire quanto dura ogni frame
};

void newAnimation(int x, int y, int width, int height, int n, animazione& a);

void frameSprite(int i, int n, animazione& a);