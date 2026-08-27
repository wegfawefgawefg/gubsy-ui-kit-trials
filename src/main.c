#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>
#include <X11/Xatom.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_VERTEX_MEMORY (512 * 1024)
#define MAX_ELEMENT_MEMORY (128 * 1024)

struct app {
    int screen, play_activity, host_mode, join_mode;
    int players_tab, settings_tab, controls_tab, mods_tab, selected;
    int fullscreen, subtitles, friendly_fire, compatible_only;
    float render_scale, brightness, master_volume, music_volume;
    float sensitivity, deadzone, vibration, trigger_deadzone;
};

struct args { int width, height, screen, benchmark; const char *capture; };

static double now_ms(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1000.0 + (double)t.tv_nsec / 1000000.0;
}

static long rss_kib(void) {
    FILE *f=fopen("/proc/self/statm","r"); long total=0,resident=0;
    if(f){if(fscanf(f,"%ld %ld",&total,&resident)!=2)resident=0;fclose(f);}
    return resident*(long)sysconf(_SC_PAGESIZE)/1024;
}

static struct args parse_args(int argc,char **argv) {
    struct args a={1280,720,0,0,NULL};
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--resolution")&&i+1<argc)sscanf(argv[++i],"%dx%d",&a.width,&a.height);
        else if(!strcmp(argv[i],"--screen")&&i+1<argc){const char *n=argv[++i];const char *names[]={"play","players","settings","controls","progress","mods"};a.screen=atoi(n);for(int j=0;j<6;j++)if(!strcmp(n,names[j]))a.screen=j;}
        else if(!strcmp(argv[i],"--benchmark")&&i+1<argc)a.benchmark=atoi(argv[++i]);
        else if(!strcmp(argv[i],"--capture")&&i+1<argc){a.capture=argv[++i];a.benchmark=5;}
    } return a;
}

static void mark_utility(SDL_Window *window) {
    SDL_SysWMinfo info;SDL_VERSION(&info.version);
    if(!SDL_GetWindowWMInfo(window,&info)||info.subsystem!=SDL_SYSWM_X11)return;
    Display *d=info.info.x11.display;Window w=info.info.x11.window;
    Atom prop=XInternAtom(d,"_NET_WM_WINDOW_TYPE",False);Atom utility=XInternAtom(d,"_NET_WM_WINDOW_TYPE_UTILITY",False);
    XChangeProperty(d,w,prop,XA_ATOM,32,PropModeReplace,(unsigned char*)&utility,1);XFlush(d);
}

static void style(struct nk_context *ctx) {
    struct nk_color bg=nk_rgba(7,18,21,255), panel=nk_rgba(12,31,34,255), panel2=nk_rgba(18,43,43,255), line=nk_rgba(48,78,78,255), text=nk_rgba(224,234,232,255), accent=nk_rgba(151,239,116,255);
    ctx->style.window.background=bg;ctx->style.window.fixed_background=nk_style_item_color(bg);ctx->style.window.border_color=line;ctx->style.window.padding=nk_vec2(10,10);ctx->style.window.spacing=nk_vec2(7,7);
    ctx->style.text.color=text;
    ctx->style.button.normal=nk_style_item_color(panel2);ctx->style.button.hover=nk_style_item_color(nk_rgba(26,62,52,255));ctx->style.button.active=nk_style_item_color(accent);ctx->style.button.border_color=line;ctx->style.button.text_normal=text;ctx->style.button.text_hover=text;ctx->style.button.text_active=nk_rgba(8,24,20,255);
    ctx->style.selectable.normal=nk_style_item_color(panel);ctx->style.selectable.hover=nk_style_item_color(panel2);ctx->style.selectable.pressed=nk_style_item_color(panel2);ctx->style.selectable.normal_active=nk_style_item_color(nk_rgba(22,55,39,255));ctx->style.selectable.hover_active=nk_style_item_color(nk_rgba(28,68,47,255));ctx->style.selectable.text_normal=text;ctx->style.selectable.text_hover=text;ctx->style.selectable.text_pressed=text;ctx->style.selectable.text_normal_active=accent;ctx->style.selectable.text_hover_active=accent;
    ctx->style.edit.normal=nk_style_item_color(panel);ctx->style.edit.hover=nk_style_item_color(panel2);ctx->style.edit.active=nk_style_item_color(panel);ctx->style.edit.border_color=line;ctx->style.edit.text_normal=text;
    ctx->style.combo.normal=nk_style_item_color(panel);ctx->style.combo.hover=nk_style_item_color(panel2);ctx->style.combo.active=nk_style_item_color(panel2);ctx->style.combo.border_color=line;ctx->style.combo.label_normal=text;ctx->style.combo.label_hover=text;ctx->style.combo.label_active=text;
    ctx->style.slider.normal=nk_style_item_color(panel2);ctx->style.slider.hover=nk_style_item_color(panel2);ctx->style.slider.active=nk_style_item_color(panel2);ctx->style.slider.bar_normal=line;ctx->style.slider.bar_hover=line;ctx->style.slider.bar_active=line;ctx->style.slider.bar_filled=accent;ctx->style.slider.cursor_normal=nk_style_item_color(accent);ctx->style.slider.cursor_hover=nk_style_item_color(accent);ctx->style.slider.cursor_active=nk_style_item_color(accent);
}

