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
	IO_Data arc_char, arc_char_ptr[10];

	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_pixelp;

	//Pikachu state
	Gfx_Tex tex_pika;
	u8 pika_frame, pika_tex_id;
	
	Animatable pika_animatable;

	//Charizard state
	Gfx_Tex tex_char;
	u8 char_frame, char_tex_id;
	
	Animatable char_animatable;
	Animatable charp_animatable;

} Back_Week1;

//Pikachu animation and rects
static const CharFrame pika_frame[] = {
	{0, { 34,   10, 90, 88}, { 52,  88}}, //0 left 1
	{0, {125,   10, 92, 88}, { 50,  88}}, //1 left 2
	{0, { 35,   122, 100, 89}, { 51,  89}}, //2 left 3
	{0, {135, 122, 112, 90}, { 51,  90}}, //3 left 4
	{1, {36, 9, 108, 90}, { 51,  90}}, //3 left 4
	
	{1, {  31,  127, 105, 86}, { 69,  86}}, //5 right 1
	{1, {136,   128,  104, 86}, { 69, 86}}, //6 right 2

};

static const Animation pika_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4,  ASCR_BACK, 1}}, //Left
	{2, (const u8[]){5, 5, 5, 6, 6, 6,  ASCR_BACK, 1}}, //Left
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
//Charizard animation and rects
static const CharFrame char_frame[] = {
	{0, {7,   24, 242, 188}, { 94,  188}}, //0 left 1
	{1, {7,   23, 243, 189}, { 95,  189}}, //1 left 2
	{2, {7,   20, 244, 192}, { 95,  192}}, //2 left 3
	{3, {5,   17, 246, 195}, { 97,  195}}, //3 left 4
	{4, {6,   18, 246, 194}, { 96,  194}}, //5 right 1
	{5, {27,   9, 206, 204}, { 74,  204}}, //5 attac

	{6, {4,     3,  151, 151}, { 140, 151}}, 
    {7, {5,     0,  151, 154}, { 139, 154}}, 
    {8, {1,     7,  155, 147}, { 144, 147}},
    {9, {5,     2,  151, 153}, { 140, 153}},
};

static const Animation char_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, 4, ASCR_BACK, 1}}, //Left
	{2, (const u8[]){5, 5, 5, 5, 5, ASCR_BACK, 1}}, //Left
};
static const Animation charp_anim[] = {
	{2, (const u8[]){6, 7, 8, 9, ASCR_BACK, 1}}, //Left
};

//Charizard functions
void Week1_Char_SetFrame(void *user, u8 frame)
{
	Back_Week1 *this = (Back_Week1*)user;
	
	//Check if this is a new frame
	if (frame != this->char_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_frame[this->char_frame = frame];
		if (cframe->tex != this->char_tex_id)
			Gfx_LoadTex(&this->tex_char, this->arc_char_ptr[this->char_tex_id = cframe->tex], 0);
	}
}

void Week1_Char_Draw(Back_Week1 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &char_frame[this->char_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_char, &src, &dst, stage.camera.bzoom);
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
		switch (stage.song_step & 3 << 1)
		{
			case 0:
				Animatable_SetAnim(&this->pika_animatable, 0);
				break;
		}
	}
	
	Animatable_Animate(&this->pika_animatable, (void*)this, Week1_Pika_SetFrame);
	
	if (stage.gameboy != 1)
	Week1_Pika_Draw(this, FIXED_DEC(220,1) - fx, FIXED_DEC(80,1) - fy);


	if (stage.gameboy != 1 && stage.thunderbolt == 1)
	Animatable_SetAnim(&this->pika_animatable, 1);

	fixed_t beat_bop;
	if ((stage.song_step & 0x3) == 0)
		beat_bop = FIXED_UNIT - ((stage.note_scroll / 24) & FIXED_LAND);
	else
		beat_bop = 0;

	if (stage.gameboy == 1)
	{
	 //Draw boppers
	static const struct Back_Week1_LowerBop
	{
		RECT src;
		RECT_FIXED dst;
	} lbop_piece[] = {
		{{0, 0, 88, 90}, {FIXED_DEC(190,1), FIXED_DEC( 10,1), FIXED_DEC(88,1), FIXED_DEC(90,1)}},
	};
	
	const struct Back_Week1_LowerBop *lbop_p = lbop_piece;
	for (size_t i = 0; i < COUNT_OF(lbop_piece); i++, lbop_p++)
	{
		RECT_FIXED lbop_dst = {
			lbop_p->dst.x - fx - (beat_bop << 1),
			lbop_p->dst.y - fy + (beat_bop << 3),
			lbop_p->dst.w + (beat_bop << 2),
			lbop_p->dst.h - (beat_bop << 3),
		};
		Stage_DrawTex(&this->tex_pixelp, &lbop_p->src, &lbop_dst, stage.camera.bzoom);
	}
}
}

