// ------------------------------------------------------------------
// Clase Segment - Segmentos en el plano
// ------------------------------------------------------------------

#ifndef SEGMENT_H
#define SEGMENT_H

#include "point.h"

class Polygon;

class Segment {
public:
	/** Segment endpoints */
	Point p1, p2;

public:
	/** Default constructor */
	Segment () {}
	~Segment () {}

	/** Constructor from two points **/
	Segment(const Point& ap1, const Point& ap2) : p1 (ap1), p2 (ap2) {}

	/** Set the beginning point */
	void setbegin(const Point& p) { p1 = p; }
	/** Set the end point */
	void setend(const Point& p) { p2 = p; }

	/** Get the beginning point */
	const Point& begin() const { return p1; }
	/** Get the end point */
	const Point& end() const { return p2; }

	/** Change the segment orientation */
	Segment& changeOrientation () { Point tmp = p1; p1 = p2; p2 = tmp; return *this; }
};

inline ostream& operator<< (ostream& o, const Segment& p) { return o << p.begin () << "-" << p.end (); }

#endif
