// Draws colored cube  

#include "Angel.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib> 
#include <ctime> 

//----------------------------------------------------------------------------
int width = 0;
int height = 0;


// remember to prototype
void generateGeometry( void );
void display( void );
void keyboard( unsigned char key, int x, int y );
float maxOfTwo(float x, float y);
void getScaleFactor();
void twistPoints();
void parsePly(std::string plyFile);
void idle(void);
mat4 makeShearMatrix(GLfloat theta);
void twistPoints();
int provideColorIndex(int i);
void createTriangles(void);

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



// handle to program
GLuint program;

using namespace std;

const int NumVertices = 90000;
const int NUMFACES = 60000;
const enum DrawPly { DRAW_AIRPLANE, DRAW_ANT, DRAW_APPLE, DRAW_BALANCE, DRAW_BEETHOVEN,
DRAW_BIG_ATC, DRAW_BIG_DODGE, DRAW_BIG_PORCHE, DRAW_BIG_SPIDER, DRAW_CANSTICK, 
DRAW_CHOPPER, DRAW_COW, DRAW_DOLPHINS, DRAW_EGRET, DRAW_F16, DRAW_FOOTBONES, 
DRAW_FRACTTREE, DRAW_GALLEON, DRAW_HAMMERHEAD, DRAW_HELIX, DRAW_HIND, DRAW_KEROLAMP,
DRAW_KETCHUP, DRAW_MUG, DRAW_PART, DRAW_PICKUP_BIG, DRAW_PUMP, DRAW_PUMPA_TB, DRAW_SANDAL,
DRAW_SARATOGA, DRAW_SCISSORS, DRAW_SHARK, DRAW_STEERINGWEEL, DRAW_STRATOCASTER, DRAW_STREETLAMP, 
DRAW_TEAPOT, DRAW_TENNIS_SHOE, DRAW_TOMMYGUN, DRAW_TRASHCAN, DRAW_TURBINE, DRAW_URN2, DRAW_WALKMAN,
DRAW_WEATHERVANE
};
int currDraw = 0;

point4 points[NumVertices];
color4 colors[NumVertices];
vec4 edgeList[NUMFACES];
int vertices;
int faces;
vec3 vertexList3[NumVertices];
vec4 vertexList[NumVertices];
GLuint pointsIndex = 0;
string fileName=  "";
float min_x = 10000000.0F;
float max_x = 0.0F;
float min_y = 10000000.0F;
float max_y = 0.0;
float min_z = 10000000.0F;
float max_z = 0.0;


float mean_x = 0.0F;
float mean_y = 0.0F;
float mean_z = 0.0F;

float translate_scale = 1.2f;
bool color_toggle = true;
bool moveX_toggle = false;
bool moveX_positive = true;
bool moveY_toggle = false;
bool moveY_positive = true;
bool moveZ_toggle = false;
bool moveZ_positive = true;

bool rotateY_toggle = false;
bool rotateY_positive = true;


float x_displacement = 0.0f;
float y_displacement = 0.0f;
float z_displacement = 0.0f;

float y_rotation = 0.0f;

float x_shear = 0.0f;

int y_twist = 0;

