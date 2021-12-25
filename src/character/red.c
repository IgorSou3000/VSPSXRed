/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "red.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Red character structure
enum
{
	Red_ArcMain_Idle0,
	Red_ArcMain_Idle1,
	Red_ArcMain_Left,
	Red_ArcMain_Down,
	Red_ArcMain_Up,
	Red_ArcMain_Right,

	Red_ArcMain_Idleb0,
	Red_ArcMain_Idleb1,
	Red_ArcMain_Leftb,
	Red_ArcMain_Downb,
	Red_ArcMain_Upb,
	Red_ArcMain_Rightb,
	
	Red_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Red_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Red;

//Red character definitions
static const CharFrame char_red_frame[] = {
	{Red_ArcMain_Idle0, {  6,   5, 103, 177}, { 44, 177}}, //0 idle 1
	{Red_ArcMain_Idle0, {109,   5, 105, 178}, { 46, 178}}, //1 idle 2
	{Red_ArcMain_Idle1, {  3,   4, 110, 178}, { 48, 178}}, //2 idle 3
	{Red_ArcMain_Idle1, {113,   4, 105, 178}, { 45, 178}}, //3 idle 4
	
	{Red_ArcMain_Left, {  12,   7,  121, 179}, { 52, 179}}, //4 left 1
	{Red_ArcMain_Left, {133,   7,  122, 179}, { 53, 179}}, //5 left 2
	
	{Red_ArcMain_Down, {  16,   12, 113, 174}, { 48, 174}}, //6 down 1
	{Red_ArcMain_Down, {129,   12, 117, 173}, { 49, 173}}, //7 down 2
	
	{Red_ArcMain_Up, {  22,   5, 97, 180}, { 44, 180}}, //8 up 1
	{Red_ArcMain_Up, {119,   5, 93, 182}, { 43, 182}}, //9 up 2
	
	{Red_ArcMain_Right, {  21,   7, 114, 178}, { 45, 178}}, //10 right 1
	{Red_ArcMain_Right, {135,   7, 117, 179}, { 45, 179}}, //11 right 2

	{Red_ArcMain_Idleb0, {  1,   8, 84, 175}, { 39, 175}}, //0 idle 1
	{Red_ArcMain_Idleb0, {85,   8, 85, 176}, { 39, 176}}, //1 idle 2
	{Red_ArcMain_Idleb0, {  170,   8, 84, 172}, { 39, 172}}, //2 idle 3
	{Red_ArcMain_Idleb1, {1,   6, 85, 174}, { 39, 174}}, //3 idle 4
	{Red_ArcMain_Idleb1, {86,   6, 84, 176}, { 39, 176}}, //3 idle 4
	{Red_ArcMain_Idleb1, {170,   6, 85, 175}, { 39, 175}}, //3 idle 4

	{Red_ArcMain_Leftb, {  16,   8,  84, 175}, { 43, 175}}, //4 left 1
	{Red_ArcMain_Leftb, {137,   10,  84, 175}, { 42, 175}}, //5 left 2
	
	{Red_ArcMain_Downb, {  23,   21, 98, 156}, { 50, 156}}, //6 down 1
	{Red_ArcMain_Downb, {121,   21, 99, 159}, { 51, 159}}, //7 down 2
	
	{Red_ArcMain_Upb, {  20,   3, 116, 184}, { 39, 184}}, //8 up 1
	{Red_ArcMain_Upb, {136,   3, 116, 182}, { 39, 182}}, //9 up 2
	
	{Red_ArcMain_Rightb, {  39,   9, 83, 174}, { 37, 174}}, //10 right 1
	{Red_ArcMain_Rightb, {122,   9, 83, 173}, { 39, 173}}, //11 right 2
};

static const Animation char_red_anim[CharAnim_Max] = {
	{2, (const u8[]){ 3,  2,  1,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  4, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 5,  4, ASCR_BACK, 1}},        //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  6, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ 7,  6, ASCR_BACK, 1}},   //CharAnim_DownAlt
	{2, (const u8[]){ 9,  8, ASCR_BACK, 1}},         //CharAnim_Up
	{2, (const u8[]){ 9,  8, ASCR_BACK, 1}},   //CharAnim_UpAlt
	{2, (const u8[]){11, 10, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){11, 10, ASCR_BACK, 1}},   //CharAnim_RightAlt
};
static const Animation char_red_anim2[CharAnim_Max] = {
	{1, (const u8[]){ 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 18, 19, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 18, 19, ASCR_BACK, 1}},        //CharAnim_LeftAlt
	{2, (const u8[]){ 20, 21, ASCR_BACK, 1}},         //CharAnim_Down
	{2, (const u8[]){ 20, 21, ASCR_BACK, 1}},   //CharAnim_DownAlt
	{2, (const u8[]){ 22, 23, ASCR_BACK, 1}},         //CharAnim_Up
	{2, (const u8[]){ 22, 23, ASCR_BACK, 1}},   //CharAnim_UpAlt
	{2, (const u8[]){ 24, 25, ASCR_BACK, 1}},         //CharAnim_Right
	{2, (const u8[]){ 24, 25, ASCR_BACK, 1}},   //CharAnim_RightAlt
};

//Red character functions
void Char_Red_SetFrame(void *user, u8 frame)
{
	Char_Red *this = (Char_Red*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_red_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Red_Tick(Character *character)
{
	Char_Red *this = (Char_Red*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	    Character_PerformIdle(character);
	
	//Animate and draw
	if (stage.gameboy != 1)
	{
	Animatable_Animate(&character->animatable, (void*)this, Char_Red_SetFrame);
	Character_Draw(character, &this->tex, &char_red_frame[this->frame]);
	}

	else if (stage.gameboy == 1)
	{
	Animatable_Animate(&character->animatable2, (void*)this, Char_Red_SetFrame);
	Character_Draw(character, &this->tex, &char_red_frame[this->frame]);
	}

}

void Char_Red_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Animatable_SetAnim(&character->animatable2, anim);
	Character_CheckStartSing(character);
}

void Char_Red_Free(Character *character)
{
	Char_Red *this = (Char_Red*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Red_New(fixed_t x, fixed_t y)
{
	//Allocate Red object
	Char_Red *this = Mem_Alloc(sizeof(Char_Red));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Red_New] Failed to allocate Red object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Red_Tick;
	this->character.set_anim = Char_Red_SetAnim;
	this->character.free = Char_Red_Free;
	
	Animatable_Init(&this->character.animatable, char_red_anim);
	Animatable_Init(&this->character.animatable2, char_red_anim2);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(-80,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(7,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\RED.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Red_ArcMain_Idle0
		"idle1.tim", //Red_ArcMain_Idle1
		"left.tim",  //Red_ArcMain_Left
		"down.tim",  //Red_ArcMain_Down
		"up.tim",    //Red_ArcMain_Up
		"right.tim", //Red_ArcMain_Right
		"idleb0.tim", //Red_ArcMain_Idle0
		"idleb1.tim", //Red_ArcMain_Idle1
		"leftb.tim",  //Red_ArcMain_Left
		"downb.tim",  //Red_ArcMain_Down
		"upb.tim",    //Red_ArcMain_Up
		"rightb.tim", //Red_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
