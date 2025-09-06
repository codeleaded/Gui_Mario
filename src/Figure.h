#ifndef FIGURE_H
#define FIGURE_H

#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Files.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"
#include "/home/codeleaded/System/Static/Library/Geometry.h"

#include "World.h"


#define FIGURE_ACC_GRD		7.0f
#define FIGURE_ACC_AIR		5.0f
#define FIGURE_ACC_MAX		100.0f
#define FIGURE_ACC_GGP		20.0f
#define FIGURE_ACC_AGP		2.0f

#define FIGURE_VEL_MAX_GRD	8.0f
#define FIGURE_VEL_MAX_AIR	10.0f
#define FIGURE_VEL_CLP		0.01f

#define FIGURE_VEL_JP 		20.0f

#define FIGURE_FALSE		0
#define FIGURE_TRUE			1

#define FIGURE_LEFT			0
#define FIGURE_RIGHT		1


typedef struct Figure{
	Vector imgs;
	Rect r;
	Vec2 v;
	Vec2 a;
	Timepoint start;
	unsigned char lookdir;
	unsigned char ground;
	unsigned char jumping;
} Figure;

char World_Figure_Block_IsPickUp(World* w,Figure* f,unsigned int x,unsigned int y){
	Block b = World_Get(w,x,y);

	if(b==BLOCK_COIN){
		World_Set(w,x,y,BLOCK_NONE);
		return 1;
	}else if(b==BLOCK_STAR_COIN){
		World_Set(w,x,y,BLOCK_NONE);
		return 1;
	}else if(b==BLOCK_REDPILZ){
		World_Set(w,x,y,BLOCK_NONE);
		return 1;
	}else if(b==BLOCK_GREENPILZ){
		World_Set(w,x,y,BLOCK_NONE);
		return 1;
	}else if(b==BLOCK_FIRE_FLOWER){
		World_Set(w,x,y,BLOCK_NONE);
		return 1;
	}
	return 0;
}
char World_Figure_Block_IsCollision(World* w,Figure* f,unsigned int x,unsigned int y,Side s){
	Block b = World_Get(w,x,y);

	if(b==BLOCK_PODEST) return s==SIDE_TOP && f->v.y>0.0f;
	return 1;
}
void World_Figure_Block_Collision(World* w,Figure* f,unsigned int x,unsigned int y,Side s){
	Block b = World_Get(w,x,y);

	if(s==SIDE_BOTTOM){
		if(b==BLOCK_BRICK) World_Set(w,x,y,BLOCK_NONE);
		else if(b==BLOCK_CLOSE_QUEST_RP){
			World_Set(w,x,y,BLOCK_OPEN_QUEST);
			World_Set(w,x,y-1,BLOCK_REDPILZ);
		}else if(b==BLOCK_CLOSE_QUEST_GP){
			World_Set(w,x,y,BLOCK_OPEN_QUEST);
			World_Set(w,x,y-1,BLOCK_GREENPILZ);
		}else if(b==BLOCK_CLOSE_QUEST_FF){
			World_Set(w,x,y,BLOCK_OPEN_QUEST);
			World_Set(w,x,y-1,BLOCK_FIRE_FLOWER);
		}
	}
}


