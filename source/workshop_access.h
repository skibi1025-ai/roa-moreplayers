static Event workshop_global_orig;
static unsigned workshop_start_frames;
static int workshop_prompted,workshop_allowed;
static ULONGLONG workshop_next_check;
static void*workshop_ugc;
static unsigned(__cdecl*workshop_state)(void*,uint64_t);
static int workshop_subscription(void){
 HMODULE steam=GetModuleHandleA("steam_api.dll");if(!steam)return -1;
 void*(__cdecl*ugc)(void)=(void*)GetProcAddress(steam,"SteamAPI_SteamUGC_v014");
 workshop_state=(void*)GetProcAddress(steam,"SteamAPI_ISteamUGC_GetItemState");
 if(!ugc||!workshop_state)return -1;
 workshop_ugc=ugc();if(!workshop_ugc)return -1;
 return (workshop_state(workshop_ugc,3840181185ULL)&1)!=0;
}
static int workshop_open_overlay(void){
 HMODULE steam=GetModuleHandleA("steam_api.dll");if(!steam)return 0;
 void*(__cdecl*friends_get)(void)=(void*)GetProcAddress(steam,"SteamAPI_SteamFriends_v017");
 void*(__cdecl*utils_get)(void)=(void*)GetProcAddress(steam,"SteamAPI_SteamUtils_v009");
 unsigned char(__cdecl*enabled)(void*)=(void*)GetProcAddress(steam,"SteamAPI_ISteamUtils_IsOverlayEnabled");
 void(__cdecl*open)(void*,const char*,int)=(void*)GetProcAddress(steam,"SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage");
 if(!friends_get||!utils_get||!enabled||!open)return 0;
 void*friends=friends_get(),*utils=utils_get();if(!friends||!utils||!enabled(utils))return 0;
 open(friends,"https://steamcommunity.com/sharedfiles/filedetails/?id=3840181185",0);return 1;
}
static int workshop_access(void){
 int state=workshop_subscription();workshop_allowed=state==1;
 if(workshop_allowed)return 1;
 if(state<0){MessageBoxA(NULL,"Local+ could not check your Workshop subscription. Start Steam, sign in, and launch Local+ again.","Local+",MB_OK|MB_ICONINFORMATION);ExitProcess(0);}
 if(!workshop_prompted){
  workshop_prompted=1;
  int choice=MessageBoxA(NULL,"Local+ requires a subscription to its Workshop stage. Open the Workshop page in the Steam overlay to subscribe?\n\nAfter subscribing, close the overlay with Shift+Tab to continue.","Local+",MB_YESNO|MB_ICONQUESTION);
  if(choice!=IDYES)ExitProcess(0);
 }else MessageBoxA(NULL,"Subscribe on the Workshop page before starting Local+. Close the Steam overlay with Shift+Tab after subscribing.","Local+",MB_OK|MB_ICONINFORMATION);
 if(!workshop_open_overlay()){MessageBoxA(NULL,"The Steam overlay is unavailable. Enable the Steam overlay for Rivals of Aether, restart Local+, and subscribe when prompted.","Local+",MB_OK|MB_ICONINFORMATION);ExitProcess(0);}
 return 0;
}
static void __cdecl workshop_global_step(void*s,void*o){
 workshop_global_orig(s,o);
 if(++workshop_start_frames<180)return;
 ULONGLONG now=GetTickCount64();if(now<workshop_next_check)return;workshop_next_check=now+1000;
 int state=workshop_subscription();workshop_allowed=state==1;
 if(!workshop_prompted&&!workshop_allowed)workshop_access();
}
