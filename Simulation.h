//////////////////////////////////////////////////////////////////////////
//						PRIMARY BREAKOUT SIMULATION						//				
//						CREATED BY ALAN TUCKWOOD						//
//								15-01-2013								//
//																		//
//	THIS FILE DEFINES THE MEMBERS USED FOR THE SIMULATION OF THE		//
//	BREAKOUT GAME.														//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "NxPhysics.h"
#include "NxCooking.h"

/// Initialise the scene.
void InitScene();

/// User defined routine.
void UpdateScene();

/// Initialise the PhysX SDK.
bool InitPhysX();

/// Release the PhysX SDK.
void ReleasePhysX();

/// Reset the PhysX SDK.
void ResetPhysX();

/// Start a single step of simulation.
void SimulationStep();

/// Collect the simulation results.
void GetPhysicsResults();

/// Launch the ball.
void FireBall();

/// Reset the ball to the bat.
void ResetBall();

/// User defined contact report.
class MyContactReport : public NxUserContactReport
{        
    void onContactNotify(NxContactPair& pair, NxU32 events);
};
