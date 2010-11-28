// Simple OpenGL example for CS184 F06 by Nuttapong Chentanez, modified from sample code for CS184 on Sp06
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>
#include <algorithm>
#include <stdio.h>
#include <list>
#include <vector>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265

using namespace std;

static double Mod(double a, double b)
{
	int result = static_cast<int>( a / b );
	return a - static_cast<double>( result ) * b;
}

static double SimplifyAngle(double angle) {
	angle = Mod(angle, (2.0 * PI));
	if( angle < -PI )
        angle += (2.0 * PI);
    else if( angle > PI )
        angle -= (2.0 * PI);
    return angle;
}

//****************************************************
// Some Classes
//****************************************************

class Viewport {
public:
	int w, h; // width and height
};

class BoneWorldSpace {
public:
	double start_x;        
    double start_y;
	double start_z;
	double end_x;
	double end_y;
	double end_z;
    double angle;    // angle in world space
    double cosAngle; // sine of angle
    double sinAngle; // cosine of angle
	
	BoneWorldSpace(double new_s_x, double new_s_y, double new_s_z, double new_e_x, double new_e_y, double new_e_z, double new_a, double new_c, double new_s) {
		start_x = new_s_x;
		start_y = new_s_y;
		start_z = new_s_z;
		end_x = new_e_x;
		end_y = new_e_y;
		end_z = new_e_z;
		angle = new_a;
		cosAngle = new_c;
		sinAngle = new_s;
	}
};

class BonePrimitive {
public:
	double x;
	double y;
	double z;
	double angel;
	
	BonePrimitive(double new_x, double new_y, double new_z,double new_a) {
		x = new_x;
		y = new_y;
		z = new_z;
		angel = new_a;
	}
};

static float getAngle(BoneWorldSpace* bone_1, BoneWorldSpace* bone_2) {
	return atan2(bone_2->end_y - bone_2->start_y, bone_2->end_x - bone_2->start_x) - atan2(bone_1->end_y - bone_1->start_y, bone_1->end_x - bone_1->start_x);
}

static float radiansToDegrees(float radians) {
	return radians * 180 / PI;
}

//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
float camera_x=0.0f, camera_y=0.0f, camera_z=10.0f;
float lx=0.0f, ly=0.0f, lz=-1.0f;
float ratio;
float circle_radius;
vector<BonePrimitive*> initial_bones;
vector<BoneWorldSpace*> world_bones;

//Get command line args, create polygon objects
void initScene(int argc, char *argv[]){
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent

	glViewport (0,0,viewport.w,viewport.h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,viewport.w, 0, viewport.h);
	
	if(argc != 2) {
		exit(1);
	}
	
	char* fileName = argv[1];
	bool firstLine = true;
	int number = 0;
	float currentX, currentY, currentZ;
	float currentNumber;
	
	ifstream inputFile;
	inputFile.open(fileName);

	while (inputFile >> currentNumber)
	{
		if (firstLine)
		{
			circle_radius = (int) currentNumber;
			firstLine = false;
		}
		else
		{
			switch (number % 3) {
			case 0:
				currentX = currentNumber;
				break;
			case 1:
				currentY = currentNumber;
				break;
			default:
				currentZ = currentNumber;
				initial_bones.push_back(new BonePrimitive(currentX, currentY, currentZ, 0));
				break;
			}
			number++;
		}
	}
	inputFile.close();
	
	BonePrimitive* rootBonePrimitive = initial_bones[0];
	BoneWorldSpace* root = new BoneWorldSpace(0, 0, 0, rootBonePrimitive->x, rootBonePrimitive->y, rootBonePrimitive->z, 90.0, 0, 0); 
	BoneWorldSpace* prev = root;
	world_bones.push_back(root);
	
	int i;
	
	for(i=1; i<(int)initial_bones.size(); i++) {
		BonePrimitive* temp = initial_bones[i];
		BoneWorldSpace* new_bone = new BoneWorldSpace(prev->end_x, prev->end_y, prev->end_z, temp->x + prev->end_x, temp->y + prev->end_y, temp->z + prev->end_z, 0, 0, 0);
		new_bone->angle = radiansToDegrees(getAngle(prev, new_bone));
		world_bones.push_back(new_bone);
		prev = new_bone;
	}
}

