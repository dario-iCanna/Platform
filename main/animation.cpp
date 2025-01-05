#include "animation.h"

void newAnimation(int x,int y ,int width, int height, int n, animazione& a) {
	a.x = x;
	a.y = y;
	a.width = width;
	a.height = height;
	a.size = n;
	a.ind = 0;
	a.fpF = new int[n];
	a.inifpF = new int[n];
}

void frameSprite(int i, int n, animazione& a) {
	a.fpF[i] = n;
	a.inifpF[i] = n;
}