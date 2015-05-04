#include <GL/glut.h>
#include <GL/glu.h>
#include <limits>
#include <algorithm>
#include <cmath>
#include "polygon.h"
#include "utilities.h"
#include "martinez.h"
#include "connector.h"
#include "greiner.h"
#include "gpc.h"
#include "timer.h"

#ifndef CALLBACK
#define CALLBACK
#endif

void init ();
void reshape (int w, int h);
void display (void);
void keyboard (unsigned char key, int x, int y);
void specialFunc (int key, int x, int y);

Martinez::BoolOpType op = Martinez::INTERSECTION;
gpc_op opVatti = GPC_INT;
Polygon* subj;
Polygon* clip;
Polygon* result;
GLuint displayListSubject, displayListClipping, displayListResult;
bool showSubject = true;
bool showClipping = true;
bool showResult = false;
bool wireframe = false;
Point min1, max1, min2, max2, mini, maxi;
float width;
float height;
float zoom = 0.0f;
float xoffset = 0.0f;
float yoffset = 0.0f;
int xx, yy;

int main (int argc, char *argv[])
{
	glutInit (&argc, argv);

	if (argc < 4) {
		cerr << "Syntax: " << argv[0] << " subject_pol clipping_pol G|V|M [I|U|D|X]\n";
		return 1;
	}
	if (argv[3][0] != 'G' && argv[3][0] != 'V' && argv[3][0] != 'M') {
		cerr << "Syntax: " << argv[0] << " subject_pol clipping_pol G|V|M [I|U|D|X]\n";
		cerr << "The third parameter set the algorithm. It is a character. It can be G (Greiner), V (Vatti) or M (Martinez)\n";
		return 2;
	}

	if (argc > 4 && argv[4][0] != 'I' && argv[4][0] != 'U' && argv[4][0] != 'D' && argv[4][0] != 'X') {
		cerr << "Syntax: " << argv[0] << " subject_pol clipping_pol G|V|M [I|U|D|X]\n";
		cerr << "The last parameter is optional. It is a character. It can be I (Intersection), U (Union), D (Difference) or X (eXclusive or)\n";
		return 3;
	}
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

	subj = new Polygon (argv[1]);
	clip = new Polygon (argv[2]);
	result = new Polygon ();
	subj->boundingbox(min1, max1);
	clip->boundingbox(min2, max2);
	mini = Point (std::min (min1.x, min2.x), std::min (min1.y, min2.y));
	maxi = Point (std::max (max1.x, max2.x), std::max (max1.y, max2.y));
	width = (maxi.x - mini.x);
	height = (maxi.y - mini.y);

	switch (argv[3][0]) {
		case 'M': {
			Martinez mr (*subj, *clip);
			mr.compute (op, *result);
			break;}
		case 'G': {
			GreinerHormann gh (*subj, *clip);
			int GreinerResult = gh.boolop (op, *result);
			if (GreinerResult == -1) {
				cerr << "Sorry, the Greiner-Hormann's method needs perturbation, and it is not implemented." << endl;
				return 4;
			} else if (GreinerResult == -2) {
				cerr << "Sorry, the Greiner-Hormann's method cannot work with this operation and polygons with more than one region." << endl;
				return 5;
			}
			break; }
		case 'V':
			gpc_polygon subject, clipping, r;
			gpc_set_polygon (*subj, &subject);
			gpc_set_polygon (*clip, &clipping);
			gpc_polygon_clip (opVatti, &subject, &clipping, &r);
			for (int c = 0; c < r.num_contours; c++) {
				Contour& contour = result->pushbackContour ();
				for (int v = 0; v < r.contour[c].num_vertices; v++)
					contour.add (Point (r.contour[c].vertex[v].x, r.contour[c].vertex[v].y));
			}
			gpc_free_polygon (&subject);
			gpc_free_polygon (&clipping);
			gpc_free_polygon (&r);
			break;
	}
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (700, 700);
	glutInitWindowPosition (100, 100);
	glutCreateWindow (argv[0]);
	init ();
	glutDisplayFunc (display);
	glutKeyboardFunc (keyboard);
	glutSpecialFunc (specialFunc);
	glutReshapeFunc (reshape);
	glutMainLoop ();
	delete subj;
	delete clip;
	delete result;
	return 0;
}

void keyboard (unsigned char key, int, int)
{
	switch (key) {
		case 'R': // switch result polygon visualization
		case 'r':
			showResult = !showResult;
			glutPostRedisplay ();
			break;
		case 'S': // switch subject polygon visualization
		case 's':
			showSubject = !showSubject;
			glutPostRedisplay ();
			break;
		case 'C': // switch clipping polygon visualization
		case 'c':
			showClipping = !showClipping;
			glutPostRedisplay ();
			break;
		case 'W': // switch wireframe-filling visualization of polygons
		case 'w':
			wireframe = !wireframe;
			glutPostRedisplay ();
			break;
		case 'Z':
			if (zoom < 0.45)
				zoom += 0.05;
			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			gluOrtho2D (mini.x + width*zoom, maxi.x - width*zoom, mini.y + height*zoom, maxi.y - height*zoom);
			glutPostRedisplay ();
			break;
		case 'z':
			if (zoom > -0.45)
				zoom -= 0.05;
			glMatrixMode (GL_PROJECTION);
			glLoadIdentity ();
			gluOrtho2D (mini.x + width*zoom, maxi.x - width*zoom, mini.y + height*zoom, maxi.y - height*zoom);
			glutPostRedisplay ();
			break;
		case 'a':
			xoffset -= 0.05*width;
			glutPostRedisplay ();
			break;
		case 'd':
			xoffset += 0.05*width;
			glutPostRedisplay ();
			break;
		case 't':
			yoffset += 0.05*height;
			glutPostRedisplay ();
			break;
		case 'g':
			yoffset -= 0.05*height;
			glutPostRedisplay ();
			break;
	}
}

