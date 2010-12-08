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

//Degrees to radians
static double degreesToRadians(double angle) {
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
class Point {
public:
	float x, y, z;
	Point(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};
class Points {
private:
	int currentIndex;
public:
	Point** myPoints;
	int numPoints;
	Points(int _numPoints)
	{
		numPoints = _numPoints;
		myPoints = new Point*[_numPoints];
		currentIndex = 0;
	}
	void Add(float x, float y, float z)
	{
		myPoints[currentIndex] = new Point(x, y, z);
		currentIndex++;
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
float ellipse_a, ellipse_b;
float eightParam;
int shapeIndex;
float x_translation, y_translation, z_translation;
float alpha = 0.05;
Points* points;
vector<BonePrimitive*> initial_bones;
vector<BoneWorldSpace*> world_bones;

// For arbitrary number of points
int currentPointsIndex = 0;
int currentNumSteps;
int currentIterationIndex = 0;
float currentTargetX;
float currentTargetY;
float currentTargetZ;
Point* currentSlope;

float targetX = 0;
float targetY = 0;
float targetZ = 0;

float epsilon = 0.0001;
bool movedBones = false;
float targetAngle = 0;

bool should_move = false;

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
	char* shapeName = new char[10];
	int number = 0;
	float currentNumber;
	float currentX, currentY, currentZ;
	float currentR, currentG, currentB;
	
	ifstream inputFile;
	inputFile.open(fileName);

	inputFile >> currentNumber;
	x_translation = currentNumber;
	inputFile >> currentNumber;
	y_translation = currentNumber;
	inputFile >> currentNumber;
	z_translation = currentNumber;

	inputFile >> shapeName;
	if (!strcmp(shapeName, "circle"))
	{
		shapeIndex = 0;
		inputFile >> currentNumber;
		circle_radius = currentNumber;
	}
	else if (!strcmp(shapeName, "ellipse"))
	{
		shapeIndex = 1;
		inputFile >> currentNumber;
		ellipse_a = currentNumber;
		inputFile >> currentNumber;
		ellipse_b = currentNumber;
	}
	else if (!strcmp(shapeName, "eight"))
	{
		shapeIndex = 2;
		inputFile >> currentNumber;
		eightParam = currentNumber;
	}
	else if (!strcmp(shapeName, "points"))
	{
		int pointsNumber;
		shapeIndex = 3;
		inputFile >> pointsNumber;
		points = new Points(pointsNumber);
		for (int i = 0; i < pointsNumber; i++)
		{
			float x, y, z;
			inputFile >> x;
			inputFile >> y;
			inputFile >> z;
			points->Add(x, y, z);
		}
	}
	delete shapeName;

	while (inputFile >> currentNumber)
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
	inputFile.close();
	
	BonePrimitive* rootBonePrimitive = initial_bones[0];
	BoneWorldSpace* root = new BoneWorldSpace(0, 0, 0, rootBonePrimitive->x, rootBonePrimitive->y, rootBonePrimitive->z, 90.0, cos(degreesToRadians(90.0)), sin(degreesToRadians(90.0)), rootBonePrimitive->r, rootBonePrimitive->g, rootBonePrimitive->b); 
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
	if (shapeIndex == 3)
	{
		Point* A = points->myPoints[0];
		Point* B = points->myPoints[1];
		currentPointsIndex = 0;
		currentIterationIndex = 0;
		currentTargetX = A->x;
		currentTargetY = A->y;
		currentTargetZ = A->z;
		float xDiff = B->x - A->x;
		float yDiff = B->y - A->y;
		float zDiff = B->z - A->z;
		float normalize = sqrt(powf(xDiff, 2.0) + powf(yDiff, 2.0) + powf(zDiff, 2.0));
		currentSlope = new Point(xDiff / normalize, yDiff / normalize, zDiff / normalize);
		currentNumSteps = normalize / alpha;
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
	int i, j;
	
	float theta = 0;
	float beta = 0;
	float thetaRadians, betaRadians;
	float sphere_param = 10;
	float cone_width = 0.2;
	float num_triangles = 20;
	float cone_angle=0;
	float new_x, new_y, new_z;
	
	//Draws bones
	for(i=0; i<(int)world_bones.size(); i++) {
		BoneWorldSpace* bns = world_bones[i];
		glColor3f(bns->r, bns->g, bns->b);
		
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(bns->end_x, bns->end_y, bns->end_z);
			for(j=0; j<num_triangles; j++) {
			
				for(theta=-90; theta<=90; theta+=360/num_triangles) {
					for(beta=-180; beta<=180; beta+=360/num_triangles) {
						thetaRadians = degreesToRadians(theta);
						betaRadians = degreesToRadians(beta);
						glVertex3f(bns->start_x+cos(thetaRadians)*cos(betaRadians)*cone_width, bns->start_y+cos(thetaRadians)*sin(betaRadians)*cone_width, bns->start_z+sin(thetaRadians)*cone_width);
					}
				}
			}
		glEnd();
		
		if(should_move) {
			glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_POINTS);
				for(theta=-90; theta<=90; theta+=10) {
					for(beta=-180; beta<=180; beta+=10) {
						thetaRadians = degreesToRadians(theta);
						betaRadians = degreesToRadians(beta);
						glVertex3f(targetX+cos(thetaRadians)*cos(betaRadians)/sphere_param, targetY+cos(thetaRadians)*sin(betaRadians)/sphere_param, targetZ+sin(thetaRadians)/sphere_param);
					}
				}
			glEnd();
		}
		
		//printf("start = (%f, %f, %f), end = (%f, %f, %f)\n", bns->start_x, bns->start_y, bns->start_z, bns->end_x, bns->end_y, bns->end_z);
	}
	
	glColor3f(205.0f/255.0f, 190.0f/255.0f, 112.0f/255.0f);
	float ang = 0;
	float path_x, path_y, path_z;
	if (shapeIndex != 3)
	{
		while(ang<=360) {
			glBegin(GL_POINTS);
				path_z = 0;
				if(shapeIndex == 0) {
					path_x = circle_radius*cos(degreesToRadians(ang));
					path_y = circle_radius*sin(degreesToRadians(ang));
				}
				// Ellipse
				else if (shapeIndex == 1) {
					path_x = ellipse_a*cos(degreesToRadians(ang));
					path_y = ellipse_b*sin(degreesToRadians(ang));
				}
				// Eight
				else if (shapeIndex == 2) {
					float denominator = 1 + powf(sin(degreesToRadians(ang)), 2.0);
					path_x = eightParam * cos(degreesToRadians(ang)) / denominator;
					path_y = eightParam * cos(degreesToRadians(ang)) * sin(degreesToRadians(ang)) / denominator;
				}
				path_x += x_translation;
				path_y += y_translation;
				path_z += z_translation;
				glVertex3f(path_x, path_y, targetZ);
			glEnd();
			ang += 0.01;
		}
	}
	else
	{
		int numPoints = points->numPoints;
		for (int i = 0; i < numPoints; i++)
		{
			// Get next two points
			Point* A = points->myPoints[i % numPoints];
			Point* B = points->myPoints[(i + 1) % numPoints];

			// Calculate slope and normalization constants
			float xDiff = B->x - A->x;
			float yDiff = B->y - A->y;
			float zDiff = B->z - A->z;
			float normalize = sqrt(powf(xDiff, 2.0) + powf(yDiff, 2.0) + powf(zDiff, 2.0));
			Point* slope = new Point(xDiff / normalize, yDiff / normalize, zDiff / normalize);

			// Calculate how many points there will be and initialize current point
			float numSteps = normalize / alpha;
			float currentX = A->x;
			float currentY = A->y;
			float currentZ = A->z;

			// Draw Points
			for (int j = 0; j < numSteps; j++)
			{
				glBegin(GL_POINTS);
				glVertex3f(currentX + x_translation, currentY + y_translation, currentZ + z_translation);
				glEnd();
				currentX += slope->x * alpha;
				currentY += slope->y * alpha;
				currentZ += slope->z * alpha;
			}
			delete slope;
		}
	}
	glFlush();
	glutSwapBuffers();					// swap buffers (we earlier set double buffer)
}

//Calculates how much the bones should move, based on code from: http://www.ryanjuckett.com/programming/animation/21-cyclic-coordinate-descent-in-2d?start=4

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
	static float totalTime = 0.0f; //time difference since we last moved bones
	static float targetTime = 0.0f; //time difference since we last moved target
	
	//Calculate movement of bones
	float endX = world_bones[world_bones.size()-1]->end_x;
	float endY = world_bones[world_bones.size()-1]->end_y;
	float endZ = world_bones[world_bones.size()-1]->end_z;
	totalTime += dt;
	targetTime += dt;
	
	int i;
	
	//Change target location
	
	//KEVIN CHANGE THE 1.0F here to change the initial delay. IE, make value 0 for no delay.
	if(totalTime > 1.0f) {
		should_move = true;
		totalTime = 0.0f;
	}
	
	if(should_move) {
		if (targetTime > 0.05f)
		{
			// Circle
			if(shapeIndex == 0) {
				targetX = circle_radius*cos(degreesToRadians(targetAngle));
				targetY = circle_radius*sin(degreesToRadians(targetAngle));
				targetZ = 0;
			}
			// Ellipse
			else if (shapeIndex == 1) {
				targetX = ellipse_a*cos(degreesToRadians(targetAngle));
				targetY = ellipse_b*sin(degreesToRadians(targetAngle));
				targetZ = 0;
			}
			// Eight
			else if (shapeIndex == 2) {
				float denominator = 1 + powf(sin(degreesToRadians(targetAngle)), 2.0);
				targetX = eightParam * cos(degreesToRadians(targetAngle)) / denominator;
				targetY = eightParam * cos(degreesToRadians(targetAngle)) * sin(degreesToRadians(targetAngle)) / denominator;
				targetZ = 0;
			}
			// Points
			else if (shapeIndex == 3) {
				targetX = currentTargetX;
				targetY = currentTargetY;
				targetZ = currentTargetZ;
				currentTargetX += currentSlope->x * alpha;
				currentTargetY += currentSlope->y * alpha;
				currentTargetZ += currentSlope->z * alpha;
			}

			currentIterationIndex++;
			if (shapeIndex == 3 && currentIterationIndex == currentNumSteps)
			{
				currentPointsIndex = (currentPointsIndex + 1) % points->numPoints;
				currentIterationIndex = 0;
				delete currentSlope;

				// Get next two points
				Point* A = points->myPoints[currentPointsIndex];
				Point* B = points->myPoints[(currentPointsIndex + 1) % points->numPoints];

				// Calculate slope and normalization constants
				float xDiff = B->x - A->x;
				float yDiff = B->y - A->y;
				float zDiff = B->z - A->z;
				float normalize = sqrt(powf(xDiff, 2.0) + powf(yDiff, 2.0) + powf(zDiff, 2.0));
				currentSlope = new Point(xDiff / normalize, yDiff / normalize, zDiff / normalize);

				// Calculate how many points there will be and initialize current point
				currentNumSteps = normalize / alpha;
				currentTargetX = A->x;
				currentTargetY = A->y;
				currentTargetZ = A->z;
			}

			targetAngle += 0.01;
			if(targetAngle>360) {
				targetAngle = 0;
			}
			targetTime = 0;
			targetX += x_translation;
			targetY += y_translation;
			targetZ += z_translation;
		}

		if(totalTime>0.01f) {
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
	}
	
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