static void label(struct nk_context *ctx,const char *s){struct nk_color old=ctx->style.text.color;ctx->style.text.color=nk_rgba(151,239,116,255);nk_label(ctx,s,NK_TEXT_LEFT);ctx->style.text.color=old;}
static void muted(struct nk_context *ctx,const char *s){struct nk_color old=ctx->style.text.color;ctx->style.text.color=nk_rgba(145,164,161,255);nk_label(ctx,s,NK_TEXT_LEFT);ctx->style.text.color=old;}
static void heading(struct nk_context *ctx,const char *s){nk_layout_row_dynamic(ctx,36,1);nk_label(ctx,s,NK_TEXT_LEFT);}

static int combo(struct nk_context *ctx,const char *const *items,int count,int selected,float width){
    if(nk_combo_begin_label(ctx,items[selected],nk_vec2(width,160))){nk_layout_row_dynamic(ctx,30,1);for(int i=0;i<count;i++)if(nk_combo_item_label(ctx,items[i],NK_TEXT_LEFT))selected=i;nk_combo_end(ctx);}return selected;
}

static void card(struct nk_context *ctx,const char *id,const char *title,const char *sub,const char *value,int active){
    struct nk_color prev=ctx->style.window.fixed_background.data.color;if(active)ctx->style.window.fixed_background=nk_style_item_color(nk_rgba(20,52,38,255));
    if(nk_group_begin(ctx,id,NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)){nk_layout_row_dynamic(ctx,20,1);nk_label(ctx,title,NK_TEXT_LEFT);nk_layout_row_dynamic(ctx,16,2);muted(ctx,sub);if(value)label(ctx,value);nk_group_end(ctx);}ctx->style.window.fixed_background=nk_style_item_color(prev);
}

static void header(struct nk_context *ctx,int width){
    nk_layout_row_begin(ctx,NK_STATIC,42,4);nk_layout_row_push(ctx,36);if(nk_button_label(ctx,"G")){}nk_layout_row_push(ctx,width*.38f);nk_label(ctx,"G U B S Y   S H E L L",NK_TEXT_LEFT);nk_layout_row_push(ctx,width*.30f);nk_label(ctx,"*  OFFLINE   |   VEGA",NK_TEXT_CENTERED);nk_layout_row_push(ctx,145);nk_label(ctx,"NUKLEAR NATIVE",NK_TEXT_RIGHT);nk_layout_row_end(ctx);
}

static void nav(struct nk_context *ctx,struct app *a,float width,float height){
    if(nk_group_begin(ctx,"primary-nav",NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)){nk_layout_row_dynamic(ctx,50,1);nk_label(ctx,"VE   ACTIVE PROFILE\n     Vega",NK_TEXT_LEFT);const char *names[]={">  Play\n    Continue or start","+  Players\n    Profiles & devices","*  Settings\n    Game preferences","~  Controls\n    Bindings & input","#  Progress\n    Campaigns & saves","o  Mods\n    Installed content"};for(int i=0;i<6;i++){int selected=a->screen==i;if(nk_selectable_label(ctx,names[i],NK_TEXT_LEFT,&selected))a->screen=i;}nk_layout_space_begin(ctx,NK_STATIC,height-430,1);nk_layout_space_push(ctx,nk_rect(0,height-485,width-20,44));nk_label(ctx,"x  Quit\n   Return to desktop",NK_TEXT_LEFT);nk_layout_space_end(ctx);nk_group_end(ctx);}
}

