
#include "CRay.h"
#include "CManglingMachine.h"

#include "engine/spritedefines.h"
#include "sdl/sound/CSound.h"
#include "CLogFile.h"
#include "graphics/effects/CVibrate.h"
#include "common/CBehaviorEngine.h"

// Arm parts
#define ARM_GO          0
#define ARM_WAIT        1

#define ARM_MOVE_SPEED   10
#define ARM_WAIT_TIME    8

// Spark parts
#define MORTIMER_SPARK_BASEFRAME        114

#define MORTIMER_LEFT_ARM_X             5
#define MORTIMER_RIGHT_ARM_X            17
#define MORTIMER_ARMS_YSTART            7
#define MORTIMER_ARMS_YEND              18

#define ARMS_DESTROY_RATE        3

#define MSPARK_IDLE              0
#define MSPARK_DESTROYARMS       1

#define SPARK_ANIMRATE          5

// heart
#define MORTIMER_HEART_BASEFRAME        146
#define HEART_ANIMRATE                  4

#define HEART_IDLE              0
#define HEART_ZAPSRUNUP         1
#define HEART_ZAPSRUNDOWN       2

#define MORTIMER_MACHINE_YSTART         2
#define MORTIMER_MACHINE_YEND           18
#define MORTIMER_MACHINE_YENDNOLEGS     14

#define MORTIMER_MACHINE_XSTART         8
#define MORTIMER_MACHINE_XEND           17

#define MACHINE_DESTROY_RATE            3
#define MORTIMER_ZAPWAVESPACING        50
#define MORTIMER_NUMZAPWAVES             5

#define ZAPSUP_NORMAL           0
#define ZAPSUP_ABOUTTOFADEOUT   1

// Legs
#define LEG_GO          0
#define LEG_WAIT        1

#define LEFTLEG_MOVE_SPEED   15
#define LEFTLEG_WAIT_TIME    36

#define RIGHTLEG_MOVE_SPEED   20
#define RIGHTLEG_WAIT_TIME    40

static int sparksleft = 0;

int mortimer_surprisedcount = 0;

bool armsDestroy = false;
bool legsDestroy = false;

CManglingMachine::CManglingMachine(CMap *p_map, Uint32 x, Uint32 y, unsigned int se_type) :
CVorticonSpriteObject(p_map, x, y, OBJ_SECTOREFFECTOR),
setype(se_type),
timer(0)
{	
	inhibitfall = true;

	switch(setype)
	{
	case SE_MORTIMER_ARM:
		armsDestroy = false;
		dir = DOWN;
		state = ARM_GO;
		break;
	case SE_MORTIMER_LEG_LEFT:
		legsDestroy = false;
		dir = UP;
		state = LEG_GO;
		break;
	case SE_MORTIMER_LEG_RIGHT:
		legsDestroy = false;
		dir = UP;
		state = LEG_GO;
		inhibitfall = 1;
		break;
	case SE_MORTIMER_SPARK:
	    sparksleft++;
	    state = MSPARK_IDLE;
	    frame = 0;
	    canbezapped = true;
	    break;
	case SE_MORTIMER_HEART:
		frame = 0;
		state = HEART_IDLE;
		inhibitfall = 1;
		canbezapped = 1;
		mortimer_surprisedcount = 0;
		break;
	case SE_MORTIMER_ZAPSUP:  break;
	case SE_MORTIMER_RANDOMZAPS:
		sprite = BLANKSPRITE;
		counter = 0;
		break;

	default:
		g_pLogFile->ftextOut("Invalid sector effector type %d", setype);
		break;
	}

}

CManglingMachine::~CManglingMachine()
{
    if(setype == SE_MORTIMER_SPARK)
    {
	if(sparksleft>0)
	    sparksleft--;
    }
}


