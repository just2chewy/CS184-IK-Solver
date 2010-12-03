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

//=)

static double Mod(double a, double b)
{
	int result = static_cast<int>( a / b );
	return a - static_cast<double>( result ) * b;
}

//Degrees to radians
static double SimplifyAngle(double angle) {
	angle = Mod(angle, (2.0 * PI));
	if( angle < -PI )
        angle += (2.0 * PI);
    else if( angle > PI )
        angle -= (2.0 * PI);
    return angle;
}

static float radiansToDegrees(float radians) {
	return radians * 180 / PI;
}

//****************************************************
// Some Classes
//****************************************************

class Viewport {
public:
	int w, h; // width and height
};

//Object angle members are always in degrees

class BoneWorldSpace {
public:
	double start_x;  //starting coords
    double start_y;
	double start_z;
	double end_x;	 //ending coordinates
	double end_y;
	double end_z;
    double angle;    // angle in world space
    double cosAngle; // sine of angle
    double sinAngle; // cosine of angle
	double r;
	double g;
	double b;
	
	BoneWorldSpace(double new_s_x, double new_s_y, double new_s_z, double new_e_x, double new_e_y, double new_e_z, double new_a, double new_c, double new_s, double new_r, double new_g, double new_b) {
		start_x = new_s_x;
		start_y = new_s_y;
		start_z = new_s_z;
		end_x = new_e_x;
		end_y = new_e_y;
		end_z = new_e_z;
		angle = new_a;
		cosAngle = new_c;
		sinAngle = new_s;
		r = new_r;
		g = new_g;
		b = new_b;
	}
};

//Input Bone class, makes the input file simpler

class BonePrimitive {
public:
	double x;
	double y;
	double z;
	double angel;
	double r;
	double g;
	double b;
	
	BonePrimitive(double new_x, double new_y, double new_z,double new_a, double new_r, double new_g, double new_b) {
		x = new_x;
		y = new_y;
		z = new_z;
		angel = new_a;
		r = new_r;
		g = new_g;
		b = new_b;
	}
};

//Calculates angle between bones

static float getAngle(BoneWorldSpace* bone_1, BoneWorldSpace* bone_2) {
	return atan2(bone_2->end_y - bone_2->start_y, bone_2->end_x - bone_2->start_x) - atan2(bone_1->end_y - bone_1->start_y, bone_1->end_x - bone_1->start_x);
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

float targetX = 5;
float targetY = 0;
float targetZ = -2;

float epsilon = 0.0001;
bool movedBones = false;
float targetAngle = 0;

//Rotates the bones from index i with the current rotation angle
void rotateFromIndex(int index, float x_term, float y_term, float z_term, float rotAngle) {
	int i;
	BoneWorldSpace* prev = world_bones[index];
	BoneWorldSpace* bns;
	
	//printf("xterm: %f, yterm: %f\n", x_term, y_term);
	
	for(i=index+1; i<(int)world_bones.size(); i++) {
		bns = world_bones[i];

		bns->end_x = bns->end_x + x_term;
		bns->start_x = prev->end_x;
		
		bns->end_y = bns->end_y + y_term;
		bns->start_y = prev->end_y;
		
		bns->end_z = bns->end_z + z_term;
		bns->start_z = prev->end_z;
		
		prev = bns;
	}
}

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
	float currentR, currentG, currentB;
	
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
			switch (number % 6) {
			case 0:
				currentX = currentNumber;
				break;
			case 1:
				currentY = currentNumber;
				break;
			case 2:
				currentZ = currentNumber;
				break;
			case 3:
				currentR = currentNumber;
				break;
			case 4:
				currentG = currentNumber;
				break;
			default:
				currentB = currentNumber;
				initial_bones.push_back(new BonePrimitive(currentX, currentY, currentZ, 0, currentR, currentG, currentB));
				break;
			}
			number++;
		}
	}
	inputFile.close();
	
	BonePrimitive* rootBonePrimitive = initial_bones[0];
	BoneWorldSpace* root = new BoneWorldSpace(0, 0, 0, rootBonePrimitive->x, rootBonePrimitive->y, rootBonePrimitive->z, 90.0, cos(SimplifyAngle(90.0)), sin(SimplifyAngle(90.0)), rootBonePrimitive->r, rootBonePrimitive->g, rootBonePrimitive->b); 
	BoneWorldSpace* prev = root;
	world_bones.push_back(root);
	
	int i;
	
	for(i=1; i<(int)initial_bones.size(); i++) {
		BonePrimitive* temp = initial_bones[i];
		BoneWorldSpace* new_bone = new BoneWorldSpace(prev->end_x, prev->end_y, prev->end_z, temp->x + prev->end_x, temp->y + prev->end_y, temp->z + prev->end_z, 0, 0, 0, temp->r, temp->g, temp->b);
		new_bone->angle = radiansToDegrees(getAngle(prev, new_bone)) + prev->angle;
		new_bone->cosAngle = cos(new_bone->angle);
		new_bone->sinAngle = sin(new_bone->angle);
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
	
	//Draws bones
	for(i=0; i<(int)world_bones.size(); i++) {
		BoneWorldSpace* bns = world_bones[i];
		glColor3f(bns->r, bns->g, bns->b);
		glBegin(GL_LINES);
			glVertex3f(bns->start_x, bns->start_y, bns->start_z);
			glVertex3f(bns->end_x, bns->end_y, bns->end_z);			
		glEnd();
		
		//printf("start = (%f, %f, %f), end = (%f, %f, %f)\n", bns->start_x, bns->start_y, bns->start_z, bns->end_x, bns->end_y, bns->end_z);
	}
	
	//Draws circle
	glColor3f(205.0f/255.0f, 190.0f/255.0f, 112.0f/255.0f);
	glBegin(GL_LINE_LOOP);
	for(i=0; i<360; i++) {
		glVertex3f(sin(SimplifyAngle(i)) * circle_radius, cos(SimplifyAngle(i)) * circle_radius, targetZ);
	}
	glEnd();
	
	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}

