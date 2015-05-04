// ------------------------------------------------------------------
// Clase Point - Punto en el plano
// ------------------------------------------------------------------

#ifndef POINT_H
#define POINT_H

#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

class Point {

public:
	/** coordinates */
	double x, y;

	Point (): x(0), y (0) {}
	Point (double ax, double ay): x (ax), y (ay) {}

/** Distance to other point */
	float dist(const Point& p) const
	{
		float dx = x - p.x;
		float dy = y - p.y;
		return sqrt (dx * dx + dy * dy);
	}
	
	bool operator== (const Point& p) const { return (x == p.x) && (y == p.y); }
	bool operator!= (const Point& p) const { return !(*this == p); }
};

inline ostream& operator<< (ostream& o, const Point& p) { return o << "(" << p.x << "," << p.y << ")"; }

inline istream& operator>> (istream& i, Point& p) { return i >> p.x >> p.y; }

#endif
