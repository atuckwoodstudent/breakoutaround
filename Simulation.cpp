//////////////////////////////////////////////////////////////////////////
//						PRIMARY BREAKOUT SIMULATION						//				
//						CREATED BY ALAN TUCKWOOD						//
//								15-01-2013								//
//																		//
//	THIS FILE EXPANDS THE MEMBERS USED FOR THE SIMULATION OF THE		//
//	BREAKOUT GAME.														//
//////////////////////////////////////////////////////////////////////////

#include "Simulation.h"
#include <stdio.h>
#include "GameClasses.h"
#include "Extras\HUD.h"
#include "VisualDebugger.h"

//global variables
NxPhysicsSDK* physx = 0;
NxActor* groundPlane = 0;
Bat* player=0;
NxActor* killzone=0;
NxActor* arena=0;
Map* map =0;
Ball* ball = 0;
Stack <Flag*>* lives;
NxScene* scene = 0;
NxReal delta_time;
ShapeFactory* shapes;
int Score=0;
bool HasLocked, endgame;
MyContactReport contactReport;

//INITIALIZE PHYSX, GRAVITY ETC
bool InitPhysX(){
	physx = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION);		//initialise the SDK
	if (!physx) 
		return false;

	//visual debugging 
	physx->setParameter(NX_VISUALIZATION_SCALE, 1);
	physx->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	physx->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);

	//attach a remote debugger: PhysX Visual Debugger
	if (physx->getFoundationSDK().getRemoteDebugger())
		physx->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425, NX_DBG_EVENTMASK_EVERYTHING);

    //scene descriptor
    NxSceneDesc sceneDesc;
	sceneDesc.gravity = NxVec3(0,-9.8f,0);	//default gravity

	//check the hardware option first
	sceneDesc.simType = NX_SIMULATION_HW;
    scene = physx->createScene(sceneDesc);


	//if not available run in software
	if(!scene)
	{ 
		sceneDesc.simType = NX_SIMULATION_SW; 
		scene = physx->createScene(sceneDesc);  
		if(!scene)
			return false;
	}

	shapes=new ShapeFactory(scene);
	scene->setActorGroupPairFlags(1,2, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	scene->setActorGroupPairFlags(1,3, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	scene->setActorGroupPairFlags(4,5, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	scene->setActorGroupPairFlags(3,5, NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_END_TOUCH);
	//update the current time
	getElapsedTime();

	return true;
}

//RELEASE ALL RESOURCES
void ReleasePhysX(){
	if (scene)
		physx->releaseScene(*scene);
	if (physx)
		physx->release();
}

//RESTART THE SDK AND SCENE
void ResetPhysX(){
	ReleasePhysX();
	InitPhysX();
	InitScene();
}

//START SIMULATION PROCESSING
void SimulationStep(){
	// Update the time step
	delta_time = getElapsedTime();

	// perform a simulation step for delta time since the last frame
	scene->simulate(delta_time);
	scene->flushStream();
}

//COLLECT SIMULATION RESULTS
void GetPhysicsResults(){
	scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
}

//RESET THE GAME BALL
void ResetBall(){
	player->CreateActor(physx,scene,shapes,0);		//reset player actor to remove any powerups
	ball->CreateActor(scene,shapes,0);				//reset ball actor to remove any powerups
	HasLocked=true;									//set locked to the paddle
	NxVec3 resetposition = player->getActor()->getGlobalPosition();	//get the player position
	resetposition.y++;												
	ball->getActor()->setGlobalPosition(resetposition);				//set the ball position
	ball->getActor()->getShapes()[0]->setLocalPosition(player->getActor()->getShapes()[0]->getLocalPosition()+NxVec3(2,0,0));	//set ball shape offset for shared bat rotation
}

//LAUNCH THE BALL FROM THE BAT
void FireBall(){
	ball->getActor()->setLinearVelocity(ball->getActor()->getGlobalOrientation()*NxVec3(1,0,0));	//set velocity of ball to 1,0,0 multiplied by the orientation of the bat
	HasLocked=false;																				//set locked to the bat as false
	ball->getActor()->setGlobalPosition(ball->getActor()->getShapes()[0]->getGlobalPosition());		//set global position to + shape offset
	ball->getActor()->getShapes()[0]->setLocalPosition(NxVec3(0,0,0));								//set shape offset to 0,0,0
}

//INITIALIZE THE PHYSX SCENE AND GAME OBJECTS
void InitScene()
{
	scene->setUserContactReport(&contactReport);	//contact reports for collisions
	groundPlane = shapes->CreateGroundPlane();		//create ground plane

	map = new Map(2);								//create map with 3 levels
	map->SetMap(0,shapes,scene,physx);					//set the current map as 0

	killzone = shapes->CreateConvex(NxVec3(4,1,4),NxVec3(0,0,0),physx);		//create the killzone
	killzone ->setGroup(3);													//set the killzone group to 3
		
	lives=new Stack<Flag*>(8);								//create a stack to hold the lives
	for(int i=0;i<3;i++)									
		lives->push(new Flag(shapes,0,NxVec3(-4,4,0)));		//add three initial lives

	player= new Bat();										//create ther player

	ball= new Ball();										//create the ball
	ball->CreateActor(scene,shapes,0);						//set the ball to a normal ball

	ResetBall();											//attach the ball to the bat
}


void UpdateScene(){	
	//if the map has no dynamic blocks 
	if(!map->hasBlocks())
		//if this is not the last map
		if(!map->isFinalMap())
			map->SetMap(map->getCurrentMap()+1,shapes,scene,physx);
		//otherwsie show the gameover sceen
		else
			endgame=true;

	//if the ball is not locked to the bat
	if(!HasLocked)
	{
		NxVec3 resetvelocity = ball->getActor()->getLinearVelocity();
		resetvelocity.setMagnitude(8);
		resetvelocity.y=0;	//keep the ball on the ground (stops bouncing over the arena)
		ball->getActor()->setLinearVelocity(resetvelocity);	
	}
	//if the ball is locked to the bat
	else
	{
		ball->getActor()->setLinearVelocity(NxVec3(0,0,0));
		ball->getActor()->setAngularVelocity(NxVec3(0,0,0));
		ball->getActor()->setGlobalPosition(NxVec3(ball->getActor()->getGlobalPosition().x,0,ball->getActor()->getGlobalPosition().z));
		ball->getActor()->setGlobalOrientation(player->getActor()->getGlobalOrientation());
	}
}

void MyContactReport::onContactNotify(NxContactPair& pair, NxU32 events) {
	//check if ON_START_TOUCH event
	if (events & NX_NOTIFY_ON_START_TOUCH)
	{
		//if a block power is one of the collision objects
		Power* power = map->powerexists(pair.actors[0]);
		Power* power2 = map->powerexists(pair.actors[1]);
		if(power)
		{
			//if the player bat is the other object
			if(pair.actors[1]==player->getActor())
			{
				int upgrade= power->getIdentifier();	
				switch(upgrade)		//apply upgrades
				{
					case 1: ball->CreateActor(scene,shapes,1); break;
					case 2: ball->CreateActor(scene,shapes,2); break;
					case 3:	lives->push(new Flag(shapes,0,NxVec3(0,0,0))); break;
					case 4: player->CreateActor(physx,scene,shapes,1);
				}
				Score+=5;
			}
			power->Hide(scene);		//always hide the powerup
		}
		else if(power2)
		{
			//if the player bat is the other object
			if(pair.actors[0]==player->getActor())
			{
				int upgrade= power2->getIdentifier();
				switch(upgrade)		//apply upgrades
				{
					case 1: ball->CreateActor(scene,shapes,1); break;
					case 2: ball->CreateActor(scene,shapes,2); break;
					case 3:	lives->push(new Flag(shapes,0,NxVec3(0,0,0))); break;
					case 4: player->CreateActor(physx,scene,shapes,1);
				}
				Score+=5;
			}
			power2->Hide(scene);	//always hide the powerup
		}		
	}
	//check if ON_END_TOUCH event
	if (events & NX_NOTIFY_ON_END_TOUCH)
	{
		//if a block is one of the associated actors colliding
		Block* block = map->blockexists(pair.actors[0]);
		Block* block2 = map->blockexists(pair.actors[1]);

		if(block)
		{	
			//remove ball effect from the health of the block, if block breaks then add score
			if(block->Hit(ball->getEffect(),physx,scene,shapes))
				Score+=10;
		}
		else if(block2)
		{
			//remove ball effect from the health of the block, if block breaks then add score
			if(block2->Hit(ball->getEffect(),physx,scene,shapes))
					Score+=10;
		}

		//if the colliding actors are the ball and the killzone then remove a life and reset the ball
		if(pair.actors[0]==ball->getActor()&&pair.actors[1]==killzone||pair.actors[1]==ball->getActor()&&pair.actors[0]==killzone)
			if(lives->top())
			{
				Flag* temp = *lives->pop();
				scene->releaseActor(*temp->getActor());
				scene->releaseCloth(*temp->getCloth()->getNxCloth());
				map->ClearPowers(scene);	//This function is required to avoid powerups applying whilst the ball is connected to the paddle.
				ResetBall();
			}
			else
			{
				endgame=true;
			}
	}
}










