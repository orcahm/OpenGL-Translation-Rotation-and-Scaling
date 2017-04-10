//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "Angel.h"
#include "glui.h"

#define XAXIS 1
#define YAXIS 2
#define ZAXIS 3
#define SMALLXAXIS 4
#define SMALLYAXIS 5
#define SMALLZAXIS 6
#define STARTSPEED 7
#define STOPSPEED 8
#define STARTANIMATION 9
#define STOPANIMATION 10
#define ORBITSTARTSPEED 11
#define ORBITSTOPSPEED 12
#define OWNSTARTSPEED 13
#define OWNSTOPSPEED 14

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
GLUI *glui;
int   main_window;
GLuint vao[2];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.2, -0.2,  0.2, 1.0),
	point4(-0.2,  0.2,  0.2, 1.0),
	point4(0.2,  0.2,  0.2, 1.0),
	point4(0.2, -0.2,  0.2, 1.0),
	point4(-0.2, -0.2, -0.2, 1.0),
	point4(-0.2,  0.2, -0.2, 1.0),
	point4(0.2,  0.2, -0.2, 1.0),
	point4(0.2, -0.2, -0.2, 1.0)
};

// RGBA olors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

mat4 smallScale = mat4(vec4(-0.5, 0.0, 0.0, 0.0),
						vec4( 0.0,-0.5, 0.0, 0.0),
						vec4( 0.0, 0.0,-0.5, 0.0),
						vec4( 0.0, 0.0, -0.0, 1.0));

mat4 smalltraslation = mat4(vec4(1.0, 0.0, 0.0, 0.0),
							vec4(0.0, 1.0, 0.0, 0.0),
							vec4(0.0, 0.0, 1.0, 0.0),
							vec4(0.75, 0.0, 0.0, 1.0));

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
int smallAxis = Zaxis;
int controlTime = 1;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint  theta;  // The location of the "theta" shader uniform variable