//Calculates how much the bones should move, based on code from: http://www.ryanjuckett.com/programming/animation/21-cyclic-coordinate-descent-in-2d?start=4

//Kevin Tseng is my hero.

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
	static float targetTime = 0.0f;
	
	//Calculate movement of bones
	float endX = world_bones[world_bones.size()-1]->end_x;
	float endY = world_bones[world_bones.size()-1]->end_y;
	float endZ = world_bones[world_bones.size()-1]->end_z;
	totalTime += dt;
	targetTime += dt;
	
	int i;
	
	
	if(targetTime > 0.3f) {
		targetX = circle_radius*cos(SimplifyAngle(targetAngle));
		targetY = circle_radius*sin(SimplifyAngle(targetAngle));
		targetAngle += 0.1;
		if(targetAngle>360) {
			targetAngle = 0;
		}
		targetTime = 0;
		//printf("Target x:%f, target y: %f\n", targetX, targetY);
	}
	
	if(!movedBones && totalTime>0.1f) {
		totalTime = 0.0;
		for(i=world_bones.size()-1; i>=0; i--) {
			BoneWorldSpace* current_bone = world_bones[i];
			 
			//********* Z-Axis Rotation **********
			// Get the vector from the current bone to the end effector position.
			float curToEndX = endX - current_bone->start_x;
			float curToEndY = endY - current_bone->start_y;
			float curToEndZ;
			float curToEndMag = sqrt(powf(curToEndX, 2.0f) + powf(curToEndY, 2.0f));
			//printf("curToEndX: %f curToEndY: %f \n", curToEndX, curToEndY);
			
			
			// Get the vector from the current bone to the target position.
			float curToTargX = targetX - current_bone->start_x;
			float curToTargY = targetY - current_bone->start_y;
			float curToTargZ;
			float curToTargMag = sqrt(powf(curToTargX, 2.0f) + powf(curToTargY, 2.0f));
			//printf("curToTargMag: %f\n", curToTargMag);
				
			float cosRotAngle;
			float sinRotAngle;
			float endTargetMag = curToEndMag * curToTargMag;
			//printf("endTargetMag: %f\n", endTargetMag);
			
			if(endTargetMag <= epsilon) {
				movedBones = true;
				cosRotAngle = 1;
				sinRotAngle = 0;
			} else {
				cosRotAngle = (curToEndX*curToTargX + curToEndY*curToTargY) / endTargetMag;
				sinRotAngle = (curToEndX*curToTargY - curToEndY*curToTargX) / endTargetMag;
			}
			
			// Clamp the cosine into range when computing the angle (might be out of range
			// due to floating point error).
			float rotAng = acos(max(-1.0f,min(1.0f,cosRotAngle)));
			if( sinRotAngle < 0.0 ) {
				rotAng = -rotAng;
			}
			
			float old_end_x = world_bones[i]->end_x;
			float old_end_y = world_bones[i]->end_y;
			float old_end_z = 0;
			
			// Rotate the end effector position.
			endX = world_bones[i]->start_x + cosRotAngle*curToEndX - sinRotAngle*curToEndY;
			endY = world_bones[i]->start_y + sinRotAngle*curToEndX + cosRotAngle*curToEndY;
			
			//printf("cos: %f, sin: %f, endTargetMag: %f\n", cosRotAngle, sinRotAngle, endTargetMag);
			//printf("endX: %f endY: %f rotAng: %f \n", endX, endY, radiansToDegrees(rotAng));
			
			// Modify bone coords
			//printf("before: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			float x_translated = current_bone->end_x - current_bone->start_x;
			float y_translated = current_bone->end_y - current_bone->start_y;
			float z_translated = 0;
			
			world_bones[i]->end_x = world_bones[i]->start_x + x_translated*cos(rotAng) - y_translated*sin(rotAng);
			world_bones[i]->end_y = world_bones[i]->start_y + x_translated*sin(rotAng) + y_translated*cos(rotAng);
			
			float x_diff = world_bones[i]->end_x - old_end_x;
			float y_diff = world_bones[i]->end_y - old_end_y;
			float z_diff = 0;
			//printf("after: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			world_bones[i]->angle = world_bones[i]->angle + rotAng;
			rotateFromIndex(i, x_diff, y_diff, z_diff, rotAng);
			
			
			//********* X-Axis Rotation **********
			// Get the vector from the current bone to the end effector position.
			curToEndY = endY - current_bone->start_y;
			curToEndZ = endZ - current_bone->start_z;
			float curToEndMagX = sqrt(powf(curToEndY, 2.0f) + powf(curToEndZ, 2.0f));
			//printf("curToEndX: %f curToEndY: %f \n", curToEndX, curToEndY);
			
			// Get the vector from the current bone to the target position.
			curToTargY = targetY - current_bone->start_y;
			curToTargZ = targetZ - current_bone->start_z;
			float curToTargMagX = sqrt(powf(curToTargY, 2.0f) + powf(curToTargZ, 2.0f));
			//printf("curToTargMag: %f\n", curToTargMag);
			
			float cosRotAngleX;
			float sinRotAngleX;
			float endTargetMagX = curToEndMagX * curToTargMagX;
			//printf("endTargetMag: %f\n", endTargetMag);
			
			if(endTargetMagX <= epsilon) {
				movedBones = true;
				cosRotAngle = 1;
				sinRotAngle = 0;
			} else {
				cosRotAngleX = (curToEndY*curToTargY + curToEndZ*curToTargZ) / endTargetMagX;
				sinRotAngleX = (curToEndY*curToTargZ - curToEndZ*curToTargY) / endTargetMagX;
			}
			
			// Clamp the cosine into range when computing the angle (might be out of range
			// due to floating point error).
			float rotAngX = acos(max(-1.0f,min(1.0f,cosRotAngleX)));
			if( sinRotAngleX < 0.0 ) {
				rotAngX = -rotAngX;
			}
			
			old_end_y = world_bones[i]->end_y;
			old_end_z = world_bones[i]->end_z;
			
			// Rotate the end effector position.
			endY = world_bones[i]->start_y + cosRotAngleX*curToEndY - sinRotAngleX*curToEndZ;
			endZ = world_bones[i]->start_z + sinRotAngleX*curToEndY + cosRotAngleX*curToEndZ;
			
			//printf("cos: %f, sin: %f, endTargetMag: %f\n", cosRotAngle, sinRotAngle, endTargetMag);
			//printf("endX: %f endY: %f rotAng: %f \n", endX, endY, radiansToDegrees(rotAng));
			
			// Modify bone coords
			//printf("before: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			y_translated = current_bone->end_y - current_bone->start_y;
			z_translated = current_bone->end_z - current_bone->start_z;
			
			world_bones[i]->end_y = world_bones[i]->start_y + y_translated*cos(rotAngX) - z_translated*sin(rotAngX);
			world_bones[i]->end_z = world_bones[i]->start_z + y_translated*sin(rotAngX) + z_translated*cos(rotAngX);
			
			y_diff = world_bones[i]->end_y - old_end_y;
			z_diff = world_bones[i]->end_z - old_end_z;
			x_diff = 0;
			//printf("after: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			world_bones[i]->angle = world_bones[i]->angle + rotAngX;
			
			rotateFromIndex(i, x_diff, y_diff, z_diff, rotAng);
			
			//********* Y-Axis Rotation **********
			// Get the vector from the current bone to the end effector position.
			curToEndX = endX - current_bone->start_x;
			curToEndZ = endZ - current_bone->start_z;
			float curToEndMagY = sqrt(powf(curToEndX, 2.0f) + powf(curToEndZ, 2.0f));
			//printf("curToEndX: %f curToEndY: %f \n", curToEndX, curToEndY);
			
			// Get the vector from the current bone to the target position.
			curToTargX = targetX - current_bone->start_x;
			curToTargZ = targetZ - current_bone->start_z;
			float curToTargMagY = sqrt(powf(curToTargX, 2.0f) + powf(curToTargZ, 2.0f));
			//printf("curToTargMag: %f\n", curToTargMag);
			
			float cosRotAngleY;
			float sinRotAngleY;
			float endTargetMagY = curToEndMagY * curToTargMagY;
			//printf("endTargetMag: %f\n", endTargetMag);
			
			if(endTargetMagY <= epsilon) {
				movedBones = true;
				cosRotAngle = 1;
				sinRotAngle = 0;
			} else {
				cosRotAngleY = (curToEndZ*curToTargZ + curToEndX*curToTargX) / endTargetMagY;
				sinRotAngleY = (curToEndZ*curToTargX - curToEndX*curToTargZ) / endTargetMagY;
			}
			
			// Clamp the cosine into range when computing the angle (might be out of range
			// due to floating point error).
			float rotAngY = acos(max(-1.0f,min(1.0f,cosRotAngleY)));
			if( sinRotAngleY < 0.0 ) {
				rotAngY = -rotAngY;
			}
			
			old_end_x = world_bones[i]->end_x;
			old_end_z = world_bones[i]->end_z;
			
			// Rotate the end effector position.
			endZ = world_bones[i]->start_z + cosRotAngleY*curToEndZ - sinRotAngleY*curToEndX;
			endX = world_bones[i]->start_x + sinRotAngleY*curToEndZ + cosRotAngleY*curToEndX;
			
			//printf("cos: %f, sin: %f, endTargetMag: %f\n", cosRotAngle, sinRotAngle, endTargetMag);
			//printf("endX: %f endY: %f rotAng: %f \n", endX, endY, radiansToDegrees(rotAng));
			
			// Modify bone coords
			//printf("before: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			x_translated = current_bone->end_x - current_bone->start_x;
			z_translated = current_bone->end_z - current_bone->start_z;
			
			world_bones[i]->end_z = world_bones[i]->start_z + z_translated*cos(rotAngY) - x_translated*sin(rotAngY);
			world_bones[i]->end_x = world_bones[i]->start_x + z_translated*sin(rotAngY) + x_translated*cos(rotAngY);
			
			x_diff = world_bones[i]->end_x - old_end_x;
			z_diff = world_bones[i]->end_z - old_end_z;
			y_diff = 0;
			//printf("after: (%f, %f)\n", world_bones[i]->end_x, world_bones[i]->end_y);
			
			world_bones[i]->angle = world_bones[i]->angle + rotAngY;
			
			rotateFromIndex(i, x_diff, y_diff, z_diff, rotAng);
			
		}
	}
	
	// Accumulate the time since the program starts
	
	// Store the time
	lastTime = currentTime;
	glutPostRedisplay();
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
