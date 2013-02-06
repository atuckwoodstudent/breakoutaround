//////////////////////////////////////////////////////////////////////////
//	MODIFIED BY ALAN TUCKWOOD FOR USE WITH HIS BREAKOUT 3D GAME			//
//							15-01-2013									//
//////////////////////////////////////////////////////////////////////////
using namespace std;
#include <string>
#include "VisualDebugger.h"
#include "Simulation.h"
#include "GameClasses.h"
#include <GL/glut.h>

//extern variables, defined in Simulation.cpp
extern NxScene* scene;
extern NxReal delta_time;
extern NxActor* groundPlane;
extern Bat* player;
extern NxActor* killzone;
extern Map* map;
extern Ball* ball;
extern Stack <Flag*>* lives;
extern int Score;
extern bool endgame;

//global variables
bool bPause = false;
bool bShadows = true;
RenderingMode rendering_mode = RENDER_SOLID;
DebugRenderer gDebugRenderer;
const NxDebugRenderable* debugRenderable = 0;
HUD hud;

// Force globals
NxVec3	gForceVec(0,0,0);
NxReal	gForceStrength	= 20000;

// Keyboard globals
#define MAX_KEYS 256
bool gKeys[MAX_KEYS];

// Camera globals
float	gCameraAspectRatio = 1.0f;
NxVec3	gCameraPos(0,16,0);
NxVec3	gCameraForward(0,-1.3,1);
NxVec3	gCameraRight(-1,0,0);
const NxReal gCameraSpeed = 10;

//ASSIGN CALL BACK, SET UP LIGHTING, SETUP GLUT
void Init(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 768);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	//callbacks
	glutSetWindow(glutCreateWindow("Breakout!"));
	glutDisplayFunc(RenderCallback);
	glutReshapeFunc(ReshapeCallback);
	glutIdleFunc(IdleCallback);
	glutKeyboardFunc(KeyPress);
	glutKeyboardUpFunc(KeyRelease);
	glutSpecialFunc(KeySpecial);
	glutMouseFunc(MouseCallback);
	glutMotionFunc(MotionCallback);
	atexit(ExitCallback);

	//default render states
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);

	//lighting
	glEnable(GL_LIGHTING);
	float AmbientColor[]    = { 0.6f, 0.6f, 0.6f, 0.0f };	
	float DiffuseColor[]    = { 0.3f, 0.3f, 0.3f, 0.0f };			
	float SpecularColor[]   = { 0.5f, 0.5f, 0.5f, 0.0f };			
	float Position[]        = { 100.0f, 100.0f, -400.0f, 1.0f };

	//default lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientColor);	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DiffuseColor);	
	glLightfv(GL_LIGHT0, GL_SPECULAR, SpecularColor);	
	glLightfv(GL_LIGHT0, GL_POSITION, Position);
	glEnable(GL_LIGHT0);

	// Initialise the PhysX SDK.
	if (!InitPhysX())
	{
		printf("Could not initialise PhysX.\n");
		ExitCallback();
	}

	// Initialise the simulation scene.
	InitScene();
	MotionCallback(0,0);
	PrintControls();
	InitHUD();
}

//RUN FIRST STEP AND ENTER MAIN GLUT LOOP
void StartMainLoop() 
{
	glutMainLoop(); 
}

//INITIALIZE HUD
void InitHUD()
{
	hud.Clear();

	//pause message
	hud.AddDisplayString("", 0.3f, 0.55f);

	//score message
	hud.AddDisplayString("", 0.3f, 0.55f);

	//level message
	hud.AddDisplayString("", 0.3f, 0.55f);

	//end game message
	hud.AddDisplayString("", 0.3f, 0.55f);
}

//DISPLAY
void Display()
{
	//clear display buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//set the camera view
	SetupCamera();
	//display scene
	if (rendering_mode != RENDER_WIREFRAME)
		RenderActors(bShadows);
	if (debugRenderable)
		gDebugRenderer.renderData(*debugRenderable);
	//render HUD
	hud.Render();
	glFlush();
	glutSwapBuffers();
}

