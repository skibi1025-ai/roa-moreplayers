
static Event harness_title_orig,harness_css_orig;
static int harness_title_frames,harness_css_frames;
static Event harness_global_orig;
static unsigned harness_updates[9],harness_ticks;
static int harness_match_started,harness_results_seen;
static unsigned harness_result_frames;
static int harness_stage_room=11;
static int harness_roster_round;
static int harness_fixture_started;
#ifdef REAL_MENU_FLOW
static Event harness_stage_orig;
static unsigned harness_stage_frames;
static Script harness_cursor_movement_orig;
static int harness_stage_target;
static double harness_stage_x,harness_stage_y;
static RV* __cdecl harness_cursor_movement(void*s,void*o,RV*r,int n,RV**a){
 if(current_room(s)==4&&harness_stage_target&&*(int*)((char*)s+0x7c)==279){*(float*)((char*)s+0xa0)=(float)harness_stage_x;*(float*)((char*)s+0xa4)=(float)harness_stage_y;release(r);*r=(RV){.real=0};return r;}
 return harness_cursor_movement_orig(s,o,r,n,a);
}
static double harness_instance_value(void*s,void*o,RV instance,const char*name){
 RV args[2]={instance,{.type=5}},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&args[1],name);builtin(0x5568090,&out,s,o,2,args);release(&args[1]);double value=number(&out);release(&out);return value;
}
static void __cdecl harness_stage(void*s,void*o){
 void*g=*(void**)(base+0x604a8d8-0x400000);unsigned frame=++harness_stage_frames;
 if(frame==180){
  for(int i=0;i<30;i++){
   RV args[2]={{.real=282},{.real=i}},instance={.type=5};builtin(0x557bc00,&instance,s,o,2,args);
   if(number(&instance)<0)break;double stage=harness_instance_value(s,o,instance,"stage");
   if(i<4)logmsg("HARNESS stage box %d stage=%g",i,stage);
   if(stage==1){double x=harness_instance_value(s,o,instance,"x"),y=harness_instance_value(s,o,instance,"y");harness_stage_x=x+12;harness_stage_y=y+12;harness_stage_target=1;*(float*)((char*)s+0xa0)=(float)harness_stage_x;*(float*)((char*)s+0xa4)=(float)harness_stage_y;logmsg("HARNESS stage cursor targeting stage=%g position=%g,%g",stage,harness_stage_x,harness_stage_y);break;}
  }
 }
 int confirm=frame>=240&&frame%60==0;if(confirm)array_number(g,0x11c5,6,1);
 harness_stage_orig(s,o);
 if(number(var(s,5922))!=0)for(int p=1;p<=8;p++)if(array_value(g,4737,p)!=0)array_number(g,4727,p,1);
 if(confirm){array_number(g,0x11c5,6,0);logmsg("HARNESS stage confirm picked=%g",number(var(s,5922)));}
 if(frame>=900){logmsg("HARNESS stage selection transition timed out");ExitProcess(49);}
}
#endif
#ifdef WORKSHOP_MATCH_TEST
static void harness_load_workshop(void*s,void*o){
 void*g=*(void**)(base+0x604a8d8-0x400000);RV*v=var(g,0xb49),size={.type=5};builtin(0x555d6d0,&size,s,o,1,v);int count=(int)number(&size),loaded=0;
 logmsg("HARNESS Workshop catalog rows=%d",count);
 for(int row=0;row<count;row++){
  RV lookup[2]={*v,{.real=row}},item={.type=5};builtin(0x555c990,&item,s,o,2,lookup);
  if((item.type&0xffffff)!=2||!item.ptr){release(&item);continue;}Array*data=item.ptr;
  Dimension*record=data->length==1?data->dims:NULL;
  if(!record||record->length<=10||!record->values){release(&item);continue;}
  int p=0;for(int column=0;column<record->length;column++){RV*value=record->values+column;if((value->type&0xffffff)==1&&value->ptr){const char*text=*(const char**)value->ptr;if(text&&strstr(text,"3799054154"))p=6;if(text&&strstr(text,"3824607103"))p=8;}}
  if(p){
   RV player={.real=p},first=record->values[3],second=record->values[10],out={.type=5};
   if((first.type&0xffffff)==1&&first.ptr)((int*)first.ptr)[1]++;
   if((second.type&0xffffff)==1&&second.ptr)((int*)second.ptr)[1]++;
   RV*args[]={&player,&first,&second};((Script)(base+0x3334d80-0x400000))(s,o,&out,3,args);
   logmsg("HARNESS loaded Workshop catalog row=%d P%d context=%g loading=%g",row,p,array_value(g,1236,p),number(var(g,0xb4a)));loaded++;
   release(&out);release(&first);release(&second);
  }
  release(&item);if(loaded==2)return;
 }
 logmsg("HARNESS Workshop character missing from catalog");ExitProcess(48);
}
#endif
#ifdef HUD_PREVIEW
static Script harness_advance_orig;
static RV*__cdecl harness_advance(void*s,void*o,RV*r,int n,RV**a){
 if(harness_match_started&&harness_ticks>=420){release(r);*r=(RV){.real=0};return r;}
 return harness_advance_orig(s,o,r,n,a);
}
#endif
static void harness_fighter(void*s,void*o){
 int p=(int)number(var(s,4725));if(p<1||p>8)return;
 if(harness_updates[p]==0){
  void*g=*(void**)(base+0x604a8d8-0x400000);
  logmsg("HARNESS initial P%d connected=%g stock=%g off=%g select=%g slot=%g game_slot=%g",p,array_value(g,4737,p),array_value(g,4815,p),array_value(g,4786,p),array_value(g,4805,p),array_value(g,4808,p),array_value(g,4754,p));
 }
 if(++harness_updates[p]==300){
  void*g=*(void**)(base+0x604a8d8-0x400000);
  logmsg("HARNESS P%d updates=300 stock=%g damage=%g x=%g y=%g",p,array_value(g,4815,p),array_value(g,4745,p),*(float*)((char*)s+0xa0),*(float*)((char*)s+0xa4));
 }
#ifndef HUD_PREVIEW
 unsigned knockout_at=300;
#ifdef WIN_SCREEN_PREVIEW
 knockout_at+=p*20;
#endif
 if(harness_updates[p]==knockout_at){
  void*g=*(void**)(base+0x604a8d8-0x400000);
  if(
#ifdef SPARSE_MATCH_TEST
#ifdef WORKSHOP_MATCH_TEST
    p==6
#else
    p==8
#endif
#else
    p>1
#endif
  &&array_value(g,4815,p)>0){
#ifdef WORKSHOP_MATCH_TEST
   array_number(g,4805,7,0);setnum(s,3566,7);logmsg("HARNESS testing invalid unselected attacker P7 during Workshop knockout");
#endif
   array_number(g,4815,p,1);*(float*)((char*)s+0xa4)=10000;logmsg("HARNESS requesting final-stock KO for P%d",p);}
 }
#endif
}
static void __cdecl harness_global(void*s,void*o){
 harness_global_orig(s,o);
 int room=current_room(s);
#ifdef REAL_MENU_FLOW
 if(room>=10&&room<=57&&!harness_match_started){harness_stage_room=room;logmsg("HARNESS normal menu flow reached gameplay room=%d",room);}
#endif
#ifdef HUD_PREVIEW
 if(room==11){void*g=*(void**)(base+0x604a8d8-0x400000);setnum(g,2914,480);setnum(g,2915,480);setnum(g,2917,0);}
#endif
 if(room==harness_stage_room)harness_match_started=1;
 if(!harness_match_started)return;
 harness_ticks++;
#ifdef SHOWCASE_CAPTURE
 if(harness_ticks==200){RV path={.type=5},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&path,"showcase-match.png");builtin(0x5582930,&out,s,o,1,&path);release(&path);}
