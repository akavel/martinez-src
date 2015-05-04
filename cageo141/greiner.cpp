#include "greiner.h"

#include <iostream>
#include <list>
#include <vector>
#include <cmath>

#include "utilities.h"

ostream& operator<< (ostream& o, const Vertex& v)
{
	return o << " Point: (" << v.x << "," << v.y << ") Intersect: " << v.intersect << " alpha: " << v.alpha;
}

Vertex::Vertex (double xp, double yp, Segment sp, bool i, double a) : x (xp), y (yp), s (sp), intersect (i), alpha (a)
{
	next = prev = 0;
	processed = entry = false;
}

GreinerContour::GreinerContour (Contour& c): v (c.nvertices ()), nint (0)
{
	for (unsigned int i = 0; i < c.nvertices (); i++)
		v[i] = Vertex (c.vertex (i).x, c.vertex (i).y, c.segment (i), false);
	for (unsigned int i = 1; i < v.size () -1; i++) {
		v[i].prev = &v[i-1];
		v[i].next = &v[i+1];
	}
	v[0].next = &v[1];
	v[0].prev = &v.back ();
	v.back ().next = &v[0];
	v.back ().prev = &v[v.size()-2];
	c.boundingbox (minbox, maxbox);
}

Vertex *GreinerContour::insert (const Vertex& ver, Vertex *vp)
{
	nint++;
	Vertex *tmp = new Vertex (ver);
	tmp->next = vp;
	tmp->prev = vp->prev;
	return vp->prev = tmp->prev->next = tmp;
}

void GreinerContour::deleteIntersections ()
{
	if (v.size () == 0 || nint == 0)
		return;
	nint = 0;
	Vertex *v = firstVertex ();
	do {
		if (v->intersect) {
			Vertex* prev = v->prev;
			do {
				Vertex* tmp = v;
				v = v->next;
				delete tmp;
			} while (v->intersect);
			prev->next = v;
			v->prev = prev;
		} else
			v = v->next;
	} while (v != firstVertex ());
}

bool GreinerContour::intersectBoundingbox (GreinerContour& gc) const
{
	if ((minbox.x > gc.maxbox.x || maxbox.x < gc.minbox.x))
		return false;
	if ((minbox.y > gc.maxbox.y || maxbox.y < gc.minbox.y))
		return false;
	return true;
}


ostream& operator<< (ostream& o, GreinerContour& gc)
{
	Vertex* v = gc.firstVertex ();
	do {
		o << *v << endl;
		v = v->next;
	} while (v != gc.firstVertex ());
	return o;
}

static bool pointInPolygon (Point& p, GreinerContour& gp)
{
	Point origin (0, 0);
	int inside = 0;
	Vertex *v = gp.firstVertex ();
	do {
		if (!v->intersect && pointInTriangle (v->s, origin, p))
			inside ^= 1;
		v = v->next;
	} while (v != gp.firstVertex ());
	return inside;
}

GreinerHormann::GreinerHormann (Polygon& subj, Polygon& cli): gp1 (), gp2 (), subject (subj), clipping (cli)
{
	for (unsigned int i = 0; i < subject.ncontours (); i++)
		gp1.push_back (new GreinerContour (subject.contour (i)));
	for (unsigned int i = 0; i < clipping.ncontours (); i++)
		gp2.push_back (new GreinerContour (clipping.contour (i)));
}

GreinerHormann::~GreinerHormann ()
{
	for (unsigned int i = 0; i < gp1.size (); i++)
		delete gp1[i];
	for (unsigned int i = 0; i < gp2.size (); i++)
		delete gp2[i];
}

int GreinerHormann::boolop (Martinez::BoolOpType op, Polygon& result)
{
	if ((subject.ncontours () > 0 || clipping.ncontours () > 0) && op != Martinez::INTERSECTION)
		return -2; // the operation cannot be done

	// Test for trivial result cases
	if (subject.ncontours () * clipping.ncontours () == 0) { // At least one of the polygons is empty
		if (op == Martinez::DIFFERENCE)
			result = subject;
		if (op == Martinez::UNION)
			result = (subject.ncontours () == 0) ? clipping : subject;
		return 0;
	}
	Point minsubj, maxsubj, minclip, maxclip;
	subject.boundingbox (minsubj, maxsubj);
	clipping.boundingbox (minclip, maxclip);
	if (minsubj.x > maxclip.x || minclip.x > maxsubj.x || minsubj.y > maxclip.y || minclip.y > maxsubj.y) {
		// the bounding boxes do not overlap
		if (op == Martinez::DIFFERENCE)
			result = subject;
		if (op == Martinez::UNION) {
			result = subject;
			for (unsigned int i = 0; i < clipping.ncontours (); i++) {
				Contour& c = result.pushbackContour ();
				c = clipping.contour (i);
			}
		}
		return 0;
	}

	// Boolean operation is not trivial

	int nint = 0;
	for (unsigned int i = 0; i < gp1.size (); i++) {
		for (unsigned int j = 0; j < gp2.size (); j++) {
			gp1[i]->deleteIntersections ();
			gp2[j]->deleteIntersections ();
			int ni = boolop (op, *gp1[i], *gp2[j], result);
			if (ni == -1)
				return -1;
			nint += nint;
		}
	}
	return nint;
}