//PERFORM ONE STEP OF SIMULATION, PROCESS INPUT KETS AND RENDER THE SCENE.
void RenderCallback()
{
	if (scene && !bPause)
	{
		//get new results
		GetPhysicsResults();
		//use debug renderer if not solid display
		debugRenderable = 0;
		if (rendering_mode != RENDER_SOLID)
			debugRenderable = scene->getDebugRenderable();
		//user defined process function
		UpdateScene();
		//handle keyboard
		KeyHold();
		//start new simulation step
		SimulationStep();
	}
	Display();
}

//DRAW A FORCE ARROW
void DrawForce(NxActor* actor, NxVec3& forceVec, const NxVec3& color)
{
	if (actor)
	{
		// Draw only if the force is large enough
		NxReal force = forceVec.magnitude();
		if (force < 0.1)  return;
		forceVec = 2*forceVec/force;
		NxVec3 pos = actor->getCMassGlobalPosition();
		DrawArrow(pos, pos + forceVec, color);
	}
}

//DRAW ALL THE GAME ACTORS
void RenderActors(bool shadows)
{
	DrawActor(groundPlane,false);		//render groundplane
	glColor4f(1.0f, 0.0f, 0.0f,1.0f);	//render colour red
	DrawActor(killzone,false);			//render killzone
	DrawActorShadow(killzone, false);
	player->Render();					//render player
	ball->Render();						//render ball
	map->Render();						//render map layout

	Flag* in;							//local flag obj
	for(int i=0;i<lives->getAmount();i++)
	{	
		in= lives->getBuffer(i);		//get flag from stack	
		in->getActor()->setGlobalPosition(NxVec3(3.5*NxMath::sin(i/8.0*2*3.1415), 2, 3.5*NxMath::cos(i/8.0*2*3.1415))); //set correct position of flag
		in->Render();					//render the flags
	}
	UpdateHUD();
}


//SET UP CAMERA VIEW
void SetupCamera()
{
	gCameraAspectRatio = (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT);

	// Setup camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	gluLookAt(gCameraPos.x, gCameraPos.y, gCameraPos.z,
	gCameraPos.x+gCameraForward.x, gCameraPos.y+gCameraForward.y, gCameraPos.z+gCameraForward.z,
	0.0f, 1.0f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//SET UP CAMERA VIEW VARIABLES
void ResetCamera()
{
	gCameraAspectRatio = 1.0f;
	gCameraPos=NxVec3(0,16,0);
	gCameraForward=NxVec3(0,-1.3,1);
	gCameraRight=NxVec3(-1,0,0);
}

//CHANGE THE VIEWPORT AND ASPECT RATIO
void ReshapeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
	gCameraAspectRatio = float(width)/float(height);
}

//MOUSE VARIABLES
int mx = 0;
int my = 0;

//HANDLE MOUSE EVENT
void MouseCallback(int button, int state, int x, int y)
{
	mx = x;
	my = y;
}

//SET UP CAMERA VIEW MOVEMENT
void MotionCallback(int x, int y)
{
	int dx = mx - x;

	gCameraForward.normalize();
	gCameraRight.cross(gCameraForward,NxVec3(0,1,0));

	NxQuat qx(NxPiF32 * dx * 20 / 180.0f, NxVec3(0,1,0));
	qx.rotate(gCameraForward);

	mx = x;
}


//SINGLE KEY PRESS EVENT
void KeyPress(unsigned char key, int x, int y)
{
	if (!gKeys[key]) // ensure the keypress is only executed once
	{
		switch (key)
		{
		case 'b': //toggle between different rendering modes
			if (rendering_mode == RENDER_SOLID)	
				rendering_mode = RENDER_WIREFRAME;			//set wireframe
			else if (rendering_mode == RENDER_WIREFRAME)
				rendering_mode = RENDER_BOTH;				//set wireframe & solid
			else if (rendering_mode == RENDER_BOTH)
				rendering_mode = RENDER_SOLID;				//set solid
			break;
		case 'p':
			bPause = !bPause; 
			if (bPause)
				hud.SetDisplayString(1, "Paused - Hit \"p\" to Unpause", 0.3f, 0.55f);
			else
				hud.SetDisplayString(1, "", 0.3f, 0.55f);	
			getElapsedTime(); 
			break; 
		case 27: //ESC
			exit(0);
			break;
		case 32: //Space
			FireBall();
		default:
			break;
		}
	}
	gKeys[key] = true;
}