void CManglingMachine::process()
{
	switch(setype)
	{
	case SE_MORTIMER_ARM: se_mortimer_arm(); break;
	case SE_MORTIMER_LEG_LEFT: se_mortimer_leg_left(); break;
	case SE_MORTIMER_LEG_RIGHT: se_mortimer_leg_right(); break;
	case SE_MORTIMER_SPARK: se_mortimer_spark(); break;
	case SE_MORTIMER_RANDOMZAPS: se_mortimer_randomzaps(); break;
	case SE_MORTIMER_ZAPSUP: se_mortimer_zapsup(); break;
	default:break;
	}

}

void CManglingMachine::getTouchedBy(CVorticonSpriteObject &theObject)
{
    bool it_is_mortimer_machine = false;
    
    it_is_mortimer_machine = (setype == SE_MORTIMER_LEG_LEFT)
			|| (setype == SE_MORTIMER_LEG_RIGHT)
			|| (setype == SE_MORTIMER_ARM)
			|| (setype == SE_MORTIMER_SPARK);
    
    if(it_is_mortimer_machine)
    {
	if (CPlayer *player = dynamic_cast<CPlayer*>(&theObject))
	{
	    player->kill();
	}
    }
    
    CVorticonSpriteObject *theVObjectPtr = dynamic_cast<CVorticonSpriteObject*>(&theObject);
    
    if( ( setype == SE_MORTIMER_SPARK || setype == SE_MORTIMER_HEART ) &&
	mHealthPoints>0 && theVObjectPtr->m_type == OBJ_RAY )
    {
	mHealthPoints--;
	sparksleft--;
    }	
}

bool CManglingMachine::isNearby(CVorticonSpriteObject &theObject) 
{     
	if(setype == SE_MORTIMER_SPARK && state == MSPARK_IDLE)
	{
		if (mHealthPoints <= 0)
		{	
			// if there are any sparks left, destroy the spark,
			// else destroy mortimer's arms			    
			if(CManglingMachine* SE = dynamic_cast<CManglingMachine*>(&theObject))
			{
			    if (SE->setype==SE_MORTIMER_SPARK && SE->exists)
			    {
				if (SE->m_index!=m_index)
				{	// other sparks still exist
				    setype = SE_MORTIMER_RANDOMZAPS;
				    return true;
				}
			    }
			}
		}
		
		if(sparksleft == 0)
		{
			// keen just destroyed the last spark
			sprite = BLANKSPRITE;
			armsDestroy = true;
			// go into a state where we'll destroy mortimer's arms
			state = MSPARK_DESTROYARMS;
			my = MORTIMER_ARMS_YSTART;
			timer = 0;
		}
	}
	
	
	switch(setype)
	{
	case SE_MORTIMER_ZAPSUP: se_mortimer_zapsup_nearby(dynamic_cast<CPlayer*>(&theObject)); break;
	case SE_MORTIMER_HEART: se_mortimer_heart(dynamic_cast<CVorticonSpriteObject*>(&theObject)); break;
	default: break;
	}

	
    
    return true;
}