static void tabs(struct nk_context *ctx,const char *const *names,int count,int *active){nk_layout_row_dynamic(ctx,34,count);for(int i=0;i<count;i++){int on=*active==i;if(nk_selectable_label(ctx,names[i],NK_TEXT_CENTERED,&on))*active=i;}}

static void play(struct nk_context *ctx,struct app *a){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / PLAY");heading(ctx,"Play");nk_layout_row_begin(ctx,NK_DYNAMIC,520,2);nk_layout_row_push(ctx,.66f);
    if(nk_group_begin(ctx,"play-setup",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,a->play_activity?"NEW EXPEDITION":"CONTINUE QUEST");heading(ctx,a->play_activity?"Choose a quest":"The Violet Reach");muted(ctx,a->play_activity?"Start a clean quest from stage one":"The Glass Caverns · Latest checkpoint · Vega");
        nk_layout_row_begin(ctx,NK_DYNAMIC,52,2);nk_layout_row_push(ctx,.50f);card(ctx,"activity-label","Activity","Continue a checkpoint or start",NULL,0);nk_layout_row_push(ctx,.50f);const char *activities[]={"Continue expedition","New expedition"};a->play_activity=combo(ctx,activities,2,a->play_activity,220);nk_layout_row_end(ctx);
        nk_layout_row_dynamic(ctx,52,1);card(ctx,"resume",a->play_activity?"Quest":"Resume point",a->play_activity?"The Violet Reach · 8 stages":"Latest checkpoint · The Violet Reach",a->play_activity?"CHOOSE QUEST >":"CHOOSE CHECKPOINT >",0);
        nk_layout_row_begin(ctx,NK_DYNAMIC,52,2);nk_layout_row_push(ctx,.50f);card(ctx,"join-label","Play with","Who may occupy remaining slots",NULL,0);nk_layout_row_push(ctx,.50f);const char *joins[]={"Solo","Friends can join","Public"};a->join_mode=combo(ctx,joins,3,a->join_mode,220);nk_layout_row_end(ctx);
        nk_layout_row_begin(ctx,NK_DYNAMIC,52,2);nk_layout_row_push(ctx,.50f);card(ctx,"host-label","Host using","Automatic, direct, or relay",NULL,0);nk_layout_row_push(ctx,.50f);const char *hosts[]={"Automatic","Direct","Relay"};a->host_mode=combo(ctx,hosts,3,a->host_mode,220);nk_layout_row_end(ctx);
        nk_layout_row_dynamic(ctx,48,1);card(ctx,"rules","Expedition rules","Standard · 4 lives · ghost at 180s","EDIT ALL >",0);card(ctx,"session-mods","Session mods","7 active · dependency set valid","MANAGE >",0);
        nk_layout_row_dynamic(ctx,38,2);nk_button_label(ctx,"Pause preview");if(nk_button_label(ctx,a->play_activity?"> Begin expedition":"> Resume latest checkpoint")){}
        nk_group_end(ctx);}nk_layout_row_push(ctx,.34f);
    if(nk_group_begin(ctx,"party",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,"PLAYERS");heading(ctx,a->join_mode?"Your party":"Solo player");nk_layout_row_dynamic(ctx,58,1);card(ctx,"p1","P1  Moss","Xbox Wireless Controller","READY",1);if(a->join_mode){card(ctx,"p2","+  Open slot","Invite friend or add locally",NULL,0);card(ctx,"p3","+  Open slot","Invite friend or add locally",NULL,0);}nk_layout_row_dynamic(ctx,34,1);nk_button_label(ctx,"Invite / copy link");muted(ctx,"CONTENT                         7 mods");muted(ctx,a->host_mode?"NETWORK                         Manual":"NETWORK                      Automatic");nk_group_end(ctx);}nk_layout_row_end(ctx);
}