Figure Figure_New(Vec2 p,Vec2 d){
	Figure f;
	f.imgs = Vector_New(sizeof(Sprite));
	f.r = Rect_New(p,d);
	f.v = (Vec2){ 0.0f,0.0f };
	f.a = (Vec2){ 0.0f,30.0f };

	f.start = 0UL;

	f.lookdir = FIGURE_RIGHT;
	f.ground = FIGURE_FALSE;
	f.jumping = FIGURE_FALSE;
	return f;
}
Figure Figure_Make(Vec2 p,Vec2 d,Sprite* s){
	Figure f = Figure_New(p,d);
	
	for(int i = 0;s[i].img && s[i].w>0 && s[i].h>0;i++){
		Vector_Push(&f.imgs,&s[i]);
	}
	return f;
}
void Figure_Update(Figure* f,const float t){
	f->a.x = F32_Clamp(f->a.x,-FIGURE_ACC_MAX,FIGURE_ACC_MAX);
	if(f->ground) 	f->v.x = F32_Clamp(f->v.x,-FIGURE_VEL_MAX_GRD,FIGURE_VEL_MAX_GRD);
	else			f->v.x = F32_Clamp(f->v.x,-FIGURE_VEL_MAX_AIR,FIGURE_VEL_MAX_AIR);

	if(f->v.x<0.0f) f->lookdir = FIGURE_LEFT;
	if(f->v.x>0.0f) f->lookdir = FIGURE_RIGHT;

	if(F32_Abs(f->v.x)>FIGURE_VEL_CLP){
		const float add = -F32_Sign(f->v.x) * (f->ground ? FIGURE_ACC_GGP : FIGURE_ACC_AGP) * t;
		f->v.x = F32_Passes(f->v.x,add,0.0f) ? 0.0f : f->v.x + add;
	}else
		f->v.x = 0.0f;

	f->v.x = f->v.x + F32_Krung(f->a.x,FIGURE_ACC_MAX) * t;
	f->v.y = f->v.y + f->a.y * t * (f->jumping ? 1.0f : 3.0f);
	
	//f->v = Vec2_Add(f->v,Vec2_Mulf(f->a,t * (f->jumping ? 1.0f : 2.0f)));
	f->r.p = Vec2_Add(f->r.p,Vec2_Mulf(f->v,t));

	if(f->r.p.x < 0.0f) f->r.p.x = 0.0f;
	if(f->r.p.y < 0.0f) f->r.p.y = 0.0f;

	f->jumping = FIGURE_FALSE;
}
void Figure_Collision(Figure* f,World* w,int (*Rect_Rect_Compare)(const void*,const void*)){
	Vector rects = Vector_New(sizeof(Rect));
	
	float searchX = F32_Max(2.0f,2.0f * f->r.d.x);
	float searchY = F32_Max(2.0f,2.0f * f->r.d.y);

	for(float x = -searchX;x<searchX;x+=1.0f) {
		for(float y = -searchY;y<searchY;y+=1.0f) {
			int sx = (int)(f->r.p.x + x);
			int sy = (int)(f->r.p.y + y);
			
            if(sy>=0 && sy<w->height && sx>=0 && sx<w->width) {
                Block b = World_Get(w,sx,sy);
                Rect br = { (Vec2){ sx,sy },(Vec2){ 1.0f,1.0f } };

				if(Overlap_Rect_Rect(f->r,br)){
					if(b!=BLOCK_NONE){
						Vector_Push(&rects,&br);
					}
				}
			}
		}
	}
	
    f->ground = FIGURE_FALSE;
	
	qsort(rects.Memory,rects.size,rects.ELEMENT_SIZE,Rect_Rect_Compare);
	
	for(int i = 0;i<rects.size;i++) {
        Rect rect = *(Rect*)Vector_Get(&rects,i);
		
		Side s = Side_Rect_Rect(f->r,rect);
		if(!World_Figure_Block_IsPickUp(w,f,rect.p.x,rect.p.y) && World_Figure_Block_IsCollision(w,f,rect.p.x,rect.p.y,s)){
			Resolve_Rect_Rect_Side(&f->r,rect,s);

			if(s==SIDE_TOP) 					f->ground = FIGURE_TRUE;
			if(s==SIDE_TOP && f->v.y>0.0f) 		f->v.y = 0.0f;
			if(s==SIDE_BOTTOM && f->v.y<0.0f) 	f->v.y = 0.0f;
			if(s==SIDE_LEFT && f->v.x>0.0f) 	f->v.x = 0.0f;
			if(s==SIDE_RIGHT && f->v.x<0.0f) 	f->v.x = 0.0f;

			World_Figure_Block_Collision(w,f,rect.p.x,rect.p.y,s);
		}
	}

	Vector_Free(&rects);
}
Sprite* Figure_Get_Img(Figure* f){
	FDuration d = Time_Elapsed(f->start,Time_Nano());
	//d *= (F32_Abs(f->v.x) / F32_Max(FIGURE_VEL_MAX_GRD,FIGURE_VEL_MAX_AIR));
	d /= F32_Abs(f->v.x);
	d = d - F32_Floor(d);
	int aimg = (int)(5.0f * d);

	if(f->v.x==0.0f) 	aimg = 5;
	if(!f->ground) 		aimg = 6;
	aimg = 2 * aimg + f->lookdir;

	if(aimg>=f->imgs.size) return NULL;

	Sprite* s = (Sprite*)Vector_Get(&f->imgs,aimg);
	return s;
}
void Figure_Resize(Figure* f,unsigned int w,unsigned int h){
	for(int i = 0;i<f->imgs.size;i++){
		Sprite* s = (Sprite*)Vector_Get(&f->imgs,i);
		
		if(s->w != w || s->h != h){
			Sprite_Reload(s,w,h);
		}
	}
}
void Figure_Render(Figure* f,TransformedView* tv,Pixel* out,unsigned int w,unsigned int h){
	const Vec2 scp = TransformedView_WorldScreenPos(tv,(Vec2){ f->r.p.x,f->r.p.y });
	const Vec2 scd = TransformedView_WorldScreenLength(tv,(Vec2){ f->r.d.x,f->r.d.y });
	
	Figure_Resize(f,(unsigned int)F32_Ceil(scd.x),(unsigned int)F32_Ceil(scd.y));
	
	Sprite* s = Figure_Get_Img(f);
	if(s) RenderSpriteAlpha(s,scp.x,scp.y);
}
void Figure_Free(Figure* f){
	for(int i = 0;i<f->imgs.size;i++){
		Sprite* s = (Sprite*)Vector_Get(&f->imgs,i);
		Sprite_Free(s);
	}
	Vector_Free(&f->imgs);
}

#endif