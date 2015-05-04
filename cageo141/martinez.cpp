/***************************************************************************
 *   Developer: Francisco Martínez del Río (2011)                          *  
 *   fmartin@ujaen.es                                                      *
 *   Version: 1.4.1                                                        *
 *                                                                         *
 *   This is a public domain program                                       *
 ***************************************************************************/

#include "martinez.h"
#include "connector.h"
#include <algorithm>
#include <iostream>
#include <cassert>
#include <signal.h>
#include <unistd.h>

// #define _DEBUG_ // uncomment this line if you want to debug the computation of the boolean operation

// This function is intended for debugging purposes
void Martinez::print (SweepEvent& e)
{
	const char* namesEventTypes[] = { " (NORMAL) ", " (NON_CONTRIBUTING) ", " (SAME_TRANSITION) ", " (DIFFERENT_TRANSITION) " };
	cout << " Point: " << e.p << " Other point: " << e.other->p << (e.left ? " (Left) " : " (Right) ")
         << (e.inside ? " (Inside) " : " (Outside) ") <<  (e.inOut ? " (In-Out) " : " (Out-In) ") << "Type: "
         << namesEventTypes[e.type] << " Polygon: " << (e.pl == SUBJECT ? " (SUBJECT)" : " (CLIPPING)") << endl;
}

// Compare two sweep events
// Return true means that e1 is placed at the event queue after e2, i.e,, e1 is processed by the algorithm after e2
bool Martinez::SweepEventComp::operator() (SweepEvent* e1, SweepEvent* e2) {
	if (e1->p.x > e2->p.x) // Different x-coordinate
		return true;
	if (e2->p.x > e1->p.x) // Different x-coordinate
		return false;
	if (e1->p != e2->p) // Different points, but same x-coordinate. The event with lower y-coordinate is processed first
		return e1->p.y > e2->p.y;
	if (e1->left != e2->left) // Same point, but one is a left endpoint and the other a right endpoint. The right endpoint is processed first
		return e1->left;
	// Same point, both events are left endpoints or both are right endpoints. The event associate to the bottom segment is processed first
	return e1->above (e2->other->p);
}

// e1 and a2 are the left events of line segments (e1->p, e1->other->p) and (e2->p, e2->other->p)
bool Martinez::SegmentComp::operator() (SweepEvent* e1, SweepEvent* e2) {
	if (e1 == e2)
		return false;
	if (signedArea (e1->p, e1->other->p, e2->p) != 0 || signedArea (e1->p, e1->other->p, e2->other->p) != 0) {
		// Segments are not collinear
		// If they share their left endpoint use the right endpoint to sort
		if (e1->p == e2->p)
			return e1->below (e2->other->p);
		
		// Different points
		SweepEventComp comp;
		if (comp (e1, e2))  // has the line segment associated to e1 been inserted into S after the line segment associated to e2 ?
			return e2->above (e1->p);
		// The line segment associated to e2 has been inserted into S after the line segment associated to e1
		return e1->below (e2->p);
	}
	// Segments are collinear. Just a consistent criterion is used
	if (e1->p == e2->p)
		return e1 < e2;
	SweepEventComp comp;
	return comp (e1, e2);
}

