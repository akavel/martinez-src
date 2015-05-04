/***************************************************************************
 *   Developer: Francisco Martínez del Río (2011)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.4.1                                                          *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "segment.h"
#include "martinez.h"
#include <list>

class PointChain {
public:
	typedef list<Point>::iterator iterator;
	PointChain () : l (), _closed (false) {}
	void init (const Segment& s);
	bool LinkSegment (const Segment& s);
	bool LinkPointChain (PointChain& chain);
	bool closed () const { return _closed; }
	iterator begin () { return l.begin (); }
	iterator end () { return l.end (); }
	void clear () { l.clear (); }
	unsigned int size () const { return l.size (); }
private:
	/** Linked point chain */
	list<Point> l;
	bool _closed; // is the chain closed, that is, is the first point is linked with the last one?
};

class Connector {
public:
	typedef list<PointChain>::iterator iterator;
	Connector () : openPolygons (), closedPolygons () {}
	~Connector () {}
	void add (const Segment& s);
	iterator begin () { return closedPolygons.begin (); }
	iterator end () { return closedPolygons.end (); }
	void clear () { closedPolygons.clear (); openPolygons.clear (); }
	unsigned int size () const { return closedPolygons.size (); }
	void toPolygon (Polygon& p);
private:
	list<PointChain> openPolygons;
	list<PointChain> closedPolygons;
};


#endif