void CManglingMachine::se_mortimer_arm()
{
	int mx,my;
	
	if(armsDestroy)
	{
	  return;
	}
	

	switch(state)
	{
	case ARM_GO:
		// vertical arm 618 620 619
		// pole 597
		// polka dot background 169
		if (timer > ARM_MOVE_SPEED)
		{
			mx = getXPosition() >> CSF;
			my = getYPosition() >> CSF;

			if (dir==DOWN)
			{
				// reached bottom?
				if (mp_Map->at(mx, my+3) == 471)
				{
					timer = 0;
					state = ARM_WAIT;
				}
				else
				{
					// add to the pole
					mp_Map->setTile(mx, my+1, 597, true);
					// create left side of pincher
					mp_Map->setTile(mx-1, my+1, 618, true);
					mp_Map->setTile(mx-1, my+2, 620, true);
					mp_Map->setTile(mx-1, my+3, 619, true);
					// create right side of pincher
					mp_Map->setTile(mx+1, my+1, 618, true);
					mp_Map->setTile(mx+1, my+2, 620, true);
					mp_Map->setTile(mx+1, my+3, 619, true);
					// erase the top of the pincher we don't need anymore
					mp_Map->setTile(mx-1, my, 169, true);
					mp_Map->setTile(mx+1, my, 169, true);
					moveDown(1<<CSF);
				}
			}
			else
			{  // arm going up

				// reached top?
				if (mp_Map->at(mx, my+1)==619)
				{
					timer = 0;
					state = ARM_WAIT;
				}
				else
				{
					// create left side of pincher
					mp_Map->changeTile(mx-1, my+1, 618);
					mp_Map->changeTile(mx-1, my+2, 620);
					mp_Map->changeTile(mx-1, my+3, 619);
					// create right side of pincher
					mp_Map->changeTile(mx+1, my+1, 618);
					mp_Map->changeTile(mx+1, my+2, 620);
					mp_Map->changeTile(mx+1, my+3, 619);
					// erase the bottom of the pincher we don't need anymore
					mp_Map->changeTile(mx-1, my+4, 169);
					mp_Map->changeTile(mx+1, my+4, 169);
					// erase the pole
					mp_Map->changeTile(mx, my+2, 169);

					moveUp(1<<CSF);
				}
			}
			timer = 0;
		}
		else timer++;
		break;
	case ARM_WAIT:
		if (timer > ARM_WAIT_TIME)
		{
			if (dir==DOWN)
			{
				dir = UP;
				moveUp(2<<CSF);
			}
			else
			{
				dir = DOWN;
				moveDown(1<<CSF);
			}

			state = ARM_GO;
			timer = 0;
		}
		else timer++;
		break;
	}
}


void CManglingMachine::se_mortimer_spark()
{
	switch(state)
	{
	case MSPARK_IDLE:
		sprite = MORTIMER_SPARK_BASEFRAME + frame;

		if (timer > SPARK_ANIMRATE)
		{
			frame++;
			if (frame > 3) frame = 0;
			timer = 0;
		}
		else timer++;
		
		if (mHealthPoints <= 0)
		{
			set_mortimer_surprised(true);
			g_pGfxEngine->setupEffect(new CVibrate(200));
			mp_Map->redrawAt(getXPosition()>>CSF, getYPosition()>>CSF);
		}		
		
		break;
	    
	case MSPARK_DESTROYARMS:
		if (!timer)
		{
			playSound(SOUND_SHOT_HIT);
			for(int x=0;x<3;x++)
			{
				int mx = MORTIMER_LEFT_ARM_X+x;
				if (mp_Map->at(mx, my) != 169)
				{
					mp_Map->setTile(mx, my, 169, true);
					// spawn a ZAP! or a ZOT!
					CRay *newobject = new CRay(mp_Map, ((mx<<4)+4)<<STC, my<<4<<STC, CENTER, DOWN);
					newobject->state = CRay::RAY_STATE_SETZAPZOT;
					newobject->inhibitfall = true;
					g_pBehaviorEngine->EventList().spawnObj(newobject);
				}

				mx = MORTIMER_RIGHT_ARM_X+x;
				if (mp_Map->at(mx, my) != 169)
				{
					mp_Map->setTile(mx, my, 169, true);
					// spawn a ZAP! or a ZOT!
					CRay *newobject = new CRay(mp_Map, ((mx<<4)+4)<<STC, my<<4<<STC, CENTER, DOWN);
					newobject->state = CRay::RAY_STATE_SETZAPZOT;
					newobject->inhibitfall = true;
					g_pBehaviorEngine->EventList().spawnObj(newobject);
				}

			}
			timer = ARMS_DESTROY_RATE;
			my++;
			if (my > MORTIMER_ARMS_YEND)
			{
	  			// destroy mortimer's arms
				exists = false;
				set_mortimer_surprised(false);
			}
		}
		else timer--;
		break;
	}
}


