/***************************************************************************
 *   Developer: Francisco Martínez del Río (2011)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.4.1                                                        *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#ifndef MARTINEZ_H
#define MARTINEZ_H

#include "polygon.h"
#include "point.h"
#include "segment.h"
#include "utilities.h"
#include <iostream>
#include <queue>
#include <vector>
#include <set>

using namespace std;

class Connector;

class Martinez {
public:
	enum BoolOpType { INTERSECTION, UNION, DIFFERENCE, XOR };
	/** Class constructor */
	Martinez (Polygon& sp, Polygon& cp) : eq (), eventHolder (), subject (sp), clipping (cp), sec (), nint (0) {}
	/** Compute the boolean operation */
	void compute (BoolOpType op, Polygon& result);
	/** Number of intersections found (for statistics) */
	int nInt () const { return nint; }

private:
	enum EdgeType { NORMAL, NON_CONTRIBUTING, SAME_TRANSITION, DIFFERENT_TRANSITION };
	enum PolygonType { SUBJECT, CLIPPING };

	struct SweepEvent {
		Point p;           // point associated with the event
		bool left;         // is the point the left endpoint of the segment (p, other->p)?
		PolygonType pl;    // Polygon to which the associated segment belongs to
		SweepEvent *other; // Event associated to the other endpoint of the segment
		/**  Does the segment (p, other->p) represent an inside-outside transition in the polygon for a vertical ray from (p.x, -infinite) that crosses the segment? */
		bool inOut;
		EdgeType type;
		bool inside; // Only used in "left" events. Is the segment (p, other->p) inside the other polygon?
		set<SweepEvent*>::iterator* poss; // Only used in "left" events. Position of the event (line segment) in S

		/** Class constructor */
		SweepEvent (const Point& pp, bool b, PolygonType apl, SweepEvent* o, EdgeType t = NORMAL) : p (pp), left (b), pl (apl), other (o), type (t), poss (0) {}
		/** Class destructor */
		~SweepEvent () { delete poss; }
 		/** Return the line segment associated to the SweepEvent */
		Segment segment () { return Segment (p, other->p); }
		/** Is the line segment (p, other->p) below point x */
		bool below (const Point& x) const { return (left) ? signedArea (p, other->p, x) > 0 : signedArea (other->p, p, x) > 0; }
		/** Is the line segment (p, other->p) above point x */
		bool above (const Point& x) const { return !below (x); }
	};
	
	static void print (SweepEvent& e); // This function is intended for debugging purposes

	struct SweepEventComp : public binary_function<SweepEvent*, SweepEvent*, bool> {
		bool operator() (SweepEvent* e1, SweepEvent* e2);
	};

	struct SegmentComp : public binary_function<SweepEvent*, SweepEvent*, bool> {
		bool operator() (SweepEvent* e1, SweepEvent* e2);
	};
	
	/** @brief Event Queue */
	priority_queue<SweepEvent*, vector<SweepEvent*>, SweepEventComp> eq;
	/** @brief It holds the events generated during the computation of the boolean operation */
	deque<SweepEvent> eventHolder;
	/** @brief Polygon 1 */
	Polygon& subject;
	/** @brief Polygon 2 */
	Polygon& clipping;
	/** To compare events */
	SweepEventComp sec;
	/** @brief Number of intersections (for statistics) */
	int nint;
	/** @brief Compute the events associated to segment s, and insert them into pq and eq */
	void processSegment (const Segment& s, PolygonType pl);
	/** @brief Process a posible intersection between the segment associated to the left events e1 and e2 */
	void possibleIntersection (SweepEvent *e1, SweepEvent *e2);
	/** @brief Divide the segment associated to left event e, updating pq and (implicitly) the status line */
	void divideSegment (SweepEvent *e, const Point& p);
	/** @brief Store the SweepEvent e into the event holder, returning the address of e */
	SweepEvent *storeSweepEvent(const SweepEvent& e) { eventHolder.push_back (e); return &eventHolder.back (); }
};

#endif
