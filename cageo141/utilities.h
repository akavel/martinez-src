// Utility functions

#ifndef UTILITIES_H
#define UTILITIES_H

#include "polygon.h"

int findIntersection (const Segment& seg0, const Segment& seg1, Point& ip0, Point& ip1);

/** Signed area of the triangle (p0, p1, p2) */
inline float signedArea (const Point& p0, const Point& p1, const Point& p2)
{ 
	return (p0.x - p2.x)*(p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
}

/** Signed area of the triangle ( (0,0), p1, p2) */
inline float signedArea (const Point& p1, const Point& p2)
{ 
	return -p2.x*(p1.y - p2.y) - -p2.y*(p1.x - p2.x);
}

/** Sign of triangle (p1, p2, o) */
inline int sign (const Point& p1, const Point& p2, const Point& o)
{
	float det = (p1.x - o.x) * (p2.y - o.y) - (p2.x - o.x) * (p1.y - o.y);
	return (det < 0 ? -1 : (det > 0 ? +1 : 0));
}

inline bool pointInTriangle (const Segment& s, Point& o, Point& p)
{
	int x = sign (s.begin (), s.end (), p);
	return ((x == sign (s.end (), o, p)) && (x == sign (o, s.begin (), p)));
}

#endif