void CManglingMachine::se_mortimer_heart(CVorticonSpriteObject *obj)
{
    CManglingMachine* SE = dynamic_cast<CManglingMachine*>(SE);
    CPlayer* player = dynamic_cast<CPlayer*>(obj);
    
    if(player)
    {	
	switch(state)
	{
	    case HEART_ZAPSRUNUP:
		if (!timer)
		{	// spawn another wave of zaps
		int x = getXPosition();
		int y = getYPosition();
		
		CManglingMachine *newobject = new CManglingMachine(mp_Map, 
								 x, y,SE_MORTIMER_ZAPSUP);
		newobject->my = MORTIMER_MACHINE_YEND;
		newobject->timer = 0;
		newobject->destroytiles = 0;
		newobject->state = ZAPSUP_NORMAL;
		newobject->hasbeenonscreen = false;
		
		timer = MORTIMER_ZAPWAVESPACING;
		if (counter > MORTIMER_NUMZAPWAVES)
		{
		    newobject->destroytiles = true;
		    exists = false;
		}
		else counter++;
		
		g_pBehaviorEngine->EventList().spawnObj(newobject);
		}
		else timer--;
		break;
	    case HEART_ZAPSRUNDOWN:
		if (!timer)
		{
		    for(int x=MORTIMER_MACHINE_XSTART;x<MORTIMER_MACHINE_XEND;x++)
		    {
			// delete the tile
			mp_Map->setTile(x,my,169);
			// spawn a ZAP! or a ZOT!
			CRay *newobject = new CRay(mp_Map, ((x<<4)+4)<<STC, my<<4<<STC, CENTER, DOWN);
			newobject->state = CRay::RAY_STATE_SETZAPZOT;
			newobject->inhibitfall = true;
			g_pBehaviorEngine->EventList().spawnObj(newobject);				
		    }
		    
		    timer = MACHINE_DESTROY_RATE;
		    if (my > MORTIMER_MACHINE_YEND)
		    {
			exists = false;
		    }
		    else my++;
		}
		else timer--;
		break;
	}	
    }
    else
    {
	
	switch(state)
	{
	    case HEART_IDLE:
		sprite = MORTIMER_HEART_BASEFRAME + frame;
		
		if (timer > HEART_ANIMRATE)
		{
		    frame ^= 1;
		    timer = 0;
		}
		else timer++;
		
		if (mHealthPoints <= 0)
		{
		    legsDestroy = true;
		    sprite = BLANKSPRITE;
		    set_mortimer_surprised(true);
		    
		    // destroy Mortimer's machine
		    g_pGfxEngine->setupEffect(new CVibrate(10000));
		    
		    CManglingMachine* SE = dynamic_cast<CManglingMachine*>(SE);
		    CPlayer* player = dynamic_cast<CPlayer*>(obj);
		    
		    if(SE)
		    {
			if(SE->setype == SE_MORTIMER_HEART ) return;
			else SE->exists = false;
		    }
		    else if( player == NULL )
		    {
			obj->kill();
			obj->exists = false;
		    }
		    
		    set_mortimer_surprised(true);
		    // have waves of zaps run up mortimer's machine
		    timer = 0;
		    state = HEART_ZAPSRUNUP;
		    counter = 0;
		}
		break;
	}
    }
}

#define TIME_AFTER_DESTROY_BEFORE_FADEOUT       500
void CManglingMachine::se_mortimer_zapsup_nearby(CPlayer *player)
{
    if(player == nullptr)
	return;

	if (!timer)
	{
		if (my <= MORTIMER_MACHINE_YSTART)
		{
			if (destroytiles)
			{
				// last wave, prepare to initiate level fadeout
				timer = TIME_AFTER_DESTROY_BEFORE_FADEOUT;
				state = ZAPSUP_ABOUTTOFADEOUT;
				player->pfalling = true;
				destroytiles = false;
				return;
			}
		}
		
		if( state == ZAPSUP_ABOUTTOFADEOUT )
		{
		    g_pBehaviorEngine->EventList().add(new EventEraseAllEnemies());
		    player->level_done = LEVEL_DONE_FADEOUT;
		    exists = false;
		    return;
		}
	}
	


	
}

