#ifndef GREINER_H
#define GREINER_H

#include <iostream>
#include <vector>
#include <limits>
#include "segment.h"
#include "polygon.h"
#include "point.h"
#include "martinez.h"

using namespace std;

struct Vertex {
	double x, y;
	Vertex *prev, *next;
	Segment s; // Segment which first point is the vertex
	bool intersect; // is the vertex an intersection point?
	bool entry; // is the intersection point an entry point on the other polygon?
	bool processed; // for step 3
	Vertex *neighbor;
	double alpha;
	Vertex (double xp, double yp, Segment sp, bool i, double a = numeric_limits<double>::max());
	Vertex () {}
};
ostream& operator<< (ostream& o, const Vertex& v);

class GreinerContour {
public:
	GreinerContour (Contour& c);
	~GreinerContour () { deleteIntersections (); }
	Vertex *firstVertex () { return &v[0]; }
	/** @brief Insert vertex (intersection) v before the vertex pointed by vp */
	Vertex *insert (const Vertex& v, Vertex *vp);
	void deleteIntersections ();
	bool intersectBoundingbox (GreinerContour& gc) const;
private:
	/** @brief It holds the original vertices of the polygon */
	vector<Vertex> v;
	/** @brief Number of intersection points */
	int nint;
	/** @ brief bounding box */
	Point minbox;
	Point maxbox;
};
ostream& operator<< (ostream& o, GreinerContour& gc);

class GreinerHormann {
public:
	GreinerHormann (Polygon& p1, Polygon& p2);
	~GreinerHormann ();
	int boolop (Martinez::BoolOpType op, Polygon& result);
private:
	vector<GreinerContour*> gp1;
	vector<GreinerContour*> gp2;
	Polygon& subject;
	Polygon& clipping;
	int boolop (Martinez::BoolOpType op, GreinerContour& gc1, GreinerContour& gc2, Polygon& result);
};

#endif
