//////////////////////////////////////////////////////////////////////////
//						PRIMARY BREAKOUT 3D CLASSES						//				
//						CREATED BY ALAN TUCKWOOD						//
//								15-01-2013								//
//																		//
//	THIS LIST OF CLASS DEFINITIONS DECLARES THE OBJECTS USED			//
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
//																		//
//	THIS HEADER ALSO DEFINES THE CLASS AND MEMBER FUNCTIONS OF A		//
//	TEMPLATE STACK CLASS.												//
//////////////////////////////////////////////////////////////////////////


#include <string>					//req for string type
#include "NxPhysics.h"				//req for physx
#include "NxCooking.h"				//req for mesh cooking
#include "Extras\Stream.h"			//req for mesh cooking
#include "Extras\MyCloth.h"			//req for cloths
#include "Extras\DrawObjects.h"		//req for rendering


//////////////////////////////////////////////////////////////////////////
//																		//
//							SHAPEFACTORY CLASS							//
//																		//
//	CREATION CLASS. THIS CLASS BUILDS THE ACTORS AND ACTOR CONNECTIONS	//
//	FOR OBJECTS IN THE SCENE											//
//////////////////////////////////////////////////////////////////////////
class ShapeFactory{
private:
	NxScene* scene;
public:
	ShapeFactory(NxScene* _scene)	//constructor
	{
		 scene = _scene;	//set a local pointer to the main physx scene
	};
	~ShapeFactory() {	};																					//destructor
	MyCloth* CreateCloth(const NxVec3&, const NxVec3&, const NxVec3&, bool, const NxReal, const NxReal);	//create a cloth
	NxActor* CreateBox(const NxVec3&, const NxVec3&, const NxReal,const NxVec3&,const NxVec3&);				//create a cuboid
	NxActor* CreateSphere(const NxReal,const NxReal,const NxVec3&,const NxVec3&);							//create a sphere
	NxActor* CreateGroundPlane();																			//create a ground plane
	NxActor* CreateConvex(const NxVec3&,const NxVec3&,NxPhysicsSDK*);										//create a convex mesh
	NxActor* CreateEnclosure(const NxVec3&,const NxVec3&,NxPhysicsSDK*);									//create a convex mesh based enclosure
	NxActor* CreateBat(const NxVec3&,const NxVec3&,NxPhysicsSDK*);											//create a player paddle
	void CreateMotor(NxVec3&,NxVec3&,NxActor*);																//create a motor for an actor
	NxDistanceJoint* CreateSpring(NxActor*, NxActor*, const NxVec3&, const NxVec3&, const NxVec3&);			//create a spring between two actors
	NxRevoluteJoint* CreateRevoluteJoint(NxActor*, NxActor*, NxVec3, NxVec3);								//create a revolute joing between two actors
};


//////////////////////////////////////////////////////////////////////////
//								SPRITE CLASS							//
//																		//
//	PARENT OF BALL, BAT, FLAG, POWER AND BLOCK CLASSES.	THIS CLASS		//
//	DEFINES SHARED FEATURES OF ALL OF ITS CHILDREN (AT LEAST ONE		//
//	ACTOR, ABILITY TO HIDE THAT ACTOR ETC).								//
//////////////////////////////////////////////////////////////////////////
class Sprite{
protected:
	NxActor* Actor;					//actor used as default for all children classes
	int identifier;					//type switcher for actors with different forms (different block types etc)
public:
	Sprite();						//constructor
	~Sprite(){};					//destructor
	virtual void CreateActor(){};	//virtual function required for child implementation
	void Hide(NxScene*);			//remove the actor from the scene
	NxActor* getActor();			//return the actor
	int getIdentifier();			//return the identifier
};

//////////////////////////////////////////////////////////////////////////
//								BALL CLASS								//
//																		//
//	CHILD OF SPRITE. THIS CLASS DESCRIBES THE BOUNCING BALL IN THE		//
//	SCENE. IDENTIFIER REPRESENTS THE TYPE OF BALL CURRENTLY ACTIVE		//
//////////////////////////////////////////////////////////////////////////
class Ball: public Sprite
{
private:
	int effect;										//effector, used for determining how much health is taken from a block
public:	
	Ball();											//constructor
	~Ball(){};										//destructor
	void CreateActor(NxScene*,ShapeFactory*, int);	//create the actor
	int getEffect();								//return the effector
	void Render();									//render the actor
};

//////////////////////////////////////////////////////////////////////////
//								BAT CLASS								//
//																		//
//	CHILD OF SPRITE. THIS CLASS DESCRIBES THE PLAYER PADDLE IN THE		//
//	SCENE. IDENTIFIER REPRESENTS THE TYPE OF BAT CURRENTLY ACTIVE		//
//////////////////////////////////////////////////////////////////////////
class Bat: public Sprite
{
private:
	NxActor* leftpower;												//actor used for left paddle attachment addon
	NxActor* rightpower;											//actor used for right paddle attachment addon
public:
	Bat();															//constructor
	~Bat(){};														//destructor
	void CreateActor(NxPhysicsSDK*,NxScene*,ShapeFactory*, int);	//create the actor
	void Render();													//render the actor
};