void CManglingMachine::se_mortimer_zapsup()
{
	int x;

	if (!timer)
	{
		playSound(SOUND_SHOT_HIT);
		for(x=MORTIMER_MACHINE_XSTART;x<MORTIMER_MACHINE_XEND;x++)
		{
			// spawn a ZAP! or a ZOT!
			CRay *newobject = new CRay(mp_Map, ((x<<4)+4)<<STC, (my+1)<<CSF, CENTER, DOWN);
			newobject->state = CRay::RAY_STATE_SETZAPZOT;
			newobject->inhibitfall = true;
			g_pBehaviorEngine->EventList().spawnObj(newobject);

			if (destroytiles)
			{
				// delete the tile
				mp_Map->changeTile(x, my,169);
			}
		}

		timer = MACHINE_DESTROY_RATE;
		if (my <= MORTIMER_MACHINE_YSTART)
		{
			if (!destroytiles)
			{
				exists = false;
				timer = 0;
			}
		}
		else my--;
	}
	else timer--;
	
}

void CManglingMachine::se_mortimer_leg_left()
{
	int mx,my;
	
	if(legsDestroy)
	  exists = false;


	switch(state)
	{
	case LEG_GO:
		// leg tiles 621 623 622
		// pole 597
		// polka dot background 169
		// bottom marker for leg 430
		// top marker for leg 420
		if (timer > LEFTLEG_MOVE_SPEED)
		{
			mx = getXPosition() >> CSF;
			my = getYPosition() >> CSF;

			if (dir==DOWN)
			{
				// reached bottom?
				if (mp_Map->at(mx, my+1) == 430)
				{
					timer = 0;
					state = LEG_WAIT;
					playSound(SOUND_FOOTSLAM);
				}
				else
				{
					// create the leg
					mp_Map->setTile(mx-3,my+1,621, true);
					mp_Map->setTile(mx-2,my+1,623, true);
					mp_Map->setTile(mx-1,my+1,623, true);
					mp_Map->setTile(mx-0,my+1,622, true);
					// erase the tiles above us that used to be the leg
					mp_Map->setTile(mx-3,my,169, true);
					mp_Map->setTile(mx-2,my,169, true);
					mp_Map->setTile(mx-1,my,169, true);
					mp_Map->setTile(mx-0,my,597, true);         // add to pole

					moveDown(1<<CSF);
				}
			}
			else
			{  // leg going up

				// reached top?
				if (mp_Map->at(mx, my-1) == 420)
				{
					timer = 0;
					state = LEG_WAIT;
				}
				else
				{
					// create the leg
					mp_Map->setTile(mx-3,my-1,621, true);
					mp_Map->setTile(mx-2,my-1,623, true);
					mp_Map->setTile(mx-1,my-1,623, true);
					mp_Map->setTile(mx-0,my-1,622, true);
					// erase the tiles beneath us that used to be the leg
					mp_Map->setTile(mx-3,my,169, true);
					mp_Map->setTile(mx-2,my,169, true);
					mp_Map->setTile(mx-1,my,169, true);
					mp_Map->setTile(mx-0,my,169, true);

					moveUp(1<<CSF);
				}
			}
			timer = 0;
		}
		else timer++;
		break;
	case LEG_WAIT:
		if (timer > LEFTLEG_WAIT_TIME)
		{
			if (dir==DOWN)
			{
				dir = UP;
			}
			else
			{
				dir = DOWN;
			}

			state = LEG_GO;
			timer = 0;
		}
		else timer++;
		break;
	}
}

