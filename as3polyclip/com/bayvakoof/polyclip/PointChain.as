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
	import com.bayvakoof.polyclip.geom.Segment;
	
	/**
	 * Represents a connected sequence of segments. The sequence can only be extended by connecting
	 * new segments that share an endpoint with the PointChain.
	 * @author Mahir Iqbal
	 */
	public class PointChain 
	{
		public var closed:Boolean;
		public var pointList:Vector.<Point>;
		
		public function PointChain(s:Segment)
		{
			pointList = new Vector.<Point>();
			pointList.push(s.start);
			pointList.push(s.end);
			closed = false;
		}
		
		// Links a segment to the pointChain
		public function linkSegment(s:Segment):Boolean
		{				
			var front:Point = pointList[0];
			var back:Point = pointList[pointList.length - 1];
			
			if (s.start.equals(front))
			{
				if (s.end.equals(back))
					closed = true;
				else
					pointList.unshift(s.end);
					
				return true;
			} else if (s.end.equals(back))
			{
				if (s.start.equals(front))
					closed = true;
				else
					pointList.push(s.start);
					
				return true;
			} else if (s.end.equals(front))
			{
				if (s.start.equals(back))
					closed = true;
				else
					pointList.unshift(s.start);
					
				return true;
			} else if (s.start.equals(back)) {
				if (s.end.equals(front))
					closed = true;
				else
					pointList.push(s.end);
					
				return true;
			}
			
			return false;
		}
		
		// Links another pointChain onto this point chain.
		public function linkPointChain(chain:PointChain):Boolean
		{	
			var firstPoint:Point = pointList[0];
			var lastPoint:Point = pointList[pointList.length - 1];
			
			var chainFront:Point = chain.pointList[0];
			var chainBack:Point = chain.pointList[chain.pointList.length - 1];
			
			if (chainFront.equals(lastPoint)) {
				pointList.pop();
				pointList = pointList.concat(chain.pointList);
				return true;
			}
			
			if (chainBack.equals(firstPoint)) {
				pointList.shift(); // Remove the first element, and join this list to chain.pointList.
				pointList = chain.pointList.concat(pointList);
				return true;
			}
			
			if (chainFront.equals(firstPoint)) {
				pointList.shift(); // Remove the first element, and join to reversed chain.pointList
				var reversedChainList:Vector.<Point> = chain.pointList.reverse(); // Don't need chain so can ruin it
				pointList = reversedChainList.concat(pointList);
				return true;
			}
			
			if (chainBack.equals(lastPoint)) {
				pointList.pop();
				pointList.reverse();
				pointList = chain.pointList.concat(pointList);
				return true;
			}
			
			return false;
		}
	}
	
}