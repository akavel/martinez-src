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
	/**
	 * EventQueue data structure.
	 * @author Mahir Iqbal
	 */
	public class EventQueue 
	{
		private var elements:Vector.<SweepEvent>;
		private var sorted:Boolean = false;
		
		public function EventQueue()
		{
			elements = new Vector.<SweepEvent>(); 
			sorted = false;
		}
		
		// If already sorted use insertionSort on the inserted item.
		public function enqueue(obj:SweepEvent):void
		{
			if (sorted)
			{
				var length:int = elements.length;
				if (length == 0)
					elements.push(obj);
				
				elements.push(null); // Expand the Vector by one.
				
				var i:int = length - 1;
				while (i >= 0 && compareSweepEvent(obj, elements[i]) == -1)
				{
					elements[i + 1] = elements[i];
					i--;
				}
				elements[i + 1] = obj;
			} else
			{
				elements.push(obj);
			}
		}
		
		// IMPORTANT NOTE: This is not the same as the function in Sweepelements.
		// The ordering is reversed because push and pop are faster.
		private function compareSweepEvent(e1:SweepEvent, e2:SweepEvent):Number
		{
			if (e1.p.x > e2.p.x) // Different x coordinate
				return -1;
				
			if (e2.p.x > e1.p.x) // Different x coordinate
				return 1;
				
			if (!e1.p.equals(e2.p)) // Different points, but same x coordinate. The event with lower y coordinate is processed first
				return (e1.p.y > e2.p.y) ? -1 : 1;
				
			if (e1.isLeft != e2.isLeft) // Same point, but one is a left endpoint and the other a right endpoint. The right endpoint is processed first
				return (e1.isLeft) ? -1 : 1;
				
			// Same point, both events are left endpoints or both are right endpoints. The event associate to the bottom segment is processed first
			return e1.isAbove(e2.otherSE.p) ? -1 : 1;
		}
		
		public function dequeue():SweepEvent
		{
			if (!sorted)
			{
				sorted = true;
				elements.sort(compareSweepEvent);
			}
			
			return elements.pop();
		}		
		
		public function isEmpty():Boolean
		{
			return elements.length == 0;
		}
	}
	
}