void Martinez::compute (BoolOpType op, Polygon& result)
{
	// Test 1 for trivial result case
	if (subject.ncontours () * clipping.ncontours () == 0) { // At least one of the polygons is empty
		if (op == DIFFERENCE)
			result = subject;
		if (op == UNION)
			result = (subject.ncontours () == 0) ? clipping : subject;
		return;
	}
	// Test 2 for trivial result case
	Point minsubj, maxsubj, minclip, maxclip;
	subject.boundingbox (minsubj, maxsubj);
	clipping.boundingbox (minclip, maxclip);
	if (minsubj.x > maxclip.x || minclip.x > maxsubj.x || minsubj.y > maxclip.y || minclip.y > maxsubj.y) {
		// the bounding boxes do not overlap
		if (op == DIFFERENCE)
			result = subject;
		if (op == UNION) {
			result = subject;
			for (unsigned int i = 0; i < clipping.ncontours (); i++)
				result.pushbackContour () = clipping.contour (i);
		}
		return;
	}

	// Boolean operation is not trivial

	// Insert all the endpoints associated to the line segments into the event queue
	for (unsigned int i = 0; i < subject.ncontours (); i++)
		for (unsigned int j = 0; j < subject.contour (i).nvertices (); j++)
			processSegment(subject.contour (i).segment (j), SUBJECT);
	for (unsigned int i = 0; i < clipping.ncontours (); i++)
		for (unsigned int j = 0; j < clipping.contour (i).nvertices (); j++)
			processSegment(clipping.contour (i).segment (j), CLIPPING);

	Connector connector; // to connect the edge solutions
	set<SweepEvent*, SegmentComp> S; // Status line
	set<SweepEvent*, SegmentComp>::iterator it, sli, prev, next;
	SweepEvent* e;
	const double MINMAXX = std::min (maxsubj.x, maxclip.x); // for optimization 1

	while (!eq.empty()) {
		e = eq.top ();
		eq.pop ();
		#ifdef _DEBUG_
		cout << "Process event: "; print (*e);
		#endif
		// optimization 1
		if ((op == INTERSECTION && (e->p.x > MINMAXX)) || (op == DIFFERENCE && e->p.x > maxsubj.x)) {
			connector.toPolygon (result);
			return;
		}
		if ((op == UNION && (e->p.x > MINMAXX))) {
			// add all the non-processed line segments to the result
			if (!e->left)
				connector.add (e->segment ());
			while (!eq.empty()) {
				e = eq.top();
				eq.pop();
				if (!e->left)
					connector.add (e->segment ());
			}
			connector.toPolygon (result);
			return;
		}
		// end of optimization 1

		if (e->left) { // the line segment must be inserted into S
			it = S.insert(e).first;
			e->poss = new set<SweepEvent*>::iterator (it);
			next = prev = it;
			(prev != S.begin()) ? --prev : prev = S.end();

			// Compute the inside and inOut flags
			if (prev == S.end ()) {           // there is not a previous line segment in S?
				e->inside = e->inOut = false;
			} else if ((*prev)->type != NORMAL) {
				if (prev == S.begin ()) { // e overlaps with prev
					e->inside = true; // it is not relevant to set true or false
					e->inOut = false;
				} else {   // the previous two line segments in S are overlapping line segments
					sli = prev;
					sli--;
					if ((*prev)->pl == e->pl) {
						e->inOut  = !(*prev)->inOut;
						e->inside = !(*sli)->inOut;
					} else {
						e->inOut  = !(*sli)->inOut;
						e->inside = !(*prev)->inOut;
					}
				}
			} else if (e->pl == (*prev)->pl) { // previous line segment in S belongs to the same polygon that "e" belongs to
				e->inside = (*prev)->inside;
				e->inOut  = ! (*prev)->inOut;
			} else {                          // previous line segment in S belongs to a different polygon that "e" belongs to
				e->inside = ! (*prev)->inOut;
				e->inOut  = (*prev)->inside;
			}

			#ifdef _DEBUG_
			cout << "Status line after insertion: " << endl;
			for (set<SweepEvent*, SegmentComp>::const_iterator it2 = S.begin(); it2 != S.end(); it2++)
				print (**it2);
			#endif

			// Process a possible intersection between "e" and its next neighbor in S
			if ((++next) != S.end())
				possibleIntersection(e, *next);

			// Process a possible intersection between "e" and its previous neighbor in S
			if (prev != S.end ())
				possibleIntersection(*prev, e);
		} else { // the line segment must be removed from S
			next = prev = sli = *(e->other->poss); // S.find (e->other);

			// Get the next and previous line segments to "e" in S
			++next;
			(prev != S.begin()) ? --prev : prev = S.end();

			// Check if the line segment belongs to the Boolean operation
			switch (e->type) {
				case (NORMAL):
					switch (op) {
						case (INTERSECTION):
							if (e->other->inside)
								connector.add (e->segment ());
							break;
						case (UNION):
							if (!e->other->inside)
								connector.add (e->segment ());
							break;
						case (DIFFERENCE):
							if (((e->pl == SUBJECT) && (!e->other->inside)) || (e->pl == CLIPPING && e->other->inside))
								connector.add (e->segment ());
							break;
						case (XOR):
							connector.add (e->segment ());
							break;
					}
					break;
				case (SAME_TRANSITION):
					if (op == INTERSECTION || op == UNION)
						connector.add (e->segment ());
					break;
				case (DIFFERENT_TRANSITION):
					if (op == DIFFERENCE)
						connector.add (e->segment ());
					break;
			}
			// delete line segment associated to e from S and check for intersection between the neighbors of "e" in S
			S.erase (sli);
			if (next != S.end() && prev != S.end())
				possibleIntersection (*prev, *next);
		}

		#ifdef _DEBUG_
		cout << "Status line after processing intersections: " << endl;
		for (set<SweepEvent*, SegmentComp>::const_iterator it2 = S.begin(); it2 != S.end(); it2++)
			print (**it2);
		cout << endl;
		#endif
	}
	connector.toPolygon (result);
}

void Martinez::processSegment (const Segment& s, PolygonType pl)
{
	if (s.begin () == s.end ()) // if the two edge endpoints are equal the segment is dicarded
		return;                 // in the future this can be done as preprocessing to avoid "polygons" with less than 3 edges
	SweepEvent* e1 = storeSweepEvent (SweepEvent(s.begin(), true, pl, 0));
	SweepEvent* e2 = storeSweepEvent (SweepEvent(s.end(), true, pl, e1));
	e1->other = e2;

	if (e1->p.x < e2->p.x) {
		e2->left = false;
	} else if (e1->p.x > e2->p.x) {
		e1->left = false;
	} else if (e1->p.y < e2->p.y) { // the line segment is vertical. The bottom endpoint is the left endpoint
		e2->left = false;
	} else {
		e1->left = false;
	}
	eq.push (e1);
	eq.push (e2);
}