void specialFunc (int key, int, int)
{
	switch (key) {
		case GLUT_KEY_LEFT:
			xoffset -= 0.05*width;
			glutPostRedisplay ();
			break;
		case GLUT_KEY_RIGHT:
			xoffset += 0.05*width;
			glutPostRedisplay ();
			break;
		case GLUT_KEY_UP:
			yoffset += 0.05*height;
			glutPostRedisplay ();
			break;
		case GLUT_KEY_DOWN:
			yoffset -= 0.05*height;
			glutPostRedisplay ();
			break;
	}
}

void CALLBACK tessErrorCB (GLenum errno)
{
	cerr << "Tesselation error: " << gluErrorString (errno) << '\n';
}

void CALLBACK tessCombineCB (GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4], GLdouble** dataOut)
{
	*dataOut = new GLdouble[3];
	(*dataOut)[0] = coords[0];
	(*dataOut)[1] = coords[1];
	(*dataOut)[2] = coords[2];
}

void drawFilledPolygon (GLUtesselator* tObj, Polygon* p)
{
	int npoints = 0;
	for (Polygon::iterator i = p->begin (); i != p->end (); i++)
		npoints += i->nvertices ();
	GLdouble* vert = new GLdouble[npoints*3];
	int pp = 0; // number of points processed
	gluTessBeginPolygon (tObj, NULL);
	for (Polygon::iterator i = p->begin (); i != p->end (); i++) {
		gluTessBeginContour(tObj);
		for (Contour::iterator j = i->begin (); j != i->end (); j++) {
			vert[pp++] = j->x;
			vert[pp++] = j->y;
			vert[pp++] = 0.0;
			gluTessVertex (tObj, &vert[pp-3], &vert[pp-3]);
		}
		gluTessEndContour (tObj);
	} 
	gluTessEndPolygon (tObj);
	delete[] vert;
}

void init ()
{
	typedef GLvoid (*parameterlessCallbackType)();
	GLUtesselator* tObj = gluNewTess ();
	gluTessCallback (tObj, GLU_TESS_BEGIN, (parameterlessCallbackType) glBegin);
	gluTessCallback (tObj, GLU_TESS_END, (parameterlessCallbackType) glEnd);
	gluTessCallback (tObj, GLU_TESS_VERTEX, (parameterlessCallbackType) glVertex3dv);
	gluTessCallback (tObj, GLU_TESS_ERROR, (parameterlessCallbackType) tessErrorCB);
	gluTessCallback (tObj, GLU_TESS_COMBINE, (parameterlessCallbackType) tessCombineCB);

	displayListSubject = glGenLists (1);
	displayListClipping = glGenLists (1);
	displayListResult = glGenLists (1);
	glNewList (displayListSubject, GL_COMPILE);
		drawFilledPolygon (tObj, subj);
	glEndList ();
	glNewList (displayListClipping, GL_COMPILE);
		drawFilledPolygon (tObj, clip);
	glEndList ();
	glNewList (displayListResult, GL_COMPILE);
		drawFilledPolygon (tObj, result);
	glEndList ();

	glClearColor (1, 1, 1, 1);
	glColor3f (0, 0, 0);
	glPointSize (5);
	glLineWidth (2.0f);
	glEnable (GL_BLEND);
}

void reshape (int w, int h)
{
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (mini.x + width*zoom, maxi.x - width*zoom, mini.y + height*zoom, maxi.y - height*zoom);
	if (width/height > (float) w / h)
		glViewport (0, (h-static_cast<int>(w * height/width))/2, w, static_cast<int>(w * height/width));
	else
		glViewport ((w-static_cast<int>(h * width/height))/2, 0, static_cast<int>(h * width/height), h);
}

void drawPolygon (Polygon* p)
{
	if (wireframe) {
		// render the vertices
		glBegin (GL_POINTS);
		for (Polygon::iterator i = p->begin (); i != p->end (); i++)
			for (Contour::iterator j = i->begin (); j != i->end (); j++)
				glVertex2f (j->x, j->y);
		glEnd ();
		// render the edges
		for (Polygon::iterator i = p->begin (); i != p->end (); i++) {
			glBegin (GL_LINE_LOOP);
			for (Contour::iterator j = i->begin (); j != i->end (); j++)
				glVertex2f (j->x, j->y);
			glEnd ();
		}
		return;
	}

	// the polygon must be rendered filled
	if (p == subj)
		glCallList (displayListSubject);
	else if (p == clip)
		glCallList (displayListClipping);
	else if (p == result)
		glCallList (displayListResult);
}

void display (void)
{
	glClear (GL_COLOR_BUFFER_BIT);
	glBlendFunc (GL_ONE, GL_ZERO);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (xoffset, yoffset, 0.0f);
	if (showSubject) {
		glColor3f (0, 1, 0);
		drawPolygon (subj);
	}
	if (showClipping) {
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f (1, 0, 0, 0.33);
		drawPolygon (clip);
	}
	if (showResult) {
		glColor3f (0, 0, 1);
		drawPolygon (result);
	}
	glutSwapBuffers ();
	glFlush ();
}