void CManglingMachine::se_mortimer_leg_right()
{
	int mx,my;
	
	if(legsDestroy)
	  exists = false;


	switch(state)
	{
	case LEG_GO:
		// leg tiles 621 623 622
		// pole 597
		// polka dot background 169
		// bottom marker for leg 430
		// top marker for leg 420
		if (timer > RIGHTLEG_MOVE_SPEED)
		{
			mx = getXPosition() >> CSF;
			my = getYPosition() >> CSF;

			if (dir==DOWN)
			{
				// reached bottom?
				if (mp_Map->at(mx, my+1) == 430)
				{
					timer = 0;
					state = LEG_WAIT;
					playSound(SOUND_FOOTSLAM);
				}
				else
				{
					// create the leg
					mp_Map->setTile(mx+3,my+1,622, true);
					mp_Map->setTile(mx+2,my+1,623, true);
					mp_Map->setTile(mx+1,my+1,623, true);
					mp_Map->setTile(mx+0,my+1,621, true);
					// erase the tiles above us that used to be the leg
					mp_Map->setTile(mx+3,my,169, true);
					mp_Map->setTile(mx+2,my,169, true);
					mp_Map->setTile(mx+1,my,169, true);
					mp_Map->setTile(mx+0,my,597, true);         // add to pole

					moveDown(1<<CSF);
				}
			}
			else
			{  // leg going up

				// reached top?
				if (mp_Map->at(mx, my-1) == 420)
				{
					timer = 0;
					state = LEG_WAIT;
				}
				else
				{
					// create the leg
					mp_Map->setTile(mx+3,my-1,622, true);
					mp_Map->setTile(mx+2,my-1,623, true);
					mp_Map->setTile(mx+1,my-1,623, true);
					mp_Map->setTile(mx+0,my-1,621, true);
					// erase the tiles beneath us that used to be the leg
					mp_Map->setTile(mx+3,my,169, true);
					mp_Map->setTile(mx+2,my,169, true);
					mp_Map->setTile(mx+1,my,169, true);
					mp_Map->setTile(mx+0,my,169, true);

					moveUp(1<<CSF);
				}
			}
			timer = 0;
		}
		else timer++;
		break;
	case LEG_WAIT:
		if (timer > RIGHTLEG_WAIT_TIME)
		{
			if (dir==DOWN)
			{
				dir = UP;
			}
			else
			{
				dir = DOWN;
			}

			state = LEG_GO;
			timer = 0;
		}
		else timer++;
		break;
	}
}

#define NUM_RANDOM_ZAPS         30
#define TIME_BETWEEN_ZAPS       2
void CManglingMachine::se_mortimer_randomzaps()
{
	int x,y;

	if (!timer)
	{
		x = rand()%((MORTIMER_MACHINE_XEND*16)-(MORTIMER_MACHINE_XSTART*16))+(MORTIMER_MACHINE_XSTART*16);
		y = rand()%((MORTIMER_MACHINE_YENDNOLEGS*16)-(MORTIMER_MACHINE_YSTART*16))+(MORTIMER_MACHINE_YSTART*16);

		// spawn a ZAP! or a ZOT!
		CRay *newobject = new CRay(mp_Map,x<<CSF, y<<CSF, RIGHT, CENTER );
		newobject->state = CRay::RAY_STATE_SETZAPZOT;
		newobject->inhibitfall = true;
		g_pBehaviorEngine->EventList().spawnObj(newobject);

		timer = TIME_BETWEEN_ZAPS;
		if (counter > NUM_RANDOM_ZAPS)
		{
			set_mortimer_surprised(false);
			exists=false;
		}
		else counter++;
	}
	else timer--;
}

void CManglingMachine::set_mortimer_surprised(bool yes)
{
	if (yes)
	{
		mortimer_surprisedcount++;
	}
	else
	{
		if (mortimer_surprisedcount>0) mortimer_surprisedcount--;
	}

	if (mortimer_surprisedcount)
	{
		//12,6 -> 610 -- give mortimer his "surprised" face
		// deanimate mortimer's hands
		mp_Map->setTile(12,6,610, true);
		mp_Map->setTile(11,6,613, true);
		mp_Map->setTile(13,6,615, true);
	}
	else
	{
		// give mortimer his normal face again
		mp_Map->setTile(12,6,607, true);
		mp_Map->setTile(11,6,613, true);
		mp_Map->setTile(13,6,616, true);
	}
}