#endif
#ifdef WORKSHOP_MATCH_TEST
 if(harness_ticks==200){RV path={.type=5},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&path,"workshop-match.png");builtin(0x5582930,&out,s,o,1,&path);release(&path);}
#endif
 if(room==8&&!harness_results_seen){harness_results_seen=1;logmsg("HARNESS reached local results screen");}
 if(room==8&&++harness_result_frames==100){
  RV path={.type=5},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&path,"results-preview.png");builtin(0x5582930,&out,s,o,1,&path);release(&path);
 }
#ifndef HUD_PREVIEW
 if(harness_ticks==780&&room==8){
  void*g=*(void**)(base+0x604a8d8-0x400000);array_number(g,4623,6,1);logmsg("HARNESS requesting return from results using P6 Start");
 }
 if(harness_ticks==781){void*g=*(void**)(base+0x604a8d8-0x400000);array_number(g,4623,6,0);}
 if(harness_ticks==1050){
  int all=1;for(int p=1;p<=8;p++){logmsg("HARNESS P%d total updates=%u",p,harness_updates[p]);
#ifdef SPARSE_MATCH_TEST
   if(p!=6&&p!=8)continue;
#endif
   if(harness_updates[p]<300)all=0;}
  int passed=all&&harness_results_seen&&room==3;
  logmsg("HARNESS finished round=%d room=%d all-eight-updated=%d results=%d returned-css=%d",harness_roster_round,room,all,harness_results_seen,room==3);
#ifdef ROSTER_TEST
  if(passed&&harness_roster_round<5){
   harness_roster_round++;harness_ticks=0;harness_match_started=0;harness_results_seen=0;harness_result_frames=0;harness_css_frames=0;harness_fixture_started=0;memset(harness_updates,0,sizeof harness_updates);return;
  }
#endif
  ExitProcess(passed?0:43);
 }