void parsePly(string plyFile) {
	cout << plyFile << "\n";
	ifstream plyFileStream;
	plyFileStream.open(plyFile);
	string plyFileLine;
	string chk;
	string garbage;
	plyFileStream >> plyFileLine;
	if (plyFileLine != "ply") {
		cout << "This is not a ply file\n";
		return;
	}
	cout << "This is a ply file\n";
	getline(plyFileStream, garbage);//ends line 1
	getline(plyFileStream, garbage); //ascii line skip
	cout << garbage << "\n";
	getline(plyFileStream, garbage, ' '); //"element"
	getline(plyFileStream, garbage, ' '); //"vertex"
	getline(plyFileStream, garbage);
	vertices = stoi(garbage);
	cout << "There are " << vertices << " vertices in the figure\n";
	getline(plyFileStream, garbage);//property
	getline(plyFileStream, garbage);//lines
	getline(plyFileStream, garbage);//skipped
	getline(plyFileStream, garbage, ' '); //"element"
	getline(plyFileStream, garbage, ' '); //"face"
	getline(plyFileStream, garbage);
	faces = stoi(garbage);
	cout << "There are " << faces << " faces in the figure\n";
	getline(plyFileStream, garbage);//property
	getline(plyFileStream, garbage);//end_header

	min_x = 10000000.0F;
	max_x = 0.0F;
	min_y = 10000000.0F;
	max_y = 0.0;
	min_z = 10000000.0F;
	max_z = 0.0;

	mean_x = 0.0F;
	mean_y = 0.0F;
	mean_z = 0.0F;
									
	for (int i = 0; i < vertices; i++) {
		plyFileStream >> vertexList3[i];
		
		if (vertexList3[i].x < min_x) {
			min_x = vertexList3[i].x;
		}
		if (vertexList3[i].x > max_x) {
			max_x = vertexList3[i].x;
		}
		if (vertexList3[i].y < min_y) {
			min_y = vertexList3[i].y;
		}
		if (vertexList3[i].y > max_y) {
			max_y = vertexList3[i].y;
		}
		if (vertexList3[i].z < min_z) {
			min_z = vertexList3[i].z;
		}
		if (vertexList3[i].z > max_z) {
			max_z = vertexList3[i].z;
		}
		
	}

	getScaleFactor();
	z_displacement = -1 * translate_scale * maxOfTwo(max_x, max_y);
	mean_x = (min_x + max_x) / 2;
	mean_y = (min_y + max_y) / 2;
	mean_z = (min_z + max_z) / 2;

	for (int i = 0; i < vertices; i++) {
		vertexList3[i].x -= mean_x; //center at x
		vertexList3[i].y -= mean_y; //center at y
		vertexList3[i].z -= mean_z; //center at z
		vertexList[i] = vec4(vertexList3[i], 1.0F);
	}

	for (int i = 0; i < faces; i++) {
		plyFileStream >> edgeList[i];
	}
	plyFileStream.clear();
	plyFileStream.close();
}

color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

int provideColorIndex(int i) {
	if (color_toggle)
		return (i + rand()) % 8;
	else
		return 1;
}

void twistPoints() {
	for (int i = 0; i < NumVertices; i++) {
		float d = points[i].y;
		d = 0.001f*d*y_twist;

		points[i].x = points[i].x*cos(d) + points[i].z * sin(d);
		points[i].z = -1 * points[i].x*sin(d) + points[i].z * cos(d);

	}
}

void createTriangles(void) {
	pointsIndex = 0;
	for (int i = 0; i < faces; i++) {
		int vert1 = edgeList[i].y;
		points[pointsIndex] = vertexList[vert1];
		colors[pointsIndex] = vertex_colors[provideColorIndex(i)]; //randomly assign one of 8 colors
		pointsIndex++;
		int vert2 = edgeList[i].z;
		points[pointsIndex] = vertexList[vert2];
		colors[pointsIndex] = vertex_colors[provideColorIndex(i)];
		pointsIndex++;
		int vert3 = edgeList[i].w;
		points[pointsIndex] = vertexList[vert3];
		colors[pointsIndex] = vertex_colors[provideColorIndex(i)];
		pointsIndex++;
	}
	twistPoints();
}




float maxOfTwo(float x, float y) {
	if (x > y)
		return x;
	else
		return y;
}


void getScaleFactor() {
	switch (currDraw)
	{
	case DRAW_AIRPLANE:
		translate_scale = 1.2f;
		break;
	case DRAW_ANT:
		translate_scale = 3.0f;
		break;
	case DRAW_APPLE:
		translate_scale = 0.1f;
		break;
	case DRAW_BIG_SPIDER:
		translate_scale = 1.0f;
		break;
	case DRAW_CANSTICK:
		translate_scale = 0.2f;
		break;
	case DRAW_CHOPPER:
		translate_scale = 1.7f;
		break;
	case DRAW_COW:
		translate_scale = 2.0f;
		break;
	case DRAW_EGRET:
		translate_scale = 2.5f;
		break;
	case DRAW_F16:
		translate_scale = 1.3f;
		break;
	case DRAW_FOOTBONES:
		translate_scale = 2.3f;
		break;
	case DRAW_GALLEON:
		translate_scale = 6.0f;
		break;
	case DRAW_KEROLAMP:
		translate_scale = 5.0f;
		break;
	case DRAW_KETCHUP:
		translate_scale = 4.0f;
		break;
	case DRAW_MUG:
		translate_scale = 0.2f;
		break;
	case DRAW_PICKUP_BIG:
		translate_scale = 4.8f;
		break;
	case DRAW_PUMP:
		translate_scale = 0.5f;
		break;

	default:
		translate_scale = 3.0f;
		break;
	}

}
void generateGeometry( void )
{	
  //  colorcube();
	fileName =  std::to_string(currDraw)+".ply";
	parsePly(fileName);
	createTriangles();
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(point4)*faces*3 + sizeof(point4)*faces * 3, //points + colors
		  NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*faces * 3, points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*faces * 3, sizeof(point4)*faces * 3, colors );


	// Load shaders and use the resulting shader program
    program = InitShader( "vshader1.glsl", "fshader1.glsl" );
    glUseProgram( program );
     // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(point4)*faces * 3) );

	// sets the default color to clear screen
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background
}

