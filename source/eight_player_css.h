static const struct {uint32_t address,before,after;} css_slot_patches[]={
 {0x4261c00,0x401c0000,0x40220000},
 {0x426263b,0x40140000,0x40220000},
 {0x42622ca,0x40340000,0x40200000}
};
static int css_cpu_level(double value){
 if(value!=value||value<1||value>9)return 5;
 return (int)value;
}
static int css_puck_point(int player,double first_x,double first_y,double last_x,double last_y,double*x,double*y){
 if(player<1||player>8)return 0;
 *x=(first_x+last_x)*.5+((player-1)%4-1.5)*18;
 *y=(first_y+last_y)*.5+((player-1)/4-.5)*20;
 return 1;
}
