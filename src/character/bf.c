/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend skull fragments
static SkullFragment char_bf_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BF_ArcMain_BF0,
	BF_ArcMain_BF1,
	BF_ArcMain_BF2,
	BF_ArcMain_BF3,
	BF_ArcMain_BF4,
	BF_ArcMain_Dead0, //BREAK
	BF_ArcMain_BFB0,
	BF_ArcMain_BFB1,
	BF_ArcMain_BFB2,
	BF_ArcMain_BFB3,
	BF_ArcMain_BFB4,
	
	BF_ArcMain_Max,
};

enum
{
	BF_ArcDead_Dead1, //Mic Drop
	BF_ArcDead_Dead2, //Twitch
	BF_ArcDead_Retry, //Retry prompt
	
	BF_ArcDead_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bf_skull)];
	u8 skull_scale;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	{BF_ArcMain_BF0, {  7,   17, 119, 103}, { 49, 103}}, //0 idle 1
	{BF_ArcMain_BF0, {127,   17, 120, 104}, { 49, 104}}, //1 idle 2
	{BF_ArcMain_BF0, {  5, 138, 119, 106}, { 49, 106}}, //2 idle 3
	{BF_ArcMain_BF0, {124, 138, 118, 106}, { 53, 106}}, //3 idle 4
	
	{BF_ArcMain_BF1, {  1,   14, 118, 107}, { 63, 107}}, //4 left 1
	{BF_ArcMain_BF1, {120,   14, 119, 107}, { 62, 107}}, //5 left 2
	
	{BF_ArcMain_BF1, {  9,  143, 120, 100}, { 47, 100}},  //6 down 1
	{BF_ArcMain_BF1, {130,  143, 121, 102}, { 47, 102}}, //7 down 2
	
	{BF_ArcMain_BF2, {  3,   9, 116, 112}, { 56, 112}}, //8 up 1
	{BF_ArcMain_BF2, {119,   9, 117, 111}, { 54, 111}}, //9 up 2
	
	{BF_ArcMain_BF2, {  9,  134, 116, 109}, { 49,  109}}, //10 right 1
	{BF_ArcMain_BF2, {125,  134, 117, 108}, { 51,  108}}, //11 right 2
	
	{BF_ArcMain_BF3, {  0,   16,  122, 105}, { 65, 105}}, //12 left miss 1
	{BF_ArcMain_BF3, { 122,   16, 120, 106}, { 64, 106}}, //13 left miss 2
	
	{BF_ArcMain_BF3, {  9, 145,  124,  98}, { 50,  98}}, //14 down miss 1
	{BF_ArcMain_BF3, { 133, 145,  120,  100}, { 48,  100}}, //15 down miss 2
	
	{BF_ArcMain_BF4, {  0,   9,  118, 112}, { 55,  112}}, //16 up miss 1
	{BF_ArcMain_BF4, { 118,   8,  117, 112}, { 55, 112}}, //17 up miss 2
	
	{BF_ArcMain_BF4, {  9, 136,  118, 107}, { 47, 107}}, //18 right miss 1
	{BF_ArcMain_BF4, {127, 136, 116, 108}, { 48, 108}}, //19 right miss 2

	{BF_ArcMain_Dead0, { 50,   8, 161,  52}, { 53,  98}}, //20 dead0 0
	{BF_ArcMain_Dead0, { 48,  62, 161,  51}, { 53,  98}}, //21 dead0 1
	{BF_ArcMain_Dead0, { 48, 115, 161,  51}, { 53,  98}}, //22 dead0 2
	{BF_ArcMain_Dead0, { 48, 172, 161,  51}, { 53,  98}}, //23 dead0 3
	
	{BF_ArcDead_Dead1, { 50,   8, 161,  52}, { 53,  98}}, //24 dead1 0
	{BF_ArcDead_Dead1, { 48,  62, 161,  51}, { 53,  98}}, //25 dead1 1
	{BF_ArcDead_Dead1, { 48, 115, 161,  51}, { 53,  98}}, //26 dead1 2
	{BF_ArcDead_Dead1, { 48, 172, 161,  51}, { 53,  98}}, //27 dead1 3
	
	{BF_ArcDead_Dead2, { 50,   8, 161,  52}, { 53,  98}}, //28 dead2 0
	{BF_ArcDead_Dead2, { 48,  62, 161,  51}, { 53,  98}}, //29 dead2 1
	{BF_ArcDead_Dead2, { 47, 113, 179,  59}, { 53,  98}}, //30 dead2 2
	{BF_ArcDead_Dead2, { 49, 176,  55,  18}, { 53,  98}}, //31 dead2 3

	{BF_ArcMain_BFB0, {  35,   22, 91, 97}, { 55, 97}},  //0 idle 1
	{BF_ArcMain_BFB0, {  126,   21, 91, 98}, { 55, 98}},  //1 idle 2
	{BF_ArcMain_BFB0, {  33,   139, 91, 94}, { 57, 94}},  //2 idle 3
	{BF_ArcMain_BFB0, {  124,   139, 91, 97}, { 55, 97}},  //3 idle 4
	
	{BF_ArcMain_BFB1, {  28,   22, 91, 94}, { 60, 94}},  //4 left 1
	{BF_ArcMain_BFB1, {  119,   23, 91, 95}, { 60, 95}},  //5 left 2
	
	{BF_ArcMain_BFB1, {  38,   146, 91, 90}, { 53, 90}},  //6 down 1
	{BF_ArcMain_BFB1, {  129,   146, 91, 92}, { 53, 92}},  //7 down 2
	
	{BF_ArcMain_BFB2, {  10,   18, 91, 102}, { 55, 102}},  //8 up 1
	{BF_ArcMain_BFB2, {  128,   20, 91, 100}, { 55, 100}},  //9 up 2
	
	{BF_ArcMain_BFB2, {  33,  144, 91, 93}, { 47, 93}},  //10 right 1
	{BF_ArcMain_BFB2, {  124,   144, 91, 94}, { 48, 94}},  //11 right 2
	
	{BF_ArcMain_BFB3, {  26,   26, 97, 91}, { 65, 91}},  //12 left miss 1
	{BF_ArcMain_BFB3, {  123,   26, 93, 93}, { 62, 93}},  //13 left miss 2
	
	{BF_ArcMain_BFB3, {  39,   145, 93, 89}, { 55, 89}},  //14 down miss 1
	{BF_ArcMain_BFB3, { 132,   145, 91, 91}, { 53, 91}},  //15 down miss 2
	
	{BF_ArcMain_BFB4, {  25,   11, 92, 100}, { 56, 100}},  //16 up miss 1
	{BF_ArcMain_BFB4, {  117,   11, 91, 102}, { 55, 102}},  //17 up miss 2
	
	{BF_ArcMain_BFB4, {  29,   147, 94, 92}, { 50, 92}},  //18 right miss 1
	{BF_ArcMain_BFB4, {  123,   147, 93, 93}, { 49, 93}},  //19 right miss 2

	{BF_ArcDead_Dead2, { 49, 199,  11,  4}, { 53,  98}}, //32 dead2 3
};