//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// loading the identity matrix for the screen

	glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window

	float ratio = 1.0* w / h;

	gluPerspective(60, ratio,1,1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_x,camera_y,camera_z,
		      camera_x+lx,camera_y+ly,camera_z+lz,
			  0.0f,1.0f,0.0f);
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// clear the color buffer
	int i;
	
	for(i=0; i<(int)world_bones.size(); i++) {
		BoneWorldSpace* bns = world_bones[i];
		glBegin(GL_LINES);
			glVertex3f(bns->start_x, bns->start_y, bns->start_z);
			glVertex3f(bns->end_x, bns->end_y, bns->end_z);			
		glEnd();
		printf("%d angle: %f\n", i, bns->angle);
	}
	
	glBegin(GL_LINE_LOOP);
	for(i=0; i<360; i++) {
		glVertex2f(sin(SimplifyAngle(i)) * circle_radius, cos(SimplifyAngle(i)) * circle_radius);
	}
	glEnd();
	
	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}

//****************************************************
// for checking if space bar is pressed
//****************************************************

void myKeyResponse(unsigned char key, int x, int y) {
	switch(key) {
	case 32:
		exit(0);
		break;
	case 43:
		glTranslatef(0.0f,0.0f,1.0f);
		myDisplay();
		break;
	case 45:
		glTranslatef(0.0f,0.0f,-1.0f);
		myDisplay();
		break;
	default:
		printf("%u\n", key);
		break;
	}
}

void mySpecialKeyResponse(int key, int x, int y) {
	int mod = glutGetModifiers();
	switch(key) {
	case GLUT_KEY_LEFT:
		if (mod == GLUT_ACTIVE_SHIFT) glTranslatef(-0.5f,0.0f,0.0f);
		else glRotatef(-0.5f,0.0f,1.0f,0.0f);
		myDisplay();
		break;
	case GLUT_KEY_RIGHT:
		if (mod == GLUT_ACTIVE_SHIFT) glTranslatef(0.5f,0.0f,0.0f);
		else glRotatef(0.5f,0.0f,1.0f,0.0f);
		myDisplay();
		break;
	case GLUT_KEY_UP:
		if (mod == GLUT_ACTIVE_SHIFT) glTranslatef(0.0f,0.5f,0.0f);
		else glRotatef(0.5f,1.0f,0.0f,0.0f);
		myDisplay();
		break;
	case GLUT_KEY_DOWN:
		if (mod == GLUT_ACTIVE_SHIFT) glTranslatef(0.0f,-0.5f,0.0f);
		else glRotatef(-0.5f,1.0f,0.0f,0.0f);
		myDisplay();
		break;
	}
}

void myFrameMove() {
	float dt;
	// Compute the time elapsed since the last time the scence is redrawn
#ifdef _WIN32
	DWORD currentTime = GetTickCount();
	dt = (float)(currentTime - lastTime)*0.001f; 
#else
	timeval currentTime;
	gettimeofday(&currentTime, NULL);
	dt = (float)((currentTime.tv_sec - lastTime.tv_sec) + 1e-6*(currentTime.tv_usec - lastTime.tv_usec));
#endif

	// Update the position of the circle
	static float totalTime = 0.0f;
	
	//Calculate movement of bones
	
	// Accumulate the time since the program starts
	totalTime += dt;
	
	// Store the time
	lastTime = currentTime;
	glutPostRedisplay();
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
  	//This initializes glut
  	glutInit(&argc, argv);
  
  	//This tells glut to use a double-buffered window with red, green, and blue channels 
  	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  	// Initalize theviewport size
  	viewport.w = 400;
  	viewport.h = 400;

  	//The size and position of the window
  	glutInitWindowSize(viewport.w, viewport.h);
  	glutInitWindowPosition(0,0);
  	glutCreateWindow(argv[0]);

   	// Initialize timer variable
	#ifdef _WIN32
	lastTime = GetTickCount();
	#else
	gettimeofday(&lastTime, NULL);
	#endif 	

  	initScene(argc, argv);							// quick function to set up scene
  
  	glutDisplayFunc(myDisplay);					// function to run when its time to draw something
  	glutReshapeFunc(myReshape);					// function to run when the window gets resized
  	glutIdleFunc(myFrameMove);			
	
 	glutKeyboardFunc(myKeyResponse);

	glutSpecialFunc(mySpecialKeyResponse);		// function for special keys
	
  	glutMainLoop();							// infinite loop that will keep drawing and resizing and whatever else
  
  	return 0;
}
