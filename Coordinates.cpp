#include "Coordinates.h"

Coordinates::Coordinates() {
	x = -1;
	y = -1;
}

Coordinates::Coordinates(int new_x, int new_y) {
	x = new_x;
	y = new_y;
}

Coordinates::~Coordinates() {
}


int Coordinates::get_x() {
	return x;
}


int Coordinates::get_y() {
	return y;
}


void Coordinates::set_x(int new_x) {
	x = new_x;
}


void Coordinates::set_y(int new_y) {
	y = new_y;
}


void Coordinates::set_coords(int new_x, int new_y) {
	x = new_x;
	y = new_y;
}