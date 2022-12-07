
/*
 *  This code is from jarikomppa: https://github.com/jarikomppa/adventofcode2021
 *  He has generously released his code into the public domain.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern unsigned int framebuffer[];

void setupgif(uint32_t delay, int motionblurframes, const char* fn);
void nextframe(int ofs);
void endgif();
void clear();
void strokerect(int x0, int y0, int x1, int y1, int stroke_width, unsigned int color);
void drawrect(int x0, int y0, int x1, int y1, unsigned int color);
void drawbox(int x0, int y0, int w, int h, unsigned int color);
void drawtri(double x0, double y0, double x1, double y1, double x2, double y2, unsigned int color);
void drawline(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawarrow(double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawarrow2(double start, double end, double x0, double y0, double x1, double y1, double w, unsigned int color);
void drawcircle(double x0, double y0, double r, unsigned int color);
void drawstring(const char* aString, int aX, int aY, int aColor);
void drawstringn(const char* aString, int aX, int aY, int n, int aColor);
void drawstringf( int aX, int aY, int n, int aColor, const char* fmt, ...);


long long getticks();

int isprime(long long val);
char* load(const char* fname, int* len);

