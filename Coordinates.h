#pragma once
class Coordinates
{
public:
	Coordinates();
	Coordinates(int new_x, int new_y);
	~Coordinates();

	int get_x();
	int get_y();
	void set_x(int new_x);
	void set_y(int new_y);
	void set_coords(int new_x, int new_y);

private:
	int x;
	int y;
};

