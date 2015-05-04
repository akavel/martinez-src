/* Copyright (c) 2011 Mahir Iqbal
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

package com.bayvakoof.polyclip
{
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	import com.bayvakoof.polyclip.sweepline.SweepEvent;
	import com.bayvakoof.polyclip.sweepline.EventQueue;
	import com.bayvakoof.polyclip.sweepline.SweepEventSet;
	
	import com.bayvakoof.polyclip.geom.Polygon;
	import com.bayvakoof.polyclip.geom.Segment;
	
	import com.bayvakoof.polyclip.Connector;
	import com.bayvakoof.polyclip.geom.Contour;
	
	/**
	 * This class contains methods for computing clipping operations on polygons. 
	 * It implements the algorithm for polygon intersection given by Francisco Martínez del Río.
	 * @see http://wwwdi.ujaen.es/~fmartin/bool_op.html
	 * @author Mahir Iqbal
	 */
	public class PolygonClipper
	{
		private var subject:Polygon;
		private var clipping:Polygon;
		
		private var eventQueue:EventQueue;
		
		private static const SUBJECT:int = 0;
		private static const CLIPPING:int = 1;
		
		public function PolygonClipper(subject:Polygon, clipping:Polygon)
		{
			this.subject = subject;
			this.clipping = clipping;
			
			eventQueue = new EventQueue();
		}
		
		/**
		 * Computes the polygon operation given by operation.
		 * @see PolygonOp for the operation codes.
		 * @param	operation	A value specifying which boolean operation to compute.
		 * @return	The resulting polygon from the specified clipping operation.
		 */
		public function compute(operation:int):Polygon
		{
			var result:Polygon;
			
			if (subject.contours.length * clipping.contours.length == 0)
			{
				if (operation == PolygonOp.DIFFERENCE)
					result = subject;
				else if (operation == PolygonOp.UNION)
					result = (subject.contours.length == 0) ? clipping : subject;
				return result;
			}
			
			var subjectBB:Rectangle = subject.boundingBox;
			var clippingBB:Rectangle = clipping.boundingBox;
			
			if (!subjectBB.intersects(clippingBB))
			{
				if (operation == PolygonOp.DIFFERENCE)
					result = subject;
				if (operation == PolygonOp.UNION)
				{
					result = subject;
					for each (var c:Contour in clipping.contours)
						result.addContour(c);
				}
				
				return result;
			}
			
			// Add each segment to the eventQueue, sorted from left to right.
			for each (var sCont:Contour in subject.contours)
				for (var pParse1:int = 0; pParse1 < sCont.points.length; pParse1++)
					processSegment(sCont.getSegment(pParse1), SUBJECT);
					
			for each (var cCont:Contour in clipping.contours)
				for (var pParse2:int = 0; pParse2 < cCont.points.length; pParse2++)
					processSegment(cCont.getSegment(pParse2), CLIPPING);
		
			var connector:Connector = new Connector();
			
			// This is the SweepLine. That is, we go through all the polygon edges
			// by sweeping from left to right.
			var S:SweepEventSet = new SweepEventSet();
			
			var e:SweepEvent;
			var MINMAX_X:Number = Math.min(subjectBB.right, clippingBB.right);
			
			var prev:SweepEvent, next:SweepEvent;
			
			while (!eventQueue.isEmpty())
			{
				prev = null;
				next = null;
				
				e = eventQueue.dequeue();
				
				if ((operation == PolygonOp.INTERSECTION && (e.p.x > MINMAX_X)) || (operation == PolygonOp.DIFFERENCE && e.p.x > subjectBB.right)) 
					return connector.toPolygon();
					
				if (operation == PolygonOp.UNION && (e.p.x > MINMAX_X))
				{
					if (!e.isLeft)
						connector.add(e.segment);
					
					while (!eventQueue.isEmpty())
					{
						e = eventQueue.dequeue();
						if (!e.isLeft)
							connector.add(e.segment);
					}
					
					return connector.toPolygon();
				}
				
				if (e.isLeft)
				{
					var pos:int = S.insert(e);
					
					prev = (pos > 0) ? S.eventSet[pos - 1] : null;
					next = (pos < S.eventSet.length - 1) ? S.eventSet[pos + 1] : null;
					
					if (!prev)
					{
						e.inside = e.inOut = false;
					} else if (prev.edgeType != EdgeType.NORMAL)
					{
						if (pos - 2 < 0)
						{
							// Not sure how to handle the case when pos - 2 < 0, but judging
							// from the C++ implementation this looks like how it should be handled.
							e.inside = e.inOut = false;
							if (prev.polygonType != e.polygonType)
								e.inside = true;
							else
								e.inOut = true;
						} else
						{						
							var prevTwo:SweepEvent = S.eventSet[pos - 2];						
							if (prev.polygonType == e.polygonType)
							{
								e.inOut = !prev.inOut;
								e.inside = !prevTwo.inOut;
							} else
							{
								e.inOut = !prevTwo.inOut;
								e.inside = !prev.inOut;
							}
						}
					} else if (e.polygonType == prev.polygonType)
					{
						e.inside = prev.inside;
						e.inOut = !prev.inOut;
					} else
					{
						e.inside = !prev.inOut;
						e.inOut = prev.inside;
					}
				
					if (next)
						possibleIntersection(e, next);
					
					if (prev)
						possibleIntersection(e, prev);
				} else
				{
					var otherPos:int = S.eventSet.indexOf(e.otherSE);
					
					if (otherPos != -1)
					{
						prev = (otherPos > 0) ? S.eventSet[otherPos - 1] : null;
						next = (otherPos < S.eventSet.length - 1) ? S.eventSet[otherPos + 1] : null;
					}
					
					switch (e.edgeType)
					{
						case EdgeType.NORMAL:
							switch (operation)
							{
								case (PolygonOp.INTERSECTION):
									if (e.otherSE.inside)
										connector.add(e.segment);
									break;
								case (PolygonOp.UNION):
									if (!e.otherSE.inside)
										connector.add(e.segment);
									break;
								case (PolygonOp.DIFFERENCE):
									if (((e.polygonType == SUBJECT) && (!e.otherSE.inside)) || (e.polygonType == CLIPPING && e.otherSE.inside))
										connector.add(e.segment);
									break;
							}
							break;
						case (EdgeType.SAME_TRANSITION):
							if (operation == PolygonOp.INTERSECTION || operation == PolygonOp.UNION)
								connector.add(e.segment);
							break;
						case (EdgeType.DIFFERENT_TRANSITION):
							if (operation == PolygonOp.DIFFERENCE)
								connector.add(e.segment);
							break;
					}
					
					if (otherPos != -1)
						S.remove(S.eventSet[otherPos]);
						
					if (next && prev)
						possibleIntersection(next, prev);				
				}
			}
			
			return connector.toPolygon();
		}
		
		private function findIntersection(seg0:Segment, seg1:Segment):Array
		{
			var pi0:Point = new Point();
			var pi1:Point = new Point();
			
			var p0:Point = seg0.start;
			var d0:Point = new Point(seg0.end.x - p0.x, seg0.end.y - p0.y);
			var p1:Point = seg1.start;
			var d1:Point = new Point(seg1.end.x - p1.x, seg1.end.y - p1.y);
			var sqrEpsilon:Number = 0.0000001; // Antes 0.001
			var E:Point = new Point(p1.x - p0.x, p1.y - p0.y);
			var kross:Number = d0.x * d1.y - d0.y * d1.x;
			var sqrKross:Number = kross * kross;
			var sqrLen0:Number = d0.length;
			var sqrLen1:Number = d1.length;

			if (sqrKross > sqrEpsilon * sqrLen0 * sqrLen1) {
				// lines of the segments are not parallel
				var s:Number = (E.x * d1.y - E.y * d1.x) / kross;
				if ((s < 0) || (s > 1)) {
					return [0, pi0, pi1];
				}
				var t:Number = (E.x * d0.y - E.y * d0.x) / kross;
				if ((t < 0) || (t > 1)) {
					return [0, pi0, pi1];
				}
				// intersection of lines is a point an each segment
				pi0.x = p0.x + s * d0.x;
				pi0.y = p0.y + s * d0.y;
				
				// Uncomment this and the block below if you're getting errors to do with precision.
				/*if (Point.distance(pi0,seg0.start) < 0.00000001) pi0 = seg0.start;
				if (Point.distance(pi0,seg0.end) < 0.00000001) pi0 = seg0.end;
				if (Point.distance(pi0,seg1.start) < 0.00000001) pi0 = seg1.start;
				if (Point.distance(pi0,seg1.end) < 0.00000001) pi0 = seg1.end;*/
				return [1, pi0, pi1];
			}
			
			// lines of the segments are parallel
			var sqrLenE:Number = E.length;
			kross = E.x * d0.y - E.y * d0.x;
			sqrKross = kross * kross;
			if (sqrKross > sqrEpsilon * sqrLen0 * sqrLenE) {
				// lines of the segment are different
				return [0, pi0, pi1];
			}
			
			// Lines of the segments are the same. Need to test for overlap of segments.
			var s0:Number = (d0.x * E.x + d0.y * E.y) / sqrLen0;  // so = Dot (D0, E) * sqrLen0
			var s1:Number = s0 + (d0.x * d1.x + d0.y * d1.y) / sqrLen0;  // s1 = s0 + Dot (D0, D1) * sqrLen0
			var smin:Number = Math.min(s0, s1);
			var smax:Number = Math.max(s0, s1);
			var w:Vector.<Number> = new Vector.<Number>();
			var imax:int = findIntersection2(0.0, 1.0, smin, smax, w);
			
			if (imax > 0) {
				pi0.x = p0.x + w[0] * d0.x;
				pi0.y = p0.y + w[0] * d0.y;
				/*if (Point.distance(pi0,seg0.start) < 0.00000001) pi0 = seg0.start;
				if (Point.distance(pi0,seg0.end) < 0.00000001) pi0 = seg0.end;
				if (Point.distance(pi0,seg1.start) < 0.00000001) pi0 = seg1.start;
				if (Point.distance(pi0,seg1.end) < 0.00000001) pi0 = seg1.end;*/
				if (imax > 1) {
					pi1.x = p0.x + w[1] * d0.x;
					pi1.y = p0.y + w[1] * d0.y;
				}
			}
			
			return [imax, pi0, pi1];
		}
		
		private function findIntersection2(u0:Number, u1:Number, v0:Number, v1:Number, w:Vector.<Number>):int
		{
			if ((u1 < v0) || (u0 > v1))
				return 0;
			if (u1 > v0) {
				if (u0 < v1) {
					w[0] = (u0 < v0) ? v0 : u0;
					w[1] = (u1 > v1) ? v1 : u1;
					return 2;
				} else {
					// u0 == v1
					w[0] = u0;
					return 1;
				}
			} else {
				// u1 == v0
				w[0] = u1;
				return 1;
			}
		}
		
		private function sec(e1:SweepEvent, e2:SweepEvent):Boolean
		{
			if (e1.p.x > e2.p.x) // Different x coordinate
				return true;
			if (e2.p.x > e1.p.x) // Different x coordinate
				return false;
			if (!e1.p.equals(e2.p)) // Different points, but same x coordinate. The event with lower y coordinate is processed first
				return e1.p.y > e2.p.y;
			if (e1.isLeft != e2.isLeft) // Same point, but one is a left endpoint and the other a right endpoint. The right endpoint is processed first
				return e1.isLeft;
			// Same point, both events are left endpoints or both are right endpoints. The event associate to the bottom segment is processed first
			return e1.isAbove(e2.otherSE.p);
		}
		
		private function possibleIntersection(e1:SweepEvent, e2:SweepEvent):void
		{
			//	if ((e1->pl == e2->pl) ) // Uncomment these two lines if self-intersecting polygons are not allowed
			//		return false;
			
			var ip1:Point, ip2:Point;
			var numIntersections:int
			
			var intData:Array = findIntersection(e1.segment, e2.segment);
			numIntersections = intData[0] as int;
			ip1 = intData[1] as Point;
			ip2 = intData[2] as Point;
			
			if (numIntersections == 0)
				return;
				
			if ((numIntersections == 1) && (e1.p.equals(e2.p) || e1.otherSE.p.equals(e2.otherSE.p)))
				return;
				
			if (numIntersections == 2 && e1.p.equals(e2.p))
				return;
				
			if (numIntersections == 1)
			{
				if (!e1.p.equals(ip1) && !e1.otherSE.p.equals(ip1))
					divideSegment (e1, ip1);
				if (!e2.p.equals(ip1) && !e2.otherSE.p.equals(ip1))
					divideSegment (e2, ip1);
				return;
			}
			
			var sortedEvents:Vector.<SweepEvent> = new Vector.<SweepEvent>();
			if (e1.p.equals(e2.p))
			{
				sortedEvents.push(null); // WTF
			} else if (sec(e1, e2))
			{
				sortedEvents.push(e2);
				sortedEvents.push(e1);
			} else
			{
				sortedEvents.push(e1);
				sortedEvents.push(e2);
			}
			
			if ( e1.otherSE.p.equals(e2.otherSE.p))
			{
				sortedEvents.push(null);
			} else if (sec(e1.otherSE, e2.otherSE))
			{
				sortedEvents.push(e2.otherSE);
				sortedEvents.push(e1.otherSE);
			} else
			{
				sortedEvents.push(e1.otherSE);
				sortedEvents.push(e2.otherSE);
			}
			
			if (sortedEvents.length == 2)
			{
				e1.edgeType = e1.otherSE.edgeType = EdgeType.NON_CONTRIBUTING;
				e2.edgeType = e2.otherSE.edgeType = ((e1.inOut == e2.inOut) ? EdgeType.SAME_TRANSITION : EdgeType.DIFFERENT_TRANSITION);
				return;
			}
			
			if (sortedEvents.length == 3) 
			{
				sortedEvents[1].edgeType = sortedEvents[1].otherSE.edgeType = EdgeType.NON_CONTRIBUTING;
				if (sortedEvents[0])         // is the right endpoint the shared point?
					sortedEvents[0].otherSE.edgeType = (e1.inOut == e2.inOut) ? EdgeType.SAME_TRANSITION : EdgeType.DIFFERENT_TRANSITION;
				else 								// the shared point is the left endpoint
					sortedEvents[2].otherSE.edgeType = (e1.inOut == e2.inOut) ? EdgeType.SAME_TRANSITION : EdgeType.DIFFERENT_TRANSITION;
				divideSegment (sortedEvents[0] ? sortedEvents[0] : sortedEvents[2].otherSE, sortedEvents[1].p);
				return;
			}
			
			if (sortedEvents[0] != sortedEvents[3].otherSE)
			{ // no segment includes totally the otherSE one
				sortedEvents[1].edgeType = EdgeType.NON_CONTRIBUTING;
				sortedEvents[2].edgeType = (e1.inOut == e2.inOut) ? EdgeType.SAME_TRANSITION : EdgeType.DIFFERENT_TRANSITION;
				divideSegment (sortedEvents[0], sortedEvents[1].p);
				divideSegment (sortedEvents[1], sortedEvents[2].p);
				return;
			}
			
			sortedEvents[1].edgeType = sortedEvents[1].otherSE.edgeType = EdgeType.NON_CONTRIBUTING;
			divideSegment (sortedEvents[0], sortedEvents[1].p);
			sortedEvents[3].otherSE.edgeType = (e1.inOut == e2.inOut) ? EdgeType.SAME_TRANSITION : EdgeType.DIFFERENT_TRANSITION;
			divideSegment (sortedEvents[3].otherSE, sortedEvents[2].p);
		}
		
		private function divideSegment(e:SweepEvent, p:Point):void
		{
			var r:SweepEvent = new SweepEvent(p, false, e.polygonType, e, e.edgeType);
			var l:SweepEvent = new SweepEvent(p, true, e.polygonType, e.otherSE, e.otherSE.edgeType);
			
			if (sec(l, e.otherSE))
			{
				e.otherSE.isLeft = true;
				e.isLeft = false;
			}
			
			e.otherSE.otherSE = l;
			e.otherSE = r;
			
			eventQueue.enqueue(l);
			eventQueue.enqueue(r);
		}
		
		private function processSegment(segment:Segment, polyType:int)
		{
			if (segment.start.equals(segment.end)) // Possible degenerate condition.
				return;
			
			var e1:SweepEvent = new SweepEvent(segment.start, true, polyType);
			var e2:SweepEvent = new SweepEvent(segment.end, true, polyType, e1);
			e1.otherSE = e2;
			
			if (e1.p.x < e2.p.x) {
				e2.isLeft = false;
			} else if (e1.p.x > e2.p.x) {
				e1.isLeft = false;
			} else if (e1.p.y < e2.p.y) { // the segment isLeft vertical. The bottom endpoint isLeft the isLeft endpoint 
				e2.isLeft = false;
			} else {
				e1.isLeft = false;
			}
			
			// Pushing it so the que is sorted from left to right, with object on the left
			// having the highest priority.
			eventQueue.enqueue(e1);
			eventQueue.enqueue(e2);
		}
	}
	
}