#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <algorithm>
#include "segment.h"

using namespace std;

class Contour {
public:
	typedef vector<Point>::iterator iterator;
	
	Contour () : points (), holes (), _external (true), _precomputedCC (false) {}

	/** Get the p-th vertex of the external contour */
	Point& vertex (unsigned p) { return points[p]; }
	Segment segment (unsigned p) const { return (p == nvertices () - 1) ? Segment (points.back (), points.front ()) : Segment (points[p], points[p+1]); }
	/** Number of vertices and edges */
	unsigned nvertices () const { return points.size (); }
	unsigned nedges () const { return points.size (); }
	/** Get the bounding box */
	void boundingbox (Point& min, Point& max);
	/** Return if the contour is counterclockwise oriented */
	bool counterclockwise ();
	/** Return if the contour is clockwise oriented */
	bool clockwise () { return !counterclockwise (); }
	void changeOrientation () { reverse (points.begin (), points.end ()); _CC = !_CC; }
	void setClockwise () { if (counterclockwise ()) changeOrientation (); }
	void setCounterClockwise () { if (clockwise ()) changeOrientation (); }

	void move (double x, double y);
	void add (const Point& s) { points.push_back (s); }
	void erase (iterator i) { points.erase (i); }
	void clear () { points.clear (); holes.clear (); }
	iterator begin () { return points.begin (); }
	iterator end () { return points.end (); }
	void addHole (unsigned ind) { holes.push_back (ind); }
	unsigned nholes () const { return holes.size (); }
	unsigned hole (unsigned p) const { return holes[p]; }
	bool external () const { return _external; }
	void setExternal (bool e) { _external = e; }

	private:
	/** Set of points conforming the external contour */
	vector<Point> points;
	/** Holes of the contour. They are stored as the indexes of the holes in a polygon class */
	vector<int> holes;
	bool _external; // is the contour an external contour? (i.e., is it not a hole?)
	bool _precomputedCC;
	bool _CC;
};

ostream& operator<< (ostream& o, Contour& c);

class Polygon {
public:
	typedef vector<Contour>::iterator iterator;
	
	Polygon () : contours () {}
	Polygon (const string& filename);
	/** Get the p-th contour */
	Contour& contour (unsigned p) { return contours[p]; }
	/** Number of contours */
	unsigned ncontours () const { return contours.size (); }
	/** Number of vertices */
	unsigned nvertices () const;
	/** Get the bounding box */
	void boundingbox (Point& min, Point& max);

	void move (double x, double y);

	Contour& pushbackContour () { contours.push_back (Contour ()); return contours.back (); }
	void deletebackContour () { contours.pop_back (); }
	void erase (iterator i) { contours.erase (i); }
	void clear () { contours.clear (); }

	iterator begin () { return contours.begin (); }
	iterator end () { return contours.end (); }
	void computeHoles ();
private:
	/** Set of contours conforming the polygon */
	vector<Contour> contours;
};

ostream& operator<< (ostream& o, Polygon& p);
istream& operator>> (istream& i, Polygon& p);
#endif
