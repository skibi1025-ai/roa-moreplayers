static Event results_create_orig,results_draw_orig,results_step_orig,results_alarm0_orig,results_alarm1_orig,results_alarm2_orig,results_other5_orig,results_other72_orig;
static int results_custom,results_frames,results_winner,results_music_loop;
static void results_music_call(uint32_t address,void*s,void*o,int track){RV result={.type=5},value={.real=track};RV*args[]={&value};((Script)(base+address-0x400000))(s,o,&result,track<0?0:1,track<0?NULL:args);release(&result);}
static void results_music_update(void*s,void*o){
 if(results_music_loop)return;void*g=*(void**)(base+0x604a8d8-0x400000);RV playing={.type=5};RV*args[]={var(g,492)};
 typedef RV*(__cdecl*Legacy)(void*,void*,RV*,int,int,RV**);
 ((Legacy)(base+0x5549fd0-0x400000))(s,o,&playing,1,*(int*)(base+0x5fb3ba4-0x400000),args);
 if(number(&playing)==0){results_music_call(0x30ce0e0,s,o,910);results_music_loop=1;logmsg("Results music loop started");}release(&playing);
}
static struct {int active,head,frame,portrait,portrait_frame,stocks,kills,place,eliminated;double damage;char name[64];} results_players[9];
static RV* results_custom_field(void*g,int p,int field){
 RV*v=var(g,0x4bf);int context=(int)array_value(g,1236,p);if((v->type&0xffffff)!=2||!v->ptr)return NULL;Array*a=v->ptr;
 if(context<0||context>=a->length||!a->dims||field<0||field>=a->dims[context].length||!a->dims[context].values)return NULL;return a->dims[context].values+field;
}
static void __cdecl results_create(void*s,void*o){
 void*g=*(void**)(base+0x604a8d8-0x400000);
 results_custom=0;results_frames=0;
 if(local_eight&&number(var(g,4208))==0)for(int p=5;p<=8;p++)if(array_value(g,4737,p)!=0)results_custom=1;
 if(results_custom){
  results_music_loop=0;results_music_call(0x30d3910,s,o,-1);results_music_call(0x30d42a0,s,o,-1);results_music_call(0x30d0400,s,o,981);
  results_winner=(int)number(var(g,2903));
  for(int p=1;p<=8;p++){
   results_players[p].active=match_slots[p]||array_value(g,4737,p)!=0;
   results_players[p].head=4034;results_players[p].frame=(int)array_value(g,4805,p)-1;
   if(hud_custom_active[p]){int head=hud_custom_sprite(g,p);if(head>=0){results_players[p].head=head;results_players[p].frame=0;}}
   results_players[p].stocks=(int)array_value(g,4815,p);results_players[p].kills=(int)array_value(g,4774,p);results_players[p].damage=array_value(g,4745,p);
   results_players[p].eliminated=match_eliminated_at[p];if(results_players[p].stocks<=0&&!results_players[p].eliminated)results_players[p].eliminated=match_ticks+1;
   results_players[p].portrait=hud_custom_active[p]?results_players[p].head:4070;results_players[p].portrait_frame=hud_custom_active[p]?0:results_players[p].frame;
   if(hud_custom_active[p]){RV*portrait=results_custom_field(g,p,6);if(portrait){int type=portrait->type&0xffffff;if(type==0||type==7||type==10){RV exists={.type=5};builtin(0x5687c20,&exists,s,o,1,portrait);if(number(&exists)!=0)results_players[p].portrait=(int)number(portrait);}}}
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
 for(int p=1;p<=8;p++)if(array_value(g,4623,p)!=0||array_value(g,4549,p)!=0)accept=1;
 RV key={.real=13},pressed={.type=5};builtin(0x55948c0,&pressed,s,o,1,&key);if(number(&pressed)!=0)accept=1;
 if(accept){RV room={.real=3},out={.type=5};builtin(0x557e310,&out,s,o,1,&room);}
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
 RV b[9]={{0},{0},{0},{0},{0},{0},{.real=1},{.real=1},{.real=1}};builtin(0x5676a10,&mat,s,o,9,b);RV set[2]={world,mat};builtin(0x5678660,&out,s,o,2,set);
 RV gw={.type=5},gh={.type=5};builtin(0x5607a50,&gw,s,o,0,NULL);builtin(0x5607a10,&gh,s,o,0,NULL);double w=number(&gw),h=number(&gh);
 static const unsigned colors[8]={0x5050f0,0xff9b50,0x80dc60,0xc980ef,0x2099ff,0xf080b0,0x55e6e6,0xe6e6e6};
 hud_value(0x5580d70,s,o,1);hud_rect(s,o,0,0,w,h,0x1f1712);hud_value(0x5580df0,s,o,0);hud_value(0x5580e50,s,o,0);int font=-1;
 int total=0,order[8];for(int p=1;p<=8;p++)if(results_players[p].active)order[total++]=p;
 for(int i=1;i<total;i++){int p=order[i],j=i;while(j>0&&results_players[order[j-1]].place>results_players[p].place){order[j]=order[j-1];j--;}order[j]=p;}
 int hero=results_winner?results_winner:(total?order[0]:1);unsigned accent=results_winner?colors[hero-1]:0x65d9ff;
 hud_value(0x5580d70,s,o,.16);hud_triangle(s,o,0,90,w,28,w,276,accent);hud_triangle(s,o,0,90,w,276,0,324,accent);
 hud_value(0x5580d70,s,o,.5);hud_triangle(s,o,40,313,324,284,352,294,accent);hud_value(0x5580d70,s,o,1);
 hud_text(s,o,48,24,results_winner?"VICTORY":"MATCH DRAW",2,font,0x65d9ff);
 if(total)results_sprite(s,o,results_players[hero].portrait,results_players[hero].portrait_frame,72,76,256,226);
 char title[48],subtitle[64];if(results_winner)snprintf(title,sizeof title,"PLAYER %d WINS",results_winner);else snprintf(title,sizeof title,"HONORS SHARED");
 hud_text(s,o,380,104,title,3,font,0xffffff);hud_text(s,o,382,164,results_players[hero].name,strlen(results_players[hero].name)>24?1.25:2,font,accent);
 snprintf(subtitle,sizeof subtitle,"LOCAL VERSUS  /  %d PLAYERS",total);hud_text(s,o,382,208,subtitle,1.1,font,0xbeb5ab);
 if(results_winner){snprintf(subtitle,sizeof subtitle,"%d STOCKS LEFT    %d KOs",results_players[hero].stocks,results_players[hero].kills);hud_text(s,o,382,244,subtitle,1.25,font,0xffffff);}
 double tile=(w-48)/8.0,start=(w-tile*total)*.5;
 for(int i=0;i<total;i++){
  int p=order[i];double x=start+tile*i,y=340;unsigned color=colors[p-1];
  hud_value(0x5580d70,s,o,.14);hud_triangle(s,o,x+4,y+12,x+tile-8,y,x+tile-12,y+130,color);hud_triangle(s,o,x+4,y+12,x+tile-12,y+130,x,y+130,color);hud_value(0x5580d70,s,o,1);
  hud_art(s,o,x,y+22,tile-8,color,p);
  char label[24],stats[24];hud_value(0x5580df0,s,o,1);snprintf(label,sizeof label,"#%d",results_players[p].place);hud_text(s,o,x+tile*.5,y-8,label,2.2,font,results_players[p].place==1?0x65d9ff:0xffffff);
  snprintf(label,sizeof label,"PLAYER %d",p);hud_text(s,o,x+tile*.5,y+27,label,1,font,color);
  results_sprite(s,o,results_players[p].head,results_players[p].frame,x+tile*.5-20,y+46,40,40);
  snprintf(stats,sizeof stats,"STOCKS %d",results_players[p].stocks);hud_text(s,o,x+tile*.5,y+104,stats,1,font,0xffffff);snprintf(stats,sizeof stats,"KOs %d",results_players[p].kills);hud_text(s,o,x+tile*.5,y+123,stats,1,font,0xbeb5ab);
 }
 hud_value(0x5580df0,s,o,1);hud_text(s,o,w*.5,h-30,"START / ENTER   CHARACTER SELECT",1.1,font,0xffffff);
 uint32_t setters[]={0x5580dd0,0x5580db0,0x5580d70,0x5580df0,0x5580e50};for(int i=0;i<5;i++)builtin(setters[i],&out,s,o,1,&saved[i]);
 set[1]=oldmat;builtin(0x5678660,&out,s,o,2,set);release(&mat);release(&oldmat);
}
static void __cdecl results_alarm0(void*s,void*o){if(!results_custom)results_alarm0_orig(s,o);}
static void __cdecl results_alarm1(void*s,void*o){if(!results_custom)results_alarm1_orig(s,o);}
static void __cdecl results_alarm2(void*s,void*o){if(!results_custom)results_alarm2_orig(s,o);}
static void __cdecl results_other5(void*s,void*o){if(!results_custom)results_other5_orig(s,o);else{results_music_call(0x30d3910,s,o,-1);results_music_call(0x30d42a0,s,o,-1);}}
static void __cdecl results_other72(void*s,void*o){if(!results_custom)results_other72_orig(s,o);}
