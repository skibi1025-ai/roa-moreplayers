static Event eight_hud_orig;

static int hud_split=0,hud_refresh,hud_capture_delay;
static unsigned hud_captured;
static int hud_custom_sprite(void*g,int p){
 int custom=(int)array_value(g,0x4d4,p);
 RV*v=var(g,0x4bf);
 if(custom<0||(v->type&0xffffff)!=2||!v->ptr)return -1;
 Array*a=v->ptr;
 if(custom>=a->length||!a->dims||a->dims[custom].length<=4||!a->dims[custom].values)return -1;
 RV*r=a->dims[custom].values+4;
 int type=r->type&0xffffff;
 if(type!=0&&type!=7&&type!=10)return -1;
 int result=(int)number(r);return result>0?result:-1;
}
static void hud_value(uint32_t fn,void*s,void*o,double value){RV a={.real=value},r={.type=5};builtin(fn,&r,s,o,1,&a);}
static void hud_rect(void*s,void*o,double x,double y,double w,double h,unsigned color){
 hud_value(0x5580db0,s,o,color);RV a[5]={{.real=x},{.real=y},{.real=x+w},{.real=y+h},{.real=0}},r={.type=5};builtin(0x5580a20,&r,s,o,5,a);
}
static void hud_triangle(void*s,void*o,double x1,double y1,double x2,double y2,double x3,double y3,unsigned color){
 hud_value(0x5580db0,s,o,color);RV a[7]={{.real=x1},{.real=y1},{.real=x2},{.real=y2},{.real=x3},{.real=y3},{.real=0}},r={.type=5};builtin(0x55823e0,&r,s,o,7,a);
}
static void hud_art(void*s,void*o,double x,double y,double w,unsigned color,int p){

 hud_value(0x5580d70,s,o,.32);
 hud_triangle(s,o,x+4,y+75,x+20,y+30,x+67,y+75,color);
 hud_triangle(s,o,x+18,y+75,x+47,y+43,x+77,y+75,color);
 hud_value(0x5580d70,s,o,.8);
 double notch=8+2*((p-1)%4);
 hud_triangle(s,o,x+3,y+78,x+15,y+66,x+w-3,y+66,color);
 hud_triangle(s,o,x+3,y+78,x+w-3,y+66,x+w-notch,y+78,color);
 hud_value(0x5580d70,s,o,1);
 hud_triangle(s,o,x+w-notch-3,y+79,x+w-6,y+76,x+w-notch-16,y+79,0xffffff);

 for(int i=0;i<1+(p-1)%3;i++){
  double ax=x+3+i*5,ay=y+60-i*4;
  hud_triangle(s,o,ax,ay,ax+6,ay-12,ax+3,ay+2,color);
 }
}
static void hud_text(void*s,void*o,double x,double y,const char*str,double scale,int font,unsigned color){
 hud_value(0x5580dd0,s,o,font);hud_value(0x5580db0,s,o,color);
 RV a[6]={{.real=x},{.real=y},{.type=5},{.real=scale},{.real=scale},{.real=0}},r={.type=5};
 ((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&a[2],str);
 builtin(0x5582200,&r,s,o,6,a);release(&a[2]);
}
static void __cdecl eight_hud_draw(void*s,void*o){
 if(!local_eight){eight_hud_orig(s,o);return;}
 void*g=*(void**)(base+0x604a8d8-0x400000);
 if(number(var(g,4208))!=0||number(var(g,4505))!=0){eight_hud_orig(s,o);return;}
 int extra=0;for(int p=5;p<=8;p++)if(array_value(g,4737,p)!=0)extra=1;
 if(!extra){eight_hud_orig(s,o);return;}

 hidden_hud_surface=(int)number(var(s,0xcae));
 eight_hud_orig(s,o);
 hidden_hud_surface=-1;
 if(hud_refresh++%30==0){char path[MAX_PATH],mode[32]={0};snprintf(path,sizeof path,"%s\\hud-layout.txt",folder);FILE*f=fopen(path,"r");if(f){fgets(mode,sizeof mode,f);fclose(f);hud_split=!strncmp(mode,"split",5);if(strstr(mode,"capture")&&!(hud_captured&(1u<<hud_split))&&!hud_capture_delay)hud_capture_delay=10;}}
 RV saved[6],out={.type=5};uint32_t getters[]={0x55804c0,0x5580450,0x5580410,0x55804e0,0x5580600};
 for(int i=0;i<5;i++){saved[i]=(RV){.type=5};builtin(getters[i],&saved[i],s,o,0,NULL);}
 RV world={.real=2},mat={.type=5};saved[5]=(RV){.type=5};builtin(0x56775f0,&saved[5],s,o,1,&world);
 RV b[9]={{0},{0},{0},{0},{0},{0},{.real=1},{.real=1},{.real=1}};builtin(0x5676a10,&mat,s,o,9,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 RV gw={.type=5},gh={.type=5};builtin(0x5607a50,&gw,s,o,0,NULL);builtin(0x5607a10,&gh,s,o,0,NULL);
 double width=number(&gw),height=number(&gh);if(width<1)width=960;if(height<1)height=540;
 hud_value(0x5580d70,s,o,1);hud_value(0x5580df0,s,o,1);hud_value(0x5580e50,s,o,0);
 int label_font=(int)number(var(g,2268)),damage_font=(int)number(var(g,2267));
 int columns=hud_split?4:8;double gap=6,margin=8,tile=(width-2*margin-gap*(columns-1))/columns;
 static const unsigned colors[8]={0x5050f0,0xff9b50,0x80dc60,0xc980ef,0x2099ff,0xf080b0,0x55e6e6,0xe6e6e6};
 for(int p=1;p<=8;p++){
  if(array_value(g,4737,p)==0)continue;
  int column=hud_split?(p-1)%4:p-1;
  double art_width=tile>160?160:tile;
  double x=margin+column*(tile+gap)+(tile-art_width)*.5,y=hud_split&&p<=4?56:height-84;
  double cx=x+art_width*.5;
  unsigned color=colors[p-1];int stocks=(int)array_value(g,4815,p);
  hud_art(s,o,x,y,art_width,color,p);
  char label[16],damage[32],stock[16];snprintf(label,sizeof label,"P%d",p);snprintf(damage,sizeof damage,"%.0f%%",array_value(g,4745,p));snprintf(stock,sizeof stock,"%d",stocks<0?0:stocks);
  int select=(int)array_value(g,4805,p);
  RV sprite[9]={{.real=4034},{.real=select-1},{.real=x+12},{.real=y+43},{.real=1},{.real=1},{.real=0},{.real=0xffffff},{.real=stocks>0?1:.4}};
  if(hud_custom_active[p]){
   int custom=hud_custom_sprite(g,p);
   if(custom>=0){sprite[0].real=custom;sprite[1].real=0;}
   else {sprite[0].real=4034;sprite[1].real=0;}
  }
  builtin(0x5580ef0,&out,s,o,9,sprite);
  hud_value(0x5580df0,s,o,0);hud_text(s,o,x+5,y+6,label,1,label_font,color);
  hud_value(0x5580df0,s,o,2);hud_text(s,o,x+art_width-5,y+17,stocks>0?damage:"OUT",stocks>0?1:1.5,stocks>0?damage_font:label_font,stocks>0?0xffffff:0x999999);
  hud_value(0x5580df0,s,o,1);hud_text(s,o,x+art_width-21,y+57,stock,1.25,label_font,0xffffff);
 }
 uint32_t setters[]={0x5580dd0,0x5580db0,0x5580d70,0x5580df0,0x5580e50};for(int i=0;i<5;i++)builtin(setters[i],&out,s,o,1,&saved[i]);
 set[1]=saved[5];builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&saved[5]);
 if(hud_capture_delay>0
#ifdef HUD_PREVIEW
    &&harness_ticks>=450
#endif
    &&!--hud_capture_delay){
  char path[MAX_PATH];snprintf(path,sizeof path,"%s\\hud-%s.png",folder,hud_split?"split":"bottom");
  RV file={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&file,path);
  builtin(0x5582930,&out,s,o,1,&file);release(&file);hud_captured|=1u<<hud_split;logmsg("HUD preview exported: %s",path);
 }
}