#endif
}
static void harness_room(void*s,void*o,int index){RV a={.real=index},r={.type=5};builtin(0x557e310,&r,s,o,1,&a);}
static void __cdecl harness_title(void*s,void*o){
 harness_title_orig(s,o);
 void*g=*(void**)(base+0x604a8d8-0x400000);
 if(++harness_title_frames==180){setnum(g,0x486,3);logmsg("HARNESS entering local CSS, loading=%g",var(g,2171)->real);harness_room(s,o,3);}
}
static void __cdecl harness_css(void*s,void*o){
 harness_css_orig(s,o);
 ++harness_css_frames;
#ifdef WORKSHOP_MATCH_TEST
 if(harness_css_frames==120)harness_load_workshop(s,o);
 if(number(var(*(void**)(base+0x604a8d8-0x400000),0xb4a))!=0)return;
#endif
#ifdef SHOWCASE_CAPTURE
 if(harness_fixture_started&&!harness_match_started){static unsigned frames;frames++;if(frames==60){RV path={.type=5},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&path,"showcase-css.png");builtin(0x5582930,&out,s,o,1,&path);release(&path);logmsg("HARNESS showcase CSS captured");}if(frames==90){showcase_css_frozen=0;harness_room(s,o,11);}return;}
#endif
#ifdef CSS_LABEL_PREVIEW
 if(harness_fixture_started){static unsigned frames;frames++;void*g=*(void**)(base+0x604a8d8-0x400000);setnum(g,3464,frames<=80?(frames-1)/10+1:8);if(frames==90){RV path={.type=5},out={.type=5};((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&path,"card-label-preview.png");builtin(0x5582930,&out,s,o,1,&path);release(&path);logmsg("HARNESS card labels rendered mask=%x",card_label_seen);}if(frames==91)ExitProcess(card_label_seen==0x1fe?0:51);return;}
#endif
 if(harness_css_frames>=180&&!harness_fixture_started){
  harness_fixture_started=1;
  void*g=*(void**)(base+0x604a8d8-0x400000);
  for(int p=1;p<=8;p++){
   array_number(g,4737,p,
#ifdef SPARSE_MATCH_TEST
    p==6||p==8
#else
    1
#endif
   );array_number(g,4727,p,1);array_number(g,4805,p,
#ifdef HUD_PREVIEW
    p+1
#elif defined(WIN_SCREEN_PREVIEW)
    p+1
#elif defined(ROSTER_TEST)
    p<=4?p+1:(harness_roster_round==5?(p==5?20:p-4):2+(harness_roster_round*4+p-5)%18)
#elif defined(WORKSHOP_MATCH_TEST)
    p==6||p==8?21:2
#else
    2
#endif
   );
   array_number(g,4795,p,0);array_number(g,4798,p,1);array_number(g,4786,p,0);
   array_number(g,4815,p,3);array_number(g,2851,p,560+32*(p-1));array_number(g,2852,p,400);
   array_number(g,4797,p,0);array_number(g,4750,p,0);
#ifdef SHOWCASE_CAPTURE
   array_number(g,4727,p,p>2?1:0);array_number(g,193,p,p%9+1);
#endif
#ifdef CSS_LABEL_PREVIEW
   array_number(g,4727,p,0);
   array_number(g,4798,p,0);
#ifndef WORKSHOP_MATCH_TEST
   array_number(g,4805,p,p+1);
#endif
#endif
   logmsg("HARNESS roster round=%d P%d character=%g",harness_roster_round,p,array_value(g,4805,p));
  }
  setnum(g,2909,0);setnum(g,2903,0);setnum(g,2914,number(var(g,2915)));
#ifdef SHOWCASE_CAPTURE
  showcase_css_frozen=1;return;
#endif
#ifdef CSS_LABEL_PREVIEW
  return;
#endif
#ifdef SPARSE_MATCH_TEST
  for(int p=6;p<=8;p+=2){
   array_number(g,4727,p,0);array_number(g,0x11c5,p,1);array_number(g,0x11ba,p,1);
   RV result={.type=5};((Script)(base+0xc9c690-0x400000))(s,o,&result,0,NULL);release(&result);
   logmsg("HARNESS stage input P%d confirm=%g left=%g",p,number(var(s,0xf0d)),number(var(s,0xe2b)));
   if(number(var(s,0xf0d))!=1||number(var(s,0xe2b))!=1)ExitProcess(46);
   array_number(g,0x11c5,p,0);array_number(g,0x11ba,p,0);array_number(g,4727,p,1);
  }
  ((Event)(base+0x418f560-0x400000))(s,o);
  logmsg("HARNESS sparse readiness ready=%g players_ready=%g",number(var(s,0x13df)),number(var(s,0x12ea)));
  if(number(var(s,0x13df))!=1||number(var(s,0x12ea))<2)ExitProcess(45);
  setnum(g,2914,number(var(g,2915)));
#endif
#ifdef WORKSHOP_MATCH_TEST
  array_number(g,4815,8,0);array_number(g,4786,8,1);logmsg("HARNESS seeding stale eliminated/disconnected P8 state before new match");
#endif
#ifdef REAL_MENU_FLOW
  harness_stage_frames=0;harness_stage_target=0;
  RV done={.type=5};((Script)(base+0x33919a0-0x400000))(s,o,&done,0,NULL);release(&done);
  ((Event)(base+0x418f560-0x400000))(s,o);
  for(int p=1;p<=8;p++)if(array_value(g,4737,p)!=0)array_number(g,4727,p,0);
  logmsg("HARNESS entering actual stage selection after LocalDoneCharSelect and readiness");harness_room(s,o,4);return;
#endif
#ifdef STAGE_SPAWN_TEST
  setnum(g,0x49f,88);array_number(g,0x4d4,0,6);harness_stage_room=57;
  logmsg("HARNESS Workshop stage fixture stage=88 custom=6 room=%d",harness_stage_room);
#endif
  logmsg("HARNESS starting native CPU fighter fixture on room %d",harness_stage_room);harness_room(s,o,harness_stage_room);
 }
}