static const Animation char_bf_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},             //CharAnim_Down
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},      //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},             //CharAnim_Up
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},             //CharAnim_Right
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_RightAl
	
	{1, (const u8[]){ 4, 12, 12, 13, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 6, 14, 14, 15, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 8, 16, 16, 17, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){10, 18, 18, 19, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{6, (const u8[]){20, 21, 20, 21, 20, 21, 22, 23, 22, 23, 22,  ASCR_CHGANI, PlayerAnim_Dead1}},
	{6, (const u8[]){23, 24, 25, 24, 25, 26, 27, 26, 27, 26,ASCR_BACK, 1}},                                                       //PlayerAnim_Dead1
	{6, (const u8[]){23, 24, 25, 24, 25, 26, 27, 26, 27, 26,ASCR_BACK, 1}}, //PlayerAnim_Dead2
	{6, (const u8[]){26, 27, 26, 27, 26, 27, 26, 27, 26, 27, ASCR_CHGANI, PlayerAnim_Dead3}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){28, 29, 27, 27, 27, 27, 27, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){30, 31, 27, 27, 27, 27, 27, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{3, (const u8[]){27, 28, 27, 28, 27, 28, 27, 28, 30, 31, 52, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{3, (const u8[]){30, 31, 27, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

static const Animation char_bf_anim2[PlayerAnim_Max] = {
	{2, (const u8[]){32, 33, 34, 35,  ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){36, 37,  ASCR_BACK, 1}},              //CharAnim_Left
	{2, (const u8[]){36, 37,  ASCR_BACK, 1}},       //CharAnim_LeftAlt
	{2, (const u8[]){38, 39,  ASCR_BACK, 1}},             //CharAnim_Down
	{2, (const u8[]){38, 39,  ASCR_BACK, 1}},       //CharAnim_DownAlt
	{2, (const u8[]){40, 41,  ASCR_BACK, 1}},             //CharAnim_Up
	{2, (const u8[]){40, 41,  ASCR_BACK, 1}},          //CharAnim_UpAlt
	{2, (const u8[]){42, 43,  ASCR_BACK, 1}},             //CharAnim_Right
	{2, (const u8[]){42, 43,  ASCR_BACK, 1}},         //CharAnim_RightAlt

	{1, (const u8[]){36, 44, 44, 45, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){38, 46, 46, 47, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){40, 48, 48, 49, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){42, 50, 50, 51, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{9, (const u8[]){20, 21, 20, 21, 20, 21, 22, 23, 22, 23, 22,  ASCR_CHGANI, PlayerAnim_Dead1}},
	{9, (const u8[]){23, 24, 25, 24, 25, 26, 27, 26, 27, 26,ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{9, (const u8[]){26, 27, 26, 27, 26, 27, 26, 27, 26, 27, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){27, ASCR_CHGANI, PlayerAnim_Dead3}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){28, 29, 27, 27, 27, 27, 27, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){30, 31, 27, 27, 27, 27, 27, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{3, (const u8[]){27, 28, 27, 28, 27, 28, 27, 28, 30, 31, 52, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){30, 31, 27, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Handle animation update

	 if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);

	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
		

		if (Animatable_Ended(&character->animatable2) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw character

	if (stage.gameboy != 1)
	{
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
	}

	else if (stage.gameboy == 1)
	{
	Animatable_Animate(&character->animatable2, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
	}

}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BF_ArcDead_Dead1
				"dead2.tim", //BF_ArcDead_Dead2
				"retry.tim", //BF_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BF_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_anim);
	Animatable_Init(&this->character.animatable2, char_bf_anim2);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 0;
	
	this->character.focus_x = FIXED_DEC(160,1);
	this->character.focus_y = FIXED_DEC(-180,1);
	this->character.focus_zoom = FIXED_DEC(7,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",   //BF_ArcMain_BF0
		"bf1.tim",   //BF_ArcMain_BF1
		"bf2.tim",   //BF_ArcMain_BF2
		"bf3.tim",   //BF_ArcMain_BF3
		"bf4.tim",   //BF_ArcMain_BF4
		"dead0.tim", //BF_ArcMain_Dead0
		"bfb0.tim",   //BF_ArcMain_BF0
		"bfb1.tim",   //BF_ArcMain_BF1
		"bfb2.tim",   //BF_ArcMain_BF2
		"bfb3.tim",   //BF_ArcMain_BF3
		"bfb4.tim",   //BF_ArcMain_BF4
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bf_skull, sizeof(char_bf_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
