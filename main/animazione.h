#ifndef ANIMAZIONE_H
#define ANIMAZIONE_H

#pragma once
#include <vector>
#include <unordered_map>
#include <string>
using namespace std;


struct animazione {
	unordered_map<string, vector<int>> rect;
	unordered_map<string, vector<pair<int, int>>> framePerImage;
	int index;
};

void newAnimation(animazione& a, int x, int y, int width, int height, string animIndex);

void addFrame(animazione& a, int framePerImage, string animIndex);

int getIndex(animazione a, string animIndex);

int getAnimSize(animazione a, string animIndex);

int getFrames(animazione a, string animIndex);

void reduceFrames(animazione& a ,string animIndex, int val);

void reset(animazione& a, string animIndex);

void goForward(animazione& a, string animIndex);

bool existsAnim(animazione a, string animIndex);

int getAnimX(animazione a, string animIndex);

int getAnimY(animazione a, string animIndex);

int getAnimWidth(animazione a, string animIndex);

int getAnimWidthByFrame(animazione a, string animIndex);

int getAnimHeight(animazione a, string animIndex);


#endif // ANIMAZIONE_H