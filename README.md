# C & ActionScript3 implementations of Martínez polygon clipping algorithm

This repository contains mirrored sources of two implementations
of a polygon clipping algorithm, published in 2009 by Martínez et al.
Note: a "polygon clipping" algorithm effectively allows for a full
set of boolean operations on polygons, such as addition, difference,
union and intersection.

## Details

The code in `cageo141/` is the **original C code** from
Martínez et al., as published on the original microsite about the 
algorithm, by authors. From the comments in the code, it looks 
released to **Public Domain**.

The code in `as3polyclip/` is an **ActionScript3 port** of the code,
by Mahir Iqbal. According to the comments in the code,
**MIT-licensed**.

The files in `polygons/` are some datafiles which were distributed
together with the C code.

The `paper.pdf` file is the **2009 paper** (describing the algorithm),
as it was published on the microsite. *Please note, that Martínez
apparently published [a new paper with a new version of the 
algorithm, in 2013](http://www.sciencedirect.com/science/article/pii/S0965997813000379);
the code here is of the old, 2009 algorithm. The new one seems
behind a paywall as of now.*

My attempt at a **Go port** of the 2009 algorithm is available at:
http://github.com/akavel/polyclip-go (the code is based on the AS3
port, and thus MIT-licensed as well).