static void players(struct nk_context *ctx,struct app *a){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / PLAYERS");heading(ctx,"Players");const char *t[]={"Local players","Profiles","Devices"};tabs(ctx,t,3,&a->players_tab);nk_layout_row_dynamic(ctx,430,1);
    if(nk_group_begin(ctx,"players-content",NK_WINDOW_BORDER)){if(a->players_tab==0){nk_layout_row_dynamic(ctx,18,1);label(ctx,"LOCAL PARTY · ASSIGN DEVICES AND PROFILES");nk_layout_row_dynamic(ctx,70,2);card(ctx,"local-p1","P1  Moss","Profile: Moss · Xbox Wireless Controller","READY",1);card(ctx,"selected-player","SELECTED PLAYER: MOSS","Multiple devices are allowed",NULL,0);card(ctx,"local-p2","P2  Open slot","Add locally or invite",NULL,0);card(ctx,"device-a","Xbox Wireless Controller","Gamepad 0 · connected","ASSIGNED",0);nk_layout_row_dynamic(ctx,36,2);nk_button_label(ctx,"Assign device");nk_button_label(ctx,"Mark not ready");}
        else if(a->players_tab==1){nk_layout_row_dynamic(ctx,20,2);label(ctx,"PROFILE LIBRARY · PROFILE DATA IS NOT A SAVE");nk_button_label(ctx,"+ New profile");nk_layout_row_dynamic(ctx,120,3);card(ctx,"moss","Moss","38h 22m · 84 runs · 21 wins","ACTIVE",1);card(ctx,"vega","Vega","14h 11m · 31 runs · 4 wins","SELECT",0);card(ctx,"guest","Guest","Never · 0 runs · 0 wins","SELECT",0);nk_layout_row_dynamic(ctx,28,1);heading(ctx,"Moss's history");nk_layout_row_dynamic(ctx,60,5);card(ctx,"plays","84","PLAYS",NULL,0);card(ctx,"wins","21","WINS",NULL,0);card(ctx,"deaths","138","DEATHS",NULL,0);card(ctx,"score","2.4M","SCORE",NULL,0);card(ctx,"replays","42","REPLAYS",NULL,0);}
        else {nk_layout_row_dynamic(ctx,18,1);label(ctx,"CONNECTED INPUT · MULTI-PLAYER ASSIGNMENT");nk_layout_row_dynamic(ctx,70,2);card(ctx,"xbox","Xbox Wireless Controller","Gamepad 0 · battery 82%","MOSS",1);card(ctx,"explorer","LIVE INPUT EXPLORER","Button South: idle · Left X: +0.04",NULL,0);card(ctx,"keyboard","Keyboard + mouse","Desktop aggregate","AVAILABLE",0);card(ctx,"joystick","T.16000M Joystick","14 axes · 32 buttons","MOSS",0);}nk_group_end(ctx);}
}

static void setting_slider(struct nk_context *ctx,const char *title,const char *sub,float *value,float min,float max){nk_layout_row_begin(ctx,NK_DYNAMIC,55,2);nk_layout_row_push(ctx,.52f);card(ctx,title,title,sub,NULL,0);nk_layout_row_push(ctx,.48f);nk_slider_float(ctx,min,value,max,1);nk_layout_row_end(ctx);}

