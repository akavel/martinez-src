/***************************************************************************
 *   Developer: Francisco Martínez del Río (2011)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.4.1                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#include "connector.h"
#include <algorithm>

void PointChain::init (const Segment& s)
{
	l.push_back (s.begin ());
	l.push_back (s.end ());
}

bool PointChain::LinkSegment (const Segment& s)
{
	if (s.begin () == l.front ()) {
		if (s.end () == l.back ())
			_closed = true;
		else
			l.push_front (s.end ());
		return true;
	}
	if (s.end () == l.back ()) {
		if (s.begin () == l.front ())
			_closed = true;
		else
			l.push_back (s.begin ());
		return true;
	}
	if (s.end () == l.front ()) {
		if (s.begin () == l.back ())
			_closed = true;
		else
			l.push_front (s.begin ());
		return true;
	}
	if (s.begin () == l.back ()) {
		if (s.end () == l.front ())
			_closed = true;
		else
			l.push_back (s.end ());
		return true;
	}
	return false;
}

bool PointChain::LinkPointChain (PointChain& chain)
{
	if (chain.l.front () == l.back ()) {
		chain.l.pop_front ();
		l.splice (l.end (), chain.l);
		return true;
	}
	if (chain.l.back () == l.front ()) {
		l.pop_front ();
		l.splice (l.begin (), chain.l);
		return true;
	}
	if (chain.l.front () == l.front ()) {
		l.pop_front ();
		reverse (chain.l.begin (), chain.l.end ());
		l.splice (l.begin (), chain.l);
		return true;
	}
	if (chain.l.back () == l.back ()) {
		l.pop_back ();
		reverse (chain.l.begin (), chain.l.end ());
		l.splice (l.end (), chain.l);
		return true;
	}
	return false;
}

void Connector::add(const Segment& s)
{
	iterator j = openPolygons.begin ();
	while (j != openPolygons.end ()) {
		if (j->LinkSegment (s)) {
			if (j->closed ())
				closedPolygons.splice (closedPolygons.end (), openPolygons, j);
			else {
				list<PointChain>::iterator k = j;
				for (++k; k != openPolygons.end (); k++) {
					if (j->LinkPointChain (*k)) {
						openPolygons.erase (k);
						break;
					}
				}
			}
			return;
		}
		j++;
	}
	// The segment cannot be connected with any open polygon
	openPolygons.push_back (PointChain ());
	openPolygons.back ().init (s);
}

void Connector::toPolygon (Polygon& p)
{
	for (iterator it = begin (); it != end (); it++) {
		Contour& contour = p.pushbackContour ();
		for (PointChain::iterator it2 = it->begin (); it2 != it->end (); it2++)
			contour.add (*it2);
	}
}
