//////////////////////////////////////////////////////////////////////////
//						PRIMARY BREAKOUT 3D CLASSES						//				
//						CREATED BY ALAN TUCKWOOD						//
//								15-01-2013								//
//																		//
//	THIS LIST OF CLASS MEMBER DEFINITIONS DECLARES THE OBJECTS USED		//
//	WITHIN A STANDARD GAME OF PHYSX BREAKOUT WHERE THE KILLZONE IS		//
//	CENTRALLY PLACED.													//
//																		//	
//	THE CLASSES DEFINED HERE ARE:										//
//	-SHAPEFACTORY														//
//	-SPRITE																//
//	-BALL																//
//	-BAT																//
//	-FLAG																//
//	-POWER																//
//	-BLOCK																//
//	-MAP																//
//////////////////////////////////////////////////////////////////////////

#include "GameClasses.h"




//////////////////////////////////////////////////////////////////////////
//																		//
//					SHAPEFACTORY MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//						CREATE A NEW CLOTH								//
//////////////////////////////////////////////////////////////////////////
MyCloth* ShapeFactory::CreateCloth(const NxVec3& pos, const NxVec3& size, const NxVec3& wind, bool tearable, const NxReal stretch, const NxReal thick)
{
	NxClothDesc clothDesc;
	clothDesc.windAcceleration=wind;
	clothDesc.globalPose.t = pos;
	NxMat33 rot; rot.rotX(NxMath::degToRad((NxF32)-90));
	clothDesc.globalPose.M = rot;
	clothDesc.bendingStiffness=0.01;
	clothDesc.dampingCoefficient=1;
	clothDesc.thickness = thick;
	clothDesc.stretchingStiffness=stretch;
	clothDesc.tearFactor=2.28;

	if(tearable){	clothDesc.flags |= NX_CLF_TEARABLE;	}
	clothDesc.flags |= NX_CLF_COLLISION_TWOWAY  | NX_CLF_VISUALIZATION | NX_CLF_SELFCOLLISION;

	//IF SIZE>0
	if(size.x>0&&size.y>0&&size.z>0)	{		return new MyCloth(scene, clothDesc, size.x, size.y, size.z);	}
	//CREATE DEFAULT SIZE CLOTH
	else	{		return new MyCloth(scene, clothDesc, 20, 20, 100);	}
}
//////////////////////////////////////////////////////////////////////////
//						CREATE A NEW CUBOID OBJECT						//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateBox(const NxVec3& pos, const NxVec3& boxDim, const NxReal den,const NxVec3& rot,const NxVec3& friction)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;
	boxDesc.dimensions = boxDim;

	//MATERIAL PROPERTIES
	NxMaterialDesc materialDesc;
	materialDesc.staticFriction = friction.x;
	materialDesc.dynamicFriction = friction.y;
    NxMaterial *newMaterial=scene->createMaterial(materialDesc); 
	boxDesc.materialIndex= newMaterial->getMaterialIndex();

	actorDesc.shapes.pushBack(&boxDesc);
	actorDesc.globalPose.t = pos;

	//IF THERE THE OBJECT HAS A DENSITY (DYNAMIC)
	if(den>0)
	{
		actorDesc.body = &bodyDesc;
		actorDesc.density = den;
	}
		
	//ADD ROTATIONS
	NxMat33 orientX, orientY, orientZ;
	orientX.rotX(NxMath::degToRad((NxF32)rot.x));
	orientY.rotY(NxMath::degToRad((NxF32)rot.y));
	orientZ.rotZ(NxMath::degToRad((NxF32)rot.z));
	NxActor* box = scene->createActor(actorDesc);
	box->setGlobalOrientation(orientX*orientY*orientZ);
	
	return box;
}
//////////////////////////////////////////////////////////////////////////
//					CREATE A NEW STATIC PLANE							//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateGroundPlane()
{
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;

	//MATERIAL PROPERTIES
	NxMaterialDesc materialDesc;
	materialDesc.staticFriction = 0.25;
	materialDesc.dynamicFriction = 0.129;
	materialDesc.restitution=0.603;
    NxMaterial *newMaterial=scene->createMaterial(materialDesc); 
	planeDesc.materialIndex= newMaterial->getMaterialIndex();

	actorDesc.shapes.pushBack(&planeDesc);
	return scene->createActor(actorDesc);
}
//////////////////////////////////////////////////////////////////////////
//					CREATE A NEW SPHERE OBJECT							//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateSphere(const NxReal den,const NxReal rad,const NxVec3& pos,const NxVec3& res)
{
	NxBodyDesc bodyDesc;
	NxActorDesc actorDesc;

	NxSphereShapeDesc sphereDesc;

	//MATERIAL PROPERTIES
	NxMaterialDesc materialDesc;
	materialDesc.staticFriction = res.x;
	materialDesc.dynamicFriction = res.y;
	materialDesc.restitution=res.z;
    NxMaterial *newMaterial=scene->createMaterial(materialDesc); 
	sphereDesc.materialIndex= newMaterial->getMaterialIndex();

	sphereDesc.radius = rad;
	actorDesc.shapes.pushBack(&sphereDesc);
	if(den>0)
	{
		actorDesc.body            = &bodyDesc;
		actorDesc.density         = den;
	}
	actorDesc.globalPose.t    = pos;
	return scene->createActor(actorDesc);
}
//////////////////////////////////////////////////////////////////////////
//					CREATE A CONVEX MESH OBJECT							//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateConvex(const NxVec3& size, const NxVec3& position, NxPhysicsSDK* physx)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	//CREATE CONVEX MESH VERTICES
	NxVec3 verts[22];
	for(int i=0;i<11;i++)
	{
		verts[i]=NxVec3(size.x*NxMath::sin(i/11.0*2*3.1415), 0, size.z*NxMath::cos(i/11.0*2*3.1415));
	}
	for(int i=0;i<11;i++)
	{
		verts[i+11]=NxVec3(size.x*NxMath::sin(i/11.0*2*3.1415), size.y, size.z*NxMath::cos(i/11.0*2*3.1415));
	}

	//DESCRIPTORS
	NxConvexMeshDesc* convexDesc;
	NxConvexShapeDesc convexShapeDesc;

	convexDesc = new NxConvexMeshDesc();
	convexDesc->numVertices			= sizeof(verts)/sizeof(NxVec3);
	convexDesc->pointStrideBytes	= sizeof(NxVec3);
	convexDesc->points				= verts;
	convexDesc->flags				= NX_CF_COMPUTE_CONVEX;
	convexShapeDesc.userData	= convexDesc;

	//MESH COOKING
	NxInitCooking();
	MemoryWriteBuffer buf;
	bool status = NxCookConvexMesh(*convexDesc, buf);
	NxConvexMesh *pMesh			= physx->createConvexMesh(MemoryReadBuffer(buf.data));
	convexShapeDesc.meshData	= pMesh;
	NxCloseCooking();

	//SAVE MESH AND RETURN
	if (pMesh)
	{
		pMesh->saveToDesc(*convexDesc);
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		NxActor* hex = scene->createActor(actorDesc);
		hex->setGlobalPosition(position);
		return hex;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//						CREATE A GAME ENCLOSURE							//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateEnclosure(const NxVec3& size, const NxVec3& position, NxPhysicsSDK* physx)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxVec3 verts[12];
	
	//SET CONVEX MESH VERTICES
	for(int i=0;i<6;i++)
	{
		verts[i]=NxVec3(size.x*NxMath::sin(i/6.0*2*3.1415), 0, size.z*NxMath::cos(i/6.0*2*3.1415));
	}
	for(int i=0;i<6;i++)
	{
		verts[i+6]=NxVec3(size.x*NxMath::sin(i/6.0*2*3.1415), size.y, size.z*NxMath::cos(i/6.0*2*3.1415));
	}

	//CREATE 12 CONVEX MESHES
	NxConvexMeshDesc* convexDesc[24];
	NxConvexShapeDesc convexShapeDesc[24];
	
	//MATERIAL
	NxMaterialDesc materialDesc;
	materialDesc.staticFriction = 0.42;
	materialDesc.dynamicFriction =0.34;
	materialDesc.restitution=0.432;
    NxMaterial *newMaterial=scene->createMaterial(materialDesc); 

	//FOR EACH MESH
	for(int i=0;i<24;i++)
	{
		//ROTATE NXMAT33 FOR CIRCULAR ENCLOSURE
		NxMat33 rot;	rot.rotY(NxMath::degToRad((NxF32)(i*(180/12))));
		convexDesc[i] = new NxConvexMeshDesc();
		convexDesc[i]->numVertices			= sizeof(verts)/sizeof(NxVec3);
		convexDesc[i]->pointStrideBytes		= sizeof(NxVec3);
		convexDesc[i]->points				= verts;
		convexDesc[i]->flags				= NX_CF_COMPUTE_CONVEX;
		convexShapeDesc[i].localPose.t		= NxVec3(22*NxMath::sin(i/24.0*2*3.1415), 0, 22*NxMath::cos(i/24.0*2*3.1415));
		convexShapeDesc[i].localPose.M = rot;
		convexShapeDesc[i].userData	= convexDesc[i];
		convexShapeDesc[i].materialIndex= newMaterial->getMaterialIndex();
	}

	//MESH COOKING
	NxInitCooking();
	for(int i=0;i<24;i++)
	{
		MemoryWriteBuffer buf;
		NxCookConvexMesh(*convexDesc[i], buf);
		NxConvexMesh *pMesh	= physx->createConvexMesh(MemoryReadBuffer(buf.data));
		convexShapeDesc[i].meshData	= pMesh;
		pMesh->saveToDesc(*convexDesc[i]);
	}	
	NxCloseCooking();

	//ADD MESHES TO SHAPE DESCRIPTION
	for(int i=0;i<24;i++)
	{
		actorDesc.shapes.pushBack(&convexShapeDesc[i]);
	}

	//RETURN ENCLOSURE
	return  scene->createActor(actorDesc);
}
//////////////////////////////////////////////////////////////////////////
//					CREATE A SEMICIRCLE CONVEX MESH						//
//////////////////////////////////////////////////////////////////////////
NxActor* ShapeFactory::CreateBat(const NxVec3& size, const NxVec3& position, NxPhysicsSDK* physx)
{
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;

	//CREATE CONVEX MESH VERTICES
	NxVec3 verts[22];
	for(int i=0;i<11;i++)
	{
		verts[i]=NxVec3(size.x*NxMath::sin(i/22.0*2*3.1415), 0, size.z*NxMath::cos(i/22.0*2*3.1415));
	}
	for(int i=0;i<11;i++)
	{
		verts[i+11]=NxVec3(size.x*NxMath::sin(i/22.0*2*3.1415), size.y, size.z*NxMath::cos(i/22.0*2*3.1415));
	}

	NxConvexMeshDesc* convexDesc;
	NxConvexShapeDesc convexShapeDesc;

	convexDesc = new NxConvexMeshDesc();
	convexDesc->numVertices			= sizeof(verts)/sizeof(NxVec3);
	convexDesc->pointStrideBytes	= sizeof(NxVec3);
	convexDesc->points				= verts;
	convexDesc->flags				= NX_CF_COMPUTE_CONVEX;
	convexShapeDesc.localPose.t		= NxVec3(size.x*4.9,0,0);
	convexShapeDesc.userData	= convexDesc;

	//MESH COOKING
	NxInitCooking();
	MemoryWriteBuffer buf;
	bool status = NxCookConvexMesh(*convexDesc, buf);
	NxConvexMesh *pMesh			= physx->createConvexMesh(MemoryReadBuffer(buf.data));
	convexShapeDesc.meshData	= pMesh;
	NxCloseCooking();

	NxMaterialDesc materialDesc;
	materialDesc.staticFriction = 0.42;
	materialDesc.dynamicFriction =0.34;
	materialDesc.restitution=0.432;
    NxMaterial *newMaterial=scene->createMaterial(materialDesc); 
	convexShapeDesc.materialIndex= newMaterial->getMaterialIndex();

	//SAVE MESH AND RETURN
	if (pMesh)
	{
		pMesh->saveToDesc(*convexDesc);
		NxActorDesc actorDesc;
		actorDesc.shapes.pushBack(&convexShapeDesc);
		actorDesc.body		= &bodyDesc;
		actorDesc.density	= 600.0f;
		NxActor* hex = scene->createActor(actorDesc);
		hex->setGlobalPosition(position);
		return hex;
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//						CREATE A SPRING JOINT							//
//////////////////////////////////////////////////////////////////////////
NxDistanceJoint* ShapeFactory::CreateSpring(NxActor* a0, NxActor* a1, const NxVec3& anchor0, const NxVec3& anchor1, const NxVec3& globalAxis)
{
	//DISTANCE JOINT WITH ENABLED SPRING
	NxDistanceJointDesc distanceDesc;
	distanceDesc.actor[0] = a0;
	distanceDesc.actor[1] = a1;
	//LEFT AND RIGHT SPRING ANCHORS
	distanceDesc.localAnchor[0] = anchor0;
	distanceDesc.localAnchor[1] = anchor1;

	//AXIS OF SPRING
	distanceDesc.setGlobalAxis(globalAxis);

	distanceDesc.maxDistance = 0.14f; // maximum rest length of the spring
	distanceDesc.minDistance = 0.0f; // minimum rest length of the spring
	NxSpringDesc spring;
	spring.spring = 100;
	spring.damper = 2;
	spring.targetValue=0;
	distanceDesc.spring = spring;  
	distanceDesc.flags = (NX_DJF_MIN_DISTANCE_ENABLED | NX_DJF_MAX_DISTANCE_ENABLED);  // combination of the bits defined by ::NxDistanceJointFlag
	distanceDesc.flags |= NX_DJF_SPRING_ENABLED; //enable spring

	distanceDesc.jointFlags |= NX_JF_VISUALIZATION;
	distanceDesc.jointFlags |= NX_JF_COLLISION_ENABLED;

	//RETURN THE SPRING
	return (NxDistanceJoint*)scene->createJoint(distanceDesc);
}
//////////////////////////////////////////////////////////////////////////
//						CREATE A REVOLUTE JOINT							//
//////////////////////////////////////////////////////////////////////////
NxRevoluteJoint* ShapeFactory::CreateRevoluteJoint(NxActor* a0, NxActor* a1, NxVec3 globalAnchor, NxVec3 globalAxis)
{
	NxRevoluteJointDesc revDesc;

	//ACTORS TO CONNECT
	revDesc.actor[0] = a0;
	revDesc.actor[1] = a1;
	//ANCHOR OF JOINT
	revDesc.setGlobalAnchor(globalAnchor);
	//AXIS OF JOINT
	revDesc.setGlobalAxis(globalAxis);

	revDesc.jointFlags |= NX_JF_COLLISION_ENABLED;
	//RETURN THE JOINT
	return (NxRevoluteJoint*)scene->createJoint(revDesc);
}
//////////////////////////////////////////////////////////////////////////
//							CREATE A MOTOR								//
//////////////////////////////////////////////////////////////////////////
void ShapeFactory::CreateMotor(NxVec3& position, NxVec3& axis, NxActor* in){
	
	//CREATE JOINT AND MOTOR
	NxRevoluteJoint* flipjoint;
	NxMotorDesc flipjointmotorDesc;
	//DISABLE GRAVITY ON PASSED ACTOR
	in->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

	//CREATE JOINT TO EMPTY SPACE
	flipjoint = CreateRevoluteJoint(0, in, position, axis);
	flipjointmotorDesc.velTarget = 0.5;
	flipjointmotorDesc.maxForce = 30;
	flipjointmotorDesc.freeSpin = false;

	//SET MOTOR TO ACTOR
	flipjoint->setMotor(flipjointmotorDesc);
}
//////////////////////////////////////////////////////////////////////////
//																		//
//					END SHAPEFACTORY MEMBER FUNCTIONS					//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							SPRITE MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Sprite::Sprite()														
{
	Actor=0;			//init actor
	identifier = 0;		//init identifier
}
//HIDE DEFAULT SPRITE ACTOR	
void Sprite::Hide(NxScene* scene)
{
	scene->releaseActor(*Actor);	//release actor from scene
	Actor=0;						//reinit actor
}
//GET DEFAULT ACTOR
NxActor* Sprite::getActor() {
	return Actor;	//return the actor 
}
//GET SPRITE TYPE IDENTIFIER
int Sprite::getIdentifier() {
	return identifier;	//return the identifier
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END SPRITE MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							FLAG MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Flag::Flag(ShapeFactory* shapes, int den, NxVec3 &pos) { cloth=0; CreateActor(shapes,pos,den); }
//BUILD ACTOR
void Flag::CreateActor(ShapeFactory* shapes, NxVec3& Position, int den){
	Actor= shapes->CreateBox(Position,NxVec3(1,0.1,0.1),den,NxVec3(0,0,0),NxVec3(0.603,0.25,0.129));		//init actor
	cloth = shapes->CreateCloth(Position,NxVec3(0.4,0.8,0.4),NxVec3(-15,0,5),false,0.7,0.7);		//init cloth
	cloth->getNxCloth()->attachToShape(Actor->getShapes()[0],NX_CLOTH_ATTACHMENT_TWOWAY);			//attach cloth to actor to create flag
	NxMat33 orientX;	orientX.rotZ(NxMath::degToRad((NxF32)-90));									//rotate flag actor
	Actor->setGlobalOrientation(Actor->getGlobalOrientation()*orientX);
}
//GET FLAG CLOTH
MyCloth* Flag::getCloth(){	
	return cloth;	//return cloth object
}
//RENDER FLAG
void Flag::Render(){
	if (Actor)
	{
		glColor4f(1.0f, 1.0f, 1.0f,1.0f);	//set flagpole colour
		DrawActor(Actor, false);			//draw flagpole
		DrawActorShadow(Actor, false);		//draw flagpole shadow
	}
	if (cloth)
	{
		glColor4f(0.0f, 0.0f, 1.0f,1.0f);	//set cloth colour
		cloth->draw(false);					//draw cloth
	}
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END FLAG MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							BALL MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Ball::Ball() {	effect=0; }
//BUILD ACTOR
void Ball::CreateActor(NxScene* scene, ShapeFactory* shapes, int in) {
	identifier=in;																					//init identifier
	NxVec3 velocity;																				//create velocity vector
	NxVec3 startpos;																				//create position vector
	if(Actor)																						//if the ball actor exists
	{
		velocity = Actor->getLinearVelocity();														//velocity vector is ball velocity
		startpos = Actor->getGlobalPosition();														//position vector is ball position
		scene->releaseActor(*Actor);																//remove the ball from scene
	}
	else																							//if actor does not exist
	{
		velocity = NxVec3(0,0,0);																	//velocity vector is 0,0,0
		startpos = NxVec3(0,0,0);																	//position vector is 0,0,0
	}
	switch(identifier)																				
	{
		case 0: effect=1; Actor = shapes->CreateSphere(0.01,0.4,startpos,NxVec3(0.597,0.31,0.23)); break;	//if identifier is 0, create a normal ball
		case 1:	effect=3; Actor = shapes->CreateSphere(0.01,0.4,startpos,NxVec3(0.597,0.31,0.23)); break;	//if identifier is 1, create a killer ball
		case 2:	effect=1; Actor = shapes->CreateSphere(3,0.9,startpos,NxVec3(0.597,0.31,0.23)); break;		//if identifier is 2, create a wrecking ball
	}
	Actor->setLinearVelocity(velocity);																//set ball velocity as velocity vector
	Actor->setGroup(1);																				//set the balls group as 1
}
//RENDER ACTOR
void Ball::Render()	
{	
	if(Actor)														//if the ball exists
	{
		switch(identifier)										
		{
			case 0: glColor4f(1.0f, 1.0f, 1.0f,1.0f); break;		//if identifier is 0 (normal ball) ball colour is white
			case 1: glColor4f(1.0f, 0.0f, 0.0f,1.0f); break;		//if identifier is 1 (killer ball) ball colour is red
			case 2: glColor4f(0.4f, 0.4f, 0.4f,1.0f); break;		//if identifier is 2 (wrecking ball) ball colour is grey
		}
		DrawActor(Actor, false);									//draw the ball
		DrawActorShadow(Actor, false);								//draw the ball's shadow
	}
}
//GET BALL COLLISION EFFECT
int Ball::getEffect() {	return effect; }
//////////////////////////////////////////////////////////////////////////
//																		//
//						END BALL MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							BAT MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Bat::Bat()
{
	rightpower=0;		//init right bat addon
	leftpower=0;		//init left bat addon
}
//BUILD ACTOR
void Bat::CreateActor(NxPhysicsSDK* physx, NxScene* scene, ShapeFactory* shapes, int in) {
	identifier=in;																							//init identifier
	NxMat33 orientation;																					//init an orientation
	NxVec3 startpos;																						//init a position vector
	
	if(rightpower)																							//if the right bat addon exists
	{
		scene->releaseActor(*rightpower);																	//release the right bat addon from scene
		rightpower=0;																						//reinit right bat addon
	}
	if(leftpower)																							//if the left bat addon exists
	{		
		scene->releaseActor(*leftpower);																	//release the left bat addon from scene
		leftpower=0;																						//reinit the left bat addon
	}
	
	if(Actor)																								//if the bat exists
	{
		orientation = Actor->getGlobalOrientation();														//orientation is bat orientation
		startpos = Actor->getGlobalPosition();																//position vecotr is bat position
		scene->releaseActor(*Actor);																		//release the bat actor from scene
	}
	else																									//if the bat doesnt exists
	{
		startpos = NxVec3(0,0,0);																			//position vector is 0,0,0
		orientation.rotY(NxMath::degToRad(NxF32(-90)));														//orientation is -90degrees in Y
	}
	Actor = shapes->CreateBat(NxVec3(1,1,2),NxVec3(0,0,0),physx);											//create bat actor
	Actor->setGroup(4);																						//set bat actor as group 4
	Actor->setGlobalOrientation(orientation);																//set bat orientation as orientation
	
	switch(identifier)
	{
		case 1:																								//if bat is type 1 (spring bat)
			leftpower = shapes->CreateBox(startpos,NxVec3(0.5,0.5,0.5),0.1,NxVec3(0,0,0),NxVec3(0,0,0));	//create bat left addon
			leftpower->setGlobalOrientation(orientation);													//set bat left addon orientation as orientation
			rightpower = shapes->CreateBox(startpos,NxVec3(0.5,0.5,0.5),0.1,NxVec3(0,0,0),NxVec3(0,0,0));	//create bat right addon
			rightpower->setGlobalOrientation(orientation);													//set bat right addon orientation as orientation

			shapes->CreateSpring(Actor,leftpower,NxVec3(5.1f,0,2),NxVec3(-0.7,0.5,-0.5),NxVec3(0,0,1));		//create springs for left addon
			shapes->CreateSpring(Actor,leftpower,NxVec3(5.1f,1,2),NxVec3(-0.7,-0.5,-0.5),NxVec3(0,0,1));	
			shapes->CreateSpring(Actor,leftpower,NxVec3(5.6f,0,2),NxVec3(-0.7,0.5,0.5),NxVec3(0,0,1));
			shapes->CreateSpring(Actor,leftpower,NxVec3(5.6f,1,2),NxVec3(-0.7,-0.5,0.5),NxVec3(0,0,1));
			
			shapes->CreateSpring(Actor,rightpower,NxVec3(5.1f,0,-2),NxVec3(0.7,-0.5,0.5),NxVec3(0,0,1));	//create springs for right addon
			shapes->CreateSpring(Actor,rightpower,NxVec3(5.1f,1,-2),NxVec3(0.7,0.5,0.5),NxVec3(0,0,1));
			shapes->CreateSpring(Actor,rightpower,NxVec3(5.6f,0,-2),NxVec3(0.7,-0.5,-0.5),NxVec3(0,0,1));
			shapes->CreateSpring(Actor,rightpower,NxVec3(5.6f,1,-2),NxVec3(0.7,0.5,-0.5),NxVec3(0,0,1));

			rightpower->setGroup(4);																		//set bat right addon actor as group 4
			leftpower->setGroup(4);																			//set bat left addon actor as group 4
			break;
	}
	
}
//RENDER ACTOR
void Bat::Render()
{
	switch(identifier)	{
		case 0: glColor4f(1.0f, 1.0f, 1.0f,1.0f); break;			//if identifier is 0 (normal bat) set bat colour as white
		case 1: glColor4f(1.0f, 1.0f, 0.0f,1.0f); break;			//if identifier is 1 (spring bat) set bat colour as yellow
	}

	if(Actor){														//if the bat exists
		DrawActor(Actor, false);									//draw the bat											
		DrawActorShadow(Actor, false);								//draw the bat shadow
	}
	if(leftpower){													//if the bat left addon exists
		DrawActor(leftpower,false);									//draw the bat left addon
		DrawActorShadow(leftpower, false);							//draw the bat left addon shadow
	}

	if(rightpower){													//if the bat right addon exists
		DrawActor(rightpower,false);								//draw the bat right addon
		DrawActorShadow(rightpower, false);							//draw the bat right addon shadow
	}
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END BAT MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							POWER MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Power::Power(int in) {
	identifier = in;	//init identifier
}
//BUILD ACTOR
void Power::CreateActor(NxPhysicsSDK* physx, ShapeFactory* shapes, NxVec3 &startpos) {
	startpos.y=0;																								//set position y to 0 (power is on the floor)
	switch(identifier)																							
	{
	  default: Actor=0; break;																					//identifier is not 1,2,3 or 4: init actor as 0 for default (no power)
      case 1: Actor = shapes->CreateSphere(20,0.4,startpos,NxVec3(0.25,0.129,0.603));  break;								//identifier is 1 (killer ball powerup)
	  case 2: Actor = shapes->CreateSphere(20,1,startpos,NxVec3(0.25,0.129,0.603)); break;									//identifier is 2 (wrecking ball powerup)
	  case 3: Actor= shapes->CreateBox(startpos,NxVec3(1,0.1,0.1),20,NxVec3(0,0,0),NxVec3(0.25,0.129,0.603)); break;	//identifier is 3 (life add powerup)
	  case 4: Actor = shapes->CreateBox(startpos,NxVec3(0.5,0.5,0.5),0.1,NxVec3(0,0,0),NxVec3(0.25,0.129,0.603));			//identifier is 4 (spring bat addon)
	}
	if(Actor)
	{
		startpos.setMagnitude(3);																				//if the power actor exists
		Actor->setLinearVelocity(-startpos);																	//set the velocity towards 0,0,0
		Actor->setGroup(5);																						//set the power actor group to 5
		Actor->raiseActorFlag(NX_AF_DISABLE_RESPONSE);															//disable collision response in power (floats through objects in the way)
		Actor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);															//disable the power gravity (doesnt fall through the floor)
	}
}
//RENDER ACTOR
void Power::Render()
{
	if(Actor)
	{
		switch(identifier)
		{
			case 1: glColor4f(1.0f, 0.0f, 0.0f,1.0f); break;		//identifier is 1 (killer ball) set power colour as red
			case 2: glColor4f(0.4f, 0.4f, 0.4f,1.0f); break;		//identifier is 2 (wrecking ball) set power colour to grey
			case 3: glColor4f(1.0f, 1.0f, 1.0f,1.0f); break;		//identifier is 3 (life add) set power colour as white
			case 4: glColor4f(1.0f, 1.0f, 0.0f,1.0f); break;		//identifier is 4 (spring bat) set power colour as yellow
		}
		Actor->setGlobalPosition(NxVec3(Actor->getGlobalPosition().x,0,Actor->getGlobalPosition().z));		//set power position y as 0 (make sure its on the ground)
		NxMat33 rot;																						//rotate powerup each render
		rot.rotY(NxMath::degToRad((NxF32)(1)));
		Actor->setGlobalOrientation(Actor->getGlobalOrientation()*rot);
		DrawActor(Actor, false);																			//draw powerup
		DrawActorShadow(Actor, false);																		//draw powerup shadow
	}
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END POWER MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							BLOCK MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Block::Block(int powertype,int life, NxVec3& pos, NxVec3& rot,ShapeFactory* shapes){	//constructor for adding blocks in scene
	identifier = life;				//init identifier
	CreateActor(shapes,rot,pos);	//create actor
	power = new Power(powertype);	//create power
}
//DEFAULT CONSTRUCTOR
Block::Block() {	//constructor for blocks in arrays so that power is assigned
	power=0;	//init power
}
//HEALTH MODIFIER FUNCTION
bool Block::Hit(int amount, NxPhysicsSDK* physx, NxScene* scene, ShapeFactory* shapes){
	//IF BLOCK IS NOT STATIC
	if(identifier!=4)
		identifier = identifier-amount<0 ? 0: identifier-amount;		//if identifier-amount<0, set it as 0, else set it as identifier-amount
	
	if(identifier==0)
	{
		power->CreateActor(physx,shapes,Actor->getGlobalPosition());	//create power actor if block runs out of health
		Hide(scene);													//hide the block from the scene
		return true;													//return that the block has dissapeared (for score)
	}
	return false;														//return that the block has not dissapeared
}
//BUILD ACTOR
void Block::CreateActor(ShapeFactory* shapes, NxVec3& rot, NxVec3& pos) {
	//if identifier!=4 then create a dynamic block, else create a static block
	Actor = identifier!=4 ?	shapes->CreateBox(pos,NxVec3(1,0.5,0.5),5,rot,NxVec3(0.25,0.129,0.603)) : shapes->CreateBox(pos,NxVec3(1,0.5,0.5),0,rot,NxVec3(1,1,1));
	Actor->setGroup(2);		//set actor group as 2
}
//RETURN BLOCK POWER
Power* Block::getPower() {
	return power;			//return the power of the block
}
//RENDER ACTOR
void Block::Render(){  
	if(Actor)													//if the block exists
	{
		switch(identifier)
		{
			case 1: glColor4f(0.0f, 1.0f, 0.0f,1.0f); break;	//identifier is 1 (health of 1) set block colour as green
			case 2: glColor4f(0.0f, 0.0f, 1.0f,1.0f); break;	//identifier is 2 (health of 2) set block colour as blue
			case 3: glColor4f(1.0f, 0.0f, 0.0f,1.0f); break;	//identifier is 3 (health of 3) set block colour as red
			case 4: glColor4f(0.0f, 0.0f, 0.0f,1.0f); break;	//identifier is 4 (block is static) set block colour as black
		}
		DrawActor(Actor, false);								//draw block actor
		DrawActorShadow(Actor, false);							//draw block actor shadow
	}
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END BLOCK MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//																		//
//							MAP MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////
//CONSTRUCTOR
Map::Map(int in){
	totalMaps=in;	//init totalMaps
	blocks=0;
}
//FINAL MAP CHECKING FUNCTION
bool Map::isFinalMap(){
	return MapNumber==totalMaps;	//return true if last map, false if not
}
//GET CURRENT MAP
int Map::getCurrentMap(){
	return MapNumber;				//return the current map number
}
//FIND VISIBILITY OF BLOCKS
bool Map::hasBlocks(){
	for(int i=0;i<60;i++)										//loop through all blocks (60 of them)
		if(blocks[i].getActor() &&blocks[i].getIdentifier()!=4)	//if block[i] exists and is not static (has a health of 4)
			return true;										//return true (there is blocks remaining)
	return false;												//if no return then return false (there are no blocks remaining)
}
//SET MAP BLOCKS
void Map::SetMap(int map, ShapeFactory *shapes, NxScene* scene, NxPhysicsSDK* physx){

	if(blocks)								//if blocks is already set
	{
		for(int i=0;i<60;i++)				//loop through all blocks
		if(blocks[i].getActor())			//if block has an actor
			blocks[i].Hide(scene);			//remove the actor

		blocks=0;							//reinit blocks
	}

	arena=shapes->CreateEnclosure(NxVec3(3.2,9,3.2),NxVec3(0,0,0), physx);			//create game enclosure
	blocks = new Block[60];															//blocks is a new block array
	MapNumber=map;																	//current map number is map


	switch(map)																		
	{
		case 0:																		//if level is 0, arrange level 0 blocks
			blocks[0]= Block(0,3,NxVec3(-2,1,17),NxVec3(0,0,0),shapes);
			blocks[1]= Block(2,3,NxVec3(2,1,17),NxVec3(0,0,0),shapes);
			blocks[2]= Block(0,3,NxVec3(6,1,12),NxVec3(0,0,0),shapes);
			blocks[3]= Block(4,3,NxVec3(3,1,12),NxVec3(0,0,0),shapes);
			blocks[4]= Block(0,3,NxVec3(0,1,12),NxVec3(0,0,0),shapes);
			blocks[5]= Block(0,3,NxVec3(-3,1,12),NxVec3(0,0,0),shapes);
			blocks[6]= Block(3,3,NxVec3(-6,1,12),NxVec3(0,0,0),shapes);
			blocks[7]= Block(0,2,NxVec3(7,2,12),NxVec3(0,0,0),shapes);
			blocks[8]= Block(0,2,NxVec3(4.5,2,12),NxVec3(0,0,0),shapes);
			blocks[9]= Block(1,2,NxVec3(1.5,2,12),NxVec3(0,0,0),shapes);
			blocks[10]= Block(0,2,NxVec3(-1.5,2,12),NxVec3(0,0,0),shapes);
			blocks[11]= Block(0,2,NxVec3(-4.5,2,12),NxVec3(0,0,0),shapes);
			blocks[12]= Block(0,2,NxVec3(-7,2,12),NxVec3(0,0,0),shapes);
			blocks[13]= Block(0,1,NxVec3(6,3,12),NxVec3(0,0,0),shapes);
			blocks[14]= Block(4,1,NxVec3(3,3,12),NxVec3(0,0,0),shapes);
			blocks[15]= Block(1,1,NxVec3(0,3,12),NxVec3(0,0,0),shapes);
			blocks[16]= Block(0,1,NxVec3(-3,3,12),NxVec3(0,0,0),shapes);
			blocks[17]= Block(2,1,NxVec3(-6,3,12),NxVec3(0,0,0),shapes);
			blocks[18]= Block(0,3,NxVec3(-3,1,-12),NxVec3(0,0,90),shapes);
			blocks[19]= Block(4,3,NxVec3(-1,1,-12),NxVec3(0,0,90),shapes);
			blocks[20]= Block(0,3,NxVec3(-3,1,-14),NxVec3(0,0,90),shapes);
			blocks[21]= Block(0,3,NxVec3(-1,1,-14),NxVec3(0,0,90),shapes);
			blocks[22]= Block(1,2,NxVec3(-2,3,-12),NxVec3(0,0,0),shapes);
			blocks[23]= Block(2,2,NxVec3(-2,3,-14),NxVec3(0,0,0),shapes);
			blocks[24]= Block(0,3,NxVec3(-2,5,-13),NxVec3(0,90,0),shapes);
			blocks[25]= Block(3,1,NxVec3(-5,1,-12),NxVec3(0,0,90),shapes);
			blocks[26]= Block(0,1,NxVec3(-4,3,-12),NxVec3(0,0,0),shapes);
			blocks[27]= Block(0,1,NxVec3(-5,1,-14),NxVec3(0,0,90),shapes);
			blocks[28]= Block(4,1,NxVec3(-4,3,-14),NxVec3(0,0,0),shapes);
			blocks[29]= Block(0,2,NxVec3(3,1,-12),NxVec3(0,0,90),shapes);
			blocks[30]= Block(0,2,NxVec3(1,1,-12),NxVec3(0,0,90),shapes);
			blocks[31]= Block(0,2,NxVec3(3,1,-14),NxVec3(0,0,90),shapes);
			blocks[32]= Block(1,2,NxVec3(1,1,-14),NxVec3(0,0,90),shapes);
			blocks[33]= Block(2,1,NxVec3(2,3,-12),NxVec3(0,0,0),shapes);
			blocks[34]= Block(0,1,NxVec3(2,3,-14),NxVec3(0,0,0),shapes);
			blocks[35]= Block(0,3,NxVec3(2,5,-13),NxVec3(0,90,0),shapes);
			blocks[36]= Block(0,3,NxVec3(5,1,-12),NxVec3(0,0,90),shapes);
			blocks[37]= Block(4,3,NxVec3(4,3,-12),NxVec3(0,0,0),shapes);
			blocks[38]= Block(0,3,NxVec3(5,1,-14),NxVec3(0,0,90),shapes);
			blocks[39]= Block(0,3,NxVec3(4,3,-14),NxVec3(0,0,0),shapes);
			blocks[40]= Block(2,4,NxVec3(-10,0.5,-8),NxVec3(0,45,0),shapes);
			blocks[41]= Block(0,4,NxVec3(10,0.5,-8),NxVec3(0,-45,0),shapes);
			blocks[42]= Block(3,4,NxVec3(-10,0.5,8),NxVec3(0,-45,0),shapes);
			blocks[43]= Block(0,4,NxVec3(10,0.5,8),NxVec3(0,45,0),shapes);
			blocks[44]= Block(1,4,NxVec3(13,0.5,0),NxVec3(0,90,0),shapes);
			blocks[45]= Block(4,4,NxVec3(-13,0.5,0),NxVec3(0,90,0),shapes);
			blocks[46]= Block(0,3,NxVec3(6,1,6),NxVec3(0,45,0),shapes);
			blocks[47]= Block(2,1,NxVec3(6,1,-6),NxVec3(0,-45,0),shapes);
			blocks[48]= Block(0,3,NxVec3(-6,1,6),NxVec3(0,-45,0),shapes);
			blocks[49]= Block(0,1,NxVec3(-6,1,-6),NxVec3(0,45,0),shapes);
			shapes->CreateMotor(NxVec3(0,2,0),NxVec3(0,1,0),blocks[46].getActor());		//create rotating motor for 4 blocks
			shapes->CreateMotor(NxVec3(0,2,0),NxVec3(0,1,0),blocks[47].getActor());
			shapes->CreateMotor(NxVec3(0,2,0),NxVec3(0,1,0),blocks[48].getActor());
			shapes->CreateMotor(NxVec3(0,2,0),NxVec3(0,1,0),blocks[49].getActor());
			break;
			case 1:																		//if level is 1, arrange level 1 blocks
			blocks[0]= Block(0,2,NxVec3(-0.5,1,14),NxVec3(0,90,0),shapes);
			blocks[1]= Block(2,1,NxVec3(0.5,1,14),NxVec3(0,90,0),shapes);
			blocks[2]= Block(0,3,NxVec3(0,2,13.5),NxVec3(0,0,0),shapes);
			blocks[3]= Block(1,2,NxVec3(0,2,14.5),NxVec3(0,0,0),shapes);
			blocks[4]= Block(0,2,NxVec3(-0.5,3,14),NxVec3(0,90,0),shapes);
			blocks[5]= Block(3,1,NxVec3(0.5,3,14),NxVec3(0,90,0),shapes);
			blocks[6]= Block(4,3,NxVec3(0,4,13.5),NxVec3(0,0,0),shapes);
			blocks[7]= Block(1,2,NxVec3(0,4,14.5),NxVec3(0,0,0),shapes);	
			blocks[8]= Block(0,2,NxVec3(-0.5,5,14),NxVec3(0,90,0),shapes);
			blocks[9]= Block(0,3,NxVec3(0.5,5,14),NxVec3(0,90,0),shapes);
			blocks[10]= Block(4,1,NxVec3(0,6,13.5),NxVec3(0,0,0),shapes);
			blocks[11]= Block(1,2,NxVec3(0,6,14.5),NxVec3(0,0,0),shapes);	
			blocks[12]= Block(1,2,NxVec3(-0.5,7,14),NxVec3(0,90,0),shapes);
			blocks[13]= Block(0,3,NxVec3(0.5,7,14),NxVec3(0,90,0),shapes);
			blocks[56]= Block(0,4,NxVec3(0,1,8),NxVec3(0,0,0),shapes);
			blocks[14]= Block(0,1,NxVec3(14,1,-0.5),NxVec3(0,0,0),shapes);
			blocks[15]= Block(2,2,NxVec3(14,1,0.5),NxVec3(0,0,0),shapes);
			blocks[16]= Block(4,3,NxVec3(13.5,2,0),NxVec3(0,90,0),shapes);
			blocks[17]= Block(1,2,NxVec3(14.5,2,0),NxVec3(0,90,0),shapes);
			blocks[18]= Block(0,3,NxVec3(14,3,-0.5),NxVec3(0,0,0),shapes);
			blocks[19]= Block(2,1,NxVec3(14,3,0.5),NxVec3(0,0,0),shapes);
			blocks[20]= Block(0,2,NxVec3(13.5,4,0),NxVec3(0,90,0),shapes);
			blocks[21]= Block(1,2,NxVec3(14.5,4,0),NxVec3(0,90,0),shapes);
			blocks[22]= Block(0,3,NxVec3(14,5,-0.5),NxVec3(0,0,0),shapes);
			blocks[23]= Block(2,1,NxVec3(14,5,0.5),NxVec3(0,0,0),shapes);
			blocks[24]= Block(4,3,NxVec3(13.5,6,0),NxVec3(0,90,0),shapes);
			blocks[25]= Block(1,2,NxVec3(14.5,6,0),NxVec3(0,90,0),shapes);
			blocks[26]= Block(0,3,NxVec3(14,7,-0.5),NxVec3(0,0,0),shapes);
			blocks[27]= Block(2,1,NxVec3(14,7,0.5),NxVec3(0,0,0),shapes);
			blocks[57]= Block(0,4,NxVec3(8,1,0),NxVec3(0,90,0),shapes);
			blocks[28]= Block(0,2,NxVec3(-0.5,1,-14),NxVec3(0,90,0),shapes);
			blocks[29]= Block(2,1,NxVec3(0.5,1,-14),NxVec3(0,90,0),shapes);
			blocks[30]= Block(0,3,NxVec3(0,2,-13.5),NxVec3(0,0,0),shapes);
			blocks[31]= Block(1,2,NxVec3(0,2,-14.5),NxVec3(0,0,0),shapes);
			blocks[32]= Block(0,1,NxVec3(-0.5,3,-14),NxVec3(0,90,0),shapes);
			blocks[33]= Block(2,1,NxVec3(0.5,3,-14),NxVec3(0,90,0),shapes);
			blocks[34]= Block(0,3,NxVec3(0,4,-13.5),NxVec3(0,0,0),shapes);
			blocks[35]= Block(4,2,NxVec3(0,4,-14.5),NxVec3(0,0,0),shapes);	
			blocks[36]= Block(0,2,NxVec3(-0.5,5,-14),NxVec3(0,90,0),shapes);
			blocks[37]= Block(0,2,NxVec3(0.5,5,-14),NxVec3(0,90,0),shapes);
			blocks[38]= Block(3,3,NxVec3(0,6,-13.5),NxVec3(0,0,0),shapes);
			blocks[39]= Block(1,2,NxVec3(0,6,-14.5),NxVec3(0,0,0),shapes);	
			blocks[40]= Block(0,2,NxVec3(-0.5,7,-14),NxVec3(0,90,0),shapes);
			blocks[41]= Block(0,3,NxVec3(0.5,7,-14),NxVec3(0,90,0),shapes);
			blocks[58]= Block(1,4,NxVec3(0,1,-8),NxVec3(0,0,0),shapes);
			blocks[42]= Block(0,2,NxVec3(-14,1,-0.5),NxVec3(0,0,0),shapes);
			blocks[43]= Block(2,1,NxVec3(-14,1,0.5),NxVec3(0,0,0),shapes);
			blocks[44]= Block(0,2,NxVec3(-13.5,2,0),NxVec3(0,90,0),shapes);
			blocks[45]= Block(1,2,NxVec3(-14.5,2,0),NxVec3(0,90,0),shapes);
			blocks[46]= Block(0,3,NxVec3(-14,3,-0.5),NxVec3(0,0,0),shapes);
			blocks[47]= Block(2,1,NxVec3(-14,3,0.5),NxVec3(0,0,0),shapes);
			blocks[48]= Block(0,2,NxVec3(-13.5,4,0),NxVec3(0,90,0),shapes);
			blocks[49]= Block(1,2,NxVec3(-14.5,4,0),NxVec3(0,90,0),shapes);
			blocks[50]= Block(0,3,NxVec3(-14,5,-0.5),NxVec3(0,0,0),shapes);
			blocks[51]= Block(2,1,NxVec3(-14,5,0.5),NxVec3(0,0,0),shapes);
			blocks[52]= Block(0,3,NxVec3(-13.5,6,0),NxVec3(0,90,0),shapes);
			blocks[53]= Block(1,2,NxVec3(-14.5,6,0),NxVec3(0,90,0),shapes);
			blocks[54]= Block(0,3,NxVec3(-14,7,-0.5),NxVec3(0,0,0),shapes);
			blocks[55]= Block(2,1,NxVec3(-14,7,0.5),NxVec3(0,0,0),shapes);
			blocks[59]= Block(4,4,NxVec3(-8,1,0),NxVec3(0,90,0),shapes);
			break;
			case 2:																		//if level is 2, arrange level 2 blocks
			blocks[0]= Block(4,3,NxVec3(0,1,9),NxVec3(0,0,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[0].getActor());		//create rotating motor for associated block
			blocks[1]= Block(1,2,NxVec3(9,1,0),NxVec3(0,90,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[1].getActor());
			blocks[2]= Block(0,1,NxVec3(-9,1,0),NxVec3(0,90,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[2].getActor());
			blocks[3]= Block(2,2,NxVec3(0,1,-9),NxVec3(0,0,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[3].getActor());
			blocks[4]= Block(0,3,NxVec3(9,1,9),NxVec3(0,45,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[4].getActor());		
			blocks[5]= Block(3,1,NxVec3(9,1,-9),NxVec3(0,135,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[5].getActor());
			blocks[6]= Block(0,2,NxVec3(-9,1,9),NxVec3(0,135,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[6].getActor());
			blocks[7]= Block(0,1,NxVec3(-9,1,-9),NxVec3(0,45,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[7].getActor());
			blocks[8]= Block(1,2,NxVec3(0,1,12),NxVec3(0,0,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[8].getActor());
			blocks[9]= Block(0,1,NxVec3(12,1,0),NxVec3(0,90,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[9].getActor());
			blocks[10]= Block(0,2,NxVec3(-12,1,0),NxVec3(0,90,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[10].getActor());
			blocks[11]= Block(2,2,NxVec3(0,1,-12),NxVec3(0,0,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[11].getActor());
			blocks[12]= Block(0,3,NxVec3(12,1,12),NxVec3(0,45,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[12].getActor());	
			blocks[13]= Block(1,1,NxVec3(-12,1,12),NxVec3(0,135,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[13].getActor());
			blocks[14]= Block(0,3,NxVec3(12,1,-12),NxVec3(0,135,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[14].getActor());
			blocks[15]= Block(3,1,NxVec3(-12,1,-12),NxVec3(0,45,0),shapes);
			shapes->CreateMotor(NxVec3(0,0,0),NxVec3(0,1,0),blocks[15].getActor());
			break;
	}
}
//FIND SPECIFIC BLOCK
Block* Map::blockexists(NxActor* in){
	for(int i=0;i<60;i++)				//loop through all blocks
		if(blocks[i].getActor()==in)	//if a specific block is found
			return &blocks[i];			//return that specific block
	return 0;							//if no block is found then return 0
}
//FIND SPECIFIC POWER
Power* Map::powerexists(NxActor* in){
	for(int i=0;i<60;i++)												//loop through all blocks
		if(blocks[i].getPower()&&blocks[i].getPower()->getActor()==in)	//if a specific block power is found
			return blocks[i].getPower();								//return that specific power
	return 0;															//if no power is found then return 0
}
//REMOVE POWERS FROM SCENE
void Map::ClearPowers(NxScene* scene)
{
	for(int i=0;i<60;i++)												//loop through all blocks
		if(blocks[i].getPower())	//if a  block power exists
			blocks[i].getPower()->Hide(scene);								//remove it from the scene													
}
//RENDER ALL MAP ACTORS
void Map::Render(){
	for(int i=0;i<60;i++)						//loop through all blocks
	{
		if(blocks[i].getActor())				//if block[i] exists
			blocks[i].Render();					//render that block
		if(blocks[i].getPower())				//if block[i]s power exists
			blocks[i].getPower()->Render();		//render that block power
	}
	glColor4f(1, 1, 1,1.0f);					//set the enclosure colour to white
	DrawActor(arena, false);					//draw the enclosure
	DrawActorShadow(arena, false);				//draw the enclosure shadow
}
//////////////////////////////////////////////////////////////////////////
//																		//
//						END MAP MEMBER FUNCTIONS						//
//																		//
//////////////////////////////////////////////////////////////////////////