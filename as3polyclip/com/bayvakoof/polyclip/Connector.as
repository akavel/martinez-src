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
	import com.bayvakoof.polyclip.geom.Contour;
	import com.bayvakoof.polyclip.geom.Polygon;
	
	/**
	 * Holds intermediate results (PointChains) of the clipping operation and forms them into
	 * the final polygon.
	 * @author Mahir Iqbal
	 */
	public class Connector 
	{
		private var openPolygons:Vector.<PointChain>;
		private var closedPolygons:Vector.<PointChain>;
		
		public function Connector()
		{
			openPolygons = new Vector.<PointChain>();
			closedPolygons = new Vector.<PointChain>();
		}
		
		public function add(s:Segment):void
		{
			// j iterates through the openPolygon chains.
			for (var j:int = 0; j < openPolygons.length; j++)
			{
				var chain:PointChain = openPolygons[j];
				if (chain.linkSegment(s))
				{
					if (chain.closed)
					{
						if (chain.pointList.length == 2)
						{
							// We tried linking the same segment (but flipped end and start) to 
							// a chain. (i.e. chain was <p0, p1>, we tried linking Segment(p1, p0)
							// so the chain was closed illegally.
							chain.closed = false;
							return;
						}
						
						openPolygons.splice(j, 1);
						closedPolygons.push(chain);
					} else
					{
						for (var i:int = j + 1; i < openPolygons.length; i++)
						{
							// Try to connect this open link to the rest of the chains. 
							// We won't be able to connect this to any of the chains preceding this one
							// because we know that linkSegment failed on those.
							if (chain.linkPointChain(openPolygons[i]))
							{
								openPolygons.splice(i, 1);
								break;
							}
						}
					}
					
					return;
				}
			}
			
			var newChain:PointChain = new PointChain(s);
			openPolygons.push(newChain);	
		}
		
		public function toPolygon():Polygon
		{
			var polygon:Polygon = new Polygon();
			for each (var pointChain:PointChain in closedPolygons)
			{
				/*if (pointChain.pointList.length == 2)
				{
					// Invalid contour...
					throw new Error("Invalid contour");
				}*/				
				
				var c:Contour = new Contour();
				for each (var p:Point in pointChain.pointList)
					c.add(p);
				
				polygon.addContour(c);
			}
			return polygon;
		}
	}	
}