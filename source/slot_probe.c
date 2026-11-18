#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "eight_player_css.h"
typedef struct { union {double real; void *ptr; int32_t i32;}; int32_t flags,type;} RV;
typedef void (__cdecl *Event)(void*,void*);
static uintptr_t base;
static HMODULE original;
static char folder[MAX_PATH];
static RV* var(void*,int);
static void ensure_workshop_arrays(void*,void*);
static Event bg_orig,cur_orig,local_orig,cur_create_orig,bg_create_orig;
static void *last_cursor,*last_bg;
static Event button_create_orig,team_create_orig,bg_draw_orig;
static void *last_button,*last_team;
typedef void (__cdecl *Builtin)(RV*,void*,void*,int,RV*);
static void builtin(uint32_t va,RV*r,void*s,void*o,int n,RV*a){((Builtin)(base+va-0x400000))(r,s,o,n,a);}
static double number(const RV*r){int t=r->type&0xffffff;if(t==7)return r->i32;if(t==10)return (double)*(const int64_t*)r;return r->real;}
static int current_room(void*s){
 RV r={.type=5};typedef void(__cdecl*Read)(void*,int,int,RV*);
 ((Read)(base+0x554b070-0x400000))(s,*(int*)(base+0x5faaff4-0x400000),(int)0x80000000,&r);return (int)number(&r);
}
static void release(RV*r){if(r->type==1||r->type==2)((void(__cdecl*)(RV*))(base+0x401680-0x400000))(r);}
static int drawing_panel;
static int drawing_panel_player;
static int hidden_hud_surface=-1;
static int preview_player;
static Event preview_orig;
static void logmsg(const char*,...);
static int panel_character_name(int,char*,size_t);
static unsigned card_label_seen;
#ifdef SHOWCASE_CAPTURE
static int showcase_css_frozen;
#endif
static Builtin surface_part_orig;
static void panel_surface(Builtin,int,RV*,void*,void*,int,RV*);
static void __cdecl preview_hook(void*s,void*o){
 preview_player=(int)var(s,4725)->real;
 preview_orig(s,o);
 preview_player=0;
}
static void __cdecl surface_part_hook(RV*r,void*s,void*o,int n,RV*a){
 if(drawing_panel){panel_surface(surface_part_orig,2,r,s,o,n,a);return;}
 if(preview_player<1||preview_player>8||n!=7){surface_part_orig(r,s,o,n,a);return;}
 static unsigned logged;
 int p=preview_player;
 if(!(logged&(1u<<p))){logged|=1u<<p;logmsg("Preview P%d surface=%g source=%g,%g %gx%g destination=%g,%g",p,a[0].real,a[1].real,a[2].real,a[3].real,a[4].real,a[5].real,a[6].real);}
 RV args[12];memcpy(args,a,7*sizeof(RV));
 args[5].real=a[5].real-238*((p-1)/4)*4+a[3].real*.25;
 args[6].real=316+112*((p-1)/4)+(a[6].real-316)*.5;
 args[7]=(RV){.real=.5};args[8]=(RV){.real=.5};args[9]=(RV){.real=0xffffff};args[10]=(RV){.real=1};
 builtin(0x5581960,r,s,o,11,args);
}
static double drawing_panel_x,drawing_panel_y;
static void panel_surface(Builtin original,int kind,RV*r,void*s,void*o,int n,RV*a){
 if(hidden_hud_surface>=0&&n>0&&(int)number(a)==hidden_hud_surface){r->real=0;r->type=0;r->flags=0;return;}
 if(!drawing_panel){original(r,s,o,n,a);return;}
 int part=kind>=2,ix=part?5:1;
 RV width={.type=5};if(!part)builtin(0x5584a40,&width,s,o,1,a);
 double w=part?a[3].real:width.real;
 if(kind==1)w*=a[3].real;else if(kind>=3)w*=a[7].real;
 static unsigned logged; if(!(logged&(1u<<kind))){logged|=1u<<kind;logmsg("Panel surface kind=%d width=%g x=%g y=%g",kind,w,a[ix].real,a[ix+1].real);}
 RV saved={.type=5},mat={.type=5},out={.type=5},world={.real=2};builtin(0x56775f0,&saved,s,o,1,&world);
 RV b[9]={{.real=a[ix].real*.5+w*.25},{.real=drawing_panel_y*.5},{0},{0},{0},{0},{.real=.5},{.real=.5},{.real=1}};
 builtin(0x5676a10,&mat,s,o,9,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 original(r,s,o,n,a);set[1]=saved;builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&saved);
}
#define SURFACE_HOOK(N) static Builtin surface_orig_##N; static void __cdecl surface_hook_##N(RV*r,void*s,void*o,int n,RV*a){panel_surface(surface_orig_##N,N,r,s,o,n,a);}
SURFACE_HOOK(0) SURFACE_HOOK(1) SURFACE_HOOK(3) SURFACE_HOOK(4)
static void panel_text(Builtin original,int kind,RV*r,void*s,void*o,int n,RV*a){
 if(!drawing_panel||n<3){original(r,s,o,n,a);return;}
 RV replacement={.type=5},args[16];int renamed=0;
 if(drawing_panel_player>=1&&drawing_panel_player<=8&&n<=16&&(a[2].type&0xffffff)==1&&a[2].ptr){
  const char*text=*(const char**)a[2].ptr;
  char label[64];
  if(text&&panel_character_name(drawing_panel_player,label,sizeof label)&&(strcmp(text,"MENU")==0||strcmp(text,"ENTER")==0||strcmp(text,"START")==0||strncmp(text,"Player ",7)==0||strncmp(text,"PLAYER ",7)==0||_stricmp(text,label)==0)){
   ((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&replacement,label);
   memcpy(args,a,n*sizeof(RV));args[0].real=drawing_panel_x+155;args[1].real+=32;args[2]=replacement;a=args;renamed=1;
   if(!(card_label_seen&(1u<<drawing_panel_player))){card_label_seen|=1u<<drawing_panel_player;logmsg("Card label P%d = %s",drawing_panel_player,label);}
  }
 }
 RV saved={.type=5},mat={.type=5},out={.type=5},world={.real=2};
 builtin(0x56775f0,&saved,s,o,1,&world);
 double x=a[0].real,y=a[1].real;
 double scale=.75;RV oldfont={.type=5};
 if(renamed){builtin(0x55804c0,&oldfont,s,o,0,NULL);RV font={.real=-1},width={.type=5};builtin(0x5580dd0,&out,s,o,1,&font);builtin(0x55843c0,&width,s,o,1,&replacement);double textscale=kind>=6&&n>5?number(a+5):kind>=4&&n>3?number(a+3):1;if(textscale<0)textscale=-textscale;if(number(&width)*textscale*scale>130)scale=130/(number(&width)*textscale);}
 RV b[9]={{.real=x*(1-scale)},{.real=drawing_panel_y*.5+y*(.5-scale)},{.real=0},{.real=0},{.real=0},{.real=0},{.real=scale},{.real=scale},{.real=1}};
 builtin(0x5676a10,&mat,s,o,9,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 original(r,s,o,n,a);set[1]=saved;builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&saved);
 if(renamed){builtin(0x5580dd0,&out,s,o,1,&oldfont);release(&replacement);}
}
#define TEXT_HOOK(N) static Builtin text_orig_##N; static void __cdecl text_hook_##N(RV*r,void*s,void*o,int n,RV*a){panel_text(text_orig_##N,N,r,s,o,n,a);}
TEXT_HOOK(0) TEXT_HOOK(1) TEXT_HOOK(2) TEXT_HOOK(3) TEXT_HOOK(4) TEXT_HOOK(5) TEXT_HOOK(6) TEXT_HOOK(7)
static void panel_sprite(Builtin original,int part,RV*r,void*s,void*o,int n,RV*a){
 if(!drawing_panel||n<(part==1?8:part==2?6:4)){original(r,s,o,n,a);return;}
 RV width={.type=5},height={.type=5};builtin(0x56882f0,&width,s,o,1,a);builtin(0x5688150,&height,s,o,1,a);
 double w=part?a[4].real:width.real,h=part?a[5].real:height.real;
 int sprite=(int)a[0].real;
 static int logged_ids[96],logged_count;int found=0;
 for(int i=0;i<logged_count;i++)if(logged_ids[i]==sprite)found=1;
 if(!found&&logged_count<96){logged_ids[logged_count++]=sprite;logmsg("Panel asset id=%d kind=%d size=%gx%g x=%g y=%g scale=%g,%g",sprite,part,w,h,a[part==1?6:2].real,a[part==1?7:3].real,n>=6?a[4].real:1,n>=6?a[5].real:1);}
 int frame=sprite==4101||sprite==4102||sprite==4139||sprite==4173||sprite==4182||sprite==4187||sprite==4199||sprite==4200||sprite==4208||sprite==4211||sprite==4914||sprite==4927||sprite==4928||sprite==4929;
 if(w<8||h<8||frame){original(r,s,o,n,a);return;}
 int ix=part==1?6:2;double x=a[ix].real,scale=part==2?1:(n>(part?8:4)?a[part?8:4].real:1);
 static int logged[3];if(!logged[part]){logged[part]=1;logmsg("Panel sprite kind=%d sprite=%d size=%gx%g",part,sprite,w,h);}
 RV saved={.type=5},mat={.type=5},out={.type=5},world={.real=2};builtin(0x56775f0,&saved,s,o,1,&world);
 RV b[9]={{.real=x*.5+w*scale*.25},{.real=drawing_panel_y*.5},{.real=0},{.real=0},{.real=0},{.real=0},{.real=.5},{.real=.5},{.real=1}};
 builtin(0x5676a10,&mat,s,o,9,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 original(r,s,o,n,a);set[1]=saved;builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&saved);
}
#define SPRITE_HOOK(N,P) static Builtin sprite_orig_##N; static void __cdecl sprite_hook_##N(RV*r,void*s,void*o,int n,RV*a){panel_sprite(sprite_orig_##N,P,r,s,o,n,a);}
SPRITE_HOOK(0,0) SPRITE_HOOK(1,0) SPRITE_HOOK(2,1) SPRITE_HOOK(3,1) SPRITE_HOOK(4,1)
SPRITE_HOOK(5,2) SPRITE_HOOK(6,2)
typedef void(__cdecl*FastSprite)(void*,int,int,float,float,float,float,float,int,float);
static FastSprite fast_sprite_orig;
static void fast_sprite_bridge(RV*r,void*s,void*o,int n,RV*a){
 fast_sprite_orig(s,(int)a[0].real,(int)a[1].real,(float)a[2].real,(float)a[3].real,(float)a[4].real,(float)a[5].real,(float)a[6].real,(int)a[7].real,(float)a[8].real);
}
static void __cdecl fast_sprite_hook(void*s,int sprite,int frame,float x,float y,float xs,float ys,float rot,int color,float alpha){
 RV a[9]={{.real=sprite},{.real=frame},{.real=x},{.real=y},{.real=xs},{.real=ys},{.real=rot},{.real=color},{.real=alpha}},out={.type=5};
 panel_sprite(fast_sprite_bridge,0,&out,s,NULL,9,a);
}
static int patch_word(uint32_t va,uint32_t before,uint32_t after){uint32_t*p=(uint32_t*)(base+va-0x400000);if(*p==after)return 1;if(*p!=before){return 0;}DWORD old;if(!VirtualProtect(p,4,PAGE_EXECUTE_READWRITE,&old))return 0;*p=after;VirtualProtect(p,4,old,&old);FlushInstructionCache(GetCurrentProcess(),p,4);return 1;}
static void __cdecl bg_draw_hook(void*s,void*o){
 ensure_workshop_arrays(s,o);
 RV saved={.type=5},mat={.type=5},out={.type=5},world={.real=2};
 builtin(0x56775f0,&saved,s,o,1,&world);
 double x=*(float*)((char*)s+0xa0),y=*(float*)((char*)s+0xa4);

 RV b[9]={{.real=0},{.real=y*.5},{.real=0},{.real=0},{.real=0},{.real=0},{.real=1},{.real=.5},{.real=1}};
 builtin(0x5676a10,&mat,s,o,9,b);RV a[2]={world,mat};builtin(0x5678660,&out,s,o,2,a);
 drawing_panel=1;drawing_panel_player=(int)number(var(s,4725));drawing_panel_x=x;drawing_panel_y=y;bg_draw_orig(s,o);drawing_panel=0;drawing_panel_player=0;a[1]=saved;builtin(0x5678660,&out,s,o,2,a);release(&mat);release(&saved);
}
static void __cdecl button_create_hook(void*s,void*o){button_create_orig(s,o);last_button=s;}
static void __cdecl team_create_hook(void*s,void*o){team_create_orig(s,o);last_team=s;}
typedef RV*(__cdecl *Script)(void*,void*,RV*,int,RV**);
static Script ugc_set_orig,read_item_name_orig;
static int loading_workshop_player;
static char workshop_display_names[9][64],workshop_directories[9][MAX_PATH];
static void workshop_capture_path(int p,int n,RV**a){
 if(p<1||p>8)return;
 for(int i=0;i<n;i++)if((a[i]->type&0xffffff)==1&&a[i]->ptr){const char*text=*(const char**)a[i]->ptr;if(!text)continue;DWORD attributes=GetFileAttributesA(text);if(attributes==INVALID_FILE_ATTRIBUTES)continue;char full[MAX_PATH];if(!GetFullPathNameA(text,sizeof full,full,NULL))continue;if(!(attributes&FILE_ATTRIBUTE_DIRECTORY)){char*slash=strrchr(full,'\\');if(slash)*slash=0;else continue;}snprintf(workshop_directories[p],sizeof workshop_directories[p],"%s",full);}
}

static RV* __cdecl ugc_set_hook(void*s,void*o,RV*r,int n,RV**a){
 int previous=loading_workshop_player,p=n>0?(int)number(a[0]):0;loading_workshop_player=p>=1&&p<=8?p:0;
 workshop_capture_path(p,n,a);
 MEMORYSTATUSEX memory={.dwLength=sizeof memory};if(GlobalMemoryStatusEx(&memory))logmsg("Workshop load P%d available virtual memory=%llu MiB",p,(unsigned long long)(memory.ullAvailVirtual/(1024*1024)));
 RV*result=ugc_set_orig(s,o,r,n,a);loading_workshop_player=previous;return result;
}
static RV* __cdecl read_item_name_hook(void*s,void*o,RV*r,int n,RV**a){
 int p=loading_workshop_player;workshop_capture_path(p,n,a);RV*result=read_item_name_orig(s,o,r,n,a);
 if(p>=1&&p<=8&&(result->type&0xffffff)==1&&result->ptr){const char*name=*(const char**)result->ptr;if(name)snprintf(workshop_display_names[p],sizeof workshop_display_names[p],"%s",name);}
 return result;
}
static Script color_orig,dark_orig;
static const unsigned extra_colors[]={0x2099ff,0xf080b0,0x55e6e6,0xe6e6e6};
static RV* player_color(Script original,int dark,void*s,void*o,RV*r,int n,RV**a){
 int p=n>0?(int)a[0]->real:0;
 if(p<5||p>8)return original(s,o,r,n,a);
 unsigned c=extra_colors[p-5];if(dark)c=(((c&255)*2/5))|((((c>>8)&255)*2/5)<<8)|((((c>>16)&255)*2/5)<<16);
 release(r);r->real=c;r->flags=0;r->type=0;return r;
}
static RV* __cdecl color_hook(void*s,void*o,RV*r,int n,RV**a){return player_color(color_orig,0,s,o,r,n,a);}
static RV* __cdecl dark_hook(void*s,void*o,RV*r,int n,RV**a){return player_color(dark_orig,1,s,o,r,n,a);}
static void setnum(void *o,int id,double n){RV*v=var(o,id);v->real=n;v->flags=0;v->type=0;}
static int seen[128],nseen;
static unsigned fixture_slots;
static void logmsg(const char *fmt,...) {char path[MAX_PATH]; snprintf(path,sizeof path,"%s\\slot-probe.log",folder);FILE*f=fopen(path,"a");if(!f)return;va_list ap;va_start(ap,fmt);vfprintf(f,fmt,ap);va_end(ap);fputc('\n',f);fclose(f);}
#include "release_guard.h"
static void __cdecl test_failure(const char *format,...){
 char message[4096];va_list ap;va_start(ap,format);vsnprintf(message,sizeof message,format,ap);va_end(ap);
 logmsg("TEST FAILED: %s",message);
 MEMORYSTATUSEX memory={.dwLength=sizeof memory};if(GlobalMemoryStatusEx(&memory))logmsg("Failure available virtual memory=%llu MiB",(unsigned long long)(memory.ullAvailVirtual/(1024*1024)));
 uintptr_t*trace=*(uintptr_t**)(base+0x604a560-0x400000);
 for(int i=0;trace&&i<24;i++){logmsg("GML %s line %lu",(char*)trace[1],(unsigned long)trace[2]);trace=(uintptr_t*)trace[0];}
 void *frames[32];USHORT count=CaptureStackBackTrace(0,32,frames,NULL);
 for(USHORT i=0;i<count;i++)logmsg("frame %u va=%08lx",i,(unsigned long)((uintptr_t)frames[i]-base+0x400000));
 if(strstr(message,"Memory allocation failed")||strstr(message,"Out of memory")||strstr(message,"out of memory"))stop_release("Rivals ran out of memory while loading Workshop content. Restart with lighter fighters or a lighter stage. Details are in slot-probe.log beside the copied game.");
 stop_release("The mod encountered an error and must close. Details were saved to slot-probe.log beside the copied game. Your Steam installation was not modified.");
}
static int install_test_error_handler(void){
 unsigned char*p=(unsigned char*)(base+0x5549940-0x400000);const unsigned char expected[]={0x81,0xec,0x04,0x04,0,0};
 if(memcmp(p,expected,6))return 0;DWORD old;if(!VirtualProtect(p,6,PAGE_EXECUTE_READWRITE,&old))return 0;
 p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)test_failure-(intptr_t)(p+5);p[5]=0x90;
 VirtualProtect(p,6,old,&old);FlushInstructionCache(GetCurrentProcess(),p,6);return 1;
}
static RV* var(void *obj,int id) {RV *a=*(RV**)((char*)obj+4);if(a)return a+id;typedef RV*(__attribute__((thiscall))*Get)(void*,int);return ((Get)(*(void***)obj)[1])(obj,id);}
#include "slot_arrays.h"
typedef struct {int length;RV *values;} Dimension;
typedef struct {int refs;Dimension*dims;void*owner;int visited,length;} Array;
static void ensure_workshop_arrays(void*s,void*o){
 void*g=*(void**)(base+0x604a8d8-0x400000);
 typedef RV*(__cdecl*Grow)(RV*,int);Grow grow=(Grow)(base+0x5555f00-0x400000);
 for(int id=7044;id<=7045;id++){
  RV*v=var(g,id);if(v->type!=2||!v->ptr)continue;Array*ar=v->ptr;
  if(ar->length!=1||!ar->dims||ar->dims[0].length<1||ar->dims[0].length>=9)continue;
  int count=ar->dims[0].length;RV sample=ar->dims[0].values[0];
  for(int p=count;p<=8;p++){
   if(id==7045){RV result={.type=5};((Script)(base+0x408ed0-0x400000))(s,o,&result,0,NULL);*grow(v,p)=result;}
   else {RV*dst=grow(v,p);*dst=sample;if(sample.type==1&&sample.ptr)((int*)sample.ptr)[1]++;}
  }
  logmsg("Initialized Workshop array %d for eight slots",id);
 }
}
static void array_number(void*g,int id,int index,double value){typedef RV*(__cdecl*Grow)(RV*,int);RV*v=((Grow)(base+0x5555f00-0x400000))(var(g,id),index);release(v);v->real=value;v->flags=0;v->type=0;}
static double array_value(void*g,int id,int index){
 RV*v=var(g,id);if(v->type!=2||!v->ptr)return 0;Array*a=v->ptr;
 if(a->length!=1||!a->dims||index<0||index>=a->dims[0].length)return 0;
 return number(a->dims[0].values+index);
}
static int panel_character_name(int p,char*label,size_t size){
 void*g=*(void**)(base+0x604a8d8-0x400000);if(p<1||p>8||array_value(g,4737,p)==0)return 0;
 int selected=(int)array_value(g,4805,p);
 static const char*names[]={"SELECT FIGHTER","RANDOM","ZETTERBURN","ORCANE","WRASTOR","KRAGG","FORSBURN","MAYPUL","ABSA","ETALUS","ORI","RANNO","CLAIREN","SYLVANOS","ELLIANA","SHOVEL KNIGHT","POMME","MOLLO","OLYMPIA","HODAN","SANDBAG"};
 const char*name=selected>=0&&selected<=20?names[selected]:workshop_display_names[p][0]?workshop_display_names[p]:"WORKSHOP FIGHTER";
 snprintf(label,size,"%s",name);for(char*c=label;*c;c++)if(*c>='a'&&*c<='z')*c-='a'-'A';return 1;
}
static Script controls_orig;
static RV* __cdecl controls_hook(void*s,void*o,RV*r,int n,RV**a){
 void*g=*(void**)(base+0x604a8d8-0x400000);
 if(number(var(g,4208))!=0||number(var(g,4197))!=0)stop_release("This build supports local play only. Close it and launch the unmodified Steam game for online play.");
 if(current_room(s)!=3){
  uint32_t limit=number(var(g,3464))==9?0x40300000:0x40200000;patch_word(0xcfe800,0x40100000,limit);
  RV*result=controls_orig(s,o,r,n,a);patch_word(0xcfe800,limit,0x40100000);return result;
 }
 int slot=(int)number(var(g,3464));
 int active=slot>=1&&slot<=8&&array_value(g,2976,slot-1)==0;
 RV key={.real=13},pressed={.type=5};builtin(0x55948c0,&pressed,s,o,1,&key);
 if(!active&&number(&pressed)!=0){
  int free_slot=9;
  for(int p=1;p<=8;p++)if(array_value(g,4737,p)==0&&array_value(g,2976,p-1)==0){free_slot=p;break;}
  setnum(g,3453,free_slot);setnum(g,3464,free_slot);slot=free_slot;
  logmsg("Keyboard Enter assigned unused slot %d",slot);
 }

 uint32_t limit=slot==9?0x40300000:0x40200000;
 patch_word(0xcfe800,0x40100000,limit);
 RV*result=controls_orig(s,o,r,n,a);
 if(number(&pressed)!=0){
  logmsg("Keyboard Enter before=%d after=%g override=%g",slot,number(var(g,3464)),number(var(g,3453)));
  for(int i=0;i<8;i++)logmsg("Input slot P%d gp_connected=%g gp_id=%g player_connected=%g",i+1,array_value(g,2976,i),array_value(g,2979,i),array_value(g,4737,i+1));
 }
 patch_word(0xcfe800,limit,0x40100000);
 return result;
}
static void extend_slot_arrays(void *global){
 typedef RV*(__cdecl*Grow)(RV*,int);Grow grow=(Grow)(base+0x5555f00-0x400000);
 for(unsigned k=0;k<sizeof(slot_array_ids)/sizeof(slot_array_ids[0]);k++){
  int id=slot_array_ids[k];RV*v=var(global,id);if((v->type&0xffffff)!=2||!v->ptr)continue;
  Array*a=v->ptr;if(a->length!=1||!a->dims)continue;int count=a->dims[0].length;if(count<2||count>8)continue;
  RV sample=a->dims[0].values[count>=5?4:1];int type=sample.type&0xffffff;
  if(id==2976){sample.real=0;sample.type=0;type=0;}
  if(id==2979){sample.real=-1;sample.type=0;type=0;}
  if(type!=0&&type!=1&&type!=7&&type!=10)continue;
  for(int i=count;i<=8;i++){RV*dst=grow(v,i);*dst=sample;if(type==1&&sample.ptr)((int*)sample.ptr)[1]++;}
  logmsg("Extended slot array %d from %d to 9",id,count);
 }
}
static Script swap_orig;
static int local_eight;
static Script css_child_create_orig;
static RV* __cdecl css_child_create_hook(void*s,void*o,RV*r,int n,RV**a){
 if(local_eight&&n>=3&&n<=16&&*(int*)((char*)s+0x7c)==224&&current_room(s)==3){
  int object=(int)number(a[2]);double parent_y=*(float*)((char*)s+0xa4),offset=number(a[1])-parent_y;
  if(object>=231&&object<=274&&offset>=120&&offset<=208){RV y={.real=parent_y+offset*.5-22};RV*args[16];memcpy(args,a,n*sizeof(RV*));args[1]=&y;return css_child_create_orig(s,o,r,n,args);}
 }
 return css_child_create_orig(s,o,r,n,a);
}
static int css_was_cpu[9];
static Script puck_position_orig;
static int css_point_value(RV*v,double*x,double*y){
 if((v->type&0xffffff)!=2||!v->ptr)return 0;Array*a=v->ptr;
 if(a->length!=1||!a->dims||a->dims[0].length<2||!a->dims[0].values)return 0;
 RV*point=a->dims[0].values;int tx=point[0].type&0xffffff,ty=point[1].type&0xffffff;
 if((tx!=0&&tx!=7&&tx!=10)||(ty!=0&&ty!=7&&ty!=10))return 0;
 *x=number(point);*y=number(point+1);return *x==*x&&*y==*y;
}
static RV* __cdecl puck_position_hook(void*s,void*o,RV*r,int n,RV**a){
 if(!local_eight||n!=2||*(int*)((char*)s+0x7c)!=220||current_room(s)!=3)return puck_position_orig(s,o,r,n,a);
 int p=(int)number(a[1]);if(p<1||p>8)return puck_position_orig(s,o,r,n,a);
 RV first={.real=1},last={.real=4},end={.type=5};RV*args[]={a[0],&first};
 RV*result=puck_position_orig(s,o,r,2,args);args[1]=&last;puck_position_orig(s,o,&end,2,args);
 double x1,y1,x4,y4,x,y;
 if(css_point_value(result,&x1,&y1)&&css_point_value(&end,&x4,&y4)&&css_puck_point(p,x1,y1,x4,y4,&x,&y)){
  typedef RV*(__cdecl*Grow)(RV*,int);Grow grow=(Grow)(base+0x5555f00-0x400000);
  RV*value=grow(result,0);release(value);*value=(RV){.real=x};value=grow(result,1);release(value);*value=(RV){.real=y};
 }
 release(&end);return result;
}
static void css_prepare_slot(void*g,int p){
 if(!local_eight||p<1||p>8)return;
 int cpu=array_value(g,4737,p)!=0&&array_value(g,4727,p)>0;
 if(cpu){
  double level=array_value(g,193,p);int valid=css_cpu_level(level);if(level!=valid)array_number(g,193,p,valid);
  if(!css_was_cpu[p]){const int menus[]={4744,4766,4770,4787,4790,4793,4804,4811};for(unsigned i=0;i<sizeof menus/sizeof menus[0];i++)array_number(g,menus[i],p,0);}
 }
 css_was_cpu[p]=cpu;
}
static int stage_spawn_point(void*g,int p,double*x,double*y){
 int stage=(int)number(var(g,0x49f)),ordinal=0;for(int i=1;i<p;i++)if(array_value(g,4737,i)!=0)ordinal++;
 RV*v=var(g,0x1707);if((v->type&0xffffff)!=2||!v->ptr)return 0;Array*a=v->ptr;
 int column=15+2*(ordinal%4);
 if(stage<0||stage>=a->length||!a->dims||a->dims[stage].length<=column+1||!a->dims[stage].values)return 0;
 RV*point=a->dims[stage].values+column;int tx=point[0].type&0xffffff,ty=point[1].type&0xffffff;
 if((tx!=0&&tx!=7&&tx!=10)||(ty!=0&&ty!=7&&ty!=10))return 0;
 *x=number(point);*y=number(point+1);return *x==*x&&*y==*y&&*x>-100000&&*x<100000&&*y>-100000&&*y<100000;
}
static Script player_init_orig;
static int initialized_spawn_instance[9];
static RV* __cdecl player_init_hook(void*s,void*o,RV*r,int n,RV**a){
 int p=(int)number(var(s,4725));RV*result=player_init_orig(s,o,r,n,a);
 void*g=*(void**)(base+0x604a8d8-0x400000);int room=current_room(s);double x,y;
 int timer_type=var(s,185)->type&0xffffff;
 if(local_eight&&p>=5&&p<=8&&array_value(g,4727,p)==0&&number(var(s,3346))!=0&&timer_type!=0&&timer_type!=7&&timer_type!=10){RV init={.type=5};((Script)(base+0x24219d0-0x400000))(s,o,&init,0,NULL);release(&init);}
 int instance=*(int*)((char*)s+0x78);

 if(local_eight&&p>=5&&p<=8&&initialized_spawn_instance[p]!=instance&&number(var(s,3346))!=0&&room>=10&&number(var(g,4208))==0&&stage_spawn_point(g,p,&x,&y)){
  *(float*)((char*)s+0xa0)=(float)x;*(float*)((char*)s+0xa4)=(float)y;
  array_number(g,2851,p,x);array_number(g,2852,p,y);
  initialized_spawn_instance[p]=instance;
  logmsg("Initialized stage %g P%d at spawn x=%g y=%g",number(var(g,0x49f)),p,x,y);
 }
 return result;
}
static Script spawn_num_orig;
static Script death_orig;
static Script stage_destination_orig;
static RV* __cdecl stage_destination_hook(void*s,void*o,RV*r,int n,RV**a){
 RV*result=stage_destination_orig(s,o,r,n,a);void*g=*(void**)(base+0x604a8d8-0x400000);
 if(local_eight&&number(var(g,4208))==0&&number(result)==53){release(result);*result=(RV){.real=54};logmsg("Rivals Eight Local uses the hazard-free Abyss stage layout");}
 return result;
}
static RV* __cdecl death_hook(void*s,void*o,RV*r,int n,RV**a){
 void*g=*(void**)(base+0x604a8d8-0x400000);int last=(int)number(var(s,3566));
 if(local_eight&&number(var(g,4208))==0&&(last<0||last>8||(last>0&&array_value(g,4805,last)<1))){logmsg("Cleared invalid knockout attacker %d select=%g",last,last>=0&&last<=8?array_value(g,4805,last):-1);setnum(s,3566,0);}
 
 int p=(int)number(var(s,4725));double before=p>=1&&p<=8?array_value(g,4815,p):-1;
 RV*result=death_orig(s,o,r,n,a);
 double after=p>=1&&p<=8?array_value(g,4815,p):-1;
 if(local_eight&&number(var(g,4208))==0&&p>=5&&p<=8&&before>after&&after>0&&number(var(s,5999))==14){
  double x,y;if(stage_spawn_point(g,p,&x,&y)){
   *(float*)((char*)s+0xa0)=(float)x;*(float*)((char*)s+0xa4)=(float)y;
   array_number(g,2851,p,x);array_number(g,2852,p,y);
   logmsg("Respawn P%d stocks=%g at %g,%g",p,after,x,y);
  }
 }
#ifdef AUTOMATED_MATCH_TEST
 if(before!=after)logmsg("KO P%d stocks=%g->%g state=%g timer=%g waiting=%g x=%g y=%g",p,before,after,number(var(s,5999)),number(var(s,6002)),number(var(s,6926)),*(float*)((char*)s+0xa0),*(float*)((char*)s+0xa4));
#endif
 return result;
}
static RV* __cdecl spawn_num_hook(void*s,void*o,RV*r,int n,RV**a){
 void*g=*(void**)(base+0x604a8d8-0x400000);int p=(int)number(var(s,4725));
 if(local_eight&&number(var(g,4208))==0&&p>=5&&p<=8){
  int ordinal=0;for(int i=1;i<p;i++)if(array_value(g,4737,i)!=0)ordinal++;
  release(r);*r=(RV){.real=ordinal%4};logmsg("Stage spawn P%d uses native spawn %d",p,ordinal%4);return r;
 }
 return spawn_num_orig(s,o,r,n,a);
}
static Script collision_orig;
static void extend_collision_array(void*s,int id){
 RV*v=var(s,id);if(v->type!=2||!v->ptr)return;Array*a=v->ptr;
 if(a->length!=1||!a->dims||a->dims[0].length<1||a->dims[0].length>=9)return;
 int count=a->dims[0].length;RV sample=a->dims[0].values[0];int type=sample.type&0xffffff;
 if(type!=0&&type!=7&&type!=10)return;
 typedef RV*(__cdecl*Grow)(RV*,int);Grow grow=(Grow)(base+0x5555f00-0x400000);
 for(int i=count;i<=8;i++)*grow(v,i)=sample;
 logmsg("Extended collision array %d to nine entries for instance %d",id,*(int*)((char*)s+0x78));
}
static RV* __cdecl collision_hook(void*s,void*o,RV*r,int n,RV**a){
 if(local_eight){extend_collision_array(s,0x637);extend_collision_array(s,0x2f4);}
 return collision_orig(s,o,r,n,a);
}
static Event fighter_create_orig,fighter_update_orig,hud_create_orig,hud_update_orig;
static void*last_fighter;
static int stage_spawned;
static int hud_custom_active[9];
static unsigned fighter_diagnostic_frames[9];
static int match_slots[9],match_eliminated_at[9],match_ticks;
static double match_previous_stocks[9];
static double spawn_anchor_x,spawn_anchor_y;
static double native_spawn_x[4],native_spawn_y[4];
static int native_spawn_count,creating_extra;
static void __cdecl fighter_create_hook(void*s,void*o){fighter_create_orig(s,o);last_fighter=s;spawn_anchor_x=*(float*)((char*)s+0xa0);spawn_anchor_y=*(float*)((char*)s+0xa4);
 if(!creating_extra&&native_spawn_count<4){int index=native_spawn_count++;native_spawn_x[index]=spawn_anchor_x;native_spawn_y[index]=spawn_anchor_y;logmsg("Native spawn %d x=%g y=%g",index,spawn_anchor_x,spawn_anchor_y);}
}
static void __cdecl hud_create_hook(void*s,void*o){stage_spawned=0;match_ticks=0;memset(match_slots,0,sizeof match_slots);memset(match_eliminated_at,0,sizeof match_eliminated_at);memset(match_previous_stocks,0,sizeof match_previous_stocks);memset(initialized_spawn_instance,0,sizeof initialized_spawn_instance);memset(fighter_diagnostic_frames,0,sizeof fighter_diagnostic_frames);hud_create_orig(s,o);}
static void spawn_extra_fighters(void*s,void*o){
 if(!local_eight||stage_spawned)return;
 void*g=*(void**)(base+0x604a8d8-0x400000);
 if(var(g,4208)->real!=0||var(g,4197)->real!=0)return;
 stage_spawned=1;extend_slot_arrays(g);
 for(int p=1;p<=8;p++){
  if(array_value(g,4737,p)==0){array_number(g,4815,p,0);continue;}
  logmsg("Preparing P%d for a new match: stock=%g off=%g",p,array_value(g,4815,p),array_value(g,4786,p));
  array_number(g,4786,p,0);array_number(g,4745,p,0);array_number(g,4750,p,0);array_number(g,4774,p,0);array_number(g,4817,p,0);array_number(g,4797,p,0);
  double stocks=number(var(g,0x17c1));if(stocks>0)array_number(g,4815,p,stocks);
 }
 double anchor_x=spawn_anchor_x,anchor_y=spawn_anchor_y;
 RV*cv=var(g,4737);if(cv->type!=2)return;Array*ca=cv->ptr;
 for(int p=5;p<=8;p++){
  if(ca->length!=1||ca->dims[0].length<=p||ca->dims[0].values[p].real==0)continue;
  int ordinal=0;for(int i=1;i<p;i++)if(array_value(g,4737,i)!=0)ordinal++;
  int point=native_spawn_count>0?ordinal%native_spawn_count:0;
  double px=native_spawn_count>0?native_spawn_x[point]:anchor_x,py=native_spawn_count>0?native_spawn_y[point]:anchor_y;stage_spawn_point(g,p,&px,&py);
  RV x={.real=px},y={.real=py},obj={.real=3},out={.type=5};RV*a[]={&x,&y,&obj};
  last_fighter=NULL;creating_extra=1;((Script)(base+0x32f3f20-0x400000))(s,o,&out,3,a);creating_extra=0;
  if(last_fighter){setnum(last_fighter,4725,p);logmsg("Spawned fighter P%d instance=%g",p,out.real);}
 }
 for(int p=1;p<=8;p++){match_slots[p]=array_value(g,4737,p)!=0;match_previous_stocks[p]=array_value(g,4815,p);}
}
static void __cdecl hud_update_hook(void*s,void*o){
 spawn_extra_fighters(s,o);
 void*g=*(void**)(base+0x604a8d8-0x400000);
 double before=number(var(g,2909));
 hud_update_orig(s,o);
 match_ticks++;
 for(int p=1;p<=8;p++)if(match_slots[p]){double stocks=array_value(g,4815,p);if(stocks<=0&&match_previous_stocks[p]>0&&!match_eliminated_at[p]){match_eliminated_at[p]=match_ticks;logmsg("Player P%d eliminated at match tick %d",p,match_ticks);}match_previous_stocks[p]=stocks;}
 if(match_ticks>120&&number(var(g,2909))==0&&number(var(g,0x17c1))>0&&number(var(g,0x18a1))==0){
  int active=0,alive=0,extra=0,winner=0;for(int p=1;p<=8;p++)if(match_slots[p]){active++;if(p>4)extra=1;if(array_value(g,4815,p)>0){alive++;winner=p;}}
  if(extra&&active>=2&&alive==1){RV result={.type=5};logmsg("Ending local stock match with surviving P%d",winner);((Script)(base+0x3480cc0-0x400000))(s,o,&result,0,NULL);release(&result);}
 }
#ifdef AUTOMATED_MATCH_TEST
 static unsigned ticks;unsigned frame=++ticks;
 double after=number(var(g,2909));
 if(frame==1||frame==60||frame==180||before!=after){
  RV object={.real=3},count={.type=5};builtin(0x557bde0,&count,s,o,1,&object);
  logmsg("HUD frame=%u ending=%g->%g timer=%g max=%g winner=%g fighters=%g",frame,before,after,number(var(g,2914)),number(var(g,2915)),number(var(g,2903)),number(&count));
  for(int p=1;p<=8;p++)logmsg("HUD P%d stock=%g connected=%g rank=%g deaths=%g",p,array_value(g,4815,p),array_value(g,4737,p),array_value(g,4797,p),array_value(g,4750,p));
#ifdef STAGE_SPAWN_TEST
  if(frame==180)for(int p=5;p<=8;p++)if(array_value(g,4737,p)!=0&&array_value(g,4815,p)!=3){logmsg("HARNESS failed initial-spawn stock check P%d",p);ExitProcess(47);}
#endif
 }
#endif
}
#ifdef AUTOMATED_MATCH_TEST
static void harness_fighter(void*s,void*o);
#endif
static void __cdecl fighter_update_hook(void*s,void*o){spawn_extra_fighters(s,o);
 int hud_player=(int)number(var(s,4725));
 if(hud_player>=1&&hud_player<=8)hud_custom_active[hud_player]=number(var(s,0x4b3))!=0||number(var(s,0x4d7))!=0;
#ifdef AUTOMATED_MATCH_TEST
 harness_fighter(s,o);
#endif
 fighter_update_orig(s,o);
 if(hud_player>=5&&hud_player<=8){unsigned tick=++fighter_diagnostic_frames[hud_player];if(tick==1||tick==60||tick==180){
  void*g=*(void**)(base+0x604a8d8-0x400000);
  logmsg("Fighter P%d tick=%u select=%g instance_select=%g custom=%g context=%g stock=%g connected=%g off=%g sprite=%g state=%g init=%g x=%g y=%g",hud_player,tick,array_value(g,4805,hud_player),number(var(s,5576)),number(var(s,1203)),array_value(g,1236,hud_player),array_value(g,4815,hud_player),array_value(g,4737,hud_player),array_value(g,4786,hud_player),array_value(g,4813,hud_player),number(var(s,5999)),number(var(s,3346)),*(float*)((char*)s+0xa0),*(float*)((char*)s+0xa4));
 }}
}
static RV* __cdecl swap_hook(void*s,void*o,RV*r,int n,RV**a){
 logmsg("Controller swap requested: %g -> %g",n>0?a[0]->real:-1,n>1?a[1]->real:-1);
 RV*result=swap_orig(s,o,r,n,a);void*g=*(void**)(base+0x604a8d8-0x400000);
 for(int id=2976;id<=2979;id+=3){RV*v=var(g,id);if(v->type!=2)continue;Array*ar=v->ptr;
  if(ar&&ar->length==1)for(int i=0;i<ar->dims[0].length;i++)logmsg("Controller array %d[%d]=%g",id,i,ar->dims[0].values[i].real);
 }
 return result;
}
static void show(const char *name,void *s) {int id=*(int*)((char*)s+0x78);for(int k=0;k<nseen;k++)if(seen[k]==id)return;if(nseen<128)seen[nseen++]=id;RV*p=var(s,4725);RV*r=var(s,5124);RV*i=var(s,3346);logmsg("%s self=%p id=%d object=%d player=%g/type%d ref=%g/type%d init=%g x=%g y=%g",name,s,id,*(int*)((char*)s+0x7c),p->real,p->type,r->real,r->type,i->real,*(float*)((char*)s+0xa0),*(float*)((char*)s+0xa4));}
static void __cdecl bg_hook(void*s,void*o){void*g=*(void**)(base+0x604a8d8-0x400000);css_prepare_slot(g,(int)number(var(s,4725)));bg_orig(s,o);show("bg",s);}
static void __cdecl cur_hook(void*s,void*o){
#ifdef SHOWCASE_CAPTURE
 if(showcase_css_frozen&&current_room(s)==3){setnum(s,3096,-1);return;}
#endif


 void*g=*(void**)(base+0x604a8d8-0x400000);
 int slot_player=(int)number(var(s,4725));css_prepare_slot(g,slot_player);
 int cpu=slot_player>=1&&slot_player<=8&&array_value(g,4727,slot_player)>0;
 if(cpu&&number(var(s,3096))>=1)setnum(s,3096,-1);
 int p6=var(s,4725)->real==6&&var(g,4208)->real==0&&var(g,4197)->real==0;
 if(p6){patch_word(0xc9738f,0x40180000,0x40220000);patch_word(0x4252749,0x40180000,0x40220000);}
 cur_orig(s,o);
 if(cpu&&number(var(s,3096))>=1)setnum(s,3096,-1);
 if(p6){patch_word(0xc9738f,0x40220000,0x40180000);patch_word(0x4252749,0x40220000,0x40180000);}
#ifdef MATCH_FIXTURE
 int slot=(int)var(s,4725)->real;
 if(slot>=1&&slot<=8&&!(fixture_slots&(1u<<slot))){
  fixture_slots|=1u<<slot;
  array_number(g,4737,slot,1);array_number(g,4727,slot,slot==1?0:1);array_number(g,4805,slot,2);
  array_number(g,4795,slot,0);array_number(g,4798,slot,1);array_number(g,4786,slot,0);setnum(s,3096,0);
  logmsg("Match fixture prepared P%d with native character 2",slot);
 }
#endif
 show("cursor",s);
}
static void __cdecl cur_create_hook(void*s,void*o){cur_create_orig(s,o);last_cursor=s;}
static void __cdecl bg_create_hook(void*s,void*o){bg_create_orig(s,o);last_bg=s;}
static void __cdecl local_hook(void*s,void*o){nseen=0;fixture_slots=0;native_spawn_count=0;memset(css_was_cpu,0,sizeof css_was_cpu);
 void*entry_global=*(void**)(base+0x604a8d8-0x400000);extend_slot_arrays(entry_global);
 patch_word(0x419c4f2,0x40140000,0x40220000);patch_word(0x419e062,0x40140000,0x40220000);
 local_orig(s,o);logmsg("Local room entered");
 void *global=*(void**)(base+0x604a8d8-0x400000);
 for(int p=5;p<=8;p++)array_number(global,4786,p,0);
 for(int device=0;device<16;device++){
  RV a={.real=device},connected={.type=5},description={.type=5};builtin(0x56b4a70,&connected,s,o,1,&a);
  if(number(&connected)!=0){builtin(0x56b4d60,&description,s,o,1,&a);logmsg("Input device %d: %s",device,description.type==1&&description.ptr?*(char**)description.ptr:"unknown");release(&description);}
 }
 if(var(global,4208)->real!=0||var(global,4197)->real!=0){logmsg("Extra slots refused: online state");return;}
 extend_slot_arrays(global);
 local_eight=1;
 for(unsigned i=0;i<sizeof css_slot_patches/sizeof css_slot_patches[0];i++)if(!patch_word(css_slot_patches[i].address,css_slot_patches[i].before,css_slot_patches[i].after))stop_release("Eight-player CPU selection signature mismatch.");
#ifdef EXPERIMENTAL_MATCH
 const uint32_t player_limits[]={0x40eb8fc,0x40ed900,0x40eea42,0x34836bc,0x34839de,0x34850ee,0xb5ac62,0xb6a50a,0xb6b667,0xb6bea6,0xb6c024};
 for(unsigned i=0;i<sizeof player_limits/sizeof player_limits[0];i++)if(!patch_word(player_limits[i],0x40100000,0x40200000))stop_release("Eight-player match classification signature mismatch.");
 const uint32_t animation_sentinels[]={0x735a30,0x735ae3,0xa64aa0};
 for(unsigned i=0;i<sizeof animation_sentinels/sizeof animation_sentinels[0];i++)if(!patch_word(animation_sentinels[i],0x40140000,0x40220000))stop_release("Workshop animation signature mismatch.");
 const uint32_t player_cutoffs[]={0xb6c8e9,0xb6e622,0xb70e4d,0x40e5732};
 for(unsigned i=0;i<sizeof player_cutoffs/sizeof player_cutoffs[0];i++)if(!patch_word(player_cutoffs[i],0x40140000,0x40220000))stop_release("Eight-player knockout signature mismatch.");
 patch_word(0x365d23d,0x40200000,0x40220000);
 patch_word(0x2c1e63d,0x40140000,0x40220000);
 patch_word(0x2c1eaa5,0x40140000,0x40220000);
 patch_word(0x4193c48,0x40140000,0x40220000);
 patch_word(0x4194a08,0x40140000,0x40220000);
 patch_word(0x3391f42,0x40140000,0x40220000);
 patch_word(0x33930e2,0x40140000,0x40220000);
 patch_word(0xc97602,0x40140000,0x40220000);
 patch_word(0xc9d782,0x40140000,0x40220000);
 patch_word(0xc9dbd2,0x40140000,0x40220000);
 patch_word(0xa6b562,0x40140000,0x40220000);
 if(!patch_word(0x4421ec2,0x40140000,0x40220000)||!patch_word(0x4423112,0x40140000,0x40220000))stop_release("Random selection signature mismatch.");
 patch_word(0x418fb77,0x40140000,0x40220000);
 patch_word(0x418ca42,0x40140000,0x40220000);
 patch_word(0x418e682,0x40140000,0x40220000);
 patch_word(0x2d33ce2,0x40100000,0x40200000);
 patch_word(0x2d33f92,0x40100000,0x40200000);
 patch_word(0x2d34182,0x40100000,0x40200000);
#ifdef MATCH_FIXTURE
 for(int p=1;p<=8;p++){
  array_number(global,4737,p,1);array_number(global,4727,p,p==1?0:1);array_number(global,4805,p,1);
  array_number(global,4795,p,0);array_number(global,4798,p,1);array_number(global,4786,p,0);
 }
 logmsg("One-human/seven-CPU baseline fixture enabled (native character 1).");
#endif
#endif
 if(!patch_word(0xcfea32,0x40140000,0x40220000)||!patch_word(0x4894232,0x40100000,0x40200000)||!patch_word(0x4894434,0x40100000,0x40200000)||!patch_word(0xcfc835,0x40100000,0x40200000)){logmsg("Input loop signature mismatch");return;}
 patch_word(0xdf41c2,0x40100000,0x40200000);
 patch_word(0xdf6782,0x40100000,0x40200000);
 for(int id_index=0;id_index<2;id_index++){
  int id=id_index?4735:1103;RV*v=var(global,id);Array*a=v->ptr;
  if((v->type&0xffffff)!=2||!a||a->length<1||!a->dims||a->dims[0].length<9){logmsg("Extra slots refused: array %d is not ready",id);return;}
 }
 Script create=(Script)(base+0x32f3f20-0x400000);
 for(int player=5;player<=8;player++){
  RV x={.real=120+238*(player-5),.type=0}, y={.real=240,.type=0},obj={.real=220,.type=0},out={.type=5};RV*args[]={&x,&y,&obj};last_cursor=NULL;
  create(s,o,&out,3,args);
  if(!last_cursor){logmsg("Failed to capture cursor %d",player);break;}
  setnum(last_cursor,4725,player);setnum(last_cursor,0xc18,player);logmsg("Created cursor P%d id=%g self=%p",player,out.real,last_cursor);
  x.real=14+238*(player-5);y.real=428;obj.real=219;last_bg=NULL;
  create(s,o,&out,3,args);
  if(!last_bg){logmsg("Failed to capture panel %d",player);break;}
  setnum(last_bg,4725,player);setnum(last_cursor,4835,out.real);
  logmsg("Created panel P%d id=%g self=%p",player,out.real,last_bg);
  x.real=20+238*(player-5);y.real=432;obj.real=224;last_button=NULL;create(s,o,&out,3,args);
  if(last_button)setnum(last_button,4725,player);
  x.real=162+238*(player-5);obj.real=225;last_team=NULL;create(s,o,&out,3,args);
  if(last_team)setnum(last_team,4725,player);
 }
}
static Event hook(uint32_t va,Event detour){unsigned char*p=(unsigned char*)(base+va-0x400000);const unsigned char expected[]={0x55,0x89,0xe5};(void)expected;if(p[0]!=0x55||p[1]!=0x89||p[2]!=0xe5||p[3]!=0x53||p[4]!=0x57||p[5]!=0x56){logmsg("Rejected hook %x",va);return NULL;}unsigned char*t=VirtualAlloc(NULL,32,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!t)return NULL;memcpy(t,p,6);t[6]=0xe9;*(int32_t*)(t+7)=(intptr_t)(p+6)-(intptr_t)(t+11);DWORD old;if(!VirtualProtect(p,6,PAGE_EXECUTE_READWRITE,&old))return NULL;p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)detour-(intptr_t)(p+5);p[5]=0x90;VirtualProtect(p,6,old,&old);FlushInstructionCache(GetCurrentProcess(),p,6);return(Event)t;}
static Builtin hook_text(uint32_t va,Builtin detour){
 unsigned char*p=(unsigned char*)(base+va-0x400000);const unsigned char expected[]={0x83,0xec,0x0c,0x56,0x57};
 if(memcmp(p,expected,5)){logmsg("Rejected text hook %x",va);return NULL;}
 unsigned char*t=VirtualAlloc(NULL,16,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!t)return NULL;
 memcpy(t,p,5);t[5]=0xe9;*(int32_t*)(t+6)=(intptr_t)(p+5)-(intptr_t)(t+10);
 DWORD old;if(!VirtualProtect(p,5,PAGE_EXECUTE_READWRITE,&old))return NULL;
 p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)detour-(intptr_t)(p+5);VirtualProtect(p,5,old,&old);FlushInstructionCache(GetCurrentProcess(),p,5);return(Builtin)t;
}
static Builtin hook_sprite(uint32_t va,Builtin detour,int bytes){
 unsigned char*p=(unsigned char*)(base+va-0x400000);
 if(p[0]!=0x83||p[1]!=0xec||p[2]!=8||p[3]!=0x8d){logmsg("Rejected sprite hook %x",va);return NULL;}
 unsigned char*t=VirtualAlloc(NULL,32,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!t)return NULL;
 memcpy(t,p,bytes);t[bytes]=0xe9;*(int32_t*)(t+bytes+1)=(intptr_t)(p+bytes)-(intptr_t)(t+bytes+5);
 DWORD old;if(!VirtualProtect(p,bytes,PAGE_EXECUTE_READWRITE,&old))return NULL;
 p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)detour-(intptr_t)(p+5);memset(p+5,0x90,bytes-5);VirtualProtect(p,bytes,old,&old);FlushInstructionCache(GetCurrentProcess(),p,bytes);return(Builtin)t;
}
static Builtin hook_surface(uint32_t va,Builtin detour){
 unsigned char*p=(unsigned char*)(base+va-0x400000);const unsigned char expected[]={0x56,0x8b,0x74,0x24,0x18};
 if(memcmp(p,expected,5)){logmsg("Rejected surface hook %x",va);return NULL;}
 unsigned char*t=VirtualAlloc(NULL,16,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!t)return NULL;
 memcpy(t,p,5);t[5]=0xe9;*(int32_t*)(t+6)=(intptr_t)(p+5)-(intptr_t)(t+10);
 DWORD old;if(!VirtualProtect(p,5,PAGE_EXECUTE_READWRITE,&old))return NULL;
 p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)detour-(intptr_t)(p+5);VirtualProtect(p,5,old,&old);FlushInstructionCache(GetCurrentProcess(),p,5);return(Builtin)t;
}
static void*hook_fast_sprite(void){
 unsigned char*p=(unsigned char*)(base+0x5586db0-0x400000);const unsigned char expected[]={0x56,0x8b,0x74,0x24,0x10};
 if(memcmp(p,expected,5)){logmsg("Rejected native sprite hook");return NULL;}
 unsigned char*t=VirtualAlloc(NULL,16,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);if(!t)return NULL;
 memcpy(t,p,5);t[5]=0xe9;*(int32_t*)(t+6)=(intptr_t)(p+5)-(intptr_t)(t+10);
 DWORD old;if(!VirtualProtect(p,5,PAGE_EXECUTE_READWRITE,&old))return NULL;
 p[0]=0xe9;*(int32_t*)(p+1)=(intptr_t)fast_sprite_hook-(intptr_t)(p+5);VirtualProtect(p,5,old,&old);FlushInstructionCache(GetCurrentProcess(),p,5);return t;
}
#ifdef AUTOMATED_MATCH_TEST
#include "match_harness.h"
#endif
#ifdef EXPERIMENTAL_MATCH
#include "eight_player_hud.h"
#include "eight_player_results.h"
#endif
static void setup(void){static int done;if(done)return;done=1;base=(uintptr_t)GetModuleHandleA(NULL);logmsg("Native probe loaded base=%p",(void*)base);
  if(!supported_executable())stop_release("Unsupported game executable. This build requires the tested Rivals of Aether 2.1.9.1 executable. No mod hooks were installed.");
  if(!install_test_error_handler()){logmsg("Test error-handler signature mismatch; no slot hooks installed");return;}
  if(!patch_word(0x331bf22,0x40140000,0x40200000)||!patch_word(0x331cd34,0x40140000,0x40200000)){logmsg("Workshop initialization signature mismatch");return;}
  unsigned char*bound=(unsigned char*)(base+0xc8f618-0x400000);DWORD old;
  if(*(uint32_t*)bound!=0x40200000||!VirtualProtect(bound,4,PAGE_EXECUTE_READWRITE,&old)){logmsg("Input initialization signature mismatch; no slot hooks installed");return;}
  *(uint32_t*)bound=0x40220000;VirtualProtect(bound,4,old,&old);FlushInstructionCache(GetCurrentProcess(),bound,4);

  bg_create_orig=hook(0x41a8c20,bg_create_hook);cur_create_orig=hook(CREATE_ADDR,cur_create_hook);bg_orig=hook(BG_ADDR,bg_hook);cur_orig=hook(CUR_ADDR,cur_hook);local_orig=hook(LOCAL_ADDR,local_hook);
  button_create_orig=hook(0x42a9320,button_create_hook);team_create_orig=hook(0x42b1a60,team_create_hook);bg_draw_orig=hook(0x41b4da0,bg_draw_hook);
  swap_orig=(Script)hook(0x3188f20,(Event)swap_hook);
  controls_orig=(Script)hook(0xcfdb30,(Event)controls_hook);
  ugc_set_orig=(Script)hook(0x3334d80,(Event)ugc_set_hook);read_item_name_orig=(Script)hook(0x3362d60,(Event)read_item_name_hook);
  puck_position_orig=(Script)hook(0x3631930,(Event)puck_position_hook);
  css_child_create_orig=(Script)hook(0x32f3f20,(Event)css_child_create_hook);
  color_orig=(Script)hook(0x2192f10,(Event)color_hook);dark_orig=(Script)hook(0x21ab730,(Event)dark_hook);
  text_orig_0=hook_text(0x5581cd0,text_hook_0);text_orig_1=hook_text(0x5581d70,text_hook_1);
  text_orig_2=hook_text(0x5581e50,text_hook_2);text_orig_3=hook_text(0x5581f00,text_hook_3);
  text_orig_4=hook_text(0x5582200,text_hook_4);text_orig_5=hook_text(0x55822d0,text_hook_5);
  text_orig_6=hook_text(0x5582000,text_hook_6);text_orig_7=hook_text(0x55820e0,text_hook_7);
  sprite_orig_0=hook_sprite(0x5580e90,sprite_hook_0,6);sprite_orig_1=hook_sprite(0x5580ef0,sprite_hook_1,7);
  sprite_orig_2=hook_sprite(0x55810e0,sprite_hook_2,7);sprite_orig_3=hook_sprite(0x55811e0,sprite_hook_3,7);
  sprite_orig_4=hook_sprite(0x5580fa0,sprite_hook_4,7);
  fast_sprite_orig=(FastSprite)hook_fast_sprite();
  sprite_orig_5=hook_sprite(0x55813f0,sprite_hook_5,6);sprite_orig_6=hook_sprite(0x5581480,sprite_hook_6,6);
  surface_orig_0=hook_surface(0x55816b0,surface_hook_0);surface_orig_1=hook_surface(0x5581710,surface_hook_1);
  surface_orig_3=hook_surface(0x5581960,surface_hook_3);surface_orig_4=hook_surface(0x55817b0,surface_hook_4);

  unsigned char*sp=(unsigned char*)(base+0x55818b0-0x400000);
  const unsigned char signature[]={0x56,0xe8,0xfa,0x4d,0x01,0};
  if(!memcmp(sp,signature,6)){
   unsigned char*t=VirtualAlloc(NULL,32,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
   if(t&&VirtualProtect(sp,6,PAGE_EXECUTE_READWRITE,&old)){
    memcpy(t,sp,6);*(int32_t*)(t+2)=(intptr_t)(sp+6+*(int32_t*)(sp+2))-(intptr_t)(t+6);
    t[6]=0xe9;*(int32_t*)(t+7)=(intptr_t)(sp+6)-(intptr_t)(t+11);
    surface_part_orig=(Builtin)t;sp[0]=0xe9;*(int32_t*)(sp+1)=(intptr_t)surface_part_hook-(intptr_t)(sp+5);sp[5]=0x90;
    VirtualProtect(sp,6,old,&old);FlushInstructionCache(GetCurrentProcess(),sp,6);
    preview_orig=hook(0x370abb0,preview_hook);
   }
  }else logmsg("Preview surface signature mismatch");
#ifdef EXPERIMENTAL_MATCH
  fighter_create_orig=hook(0x3655e60,fighter_create_hook);fighter_update_orig=hook(0x365c530,fighter_update_hook);hud_create_orig=hook(0x40cf690,hud_create_hook);
  spawn_num_orig=(Script)hook(0xc5b9d0,(Event)spawn_num_hook);
  player_init_orig=(Script)hook(0xa7ffa0,(Event)player_init_hook);
  death_orig=(Script)hook(0xb454b0,(Event)death_hook);
  stage_destination_orig=(Script)hook(0x2b43910,(Event)stage_destination_hook);
  hud_update_orig=hook(0x40e2100,hud_update_hook);
  collision_orig=(Script)hook(0x2085ed0,(Event)collision_hook);
  eight_hud_orig=hook(0x41200f0,eight_hud_draw);
  results_create_orig=hook(0x468c5d0,results_create);results_draw_orig=hook(0x46af100,results_draw);results_step_orig=hook(0x46a93d0,results_step);
  results_alarm0_orig=hook(0x46a92d0,results_alarm0);results_alarm1_orig=hook(0x46a8aa0,results_alarm1);results_alarm2_orig=hook(0x46a82c0,results_alarm2);
  results_other5_orig=hook(0x46aea10,results_other5);results_other72_orig=hook(0x46ae8f0,results_other72);
#endif
#ifdef AUTOMATED_MATCH_TEST
  harness_title_orig=hook(0x41a2a10,harness_title);harness_css_orig=hook(0x41936c0,harness_css);
  harness_global_orig=hook(0x4893560,harness_global);
#ifdef REAL_MENU_FLOW
  harness_stage_orig=hook(0x43fae50,harness_stage);harness_stagebox_create_orig=hook(0x441d270,harness_stagebox_create);
  harness_cursor_movement_orig=(Script)hook(0xf18320,(Event)harness_cursor_movement);
#endif
#ifdef HUD_PREVIEW
  harness_advance_orig=(Script)hook(0x2b63ac0,(Event)harness_advance);
#endif
#endif
}
__declspec(dllexport) double __cdecl catch_error_is_available(void){return 1.0;}
__declspec(dllexport) double __cdecl catch_error_preinit_raw(const char*s){typedef double(__cdecl*F)(const char*);F f=(F)GetProcAddress(original,"catch_error_preinit_raw");double r=f?f(s):0;setup();return r;}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID reserved){if(reason==DLL_PROCESS_ATTACH){DisableThreadLibraryCalls(h);GetModuleFileNameA(h,folder,sizeof folder);char*slash=strrchr(folder,'\\');if(slash)*slash=0;char p[MAX_PATH];snprintf(p,sizeof p,"%s\\catch_error_original.dll",folder);original=LoadLibraryA(p);}return TRUE;}
