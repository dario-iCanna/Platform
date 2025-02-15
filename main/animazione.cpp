#include "animazione.h"
void newAnimation(animazione& a, int x, int y, int width, int height, string animIndex) {
	a.rect[animIndex].resize(4);
	a.rect[animIndex][0] = x;
	a.rect[animIndex][1] = y;
	a.rect[animIndex][2] = width;
	a.rect[animIndex][3] = height;
}

void addFrame(animazione& a, int framePerImage, string animIndex) {
	a.framePerImage[animIndex].push_back(make_pair(framePerImage, framePerImage));
}

int getIndex(animazione a) {
	return a.index;
}

int getAnimSize(animazione a, string animIndex) {
	return a.framePerImage[animIndex].size();
}

void reduceFrames(animazione& a, string animIndex, int val) {
	a.framePerImage[animIndex][a.index].first -= val;
}

int getFrames(animazione a, string animIndex) {
	return a.framePerImage[animIndex][a.index].first;
}

void reset(animazione& a, string animIndex) {
	a.framePerImage[animIndex][a.index].first = a.framePerImage[animIndex][a.index].second;
	a.index = 0;
}

void goForward(animazione& a, string animIndex) {
	a.framePerImage[animIndex][a.index].first = a.framePerImage[animIndex][a.index].second;
	if (a.index < a.framePerImage[animIndex].size() - 1) {
		a.index += 1;
	}
	else {
		a.index = 0;
	}
}

bool existsAnim(animazione a, string animIndex) {
	return a.framePerImage.count(animIndex);
}

int getAnimX(animazione a, string animIndex) {
	return a.rect[animIndex][0];
}

int getAnimY(animazione a, string animIndex) {
	return a.rect[animIndex][1];
}

int getAnimWidth(animazione a, string animIndex) {
	return a.rect[animIndex][2];
}

int getAnimWidthByFrame(animazione a, string animIndex) {
	return a.rect[animIndex][2] * (a.index);
}

int getAnimHeight(animazione a, string animIndex) {
	return a.rect[animIndex][3];
}