void drawCube(void)
{
	// change to GL_FILL
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// draw functions should enable then disable the features 
	// that are specifit the themselves
	// the depth is disabled after the draw 
	// in case you need to draw overlays
	glEnable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLES, 0, faces * 3);
	glDisable( GL_DEPTH_TEST ); 
}

mat4 makeShearMatrix(GLfloat theta) {
	mat4 c;
	c = Angel::identity();
	GLfloat angle = DegreesToRadians * (90 - theta);
	c[0][1] = cos(angle) / sin(angle);
	return c;
}




//----------------------------------------------------------------------------
// this is where the drawing should happen
void display( void )
{
	// SOME RANDOM TIPS
	//========================================================================
	// remember to enable depth buffering when drawing in 3d

	// avoid using glTranslatex, glRotatex, push and pop
	// pass your own view matrix to the shader directly
	// refer to the latest OpenGL documentation for implementation details

    // Do not set the near and far plane too far appart!
	// depth buffers do not have unlimited resolution
	// surfaces will start to fight as they come nearer to each other
	// if the planes are too far appart (quantization errors :(   )

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );     // clear the window

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)45.0, (GLfloat)width/(GLfloat)height, (GLfloat)0.1, (GLfloat)5000.0);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0];viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0];viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0];viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0];viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2];viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2];viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2];viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2];viewMatrixf[15] = perspectiveMat[3][3];

	/*DRAW_AIRPLANE, DRAW_ANT, DRAW_APPLE, DRAW_BALANCE, DRAW_BEETHOVEN,
		DRAW_BIG_ATC, DRAW_BIG_DODGE, DRAW_BIG_PORCHE, DRAW_BIG_SPIDER, DRAW_CANSTICK,
		DRAW_CHOPPER, DRAW_COW, DRAW_DOLPHINS, DRAW_EGRET, DRAW_F16, DRAW_FOOTBONES*/

/*	switch (currDraw)
	{
	case DRAW_AIRPLANE:
		translate_scale = 1.2f;
		break;
	case DRAW_ANT:
		translate_scale = 3.0f;
		break;
	case DRAW_APPLE:
		translate_scale = 0.1f;
		break;
	case DRAW_BIG_SPIDER:
		translate_scale = 1.0f;
		break;
	case DRAW_CANSTICK:
		translate_scale = 0.2f;
		break;
	case DRAW_CHOPPER:
		translate_scale = 1.7f;
		break;
	case DRAW_COW:
		translate_scale = 2.0f;
		break;
	case DRAW_EGRET:
		translate_scale = 2.5f;
		break;
	case DRAW_F16:
		translate_scale = 1.3f;
		break;
	case DRAW_FOOTBONES:
		translate_scale = 2.3f;
		break;
	case DRAW_GALLEON:
		translate_scale = 6.0f;
		break;	
	case DRAW_KEROLAMP:
		translate_scale = 5.0f;
		break;
	case DRAW_KETCHUP:
		translate_scale = 4.0f;
		break;
	case DRAW_MUG:
		translate_scale = 0.2f;
		break;	
	case DRAW_PICKUP_BIG:
		translate_scale = 4.8f;
		break;
	case DRAW_PUMP:
		translate_scale = 0.5f;
		break;
		
	default:
		translate_scale = 3.0f;
		break;
	}
	*/
	Angel::mat4 modelMat = Angel::identity();

	//z_displacement = -1 * translate_scale * maxOfTwo(max_x, max_y);
	modelMat = modelMat * makeShearMatrix(x_shear) * Angel::Translate(x_displacement, y_displacement, z_displacement)*Angel::RotateY(y_rotation);// * Angel::RotateY(0.0f) * Angel::RotateX(0.0f);
	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0];modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0];modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0];modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0];modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2];modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2];modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2];modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2];modelMatrixf[15] = modelMat[3][3];
	
	// set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv( modelMatrix, 1, GL_FALSE, modelMatrixf );
	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv( viewMatrix, 1, GL_FALSE, viewMatrixf);

	drawCube();
    glFlush(); // force output to graphics hardware

	// use this call to double buffer
	glutSwapBuffers();
	// you can implement your own buffers with textures
}