static void settings(struct nk_context *ctx,struct app *a){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / SETTINGS");heading(ctx,"Settings");const char *t[]={"Display","Audio","Accessibility","Gameplay"};tabs(ctx,t,4,&a->settings_tab);nk_layout_row_begin(ctx,NK_DYNAMIC,430,2);nk_layout_row_push(ctx,.54f);
    if(nk_group_begin(ctx,"settings-list",NK_WINDOW_BORDER)){if(a->settings_tab==0){nk_layout_row_dynamic(ctx,52,2);card(ctx,"fullscreen","Fullscreen","Use the selected display",NULL,0);nk_checkbox_label(ctx,a->fullscreen?"Enabled":"Disabled",&a->fullscreen);const char *res[]={"1280 x 720","1920 x 1080","2560 x 1440"};static int ri=1;nk_layout_row_dynamic(ctx,52,2);card(ctx,"resolution","Display resolution","Output size",NULL,0);ri=combo(ctx,res,3,ri,210);setting_slider(ctx,"Render scale","Internal 3D resolution",&a->render_scale,50,150);const char *caps[]={"60 FPS","120 FPS","144 FPS","Unlimited"};static int ci=2;nk_layout_row_dynamic(ctx,52,2);card(ctx,"cap","Frame cap","Maximum simulation rate",NULL,0);ci=combo(ctx,caps,4,ci,210);setting_slider(ctx,"Brightness","Fine-tune scene visibility",&a->brightness,0,100);}
        else if(a->settings_tab==1){setting_slider(ctx,"Master volume","Overall output",&a->master_volume,0,100);setting_slider(ctx,"Music volume","Score and ambience",&a->music_volume,0,100);const char *range[]={"Night","Balanced","Wide"};static int r=1;nk_layout_row_dynamic(ctx,52,2);card(ctx,"range","Dynamic range","Loud/quiet compression",NULL,0);r=combo(ctx,range,3,r,210);}
        else if(a->settings_tab==2){nk_layout_row_dynamic(ctx,58,1);card(ctx,"subtitles","Subtitles","Dialogue and important callouts","ON",1);card(ctx,"color","Color distinction","Shape and label reinforcement","ON",0);card(ctx,"motion","Reduced motion","Limit camera and menu animation","OFF",0);card(ctx,"text-scale","Text scale","Interface and subtitle size","110%",0);}
        else {nk_layout_row_dynamic(ctx,58,1);card(ctx,"pause","Pause when unfocused","Suspend local simulation","ON",1);card(ctx,"tutorial","Tutorial prompts","Context-sensitive hints","ON",0);card(ctx,"damage","Damage numbers","Combat feedback","OFF",0);}nk_group_end(ctx);}nk_layout_row_push(ctx,.46f);
    if(nk_group_begin(ctx,"setting-detail",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,"SELECTED SETTING");heading(ctx,a->settings_tab==0?"Render scale":a->settings_tab==1?"Master volume":a->settings_tab==2?"Subtitles":"Pause when unfocused");nk_layout_row_dynamic(ctx,54,1);nk_label_wrap(ctx,"The game supplies the schema. Gubsy supplies stable controls, profile storage, serialization, and synchronization.");nk_layout_row_dynamic(ctx,22,1);label(ctx,"CURRENT VALUE");char value[32];snprintf(value,sizeof value,"%.0f%%",a->settings_tab?a->master_volume:a->render_scale);heading(ctx,a->settings_tab<2?value:"Enabled");nk_layout_row_dynamic(ctx,48,1);card(ctx,"scope","Scope","Current machine",NULL,0);card(ctx,"profile","Profile","Vega",NULL,0);card(ctx,"restart","Restart","Not required",NULL,0);nk_layout_row_dynamic(ctx,36,2);nk_button_label(ctx,"Apply settings");nk_button_label(ctx,"Reset category");nk_group_end(ctx);}nk_layout_row_end(ctx);
}

