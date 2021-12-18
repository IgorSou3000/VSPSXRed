/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../animation.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_pika, arc_pika_ptr[2];

	Gfx_Tex tex_back0; //Stage and back

	//Pikachu state
	Gfx_Tex tex_pika;
	u8 pika_frame, pika_tex_id;
	
	Animatable pika_animatable;


} Back_Week1;

//Pikachu animation and rects
static const CharFrame pika_frame[] = {
	{0, {  0,   0, 125, 118}, { 71,  97}}, //0 left 1
	{0, {125,   0, 131, 118}, { 36,  97}}, //1 left 2
	{0, {  0, 118, 135, 128}, { 71,  92}}, //2 left 3
	{0, {135, 118, 121, 128}, { 36,  92}}, //3 left 4
	
	{1, {  0,   0, 140, 118}, { 71,  97}}, //5 right 1
	{1, {101,   0,  99, 101}, { 33, 100}}, //6 right 2
};

static const Animation pika_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4, ASCR_BACK, 1}}, //Left
	{1, (const u8[]){5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, ASCR_BACK, 1}}, //Right
};

//Pikachu functions
void Week1_Pika_SetFrame(void *user, u8 frame)
{
	Back_Week1 *this = (Back_Week1*)user;
	
	//Check if this is a new frame
	if (frame != this->pika_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &pika_frame[this->pika_frame = frame];
		if (cframe->tex != this->pika_tex_id)
			Gfx_LoadTex(&this->tex_pika, this->arc_pika_ptr[this->pika_tex_id = cframe->tex], 0);
	}
}

void Week1_Pika_Draw(Back_Week1 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &pika_frame[this->pika_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_pika, &src, &dst, stage.camera.bzoom);
}

//Week 1 background functions
void Back_Week1_DrawFG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw Pikachu
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 7)
		{
			case 0:
				Animatable_SetAnim(&this->pika_animatable, 0);
				break;
		}
	}
	Animatable_Animate(&this->pika_animatable, (void*)this, Week1_Pika_SetFrame);
	
	Week1_Pika_Draw(this, FIXED_DEC(160,1) - fx, FIXED_DEC(80,1) - fy);
}

void Back_Week1_DrawBG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	fixed_t fx, fy;
	
	//Draw stage
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back_src = {0, 0, 256, 141};
	RECT_FIXED back_dst = {
		FIXED_DEC(-278,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(540,1),
		FIXED_DEC(341,1)
	};

	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free Pikachu archive
	Mem_Free(this->arc_pika);


	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week1_New(void)
{
	//Allocate background structure
	Back_Week1 *this = (Back_Week1*)Mem_Alloc(sizeof(Back_Week1));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week1_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week1_DrawBG;
	this->back.free = Back_Week1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Mem_Free(arc_back);

	//Load pikachu textures
	this->arc_pika = IO_Read("\\WEEK1\\PIKA.ARC;1");
	this->arc_pika_ptr[0] = Archive_Find(this->arc_pika, "pika0.tim");
	this->arc_pika_ptr[1] = Archive_Find(this->arc_pika, "pika1.tim");

	//Initialize pikachu state
	Animatable_Init(&this->pika_animatable, pika_anim);
	Animatable_SetAnim(&this->pika_animatable, 0);
	this->pika_frame = this->pika_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