//----------------------------------------------------------------------------

// keyboard handler
void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
	case 0127: //W
		x_displacement = 0;
		y_displacement = 0;
		z_displacement = -1 * translate_scale * maxOfTwo(max_x, max_y);
		y_twist = 0;
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0116: //N
		if (currDraw == DRAW_WEATHERVANE)
			break;
		currDraw++;
		y_twist = 0;
		x_displacement = 0;
		y_displacement = 0;
		z_displacement = -1 * translate_scale * maxOfTwo(max_x, max_y);
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0120:  //P
		if (currDraw == DRAW_AIRPLANE)
			break;
		currDraw--;
		y_twist = 0;
		x_displacement = 0;
		y_displacement = 0;
		z_displacement = -1 * translate_scale * maxOfTwo(max_x, max_y);
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0130: //X
		if (moveX_toggle)
			moveX_toggle = false;
		else {
			moveX_toggle = true;
			moveX_positive = true;
		}
		break;
	case 0131: //Y
		if (moveY_toggle)
			moveY_toggle = false;
		else {
			moveY_toggle = true;
			moveY_positive = true;
		}
		break;
	case 0132: //Z
		if (moveZ_toggle)
			moveZ_toggle = false;
		else {
			moveZ_toggle = true;
			moveZ_positive = true;
		}
		break;
	case 0122: //R
		if (rotateY_toggle)
			rotateY_toggle = false;
		else {
			moveX_toggle = false;
			moveY_toggle = false;
			moveZ_toggle = false;
			rotateY_toggle = true;
		}
		break;
	case 0143: //c
		if (color_toggle)
			color_toggle = false;
		else
			color_toggle = true;
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0110: //H
		x_shear -= 1.0f;
		glutPostRedisplay();
		break;
	case 0150: //h
		x_shear += 1.0f;
		glutPostRedisplay();
		break;
	case 0124: //T
		y_twist--;
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0164: //t
		y_twist++;
		generateGeometry();
		glutPostRedisplay();
		break;
	case 0170: //x
		if (moveX_toggle)
			moveX_toggle = false;
		else {
			moveX_toggle = true;
			moveX_positive = false;
		}
		break;
	case 0171: //y
		if (moveY_toggle)
			moveY_toggle = false;
		else {
			moveY_toggle = true;
			moveY_positive = false;
		}
		break;
	case 0172: //z
		if (moveZ_toggle)
			moveZ_toggle = false;
		else {
			moveZ_toggle = true;
			moveZ_positive = false;
		}
		break;
    }
}

void idle(void) {
	if (moveX_toggle) {
		if (moveX_positive)
			x_displacement += 2;
		else
			x_displacement -= 2;
	}

	if (moveY_toggle) {
		if (moveY_positive)
			y_displacement += 2;
		else
			y_displacement -= 2;
	}


	if (moveZ_toggle) {
		if (moveZ_positive)
			z_displacement += 2;
		else
			z_displacement -= 2;
	}

	if (rotateY_toggle) {


		if (currDraw%2 == 0 || currDraw == 0)
			y_rotation += 2.0f;
		else
			y_rotation -= 2.0f;
		
		if (y_rotation > 360.0f || y_rotation < -360.0f) {
			y_rotation = 0.0f;
			x_displacement = 0.0f;
			y_displacement = 0.0f;

			if (currDraw != DRAW_WEATHERVANE) 
				currDraw++;
			else 
				currDraw = DRAW_AIRPLANE;
			
			
			generateGeometry();
		}
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
// entry point
int main( int argc, char **argv )
{
	// init glut
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 1024, 1024);
	width = 1024;
	height = 1024;

	// create window
	// opengl can be incorperated into other packages like wxwidgets, fltoolkit, etc.
    glutCreateWindow( "Color Cube" );

	// init glew
    glewInit();

    generateGeometry();

	// assign handlers
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
	glutIdleFunc(idle);
	// should add menus
	// add mouse handler
	// add resize window functionality (should probably try to preserve aspect ratio)

	// enter the drawing loop
	// frame rate can be controlled with 
    glutMainLoop();
    return 0;
}
