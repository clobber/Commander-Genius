#include "CObjectAI.h"
#include "../../sdl/sound/CSound.h"
#include "../../keen.h"

// Yorp (ep1)y
enum
{
	YORP_LOOK,     // yorp is "looking around" (waving his eye)
	YORP_MOVE,     // yorp is walking either left or right
	YORP_STUNNED,  // the yorp is stunned
	YORP_DYING,    // the yorp is getting fried!
	YORP_DEAD      // they look so sad when they're dead
};

#define YORP_LOOK_TIME  24   // time each frame of YORP_LOOK is shown
#define YORP_STUN_ANIM_TIME  10
#define YORP_WALK_ANIM_TIME  12
#define YORP_WALK_SPEED      12
#define YORP_WALK_ANIM_TIME_FAST  72
#define YORP_WALK_SPEED_FAST      28

#define YORP_NUM_LOOKS  9      // number of times yorp look frame is changed
#define YORP_STUNTIME   7     // YORP_NUM_LOOKS for stun

// for INCREASE_DIFFICULTY
#define YORP_NUM_LOOKS_FAST  12      // number of times yorp look frame is changed
#define YORP_STUNTIME_FAST   48     // YORP_NUM_LOOKS for stun

#define YORP_LOOK_LEFT  49
#define YORP_STAND      50
#define YORP_LOOK_RIGHT 51
#define YORP_WALK_RIGHT 52
#define YORP_WALK_LEFT  54
#define YORP_STUNFRAME  56

#define YORP_JUMP_PROB      20
#define YORP_JUMP_HEIGHT    -21

#define YORP_DYING_FRAME   58
#define YORP_DEAD_FRAME    59
#define YORP_DIE_TIME      22

#define YORPDIE_START_INERTIA      -10
#define YORPDIE_MAX_INERTIA         480
#define YORPDIE_INERTIA_DECREASE    8

// How much Yorps pushes keen
#define YORP_PUSH_AMT_NO_WALK	18

#define YORP_PUSH_AMT_P_WALK_HARD	35
#define YORP_PUSH_AMT_P_WALK		25

#define YORP_PUSH_AMT_P_STAND_HARD	24
#define YORP_PUSH_AMT_P_STAND		18

unsigned int rnd(void);