static void controls(struct nk_context *ctx,struct app *a){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / CONTROLS");heading(ctx,"Controls");const char *t[]={"Bindings","Devices","Input tuning"};tabs(ctx,t,3,&a->controls_tab);nk_layout_row_begin(ctx,NK_DYNAMIC,430,2);nk_layout_row_push(ctx,.5f);
    if(nk_group_begin(ctx,"controls-list",NK_WINDOW_BORDER)){if(a->controls_tab==0){static char search[64];static int len=0;nk_layout_row_dynamic(ctx,34,1);nk_edit_string(ctx,NK_EDIT_FIELD,search,&len,63,nk_filter_default);const char *n[]={"Menu Up","Menu Down","Menu Left","Menu Right","Activate","Cancel","Move","Look"};const char *b[]={"D-Pad Up","D-Pad Down","D-Pad Left","D-Pad Right","Gamepad A","Gamepad B","Left Stick","Right Stick"};nk_layout_row_dynamic(ctx,42,2);for(int i=0;i<8;i++){int on=a->selected==i;if(nk_selectable_label(ctx,n[i],NK_TEXT_LEFT,&on))a->selected=i;nk_label(ctx,b[i],NK_TEXT_RIGHT);}}
        else if(a->controls_tab==1){nk_layout_row_dynamic(ctx,68,1);card(ctx,"dev-xbox","Xbox Wireless Controller","Gamepad 0 · connected","MOSS",1);card(ctx,"dev-kbm","Keyboard + mouse","Desktop aggregate","AVAILABLE",0);card(ctx,"dev-stick","T.16000M Joystick","14 axes · 32 buttons","MOSS",0);}
        else {nk_layout_row_dynamic(ctx,18,1);label(ctx,"INPUT PROFILE · STANDARD");setting_slider(ctx,"Look sensitivity","Camera speed",&a->sensitivity,0,100);setting_slider(ctx,"Stick deadzone","Ignore center noise",&a->deadzone,0,40);setting_slider(ctx,"Vibration strength","Rumble output",&a->vibration,0,100);setting_slider(ctx,"Trigger deadzone","Minimum trigger travel",&a->trigger_deadzone,0,30);}nk_group_end(ctx);}nk_layout_row_push(ctx,.5f);
    if(nk_group_begin(ctx,"control-detail",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,a->controls_tab==0?"SELECTED ACTION":a->controls_tab==1?"INPUT EXPLORER":"DEVICE RESPONSE");heading(ctx,a->controls_tab==0?"Menu action":a->controls_tab==1?"Live device input":"Live response");if(a->controls_tab==0){nk_layout_row_dynamic(ctx,58,1);card(ctx,"binding-a","1  Gamepad binding","D-pad / face / transformed axis","REPLACE",0);card(ctx,"binding-b","2  Keyboard binding","Keyboard key","REPLACE",0);nk_layout_row_dynamic(ctx,36,2);nk_button_label(ctx,"Listen for input");nk_button_label(ctx,"Browse controls");nk_layout_row_dynamic(ctx,60,1);nk_label_wrap(ctx,"Manual browse supports trigger-to-button, wheel pedals, joysticks, and macro pads.");}else if(a->controls_tab==1){nk_layout_row_dynamic(ctx,50,1);card(ctx,"south","Button South","idle",NULL,0);card(ctx,"axis-x","Axis Left X","+0.04",NULL,0);card(ctx,"trigger","Axis Right Trigger","0.00",NULL,0);}else{nk_size progress=(nk_size)a->sensitivity;nk_layout_row_dynamic(ctx,30,1);nk_progress(ctx,&progress,100,NK_FIXED);card(ctx,"response-device","Device","Xbox Wireless Controller",NULL,0);card(ctx,"curve","Curve","Smooth",NULL,0);}nk_group_end(ctx);}nk_layout_row_end(ctx);
}

static void progress_view(struct nk_context *ctx){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / PROGRESS");heading(ctx,"Progress");label(ctx,"GAME-PROVIDED PROGRESSION · 3 CAMPAIGNS");nk_layout_row_begin(ctx,NK_DYNAMIC,430,2);nk_layout_row_push(ctx,.5f);if(nk_group_begin(ctx,"campaign-list",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,68,1);card(ctx,"glass","The Glass Caverns","Moss · Temple Depths · 42%","READY",1);card(ctx,"quiet","A Quiet Beginning","Vega · Green Valley · 11%","READY",0);card(ctx,"old","Old Expedition","Moss · Version 0.7 data","INCOMPATIBLE",0);nk_group_end(ctx);}nk_layout_row_push(ctx,.5f);if(nk_group_begin(ctx,"campaign-detail",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,"ASSOCIATED PROFILE");nk_layout_row_dynamic(ctx,54,1);card(ctx,"owner","MO  Moss","Ownership recorded by provider",NULL,0);label(ctx,"RECORDED MOD SET · 7 PACKAGES");card(ctx,"base","Base Content","Required package","v1.4.0",0);card(ctx,"desk","Cartographer's Desk","Quest dependency","v0.8.2",0);card(ctx,"old-lanterns","Old Lanterns","Installed package","UPDATE",0);label(ctx,"CHECKPOINT HISTORY");card(ctx,"safe","Temple safe room","Today, 06:52 · Stage 4","RESUME",0);card(ctx,"flooded","Flooded archive","Yesterday · Stage 3","BACKUP",0);nk_layout_row_dynamic(ctx,36,2);nk_button_label(ctx,"> Resume campaign");nk_button_label(ctx,"Export");nk_group_end(ctx);}nk_layout_row_end(ctx);
}

