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

package com.bayvakoof.polyclip.sweepline
{
	import flash.geom.Point;
	import com.bayvakoof.polyclip.geom.Segment;
	
	/**
	 * A container for SweepEvent data. A SweepEvent represents a location of interest (vertex between two polygon edges)
	 * as the sweep line passes through the polygons.
	 * @author Mahir Iqbal
	 */
	public class SweepEvent 
	{		
		public var p:Point;
		public var isLeft:Boolean; 		// Is the point the left endpoint of the segment (p, other->p)?
		public var polygonType:int; 		// PolygonType to which this event belongs to: either PolygonClipper.SUBJECT, or PolygonClipper.CLIPPING
		public var otherSE:SweepEvent; 	// Event associated to the other endpoint of the segment
		
		/* Does the segment (p, other->p) represent an inside-outside transition
		 * in the polygon for a vertical ray from (p.x, -infinite) that crosses the segment? 
		 */
		public var inOut:Boolean;
		public var edgeType:int; 			// The EdgeType. @see EdgeType.as
		
		public var inside:Boolean; 		// Only used in "left" events. Is the segment (p, other->p) inside the other polygon?
		
		public function SweepEvent(p:Point, isLeft:Boolean, polyType:int, otherSweepEvent:SweepEvent = null, edgeType:int = 0)
		{
			this.p = p;
			this.isLeft = isLeft;
			polygonType = polyType;
			otherSE = otherSweepEvent;
			this.edgeType = edgeType;
		}
		
		public function get segment():Segment
		{
			return new Segment(p, otherSE.p);
		}
		
		// Checks if this sweep event is below point p.
		public function isBelow(x:Point):Boolean
		{
			return (isLeft) ? (signedArea(p, otherSE.p, x) > 0) : (signedArea(otherSE.p, p, x) > 0);
			
			function signedArea(p0:Point, p1:Point, p2:Point):Number
			{
				return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
			}
		}
		
		public function isAbove(x:Point):Boolean
		{
			return !isBelow(x);
		}
	}
	
}