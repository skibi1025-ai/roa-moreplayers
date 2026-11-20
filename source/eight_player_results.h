static Event results_create_orig,results_draw_orig,results_step_orig,results_alarm0_orig,results_alarm1_orig,results_alarm2_orig,results_other5_orig,results_other72_orig;
static int results_custom,results_frames,results_winner,results_music_loop,results_stats;
static void results_music_call(uint32_t address,void*s,void*o,int track){RV result={.type=5},value={.real=track};RV*args[]={&value};((Script)(base+address-0x400000))(s,o,&result,track<0?0:1,track<0?NULL:args);release(&result);}
static int results_music_started,results_music_track,results_confirmed,results_ready_timer;
static double results_play_audio(void*s,void*o,int track,int loop){
 void*g=*(void**)(base+0x604a8d8-0x400000);RV args[3]={{.real=track},{.real=100},{.real=loop}},handle={.type=5};builtin(0x56ad2c0,&handle,s,o,3,args);
 RV out={.type=5},zero={0};RV*gain[]={&handle,var(g,4114),&zero};((Script)(base+0x3069020-0x400000))(s,o,&out,3,gain);release(&out);double id=number(&handle);release(&handle);return id;
}
static void results_music_update(void*s,void*o){
 void*g=*(void**)(base+0x604a8d8-0x400000);RV group={.real=2},loaded={.type=5};builtin(0x56ade00,&loaded,s,o,1,&group);int ready=number(&loaded)!=0;release(&loaded);if(!ready)return;
 if(!results_music_started){setnum(g,492,results_play_audio(s,o,results_music_track,0));results_music_started=1;logmsg("Results victory music track=%d volume=%g",results_music_track,number(var(g,4114)));return;}
 if(results_music_loop)return;RV playing={.type=5};builtin(0x56ad140,&playing,s,o,1,var(g,492));
 if(number(&playing)==0){setnum(g,488,results_play_audio(s,o,1039,1));results_music_loop=1;logmsg("Results music loop started track=1039");}release(&playing);
}
static struct {int active,head,frame,portrait,portrait_frame,stocks,kills,place,eliminated,row_sprite,row_frame,cpu,ready,deaths,suicides,parries,combo;double damage,given,taken,accuracy;char name[64];} results_players[9];
static RV* results_custom_field(void*g,int p,int field){
 RV*v=var(g,0x4bf);int context=(int)array_value(g,1236,p);if((v->type&0xffffff)!=2||!v->ptr)return NULL;Array*a=v->ptr;
 if(context<0||context>=a->length||!a->dims||field<0||field>=a->dims[context].length||!a->dims[context].values)return NULL;return a->dims[context].values+field;
}
static int results_owned[9][2];
static struct {char path[MAX_PATH];int sprite;} results_image_cache[9];
static int results_image_count;
static uint64_t results_image_bytes;
static void results_release_images(void*s,void*o){
 for(int i=0;i<results_image_count;i++)if(results_image_cache[i].sprite>=0){RV image={.real=results_image_cache[i].sprite},out={.type=5};builtin(0x5687b50,&out,s,o,1,&image);release(&out);}
 results_image_count=0;results_image_bytes=0;
 for(int p=1;p<=8;p++)for(int k=0;k<2;k++)results_owned[p][k]=-1;
}
static uint64_t results_png_bytes(const char*path){
 unsigned char h[24];FILE*f=fopen(path,"rb");if(!f)return UINT64_MAX;size_t n=fread(h,1,sizeof h,f);fclose(f);
 static const unsigned char signature[]={137,80,78,71,13,10,26,10};if(n!=sizeof h||memcmp(h,signature,8)||memcmp(h+12,"IHDR",4))return UINT64_MAX;
 uint64_t w=((uint32_t)h[16]<<24)|((uint32_t)h[17]<<16)|((uint32_t)h[18]<<8)|h[19];
 uint64_t height=((uint32_t)h[20]<<24)|((uint32_t)h[21]<<16)|((uint32_t)h[22]<<8)|h[23];
 if(!w||!height||w>16384||height>16384)return UINT64_MAX;
 return w*height*4;
}
static int results_load_image(void*g,void*s,void*o,int p,int field,int owned){
 RV value={.type=5};RV*path=results_custom_field(g,p,field);char filename[MAX_PATH];
 if(workshop_directories[p][0]){snprintf(filename,sizeof filename,"%s\\%s",workshop_directories[p],field==2?"portrait.png":"result_small.png");if(GetFileAttributesA(filename)!=INVALID_FILE_ATTRIBUTES){((void(__cdecl*)(RV*,const char*))(base+0x554a750-0x400000))(&value,filename);path=&value;}}
 if(!path||(path->type&0xffffff)!=1||!path->ptr)return -1;
 const char*existing=*(const char**)path->ptr;if(!existing||GetFileAttributesA(existing)==INVALID_FILE_ATTRIBUTES){release(&value);return -1;}
 for(int i=0;i<results_image_count;i++)if(!_stricmp(results_image_cache[i].path,existing)){int id=results_image_cache[i].sprite;release(&value);return id;}
 uint64_t bytes=results_png_bytes(existing);MEMORYSTATUSEX memory={.dwLength=sizeof memory};
 if(results_image_count>=9||bytes>16ULL*1024*1024||results_image_bytes+bytes>32ULL*1024*1024||(GlobalMemoryStatusEx(&memory)&&memory.ullAvailVirtual<128ULL*1024*1024)){logmsg("Results image P%d uses existing head to preserve memory",p);release(&value);return -1;}
 RV args[6]={*path,{.real=1},{0},{0},{0},{0}},out={.type=5};builtin(0x5687620,&out,s,o,6,args);int sprite=(int)number(&out);release(&out);if(sprite>=0){snprintf(results_image_cache[results_image_count].path,MAX_PATH,"%s",existing);results_image_cache[results_image_count++].sprite=sprite;results_image_bytes+=bytes;}release(&value);
 if(sprite>=0){results_owned[p][owned]=sprite;logmsg("Results image P%d field=%d loaded sprite=%d",p,field,sprite);}return sprite;
}
static void __cdecl results_create(void*s,void*o){memory_report("results",0);
 void*g=*(void**)(base+0x604a8d8-0x400000);
 results_release_images(s,o);results_custom=0;results_frames=0;results_stats=0;results_confirmed=0;results_ready_timer=0;for(int p=1;p<=8;p++)for(int k=0;k<2;k++)results_owned[p][k]=-1;
 if(local_eight&&number(var(g,4208))==0)for(int p=5;p<=8;p++)if(array_value(g,4737,p)!=0)results_custom=1;
 if(results_custom){
  results_music_loop=0;results_music_started=0;results_music_call(0x30d3910,s,o,-1);results_music_call(0x30d42a0,s,o,-1);RV group={.real=2},loaded={.type=5};builtin(0x56adf30,&loaded,s,o,1,&group);release(&loaded);
  results_winner=(int)number(var(g,2903));
  for(int p=1;p<=8;p++){
   results_players[p].active=match_slots[p]||array_value(g,4737,p)!=0;results_players[p].cpu=array_value(g,4727,p)!=0;results_players[p].ready=results_players[p].cpu;
#ifdef RESULTS_READY_TEST
   results_players[p].cpu=p>2;results_players[p].ready=p>2;
#endif

   results_players[p].head=4034;results_players[p].frame=(int)array_value(g,4805,p)-1;
   if(hud_custom_active[p]){int head=hud_custom_sprite(g,p);if(head>=0){results_players[p].head=head;results_players[p].frame=0;}}
   results_players[p].stocks=(int)array_value(g,4815,p);results_players[p].kills=(int)array_value(g,4774,p);results_players[p].damage=array_value(g,4745,p);
   results_players[p].deaths=(int)array_value(g,4750,p);results_players[p].suicides=(int)array_value(g,4817,p);results_players[p].parries=(int)array_value(g,4788,p);results_players[p].combo=(int)array_value(g,4778,p);
   results_players[p].given=array_value(g,4746,p);results_players[p].taken=array_value(g,4747,p);double attacks=array_value(g,4729,p);results_players[p].accuracy=attacks>0?100*array_value(g,4728,p)/attacks:0;
   results_players[p].eliminated=match_eliminated_at[p];if(results_players[p].stocks<=0&&!results_players[p].eliminated)results_players[p].eliminated=match_ticks+1;
   results_players[p].portrait=hud_custom_active[p]?results_players[p].head:4070;results_players[p].portrait_frame=hud_custom_active[p]?0:results_players[p].frame;
   results_players[p].row_sprite=results_players[p].head;results_players[p].row_frame=results_players[p].frame;
   if(results_players[p].active&&hud_custom_active[p]){int small=results_load_image(g,s,o,p,3,0);if(small>=0){results_players[p].row_sprite=small;results_players[p].row_frame=0;}}

   static const char*names[]={"","RANDOM","ZETTERBURN","ORCANE","WRASTOR","KRAGG","FORSBURN","MAYPUL","ABSA","ETALUS","ORI","RANNO","CLAIREN","SYLVANOS","ELLIANA","SHOVEL KNIGHT","POMME","MOLLO","OLYMPIA","HODAN","SANDBAG"};int select=(int)array_value(g,4805,p);
   snprintf(results_players[p].name,sizeof results_players[p].name,"%s",select>=2&&select<=20?names[select]:"WORKSHOP FIGHTER");
   if(hud_custom_active[p]&&workshop_display_names[p][0])snprintf(results_players[p].name,sizeof results_players[p].name,"%s",workshop_display_names[p]);

  }
  int survivor=0,remaining=0;for(int p=1;p<=8;p++)if(results_players[p].active&&results_players[p].stocks>0){survivor=p;remaining++;}
  if(remaining==1)results_winner=survivor;
  for(int p=1;p<=8;p++)if(results_players[p].active){
   int place=1;for(int q=1;q<=8;q++)if(q!=p&&results_players[q].active){
    int alive_p=results_players[p].stocks>0,alive_q=results_players[q].stocks>0,better=0;
    if(alive_p!=alive_q)better=alive_q;
    else if(!alive_p)better=results_players[q].eliminated>results_players[p].eliminated;
    else better=results_players[q].stocks>results_players[p].stocks||(results_players[q].stocks==results_players[p].stocks&&results_players[q].damage<results_players[p].damage);
    place+=better;
   }results_players[p].place=place;logmsg("Results P%d place=%d stocks=%d eliminated=%d",p,place,results_players[p].stocks,results_players[p].eliminated);
  }
  int first=0,first_count=0;for(int p=1;p<=8;p++)if(results_players[p].active&&results_players[p].place==1){first=p;first_count++;}results_winner=first_count==1?first:0;
  static const int tracks[]={1000,1000,1000,1003,1001,1002,1005,1004,1006,1007,1012,1008,1009,1010,1011,1013,1043,1044,1045,1046,1000};int selection=results_winner?(int)array_value(g,4805,results_winner):2;results_music_track=selection>=2&&selection<=20?tracks[selection]:1000;

  if(results_winner&&hud_custom_active[results_winner]){int portrait=results_load_image(g,s,o,results_winner,2,1);if(portrait>=0)results_players[results_winner].portrait=portrait;}
#ifdef WIN_SCREEN_PREVIEW
  unsigned places=0;for(int p=1;p<=8;p++)if(results_players[p].active)places|=1u<<results_players[p].place;
  if(results_winner!=1||remaining!=1||places!=0x1fe){logmsg("HARNESS incorrect final standings winner=%d survivors=%d placements=%x",results_winner,remaining,places);ExitProcess(50);}
#endif
 }
 if(!results_custom)results_create_orig(s,o);
 if(results_custom)for(int p=5;p<=8;p++)if(hud_custom_active[p]){
  logmsg("Restoring Workshop CSS selection P%d from %g to 21",p,array_value(g,4805,p));array_number(g,4805,p,21);
 }
}
static void __cdecl results_step(void*s,void*o){
 if(!results_custom){results_step_orig(s,o);return;}
 results_music_update(s,o);if(++results_frames<60)return;
 void*g=*(void**)(base+0x604a8d8-0x400000);int accept=0;
#ifdef RESULTS_READY_TEST
 if(results_frames==190)array_number(g,4549,1,1);if(results_frames==191)array_number(g,4549,1,0);
 if(results_frames==230)array_number(g,4549,2,1);if(results_frames==231)array_number(g,4549,2,0);
#endif
 int toggle=0,confirmed=0,menu_confirm=0,humans=0;
 for(int p=1;p<=8;p++)if(results_players[p].active){
  if(!results_players[p].cpu)humans++;if(array_value(g,4549,p)!=0)menu_confirm=1;
  if(array_value(g,4623,p)!=0)accept=1;
  if(array_value(g,4561,p)!=0)toggle=1;
  if(!results_players[p].cpu&&array_value(g,4549,p)!=0){if(!results_players[p].ready)logmsg("Results P%d ready",p);results_players[p].ready=1;confirmed=1;}
 }
 RV key={.real=13},pressed={.type=5};builtin(0x55948c0,&pressed,s,o,1,&key);if(number(&pressed)!=0)accept=1;
 key.real=9;builtin(0x55948c0,&pressed,s,o,1,&key);if(number(&pressed)!=0)toggle=1;
 key.real=90;builtin(0x55948c0,&pressed,s,o,1,&key);
#ifdef RESULTS_ALLCPU_READY_TEST
 if(results_frames==230)pressed=(RV){.real=1};
#endif
 if(!humans&&(menu_confirm||number(&pressed)!=0))accept=1;
 if(number(&pressed)!=0&&!confirmed)for(int p=1;p<=8;p++)if(results_players[p].active&&!results_players[p].cpu&&!results_players[p].ready){results_players[p].ready=1;confirmed=1;break;}
 if(toggle)results_stats=!results_stats;if(confirmed)results_confirmed=1;
 int all_ready=1;for(int p=1;p<=8;p++)if(results_players[p].active&&!results_players[p].ready)all_ready=0;
 if(results_confirmed&&all_ready&&++results_ready_timer>=18)accept=1;
 release(&pressed);
 if(accept){logmsg("Results continuing to character selection ready=%d timer=%d",all_ready,results_ready_timer);RV room={.real=3},out={.type=5};builtin(0x557e310,&out,s,o,1,&room);}
}
static void results_sprite(void*s,void*o,int sprite,int frame,double x,double y,double w,double h){
 RV id={.real=sprite},exists={.type=5},sw={.type=5},sh={.type=5},ox={.type=5},oy={.type=5},frames={.type=5},out={.type=5};
 builtin(0x5687c20,&exists,s,o,1,&id);if(number(&exists)==0)return;
 builtin(0x56882f0,&sw,s,o,1,&id);builtin(0x5688150,&sh,s,o,1,&id);builtin(0x5688330,&ox,s,o,1,&id);builtin(0x5688370,&oy,s,o,1,&id);builtin(0x56881b0,&frames,s,o,1,&id);
 double width=number(&sw),height=number(&sh);if(width<=0||height<=0)return;
 double scale=w/width;if(h/height<scale)scale=h/height;if(scale>=1)scale=(int)scale;
 int count=(int)number(&frames);if(count<=0)return;if(frame<0)frame=0;if(frame>=count)frame=count-1;
 RV args[9]={{.real=sprite},{.real=frame},{.real=x+(w-width*scale)*.5+number(&ox)*scale},{.real=y+(h-height*scale)*.5+number(&oy)*scale},{.real=scale},{.real=scale},{.real=0},{.real=0xffffff},{.real=1}};builtin(0x5580ef0,&out,s,o,9,args);
}
static void __cdecl results_draw(void*s,void*o){
 if(!results_custom){results_draw_orig(s,o);return;}
 void*g=*(void**)(base+0x604a8d8-0x400000);
 RV out={.type=5},saved[5],mat={.type=5},world={.real=2},oldmat={.type=5};
 uint32_t getters[]={0x55804c0,0x5580450,0x5580410,0x55804e0,0x5580600};
 for(int i=0;i<5;i++){saved[i]=(RV){.type=5};builtin(getters[i],&saved[i],s,o,0,NULL);}
 builtin(0x56775f0,&oldmat,s,o,1,&world);
 RV b[9]={{0},{0},{0},{0},{0},{0},{.real=1},{.real=1},{.real=1}};ui_matrix(&mat,s,o,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 RV gw={.type=5},gh={.type=5};builtin(0x5607a50,&gw,s,o,0,NULL);builtin(0x5607a10,&gh,s,o,0,NULL);double w=number(&gw),h=number(&gh);
 static const unsigned colors[8]={0x3028ee,0xe6a400,0x43b630,0xba70df,0x167ef7,0xda79bb,0x4edede,0xc8c8c8};
 int total=0,order[8];for(int p=1;p<=8;p++)if(results_players[p].active)order[total++]=p;
 for(int i=1;i<total;i++){int p=order[i],j=i;while(j>0&&results_players[order[j-1]].place>results_players[p].place){order[j]=order[j-1];j--;}order[j]=p;}
 int hero=results_winner?results_winner:(total?order[0]:1),font=4;
 hud_value(0x5580d70,s,o,1);hud_rect(s,o,0,0,w,h,0x38204b);
 results_sprite(s,o,4060,0,0,0,w,h);
 hud_value(0x5580d70,s,o,.48);hud_rect(s,o,0,0,w,h,0x6f328f);hud_value(0x5580d70,s,o,1);
 hud_rect(s,o,0,0,w,54,0x261735);hud_rect(s,o,0,h-48,w,48,0x261735);
 hud_value(0x5580df0,s,o,0);hud_value(0x5580e50,s,o,0);
 hud_text(s,o,20,14,"VERSUS MODE",1.2,font,0xffffff);
 hud_value(0x5580df0,s,o,2);hud_text(s,o,w-20,18,results_stats?"MATCH STATISTICS":"MATCH RESULTS",.75,font,0xdfcff1);
 double left=results_stats?22:w*.405,top=70,rowh=(h-136)/8.0,rw=w-left-18;
 if(!results_stats&&total){
  results_sprite(s,o,results_players[hero].portrait,results_players[hero].portrait_frame,-20,58,w*.435,h-125);
  char title[96];snprintf(title,sizeof title,results_winner?"%s WINS!":"DRAW",results_players[hero].name);
  double ts=1.4;if(strlen(title)>16)ts=1;if(strlen(title)>25)ts=.72;
  hud_value(0x5580d70,s,o,.88);hud_rect(s,o,0,h-128,left-8,62,0x261735);hud_value(0x5580d70,s,o,1);
  hud_value(0x5580df0,s,o,1);hud_text(s,o,(left-8)*.5,h-113,title,ts,font,0xffffff);
 }
 for(int i=0;i<total;i++){
  int p=order[i],rank=results_players[p].place;double y=top+i*rowh,cut=14,rh=rowh-5;unsigned color=colors[p-1];
  hud_value(0x5580d70,s,o,1);hud_rect(s,o,left+cut,y,rw-cut,rh,0x100b18);hud_triangle(s,o,left,y+rh,left+cut,y,left+cut,y+rh,0x100b18);
  hud_rect(s,o,left+cut+2,y+2,rw-cut-4,rh-4,color);hud_triangle(s,o,left+3,y+rh-2,left+cut+2,y+2,left+cut+2,y+rh-2,color);
  hud_value(0x5580df0,s,o,0);
  if(rank<=4)results_sprite(s,o,4075,rank-1,left+12,y+5,38,rh-9);
  else {char label[12];snprintf(label,sizeof label,"%dTH",rank);hud_text(s,o,left+14,y+12,label,.83,font,0xffffff);}
  double px=left+58,pw=results_stats?98:104;
  hud_rect(s,o,px,y+3,pw,rh-6,0x100b18);
  results_sprite(s,o,results_players[p].row_sprite,results_players[p].row_frame,px+1,y+4,pw-2,rh-19);
  char player[8];snprintf(player,sizeof player,"P%d",p);hud_value(0x5580df0,s,o,2);hud_text(s,o,px+pw-3,y+4,player,.55,font,0xffffff);
  char name[64];snprintf(name,sizeof name,"%s",results_players[p].name);if(strlen(name)>16){name[13]='.';name[14]='.';name[15]='.';name[16]=0;}
  hud_value(0x5580df0,s,o,1);hud_text(s,o,px+pw*.5,y+rh-15,name,.55,font,0xffffff);
  double sx=px+pw+12,sw=left+rw-sx-84;
  hud_rect(s,o,sx-3,y+5,sw+3,rh-10,0x100b18);
  hud_rect(s,o,left+rw-76,y+5,70,rh-10,0x100b18);hud_value(0x5580df0,s,o,1);hud_text(s,o,left+rw-41,y+14,results_players[p].cpu?"CPU":results_players[p].ready?"READY":"Z / A",.52,font,results_players[p].ready?0x89ef9b:0x75eaff);
  char line[48];hud_value(0x5580df0,s,o,0);
  if(results_stats){
   double col=sw/4;
   snprintf(line,sizeof line,"KOs  %d",results_players[p].kills);hud_text(s,o,sx+5,y+7,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"DEATHS  %d",results_players[p].deaths);hud_text(s,o,sx+col,y+7,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"SUICIDES  %d",results_players[p].suicides);hud_text(s,o,sx+2*col,y+7,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"PARRIES  %d",results_players[p].parries);hud_text(s,o,sx+3*col,y+7,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"DMG GIVEN  %.0f",results_players[p].given);hud_text(s,o,sx+5,y+26,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"DMG TAKEN  %.0f",results_players[p].taken);hud_text(s,o,sx+col,y+26,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"COMBO  %d",results_players[p].combo);hud_text(s,o,sx+2*col,y+26,line,.45,font,0xffffff);
   snprintf(line,sizeof line,"ACCURACY  %.0f%%",results_players[p].accuracy);hud_text(s,o,sx+3*col,y+26,line,.45,font,0xffffff);
  }else{
   snprintf(line,sizeof line,"KOs  %d",results_players[p].kills);hud_text(s,o,sx+5,y+12,line,.75,font,0xffffff);
   snprintf(line,sizeof line,"STOCKS  %d",results_players[p].stocks);hud_text(s,o,sx+sw*.5,y+12,line,.75,font,0xffffff);
  }
 }
 hud_value(0x5580df0,s,o,0);hud_text(s,o,20,h-42,"Z / A  READY     X / TAB  STATS",.6,font,0xffffff);
 hud_value(0x5580df0,s,o,2);hud_text(s,o,w-20,h-42,"START / ENTER  CONTINUE",.65,font,0xffffff);
 uint32_t setters[]={0x5580dd0,0x5580db0,0x5580d70,0x5580df0,0x5580e50};for(int i=0;i<5;i++)builtin(setters[i],&out,s,o,1,&saved[i]);
 set[1]=oldmat;builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&oldmat);
}
static void __cdecl results_alarm0(void*s,void*o){if(!results_custom)results_alarm0_orig(s,o);}
static void __cdecl results_alarm1(void*s,void*o){if(!results_custom)results_alarm1_orig(s,o);}
static void __cdecl results_alarm2(void*s,void*o){if(!results_custom)results_alarm2_orig(s,o);}
static void __cdecl results_other5(void*s,void*o){results_release_images(s,o);if(!results_custom)results_other5_orig(s,o);else{results_music_call(0x30d3910,s,o,-1);results_music_call(0x30d42a0,s,o,-1);}}
static void __cdecl results_other72(void*s,void*o){if(!results_custom)results_other72_orig(s,o);}
