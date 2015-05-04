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
	
	/**
	 * This is the data structure that simulates the SweepLine as it parses through
	 * EventQueue, which holds the events sorted from left to right (x-coordinate).
	 * @author Mahir Iqbal
	 */
	public class SweepEventSet
	{
		public var eventSet:Vector.<SweepEvent>;
		
		public function SweepEventSet()
		{
			eventSet = new Vector.<SweepEvent>();
		}
		
		public function remove(key:SweepEvent):void
		{
			var keyIndex:int = eventSet.indexOf(key);
			if (keyIndex == -1)
				return;
			
			eventSet.splice(keyIndex, 1);
		}
		
		public function insert(item:SweepEvent):int
		{
			var length:int = eventSet.length;
			if (length == 0)
			{
				eventSet.push(item);
				return 0;
			}
			
			eventSet.push(null); // Expand the Vector by one.
			
			var i:int = length - 1;
			while (i >= 0 && segmentCompare(item, eventSet[i]))// reverseSC(eventSet[i], item) == 1)
			{
				eventSet[i + 1] = eventSet[i];
				i--;
			}
			eventSet[i + 1] = item;
			return i + 1;
			
			// Actual insertion sort
			/*for (var j:int = 1; j < eventSet.length; j++)
			{
				var key:SweepEvent = eventSet[j];
				var i:int = j - 1;
				while (i >= 0 && reverseSC(eventSet[i], key) == 1)
				{
					eventSet[i + 1] = eventSet[i];
					i--
				}
				eventSet[i + 1] = key;
			}*/
		}
		
		private function segmentCompare(e1:SweepEvent, e2:SweepEvent):Boolean
		{
			if (e1 == e2)
				return false;
			
			if (signedArea(e1.p, e1.otherSE.p, e2.p) != 0 || signedArea(e1.p, e1.otherSE.p, e2.otherSE.p) != 0)
			{
				if (e1.p.equals(e2.p))
					return e1.isBelow(e2.otherSE.p);
				
				if (compareSweepEvent(e1, e2))
					return e2.isAbove(e1.p);
				
				return e1.isBelow(e2.p);
			}
			
			if (e1.p.equals(e2.p)) // Segments colinear
				return false;
			
			return compareSweepEvent(e1, e2);
				
			function signedArea(p0:Point, p1:Point, p2:Point):Number
			{
				return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
			}
		}
		
		// Should only be called by segmentCompare
		private function compareSweepEvent(e1:SweepEvent, e2:SweepEvent):Boolean
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
	}
	
}