//////////////////////////////////////////////////////////////////////////
//								FLAG CLASS								//
//																		//
//	CHILD OF SPRITE. THIS CLASS DESCRIBES THE FLAGS USED TO REPRESENT	//
//	PLAYER REMAINING LIVES												//
//////////////////////////////////////////////////////////////////////////
class Flag: public Sprite
{
private:
	MyCloth* cloth;									//flag cloth
public:
	Flag(ShapeFactory*,int,NxVec3&);				//constructor
	~Flag(){};										//destructor
	void CreateActor(ShapeFactory*,NxVec3&,int);	//create the actor
	void Render();									//render the cloth and the actor
	MyCloth* getCloth();							//return the cloth
};


//////////////////////////////////////////////////////////////////////////
//								POWER CLASS								//
//																		//
//	CHILD OF SPRITE. THIS CLASS DESCRIBES THE POWERUPS IN THE GAME		//
//	SCENE. IDENTIFIER REPRESENTS THE TYPE OF POWERUP.					//
//////////////////////////////////////////////////////////////////////////
class Power: public Sprite
{
public:
	Power(int);													//constructor
	~Power(){};													//destructor
	void CreateActor(NxPhysicsSDK*, ShapeFactory*, NxVec3&);	//create the actor
	void Render();												//render the actor
};


//////////////////////////////////////////////////////////////////////////
//								BLOCK CLASS								//
//																		//
//	CHILD OF SPRITE. THIS CLASS DESCRIBES THE VARIOUS BLOCKS IN THE		//
//	SCENE. IDENTIFIER REPRESENTS THE TYPE OF BLOCK						//
//////////////////////////////////////////////////////////////////////////
class Block: public Sprite
{
private:
	Power* power;											//power contained in destroyed block
public:
	Block();												//default constructor
	Block(int,int,NxVec3&,NxVec3&,ShapeFactory*);			//constructor
	~Block(){};												//destructor
	bool Hit(int,NxPhysicsSDK*,NxScene*,ShapeFactory*);		//hit the actor, affects the identifier
	void CreateActor(ShapeFactory*,NxVec3&,NxVec3&);		//create the actor
	Power* getPower();										//return power
	void Render();											//render the actor
};

//////////////////////////////////////////////////////////////////////////
//								MAP CLASS								//
//																		//
//	THIS CLASS CONTROLS THE BLOCKS AND THEIR LAYOUTS IN THE SCENE.		//
//////////////////////////////////////////////////////////////////////////
class Map
{
	private:
		Block* blocks;														//block array for layouts
		NxActor* arena;														//actor of the game enclosure
		int MapNumber;														//current map number or 'level'
		int totalMaps;														//total maps in the game
	public:
		Map(int);															//constructor
		~Map(){};															//destructor
		void SetMap(int, ShapeFactory *shapes, NxScene*, NxPhysicsSDK*);	//initialize the map to a certain mapnumber
		bool hasBlocks();													//return if there are visible blocks
		int getCurrentMap();												//return mapnumber
		Block* blockexists(NxActor*);										//return block if it exists
		Power* powerexists(NxActor*);										//return power if it exists
		bool isFinalMap();													//return if map==totalmaps
		void ClearPowers(NxScene*);													//remove active powers from the scene
		void Render();														//render all blocks and blocks powers in map
};


//////////////////////////////////////////////////////////////////////////
//																		//
//				TEMPLATE STACK CLASS AND MEMBER FUNCTIONS				//
//																		//
//	THIS ADDITIONAL CLASS IS NECESSARY TO COLLECT THE LIVES OF THE		//
//	PLAYER, WHICH WOULD OTHERWISE BE UNKNOWN IN QUANTITY DURING RUNTIME	//
//////////////////////////////////////////////////////////////////////////
template <class E> class Stack					
{
	private:									
		int maxSize;								//stack maximum size
		int currentSize;							//current stack size
		E* buffer;									//stack array

	public:
		Stack(int _max){							//constructor
			maxSize=_max;							
			currentSize=0;							
			buffer = new E[maxSize];				
		};
		void push(E _in)							//add object function
		{
			if(currentSize<maxSize)					
			{
				buffer[currentSize]=_in;
				currentSize++;
			}
		}
		E* pop()									//remove and return most recent object function
		{
			if(currentSize>0)
			{
				currentSize--;
				E *temp = &buffer[currentSize];
				return temp;
			}
			return 0;
		}
		E* top()									//return most recent object function
		{
			if(currentSize>0)
			{
				return &buffer[currentSize-1];
			}
			return 0;
		}
		int getAmount()								//get size of current stack
		{
			return currentSize;
		}
		E getBuffer(int in)							//get stack array
		{
			return buffer[in];
		}
};