static void mods(struct nk_context *ctx,struct app *a){
    nk_layout_row_dynamic(ctx,18,1);label(ctx,"SPLONKS / MODS");heading(ctx,"Mods");const char *t[]={"Installed","Browse catalog"};tabs(ctx,t,2,&a->mods_tab);nk_layout_row_begin(ctx,NK_DYNAMIC,36,3);nk_layout_row_push(ctx,.58f);static char search[96];static int len=0;nk_edit_string(ctx,NK_EDIT_FIELD,search,&len,95,nk_filter_default);nk_layout_row_push(ctx,.25f);nk_checkbox_label(ctx,"Compatible only",&a->compatible_only);nk_layout_row_push(ctx,.17f);nk_button_label(ctx,"Refresh");nk_layout_row_end(ctx);
    const char *names[]={"Mycelium Below","Brassline Grapple Kit","Skybreak Caverns","Abyssal Tide","Old Lanterns","Pocket Expedition","Temple Weather","Mirror Depths","Clockwork Orchard","Lantern Cartography","Fungal Friends","Deep Relay","Cave Radio","Run History+","Accessible Traps","Quiet Ghost","Shared Wallet","Daily Seed Lab","Rope Physics","Vanilla Plus"};nk_layout_row_begin(ctx,NK_DYNAMIC,390,2);nk_layout_row_push(ctx,.5f);if(nk_group_begin(ctx,"mod-list",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,42,1);for(int i=0;i<20;i++){int on=a->selected==i;if(nk_selectable_label(ctx,names[i],NK_TEXT_LEFT,&on))a->selected=i;}nk_group_end(ctx);}nk_layout_row_push(ctx,.5f);if(nk_group_begin(ctx,"mod-detail",NK_WINDOW_BORDER)){nk_layout_row_dynamic(ctx,18,1);label(ctx,a->mods_tab?"CATALOG ENTRY":"INSTALLED PACKAGE");heading(ctx,names[a->selected]);nk_layout_row_dynamic(ctx,46,1);nk_label_wrap(ctx,"A substantial package with integrated rooms, mechanics, artwork, and co-op synchronization.");label(ctx,"COMPATIBILITY & DEPENDENCIES");nk_layout_row_dynamic(ctx,48,1);card(ctx,"dep-base","Base Content ≥ 1.4.0","Core dependency","INSTALLED",0);card(ctx,"dep-river","Underground Rivers ≥ 2.2.0","Automatic dependency","WILL INSTALL",0);label(ctx,"REQUIRED BY");card(ctx,"required-temple","Temple Weather","Dependent package","ACTIVE",0);card(ctx,"required-pocket","Pocket Expedition","Dependent package","ACTIVE",0);nk_layout_row_dynamic(ctx,36,2);nk_button_label(ctx,a->mods_tab?"Install & add to session":"Update");nk_button_label(ctx,a->mods_tab?"Install only":"Open files");nk_group_end(ctx);}nk_layout_row_end(ctx);
}

static void draw_ui(struct nk_context *ctx,struct app *a,int w,int h){
    if(nk_begin(ctx,"Gubsy shell",nk_rect(0,0,(float)w,(float)h),NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BACKGROUND)){header(ctx,w);const int compact=w<900||h<620;if(!compact){nk_layout_row_begin(ctx,NK_STATIC,h-66,2);nk_layout_row_push(ctx,230);nav(ctx,a,230,(float)h);nk_layout_row_push(ctx,w-250);}else nk_layout_row_dynamic(ctx,h-118,1);
        if(nk_group_begin(ctx,"content",NK_WINDOW_NO_SCROLLBAR)){switch(a->screen){case 0:play(ctx,a);break;case 1:players(ctx,a);break;case 2:settings(ctx,a);break;case 3:controls(ctx,a);break;case 4:progress_view(ctx);break;case 5:mods(ctx,a);break;}nk_group_end(ctx);}if(!compact)nk_layout_row_end(ctx);else{const char *n[]={"Play","Players","Settings","Controls","Progress","Mods"};nk_layout_row_dynamic(ctx,48,6);for(int i=0;i<6;i++)if(nk_button_label(ctx,n[i]))a->screen=i;}}nk_end(ctx);
}

