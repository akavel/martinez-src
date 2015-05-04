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

package com.bayvakoof.polyclip.geom
{
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	/**
	 * These are simple polgyons. They contain a sequence of connected points.
	 * These also represent holes inside (complex) Polygons. The algorithm does not require
	 * the knowledge of whether a Contour is a hole.
	 * @author Mahir Iqbal
	 */
	public class Contour 
	{
		public var points:Vector.<Point>;
		public var bounds:Rectangle;
		
		public function Contour()
		{
			points = new Vector.<Point>();
		}
		
		public function add(p:Point):void
		{
			points.push(p);
		}
		
		public function get boundingBox():Rectangle
		{
			if (bounds)
				return bounds;
				
			var minX:Number = Number.POSITIVE_INFINITY, minY:Number = Number.POSITIVE_INFINITY;
			var maxX:Number = Number.NEGATIVE_INFINITY, maxY:Number = Number.NEGATIVE_INFINITY;
			
			for each (var p:Point in points)
			{
				if (p.x > maxX)
					maxX = p.x;
				if (p.x < minX)
					minX = p.x;
				if (p.y > maxY)
					maxY = p.y;
				if (p.y < minY)
					minY = p.y;
			}
			
			return bounds = new Rectangle(minX, minY, maxX - minX, maxY - minY);
		}
		
		public function getSegment(index:int):Segment
		{
			if (index == points.length - 1)
				return new Segment(points[points.length - 1], points[0]);
			
			return new Segment(points[index], points[index + 1]);
		}
		
		/**
		 * Checks if a point is inside a contour using the point in polygon raycast method.
		 * This works for all polygons, whether they are clockwise or counter clockwise,
		 * convex or concave.
		 * @see 	http://en.wikipedia.org/wiki/Point_in_polygon#Ray_casting_algorithm
		 * @param	p
		 * @param	contour
		 * @return	True if p is inside the polygon defined by contour
		 */
		public function containsPoint(p:Point):Boolean
		{
			// Cast ray from p.x towards the right
			var intersections:int = 0;
			for (var i:int = 0; i < points.length; i++)
			{
				var curr:Point = points[i];
				var next:Point = (i == points.length - 1) ? points[0] : points[i + 1];
				
				// Edge is from curr to next.
				if ((p.y < next.y && p.y > curr.y) || (p.y < curr.y && p.y > next.y))
				{
					if (p.x < Math.max(curr.x, next.x) && next.y != curr.y)
					{
						// Find where the line intersects...
						var xInt:Number = (p.y - curr.y) * (next.x - curr.x) / (next.y - curr.y) + curr.x;
						if (curr.x == next.x || p.x <= xInt)
							intersections++;
					}
				}
			}
			
			if (intersections % 2 == 0)
				return false;
			else
				return true;			
		}
	}
	
}