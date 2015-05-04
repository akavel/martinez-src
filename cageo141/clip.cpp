#include "polygon.h"
#include "utilities.h"
#include "martinez.h"
#include "connector.h"
#include "greiner.h"
#include "gpc.h"
#include "timer.h"
#include <fstream>

using namespace std;


int main (int argc, char* argv[])
{
	if (argc < 4) {
		cerr << "Syntax: " << argv[0] << " subject_pol clipping_pol result_pol [I|U|D|X]\n";
		return 1;
	}
	if (argc > 4 && argv[4][0] != 'I' && argv[4][0] != 'U' && argv[4][0] != 'D' && argv[4][0] != 'X') {
		cerr << "Syntax: " << argv[0] << " subject_pol clipping_pol result_pol [I|U|D|X]\n";
		cerr << "The last parameter is optional. It is a character. It can be I (Intersection), U (Union), D (Difference) or X (eXclusive or)\n";
		return 2;
	}
	Martinez::BoolOpType op = Martinez::INTERSECTION;
	gpc_op opVatti = GPC_INT;
	if (argc > 4) {
		switch (argv[4][0]) {
			case 'I':
				op = Martinez::INTERSECTION;
				opVatti = GPC_INT;
				break;
			case 'U':
				op = Martinez::UNION;
				opVatti = GPC_UNION;
				break;
			case 'D':
				op = Martinez::DIFFERENCE;
				opVatti = GPC_DIFF;
				break;
			case 'X':
				op = Martinez::XOR;
				opVatti = GPC_XOR;
				break;
		}
	}

	int ntests = 0; // number of tests
	Polygon subj (argv[1]);
	Polygon clip (argv[2]);
	Polygon martinezResult;
	Timer timer;
	int GreinerResult;
	float Martacum = 0;
	float Greineracum = 0;
	float Vattiacum = 0;
	while (Martacum < 1.0f && Greineracum < 1.0f && Vattiacum < 1.0f) {
		ntests++;
		martinezResult.clear ();
		// Martínez-Rueda's algorithm
		timer.start ();
		Martinez mr (subj, clip);
		mr.compute (op, martinezResult);
		timer.stop ();
		Martacum += timer.timeSecs();
		// Greiner-Hormann's algorithm
		Polygon greinerResult;
		timer.start ();
		GreinerHormann gh (subj, clip);
		GreinerResult = gh.boolop (op, greinerResult);
		timer.stop ();
		Greineracum += timer.timeSecs();
		// Vatti's algorithm
		gpc_polygon subject, clipping, result;
		gpc_set_polygon (subj, &subject);
		gpc_set_polygon (clip, &clipping);
		timer.start ();
		gpc_polygon_clip (opVatti, &subject, &clipping, &result);
		timer.stop ();
		Vattiacum += timer.timeSecs();
		gpc_free_polygon (&subject);
		gpc_free_polygon (&clipping);
		gpc_free_polygon (&result);
	}
	cout << "Martínez-Rueda's time: " << Martacum / ntests << endl;
	cout << "Vatti's time: " << Vattiacum / ntests << endl;
	switch (GreinerResult) {
		case -1:
			cout << "Sorry, the Greiner-Hormann's method needs perturbation, and it is not implemented." << endl;
			break;
		case -2:
			cout << "Sorry, the Greiner-Hormann's method cannot work with this operation and polygons with more than one region." << endl;
			break;
		default:
			cout << "Greiner-Hormann's time: " << Greineracum / ntests << endl;
			break;
	}
	cout << "Possible intersections: " << subj.nvertices () << " x " << clip.nvertices () << " = " << subj.nvertices()*clip.nvertices() << endl;
	cout << "Number of tests: " << ntests << endl;
	ofstream f (argv[3]);
	if (!f) 
		cerr << "can't open " << argv[3] << '\n';
	else
		f << martinezResult;
	return 0;
}