//HANDLE KEY RELEASES
void KeyRelease(unsigned char key, int x, int y)
{
	gKeys[key] = false;
}

//HANDLE HELD KEY EVENTS
void KeyHold()
{
	for (int i = 0; i < MAX_KEYS; i++)
	{	
		if (!gKeys[i])  { continue; }
		switch (i)
		{
		case 'a':
			MovePlayer(0,1,0); //move paddle left
			break;
		case 'd':
			MovePlayer(0,-1,0); //move paddle right
			break;
		case 'j':
			MotionCallback(mx-1,0);	//move camera left
			break;
		case 'l':
			MotionCallback(mx+1,0);	//move camera right
			break;
		}
	}
}

//HANDLE RESTART GAME SPECIAL KEY (F10)
void KeySpecial(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F10: // Reset PhysX and View
		ResetPhysX();
		ResetCamera();
		bPause=false;
		endgame=false;
		hud.SetDisplayString(3, "", 0.3f, 0.55f);
		Score=0;
		break; 
	default:
		break;
	}
}

void IdleCallback() { glutPostRedisplay(); }

//RELEASE PHYSX AND EXIT
void ExitCallback() { ReleasePhysX(); }

//GAME INSTRUCTIONS PRINTED IN CONSOLE
void PrintControls()
{
	printf("\n +++++++++++++++++++++++++++++\n ----BREAKOUT INSTRUCTIONS----\n +++++++++++++++++++++++++++++ \n");
	printf("\n Keyboard Controls:\n ----------------\n a = Move Paddle Left, d = Move Paddle Right, space = Release Ball \n j = Move Camera Left, l = Move Camera Right\n");
    printf("\n Mouse Controls:\n ---------------\n click+drag = Move Camera Left & Move Camera Right\n");
	printf("\n Miscellaneous:\n --------------\n p   = Pause\n b   = Toggle Visualisation Mode\n F10 = Reset Game\n ESC = Exit Game\n");
}

//ROTATE THE PLAYER AROUND 0,0,0 AT X,Y,Z
void MovePlayer(float x, float y, float z)
{
	NxMat33 orientX, orientY, orientZ;			//create rotation NxMat33 for x ,y & z
	orientX.rotX(NxMath::degToRad((NxF32)x/5));	//create x rotation
	orientY.rotY(NxMath::degToRad((NxF32)y/5));	//create y rotation
	orientZ.rotZ(NxMath::degToRad((NxF32)z/5));	//create z rotation
	player->getActor()->setGlobalOrientation(player->getActor()->getGlobalOrientation()*orientX*orientY*orientZ); //add rotation to current paddle rotation
}

//UPDATE GAME HUD ELEMENTS
void UpdateHUD()
{
	if(endgame)
	{
		bPause = true; 
		hud.SetDisplayString(3, "Game Over! Press F10 to restart!", 0.3f, 0.55f);
	}

	char printstring[32];

	sprintf(printstring, "%d", Score);
	char scorestring[64] = "Score: ";
	strcat(scorestring, printstring);

	sprintf(printstring, "%d", map->getCurrentMap()+1);
	char levelstring[64] = "Level: ";
	strcat(levelstring, printstring);

	sprintf(printstring, "%d", lives->getAmount());
	char flagstring[64] = "Flags Left: ";
	strcat(flagstring, printstring);

	hud.SetDisplayString(0,flagstring,0.44f,0.04f);	//render updated flag count
	hud.SetDisplayString(2,levelstring,0.84f,0.04f);	//render updated level number
	hud.SetDisplayString(1,scorestring,0.04f,0.04f); //render updated score
}