static void save_capture(const char *path,int w,int h){
    unsigned char *pixels=malloc((size_t)w*h*4);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    SDL_Surface *surface=SDL_CreateRGBSurfaceWithFormatFrom(pixels,w,h,32,w*4,SDL_PIXELFORMAT_RGBA32);
    SDL_Surface *flipped=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
    for(int y=0;y<h;y++)memcpy((unsigned char*)flipped->pixels+y*flipped->pitch,(unsigned char*)surface->pixels+(h-1-y)*surface->pitch,(size_t)w*4);
    SDL_SaveBMP(flipped,path);SDL_FreeSurface(flipped);SDL_FreeSurface(surface);free(pixels);
}

int main(int argc,char **argv){
    struct args args=parse_args(argc,argv);int hidden=args.benchmark>0;SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED,"0");SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_GAMECONTROLLER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    Uint32 flags=SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE|(hidden?SDL_WINDOW_HIDDEN:SDL_WINDOW_SHOWN);SDL_Window *win=SDL_CreateWindow("Gubsy Nuklear trial",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,args.width,args.height,flags);mark_utility(win);if(!hidden)SDL_ShowWindow(win);SDL_GLContext gl=SDL_GL_CreateContext(win);SDL_GL_SetSwapInterval(hidden?0:1);
    struct nk_context *ctx=nk_sdl_init(win);struct nk_font_atlas *atlas;nk_sdl_font_stash_begin(&atlas);struct nk_font *font=nk_font_atlas_add_from_file(atlas,GUBSY_NUKLEAR_FONT_PATH,15,NULL);nk_sdl_font_stash_end();if(font)nk_style_set_font(ctx,&font->handle);style(ctx);
    struct app app={0};app.screen=args.screen;app.join_mode=1;app.mods_tab=1;app.render_scale=100;app.brightness=64;app.master_volume=80;app.music_volume=72;app.sensitivity=45;app.deadzone=12;app.vibration=80;app.trigger_deadzone=5;
    int running=1,frame=0;double build_total=0,render_total=0,frame_total=0;
    while(running){double fs=now_ms();SDL_Event ev;nk_input_begin(ctx);while(SDL_PollEvent(&ev)){if(ev.type==SDL_QUIT)running=0;if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym>=SDLK_F1&&ev.key.keysym.sym<=SDLK_F6)app.screen=ev.key.keysym.sym-SDLK_F1;nk_sdl_handle_event(&ev);}nk_input_end(ctx);int w,h;SDL_GetWindowSize(win,&w,&h);double bs=now_ms();draw_ui(ctx,&app,w,h);build_total+=now_ms()-bs;glViewport(0,0,w,h);glClearColor(.008f,.025f,.03f,1);glClear(GL_COLOR_BUFFER_BIT);double rs=now_ms();nk_sdl_render(NK_ANTI_ALIASING_ON,MAX_VERTEX_MEMORY,MAX_ELEMENT_MEMORY);render_total+=now_ms()-rs;SDL_GL_SwapWindow(win);frame_total+=now_ms()-fs;frame++;if(args.capture&&frame==5){save_capture(args.capture,w,h);running=0;}if(args.benchmark>5&&frame>=args.benchmark)running=0;}
    if(args.benchmark>5)printf("backend=nuklear frames=%d viewport=%dx%d ui_build_ms=%.4f render_cpu_ms=%.4f frame_cpu_ms=%.4f rss_kib=%ld\n",frame,args.width,args.height,build_total/frame,render_total/frame,frame_total/frame,rss_kib());
    nk_sdl_shutdown();SDL_GL_DeleteContext(gl);SDL_DestroyWindow(win);SDL_Quit();return 0;
}
