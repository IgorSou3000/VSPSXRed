/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "redm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//redm character structure
enum
{
	redm_ArcMain_Idle0,
	redm_ArcMain_Idle1,
	
	redm_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[redm_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_redm;

//redm character definitions
static const CharFrame char_redm_frame[] = {
	{redm_ArcMain_Idle0, {  0,   0, 59, 123}, { 0, 123}},
	{redm_ArcMain_Idle0, {  59,   0, 59, 123}, { 0, 123}},
	{redm_ArcMain_Idle0, {  118,   0, 59, 120}, { 0, 120}},
	{redm_ArcMain_Idle1, {  0,   0, 59, 122}, { 0, 122}},
	{redm_ArcMain_Idle1, {  59,   0, 59, 123}, { 0, 123}},
	{redm_ArcMain_Idle1, {  118,   0, 59, 123}, { 0, 123}},
};

static const Animation char_redm_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_BACK, 0}}, //CharAnim_Idle
};

//redm character functions
void Char_redm_SetFrame(void *user, u8 frame)
{
	Char_redm *this = (Char_redm*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_redm_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_redm_Tick(Character *character)
{
	Char_redm *this = (Char_redm*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_redm_SetFrame);
	Character_Draw(character, &this->tex, &char_redm_frame[this->frame]);
}

void Char_redm_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_redm_Free(Character *character)
{
	Char_redm *this = (Char_redm*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_redm_New(fixed_t x, fixed_t y)
{
	//Allocate redm object
	Char_redm *this = Mem_Alloc(sizeof(Char_redm));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_redm_New] Failed to allocate redm object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_redm_Tick;
	this->character.set_anim = Char_redm_SetAnim;
	this->character.free = Char_redm_Free;
	
	Animatable_Init(&this->character.animatable, char_redm_anim);
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
		"idle0.tim", //redm_ArcMain_Idle0
		"idle1.tim", //redm_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