void CObjectAI::yorp_ai(CObject &object, CPlayer *p_player, bool hardmode)
{
	char numlooks;
	int pushamt;
	unsigned int tb;
	
	// fix where yorps can get stunned, go offscreen, then
	// come back hours later and they're still doing the stun animation
	if (object.wasoffscreen)
	{
		object.wasoffscreen = false;
		if (object.ai.yorp.state==YORP_STUNNED)
			object.needinit = 1;
	}
	if (object.needinit)
	{  // first time initilization
		object.ai.yorp.state = YORP_LOOK;
		object.ai.yorp.looktimes = YORP_NUM_LOOKS+1;
		object.ai.yorp.timer = 0;
		object.ai.yorp.lookposition = 0;
		object.needinit = 0;
		object.canbezapped = true;
	}
	// hey after all, dead yorps don't talk. Or do anything
	// but return to the calling procedure.
	if (object.ai.yorp.state==YORP_DEAD) return;
	
	if (!object.hasbeenonscreen) return;

	tb = object.touchedBy;
	
	// code for the yorps to push keen, and code for them to get stunned
	if (object.touchPlayer && object.ai.yorp.state != YORP_STUNNED
		&& object.ai.yorp.state != YORP_DYING  && !p_player[tb].pdie)
	{
		if (p_player[tb].pfalling)
		{  // falling, see if he bonked the yorp on the head
			// this happens if keen's feet are higher than the top
			// half of the yorp
			if ((p_player[tb].y>>STC)+16 < (object.y>>STC)+12)
			{
				// must have pogo out to stun yorps in High Difficulty
				if (!hardmode || p_player[tb].ppogostick)
				{
					p_player[tb].ppogostick = false; // No pogo, You get it disabled at this point
					g_pSound->playStereofromCoord(SOUND_YORP_STUN, PLAY_NOW, object.scrx);
					object.ai.yorp.state = YORP_STUNNED;
					object.ai.yorp.looktimes = 0;
					object.ai.yorp.timer = 0;
					object.ai.yorp.lookposition = 0;
				}
				
            	// make the yorp look a little less "soft" by
                // offering a bit of resistance
                // (actually, having keen do a small jump)
                p_player[object.touchedBy].pjumptime = 0;
                p_player[object.touchedBy].pjumpupdecreaserate = 0;
                p_player[object.touchedBy].pjumpupspeed = 7;
                p_player[object.touchedBy].pjumping = PJUMPUP;
                //p_player[object.touchedBy].pjumpupspeed_decreasetimer = 0;
                p_player[object.touchedBy].pjustjumped = 1;
			}
		}
		else
		{
			g_pSound->playStereofromCoord(SOUND_YORP_BUMP, PLAY_NOW, object.scrx);
			
			// if yorp is moving, also push in direction he's moving
			// in. this allows walking through a yorp if he is walking
			// away from Keen
			pushamt = 0;
			if (object.ai.yorp.state==YORP_MOVE)
			{
				if (p_player[tb].pshowdir != object.ai.yorp.movedir)
				{	// p_player pushing against yorp
					if (p_player[tb].pwalking)
						pushamt = hardmode ? YORP_PUSH_AMT_P_WALK_HARD : YORP_PUSH_AMT_P_WALK;
					else
						pushamt = hardmode ? YORP_PUSH_AMT_P_STAND_HARD : YORP_PUSH_AMT_P_STAND;
					
					if (object.ai.yorp.movedir==LEFT) pushamt = -pushamt;
				}
				else // yorp not moving
					pushamt = (p_player[tb].x < object.x) ? -YORP_PUSH_AMT_NO_WALK:YORP_PUSH_AMT_NO_WALK;
			}
			else
			{   // player "walking through" yorp--provide resistance
					pushamt = (p_player[tb].pshowdir==LEFT) ? YORP_PUSH_AMT_NO_WALK/2:-YORP_PUSH_AMT_NO_WALK/2;
			}

				
 			if (p_player[tb].pwalking)
 			{
 				if (pushamt > 0 && p_player[tb].blockedr) pushamt = 0;
 				if (pushamt < 0 && p_player[tb].blockedl) pushamt = 0;
 			}
				
 			if (pushamt)
 			{
 				p_player[tb].bump( pushamt, false );
 			}
		}
 	}
	
	// did the poor guy get shot?
	if(object.zapped)
	{
		// what'd you kill an innocent yorp for, you bastard!
		object.ai.yorp.state = YORP_DYING;
		object.ai.yorp.dietimer = 0;
		object.canbezapped = false;
		object.sprite = YORP_DYING_FRAME;
		object.zapped = 0;
		object.ai.yorp.yorpdie_inertia_y = YORPDIE_START_INERTIA;
		object.y -= 10;
		object.inhibitfall = 1;
		g_pSound->playStereofromCoord(SOUND_VORT_DIE, PLAY_NOW, object.scrx);
	}
	
	switch(object.ai.yorp.state)
	{
		case YORP_DYING:
			object.y += object.ai.yorp.yorpdie_inertia_y;
			if (object.ai.yorp.dietimer>YORPDIE_INERTIA_DECREASE)
			{
				if (object.ai.yorp.yorpdie_inertia_y < YORPDIE_MAX_INERTIA)
					object.ai.yorp.yorpdie_inertia_y += (1<<TILE_S);
				object.ai.yorp.dietimer = 0;
			}
			else object.ai.yorp.dietimer++;
			if (object.ai.yorp.yorpdie_inertia_y >= 0 && object.blockedd)
			{
				object.sprite = YORP_DEAD_FRAME;
				object.inhibitfall = 0;
				object.ai.yorp.state = YORP_DEAD;
				object.dead = 1;
			}
			break;
		case YORP_LOOK:
			numlooks = hardmode ? YORP_NUM_LOOKS_FAST : YORP_NUM_LOOKS;
			
			if (object.ai.yorp.looktimes>numlooks &&\
				object.ai.yorp.timer==YORP_LOOK_TIME-(YORP_LOOK_TIME/4))
			{
				// 75% prob, try to head towards Keen...
				if (p_player[0].x < object.x)
				{ object.ai.yorp.movedir = LEFT; }
				else
				{ object.ai.yorp.movedir = RIGHT; }
				if (rnd()%3==1)
				{ // 25% prob go the other way
					object.ai.yorp.movedir ^= 1;
				}
				
				// unless we're can't go that way
				if (object.blockedl) object.ai.yorp.movedir = RIGHT;
				if (object.blockedr) object.ai.yorp.movedir = LEFT;
				
				object.ai.yorp.timer = 0;
				object.ai.yorp.walkframe = 0;
				object.ai.yorp.dist_traveled = 0;
				object.ai.yorp.state = YORP_MOVE;
			}
			
			if (!object.ai.yorp.timer)
			{
				object.ai.yorp.looktimes++;
				
				switch(object.ai.yorp.lookposition)
				{
					case 0: object.sprite = YORP_LOOK_LEFT; break;
					case 1: object.sprite = YORP_STAND; break;
					case 2: object.sprite = YORP_LOOK_RIGHT; break;
					case 3: object.sprite = YORP_STAND; break;
				}
				object.ai.yorp.looktimes++;
				
				if (++object.ai.yorp.lookposition>3)
					object.ai.yorp.lookposition=0;
				object.ai.yorp.timer=YORP_LOOK_TIME;
			}
			else
				object.ai.yorp.timer--;
			break;
		case YORP_MOVE:
#define YORP_LOOK_PROB    20
#define YORP_MINTRAVELDIST    50
			// looking
			if (object.ai.yorp.dist_traveled > YORP_MINTRAVELDIST)
			{
				// hopping
				if (object.blockedd && rnd()%YORP_LOOK_PROB==1)
				{
					object.ai.yorp.looktimes = 0;
					object.ai.yorp.timer = 0;
					object.ai.yorp.lookposition = 1;
					object.ai.yorp.state = YORP_LOOK;
					break;
				}
			}
			
			if (object.blockedd)
				if (rand()%YORP_JUMP_PROB==1)
				{
					object.yinertia = YORP_JUMP_HEIGHT - (rnd()%3);
					object.y--;
				}
			
			if (object.ai.yorp.movedir==LEFT)
			{  // yorp is walking left
				object.sprite = YORP_WALK_LEFT + object.ai.yorp.walkframe;
				if (!object.blockedl)
				{
					object.x -= hardmode ? YORP_WALK_SPEED_FAST:YORP_WALK_SPEED;
					object.ai.yorp.dist_traveled++;
				}
				else
				{
					object.ai.yorp.looktimes = 4;
					object.ai.yorp.timer = 0;
					object.ai.yorp.lookposition = 1;
					object.ai.yorp.state = YORP_LOOK;
				}
			}
			else
			{  // yorp is walking right
				object.sprite = YORP_WALK_RIGHT + object.ai.yorp.walkframe;
				if (!object.blockedr)
				{
					object.x += hardmode ? YORP_WALK_SPEED_FAST:YORP_WALK_SPEED;
					object.ai.yorp.dist_traveled++;
				}
				else
				{
					object.ai.yorp.looktimes = 4;
					object.ai.yorp.timer = 0;
					object.ai.yorp.lookposition = 1;
					object.ai.yorp.state = YORP_LOOK;
				}
			}
			// walk animation
			if (object.ai.yorp.timer > YORP_WALK_ANIM_TIME || \
				(object.ai.yorp.timer > YORP_WALK_ANIM_TIME_FAST && hardmode))
			{
				object.ai.yorp.walkframe ^= 1;
				object.ai.yorp.timer = 0;
			}
			else
				object.ai.yorp.timer++;
			
			break;
		case YORP_STUNNED:
			object.sprite = YORP_STUNFRAME + object.ai.yorp.walkframe;
			if (object.ai.yorp.timer > YORP_STUN_ANIM_TIME)
			{
				numlooks = hardmode ? YORP_STUNTIME_FAST:YORP_STUNTIME;
				
				if (object.ai.yorp.looktimes>numlooks)
				{
					object.ai.yorp.looktimes = 0;
					object.ai.yorp.timer = 0;
					object.ai.yorp.lookposition = 1;
					object.ai.yorp.state = YORP_LOOK;
				} else object.ai.yorp.looktimes++;
				object.ai.yorp.walkframe ^= 1;
				object.ai.yorp.timer = 0;
			}
			else
				object.ai.yorp.timer++;
			
			break;
	}
}
