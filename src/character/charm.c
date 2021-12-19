/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "charm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//charm character structure
enum
{
	charm_ArcMain_Charm,
	
	charm_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[charm_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_charm;

//charm character definitions
static const CharFrame char_charm_frame[] = {
	{charm_ArcMain_Charm, {  0,   0, 65, 72}, { 0, 72}},
	{charm_ArcMain_Charm, {  65,   0, 66, 71}, { 0, 71}},
	{charm_ArcMain_Charm, {  0,   73, 67, 68}, { 0, 68}},
	{charm_ArcMain_Charm, {  67,   73, 63, 72}, { 0, 72}},
};

static const Animation char_charm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 0}}, //CharAnim_Idle
};

//charm character functions
void Char_charm_SetFrame(void *user, u8 frame)
{
	Char_charm *this = (Char_charm*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_charm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_charm_Tick(Character *character)
{
	Char_charm *this = (Char_charm*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		Character_CheckEndSing(character);
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x3) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_charm_SetFrame);
	Character_Draw(character, &this->tex, &char_charm_frame[this->frame]);
}

void Char_charm_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_charm_Free(Character *character)
{
	Char_charm *this = (Char_charm*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_charm_New(fixed_t x, fixed_t y)
{
	//Allocate charm object
	Char_charm *this = Mem_Alloc(sizeof(Char_charm));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_charm_New] Failed to allocate charm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_charm_Tick;
	this->character.set_anim = Char_charm_SetAnim;
	this->character.free = Char_charm_Free;
	
	Animatable_Init(&this->character.animatable, char_charm_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(-80,1);
	this->character.focus_y = FIXED_DEC(-10,1);
	this->character.focus_zoom = FIXED_DEC(8,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MENUCHAR.ARC;1");
	
	const char **pathp = (const char *[]){
		"char.tim", //charm_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
