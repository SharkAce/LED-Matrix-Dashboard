#include "include/led-matrix.h"
#include "include/graphics.h"
#include <string>

// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int main(int argc, char *argv[]){
	rgb_matrix::Font font;
	font.LoadFont("./fonts/6x12.bdf");
	rgb_matrix::Font *outline_font = font.CreateOutlineFont();

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv);

	canvas->Fill(0,255,0);
}