void Martinez::possibleIntersection (SweepEvent* e1, SweepEvent* e2)
{
//	if ((e1->pl == e2->pl) ) // you can uncomment these two lines if self-intersecting polygons are not allowed
//		return false;

	Point ip1, ip2;  // intersection points
	int nintersections;

	if (!(nintersections = findIntersection(e1->segment (), e2->segment (), ip1, ip2)))
		return;

	if ((nintersections == 1) && ((e1->p == e2->p) || (e1->other->p == e2->other->p)))
		return; // the line segments intersect at an endpoint of both line segments

	if (nintersections == 2 && e1->pl == e2->pl)
		return; // the line segments overlap, but they belong to the same polygon

	// The line segments associated to e1 and e2 intersect
	nint += nintersections;

	if (nintersections == 1) {
		if (e1->p != ip1 && e1->other->p != ip1)  // if ip1 is not an endpoint of the line segment associated to e1 then divide "e1"
			divideSegment (e1, ip1);
		if (e2->p != ip1 && e2->other->p != ip1)  // if ip1 is not an endpoint of the line segment associated to e2 then divide "e2"
			divideSegment (e2, ip1);
		return;
	}

	// The line segments overlap
	vector<SweepEvent *> sortedEvents;
	if (e1->p == e2->p) {
		sortedEvents.push_back (0);
	} else if (sec (e1, e2)) {
		sortedEvents.push_back (e2);
		sortedEvents.push_back (e1);
	} else {
		sortedEvents.push_back (e1);
		sortedEvents.push_back (e2);
	}
	if (e1->other->p == e2->other->p) {
		sortedEvents.push_back (0);
	} else if (sec (e1->other, e2->other)) {
		sortedEvents.push_back (e2->other);
		sortedEvents.push_back (e1->other);
	} else {
		sortedEvents.push_back (e1->other);
		sortedEvents.push_back (e2->other);
	}

	if (sortedEvents.size () == 2) { // are both line segments equal?
		e1->type = e1->other->type = NON_CONTRIBUTING;
		e2->type = e2->other->type = (e1->inOut == e2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
		return;
	}
	if (sortedEvents.size () == 3) { // the line segments share an endpoint
		sortedEvents[1]->type = sortedEvents[1]->other->type = NON_CONTRIBUTING;
		if (sortedEvents[0])         // is the right endpoint the shared point?
			sortedEvents[0]->other->type = (e1->inOut == e2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
		 else 								// the shared point is the left endpoint
			sortedEvents[2]->other->type = (e1->inOut == e2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
		divideSegment (sortedEvents[0] ? sortedEvents[0] : sortedEvents[2]->other, sortedEvents[1]->p);
		return;
	}
	if (sortedEvents[0] != sortedEvents[3]->other) { // no line segment includes totally the other one
		sortedEvents[1]->type = NON_CONTRIBUTING;
		sortedEvents[2]->type = (e1->inOut == e2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
		divideSegment (sortedEvents[0], sortedEvents[1]->p);
		divideSegment (sortedEvents[1], sortedEvents[2]->p);
		return;
	}
	 // one line segment includes the other one
	sortedEvents[1]->type = sortedEvents[1]->other->type = NON_CONTRIBUTING;
	divideSegment (sortedEvents[0], sortedEvents[1]->p);
	sortedEvents[3]->other->type = (e1->inOut == e2->inOut) ? SAME_TRANSITION : DIFFERENT_TRANSITION;
	divideSegment (sortedEvents[3]->other, sortedEvents[2]->p);
}

void Martinez::divideSegment (SweepEvent* e, const Point& p)
{
	// "Right event" of the "left line segment" resulting from dividing e (the line segment associated to e)
	SweepEvent *r = storeSweepEvent(SweepEvent(p, false, e->pl, e, e->type));
	// "Left event" of the "right line segment" resulting from dividing e (the line segment associated to e)
	SweepEvent *l = storeSweepEvent(SweepEvent(p, true, e->pl, e->other, e->other->type));
	if (sec (l, e->other)) { // avoid a rounding error. The left event would be processed after the right event
		cout << "Oops" << endl;
		e->other->left = true;
		l->left = false;
	}
	if (sec (e, r)) { // avoid a rounding error. The left event would be processed after the right event
		cout << "Oops2" << endl;
//		cout << *e << endl;
	}
	e->other->other = l;
	e->other = r;
	eq.push(l);
	eq.push(r);
}