int GreinerHormann::boolop (Martinez::BoolOpType op, GreinerContour& gc1, GreinerContour& gc2, Polygon& result)
{
	// step 1
	if (!gc1.intersectBoundingbox (gc2))
		return 0;

	int nint = 0; // number of intersections
	Vertex *v1p = gc1.firstVertex ();
	do {
		if (!v1p->intersect) {
			Vertex *v2p = gc2.firstVertex ();
			do {
				if (!v2p->intersect) {
					Point inter;
					if (findIntersection (v1p->s, v2p->s, inter, inter) > 0) {
						if (inter == Point (v1p->x, v1p->y) || inter == Point (v2p->x, v2p->y)) {
							return -1; // the edge should be peturbed
						} else {
							nint++;
							double distTov1p = (v1p->x - inter.x) * (v1p->x - inter.x) + (v1p->y - inter.y) * (v1p->y - inter.y);
							double distTov2p = (v2p->x - inter.x) * (v2p->x - inter.x) + (v2p->y - inter.y) * (v2p->y - inter.y);
							Vertex *x, *y;
							for (x = v1p->next; x->alpha < distTov1p; x = x->next);
							for (y = v2p->next; y->alpha < distTov2p; y = y->next);
							Segment noused;
							Vertex *v1_address = gc1.insert (Vertex (inter.x, inter.y, noused, true, distTov1p), x);
							Vertex *v2_address = gc2.insert (Vertex (inter.x, inter.y, noused, true, distTov2p), y);
							v1_address->neighbor = v2_address;
							v2_address->neighbor = v1_address;
						}
					}
				}
				v2p = v2p->next;
			} while (v2p != gc2.firstVertex ());
		}
		v1p = v1p->next;
	} while (v1p != gc1.firstVertex ());

	if (nint == 0) { // Hay intersección si uno está incluido en el otro
		Point p (gc1.firstVertex ()->x, gc1.firstVertex ()->y);
		if (pointInPolygon (p, gc2)) {
			Contour& contour = result.pushbackContour ();
			Vertex *v = gc1.firstVertex ();
			do {
				contour.add (Point (v->x, v->y));
				v = v->next;
			} while (v != gc1.firstVertex ());
		}
		p = Point (gc2.firstVertex ()->x, gc2.firstVertex ()->y);
		if (pointInPolygon (p, gc1)) {
			Contour& contour = result.pushbackContour ();
			Vertex *v = gc2.firstVertex ();
			do {
				contour.add (Point (v->x, v->y));
				v = v->next;
			} while (v != gc2.firstVertex ());
		}
		return nint;
	}
	
	// step 2
	Point p (gc1.firstVertex ()->x, gc1.firstVertex ()->y);
	bool status = ! pointInPolygon (p, gc2);
	Vertex *v = gc1.firstVertex ();
	do {
		if (v->intersect) {
			v->entry = status;
			status = !status;
		}
		v = v->next;
	} while (v != gc1.firstVertex ());

	p = Point (gc2.firstVertex ()->x, gc2.firstVertex ()->y);
	status = ! pointInPolygon (p, gc1);
	v = gc2.firstVertex ();
	do {
		if (v->intersect) {
			v->entry = status;
			status = !status;
		}
		v = v->next;
	} while (v != gc2.firstVertex ());
	
	// step 3
	v = gc2.firstVertex ();
	do {
		if (v->intersect && !v->processed) {
			Contour& contour = result.pushbackContour ();
			contour.add (Point (v->x, v->y));
			double x = v->x;
			double y = v->y;
			Vertex *current = v;
			current->processed = true;
			bool runingPolygon2 = true; // needed for the difference operation
			do {
				if (current->entry) {
					do {
						switch (op) {
							case Martinez::INTERSECTION:
								current = current->next;
								break;
							case Martinez::UNION:
								current = current->prev;
								break;
							case Martinez::DIFFERENCE:
								current = (runingPolygon2) ? current->next : current->prev;
								break;
						}
						contour.add (Point (current->x, current->y));
					} while (!current->intersect);
				} else {
					do {
						switch (op) {
							case Martinez::INTERSECTION:
								current = current->prev;
								break;
							case Martinez::UNION:
								current = current->next;
								break;
							case Martinez::DIFFERENCE:
								current = (runingPolygon2) ? current->prev : current->next;
								break;
						}
						contour.add (Point (current->x, current->y));
					} while (!current->intersect);
				}
				current->processed = true;
				current = current->neighbor;
				current->processed = true;
				runingPolygon2 = !runingPolygon2;
				
			} while ((current->x != x) || (current->y != y));
			// delete the last point, that has been inserted twice
			Contour::iterator i = contour.end ();
			i--;
			contour.erase (i);
		}
		v = v->next;
	} while (v != gc2.firstVertex ());
	return nint;
}