GLfloat  ThetaS[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint  thetaS;

GLfloat  ThetaOrbit[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint  thetaOrbit;

GLfloat centerScale = -0.1;
GLuint scale;

GLint sec = 10;
GLuint time_lock;

int speed = 5;
int smallspeed = 5;
int orbitspeed = 5;

GLuint programbig;
GLuint programsmall;
GLuint myScale;
GLuint myTrans;
				//----------------------------------------------------------------------------

				// quad generates two triangles for each face and assigns colors
				//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object
	
	glGenVertexArrays(2, vao);
	glBindVertexArray(vao[0]);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	programbig = InitShader("vshaderbig.glsl", "fshader.glsl");
	glUseProgram(programbig);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(programbig, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(programbig, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	theta = glGetUniformLocation(programbig, "theta");
	scale = glGetUniformLocation(programbig, "scale");
	time_lock = glGetUniformLocation(programbig, "time");

	glBindVertexArray(vao[1]);

	// Create and initialize a buffer object
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	programsmall = InitShader("vshadersmall.glsl", "fshader.glsl");
	glUseProgram(programsmall);

	// set up vertex arrays
	GLuint vPositionS = glGetAttribLocation(programsmall, "vPosition");
	glEnableVertexAttribArray(vPositionS);
	glVertexAttribPointer(vPositionS, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColorS = glGetAttribLocation(programsmall, "vColor");
	glEnableVertexAttribArray(vColorS);
	glVertexAttribPointer(vColorS, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	myScale = glGetUniformLocation(programsmall, "smallScale");
	myTrans = glGetUniformLocation(programsmall, "smalltraslation");
	thetaS = glGetUniformLocation(programsmall, "theta");
	thetaOrbit = glGetUniformLocation(programsmall, "orbit");
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programsmall);
	glUniform3fv(thetaS, 1, ThetaS);
	glUniform3fv(thetaOrbit, 1, ThetaOrbit);
	glUniformMatrix4fv(myScale, 1, GLU_TRUE ,smallScale);
	glUniformMatrix4fv(myTrans, 1, GL_FALSE, smalltraslation);
	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glUseProgram(programbig);
	glUniform3fv(theta, 1, Theta);
	glUniform1f(scale, centerScale);
	glUniform1i(time_lock, sec);
	
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void myTimer(int fps)     // timer function to update time
{	
	if (controlTime == 1) {
		sec += 1;			// increment second
		if (sec == 150) {
			controlTime = -1;
		}
	}
	else if (controlTime == -1) {
		sec -= 1;			// increment second
		if (sec == 50) {
			controlTime = 1;
		}
	}
	else if (controlTime == 0) {
		sec = sec;
	}

	glutPostRedisplay();
	glutTimerFunc(10, myTimer, 1);  //repost timer 
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
		case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
		case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
		}
	}
}

//----------------------------------------------------------------------------

void
idle(void)
{
	Theta[Axis] += 0.01*speed;

	if (Theta[Axis] > 360.0) {
		Theta[Axis] -= 360.0;
	}

	ThetaS[smallAxis] += 0.01*smallspeed;

	if (ThetaS[smallAxis] > 360.0) {
		ThetaS[smallAxis] -= 360.0;
	}

	ThetaOrbit[Zaxis] += 0.0002*orbitspeed;

	if (ThetaOrbit[Zaxis] > 360.0) {
		ThetaOrbit[Zaxis] -= 360.0;
	}
	glutSetWindow(main_window);
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void myGlutReshape(int x, int y)
{
	float xy_aspect;

	xy_aspect = (float)x / (float)y;
	GLUI_Master.auto_set_viewport();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xy_aspect*.08, xy_aspect*.08, -.08, .08, .1, 15.0);

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void control_cb(int control)
{
	if (control == XAXIS) {
		Axis = Xaxis;
	}
	else if (control == YAXIS) {
		Axis = Yaxis;
	}
	else if (control == ZAXIS) {
		Axis = Zaxis;
	}
	else if (control == SMALLXAXIS) {
		smallAxis = Xaxis;
	}
	else if (control == SMALLYAXIS) {
		smallAxis = Yaxis;
	}
	else if (control == SMALLZAXIS) {
		smallAxis = Zaxis;
	}
	else if (control == STARTSPEED) {
		speed = 5;
	}
	else if (control == STOPSPEED) {
		speed = 0;
	}
	else if (control == STARTANIMATION) {
		speed = 5;
		smallspeed = 5;
		orbitspeed = 5;
		controlTime = 1;
	}
	else if (control == STOPANIMATION) {
		speed = 0;
		smallspeed = 0;
		orbitspeed = 0;
		controlTime = 0;
	}
	else if (control == OWNSTARTSPEED) {
		smallspeed = 5;
	}
	else if (control == OWNSTOPSPEED) {
		smallspeed = 0;
	}
	else if (control == ORBITSTARTSPEED) {
		orbitspeed = 5;
	}
	else if (control == ORBITSTOPSPEED) {
		orbitspeed = 0;
	}
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(712, 512);
	main_window = glutCreateWindow("Color Cube");
	
	glewExperimental = true;

	glewInit();

	init();
	
	glutDisplayFunc(display);
	//glutReshapeFunc(myGlutReshape);

	glui = GLUI_Master.create_glui_subwindow(main_window , GLUI_SUBWINDOW_RIGHT);
	new GLUI_StaticText(glui, "Center Cube");
	(new GLUI_Spinner(glui, "Rotate Speed:", &speed))
		->set_int_limits(-5, 5);
	(new GLUI_Spinner(glui, "Scale Speed:", &controlTime))
		->set_int_limits(-1, 1);
	new GLUI_Button(glui, "Start Rotate", STARTSPEED, control_cb);
	new GLUI_Button(glui, "Stop Rotate", STOPSPEED, control_cb);
	new GLUI_Button(glui, "Rotate X", XAXIS, control_cb);
	new GLUI_Button(glui, "Rotate Y", YAXIS, control_cb);
	new GLUI_Button(glui, "Rotate Z", ZAXIS, control_cb);
	
	new GLUI_StaticText(glui, "Orbit Cube");
	(new GLUI_Spinner(glui, "Rotate Speed:", &smallspeed))
		->set_int_limits(-5, 5);
	(new GLUI_Spinner(glui, "Rotate Speed Orbit:", &orbitspeed))
		->set_int_limits(-5, 5);
	new GLUI_Button(glui, "Start Rotate Its Axis", OWNSTARTSPEED, control_cb);
	new GLUI_Button(glui, "Stop Rotate Its Axis", OWNSTOPSPEED, control_cb);
	new GLUI_Button(glui, "Start Rotate Orbit", ORBITSTARTSPEED, control_cb);
	new GLUI_Button(glui, "Stop Rotate Orbit", ORBITSTOPSPEED, control_cb);
	new GLUI_Button(glui, "Rotate X Orbit Cube", SMALLXAXIS, control_cb);
	new GLUI_Button(glui, "Rotate Y Orbit Cube", SMALLYAXIS, control_cb);
	new GLUI_Button(glui, "Rotate Z Orbit Cube", SMALLZAXIS, control_cb);

	new GLUI_StaticText(glui, "Animation");
	new GLUI_Button(glui, "Animation Start", STARTANIMATION, control_cb);
	new GLUI_Button(glui, "Animation Stop", STOPANIMATION, control_cb);

	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutIdleFunc(idle);

	glutKeyboardFunc(keyboard);
	
	glui->set_main_gfx_window(main_window);
	
	glutTimerFunc(10, myTimer, 1);

	glutMainLoop();
	return 0;
}