void Back_Week1_DrawBG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	fixed_t fx, fy;
	
	//Draw stage
	fx = stage.camera.x;
	fy = stage.camera.y;

	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 3 << 1)
		{
			case 0:
				Animatable_SetAnim(&this->char_animatable,  0);
				Animatable_SetAnim(&this->charp_animatable,  0);
				break;
		}
	}
    

	 if (stage.gameboy != 1)
	 {
	Animatable_Animate(&this->char_animatable, (void*)this, Week1_Char_SetFrame);
	Week1_Char_Draw(this, FIXED_DEC(86,1) - fx, FIXED_DEC(60,1) - fy);
	 }
    
	
	else if (stage.gameboy == 1)
	{
	Animatable_Animate(&this->charp_animatable, (void*)this, Week1_Char_SetFrame);
    Week1_Char_Draw(this, FIXED_DEC(156,1) - fx, FIXED_DEC(60,1) - fy);
	}




	if (stage.gameboy != 1 && stage.flame == 1)
	Animatable_SetAnim(&this->char_animatable, 1);
	

	if (stage.gameboy != 1)
	{
	RECT back_src = {0, 0, 256, 141};
	RECT_FIXED back_dst = {
		FIXED_DEC(-310,1) - fx,
		FIXED_DEC(-133,1) - fy,
		FIXED_DEC(630,1),
		FIXED_DEC(390,1)
	};

	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);
	}
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free Pikachu archive
	Mem_Free(this->arc_pika);
    Mem_Free(this->arc_char);

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
	Gfx_LoadTex(&this->tex_pixelp, Archive_Find(arc_back, "pixelp.tim"), 0);
	Mem_Free(arc_back);

	//Load pikachu textures
	this->arc_pika = IO_Read("\\WEEK1\\PIKA.ARC;1");
	this->arc_pika_ptr[0] = Archive_Find(this->arc_pika, "pika0.tim");
	this->arc_pika_ptr[1] = Archive_Find(this->arc_pika, "pika1.tim");

	//Load charizard textures
	this->arc_char = IO_Read("\\WEEK1\\CHAR.ARC;1");
	this->arc_char_ptr[0] = Archive_Find(this->arc_char, "char0.tim");
	this->arc_char_ptr[1] = Archive_Find(this->arc_char, "char1.tim");
	this->arc_char_ptr[2] = Archive_Find(this->arc_char, "char2.tim");
	this->arc_char_ptr[3] = Archive_Find(this->arc_char, "char3.tim");
	this->arc_char_ptr[4] = Archive_Find(this->arc_char, "char4.tim");
	this->arc_char_ptr[5] = Archive_Find(this->arc_char, "char5.tim");
	this->arc_char_ptr[6] = Archive_Find(this->arc_char, "pixelc0.tim");
	this->arc_char_ptr[7] = Archive_Find(this->arc_char, "pixelc1.tim");
	this->arc_char_ptr[8] = Archive_Find(this->arc_char, "pixelc2.tim");
	this->arc_char_ptr[9] = Archive_Find(this->arc_char, "pixelc3.tim");

	//Initialize pikachu state
	Animatable_Init(&this->pika_animatable, pika_anim);
	Animatable_SetAnim(&this->pika_animatable, 0);
	this->pika_frame = this->pika_tex_id = 0xFF; //Force art load

	//Initialize charizard state
	Animatable_Init(&this->char_animatable, char_anim);
	Animatable_Init(&this->charp_animatable, charp_anim);
	Animatable_SetAnim(&this->char_animatable, 0);
	Animatable_SetAnim(&this->charp_animatable, 0);
	this->char_frame = this->char_tex_id = 0xFF; //Force art load

	//Use sky coloured background
	Gfx_SetClear(255, 255, 255);
	
	return (StageBack*)this;
}
