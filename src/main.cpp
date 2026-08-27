#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <unistd.h>

void MarkWindowUtility(const char* title);

namespace {

constexpr Color kBg{8, 18, 21, 255};
constexpr Color kPanel{12, 31, 34, 255};
constexpr Color kPanel2{18, 43, 43, 255};
constexpr Color kLine{48, 78, 78, 255};
constexpr Color kText{224, 234, 232, 255};
constexpr Color kMuted{145, 164, 161, 255};
constexpr Color kAccent{151, 239, 116, 255};
constexpr Color kCyan{91, 213, 224, 255};
constexpr Color kDanger{244, 103, 103, 255};

enum class Screen { Play, Players, Settings, Controls, Progress, Mods };

struct App {
    Screen screen = Screen::Play;
    int play_activity = 0;
    int host_mode = 0;
    int join_mode = 1;
    bool pause_preview = false;
    bool friendly_fire = false;
    bool shared_treasury = true;
    bool discovered_shortcuts = true;
    float shared_lives = 4.0f;
    float starting_health = 4.0f;
    float ghost_arrival = 180.0f;
    int players_tab = 0;
    int settings_tab = 0;
    int controls_tab = 0;
    int mods_tab = 1;
    bool fullscreen = false;
    int resolution = 1;
    int frame_cap = 2;
    float render_scale = 100.0f;
    float brightness = 64.0f;
    float master_volume = 80.0f;
    float music_volume = 72.0f;
    float look_sensitivity = 45.0f;
    float stick_deadzone = 12.0f;
    float vibration = 80.0f;
    float trigger_deadzone = 5.0f;
    bool compatible_only = false;
    bool dropdown_open = false;
    int open_dropdown = -1;
    Vector2 list_scroll{};
    Vector2 detail_scroll{};
    int selected_row = 0;
    int benchmark_frames = 0;
    int frame_index = 0;
    double ui_ms_total = 0.0;
    std::string capture;
};

struct Args {
    int width = 1280;
    int height = 720;
    int screen = 0;
    int benchmark = 0;
    std::string capture;
};

Args ParseArgs(int argc, char** argv) {
    Args out;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--resolution" && i + 1 < argc) {
            std::sscanf(argv[++i], "%dx%d", &out.width, &out.height);
        } else if (arg == "--screen" && i + 1 < argc) {
            const std::string name = argv[++i];
            const std::array names{"play", "players", "settings", "controls", "progress", "mods"};
            auto it = std::find(names.begin(), names.end(), name);
            out.screen = it == names.end() ? std::atoi(name.c_str()) : int(it - names.begin());
        } else if (arg == "--benchmark" && i + 1 < argc) {
            out.benchmark = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--capture" && i + 1 < argc) {
            out.capture = argv[++i];
        }
    }
    return out;
}

long RssKiB() {
    FILE* file = std::fopen("/proc/self/statm", "r");
    long total = 0, resident = 0;
    if (file) {
        if (std::fscanf(file, "%ld %ld", &total, &resident) != 2) resident = 0;
        std::fclose(file);
    }
    return resident * long(sysconf(_SC_PAGESIZE)) / 1024;
}

Rectangle Inset(Rectangle r, float v) { return {r.x + v, r.y + v, r.width - v * 2, r.height - v * 2}; }

void Text(const char* text, float x, float y, int size = 16, Color color = kText) {
    DrawText(text, int(x), int(y), size, color);
}

void Label(const char* text, float x, float y) { Text(text, x, y, 11, kAccent); }

void Panel(Rectangle r) {
    DrawRectangleRec(r, kPanel);
    DrawRectangleLinesEx(r, 1, kLine);
}

bool Button(Rectangle r, const char* label, bool primary = false) {
    if (primary) {
        DrawRectangleRec(r, kAccent);
        DrawRectangleLinesEx(r, 1, kAccent);
        const int width = MeasureText(label, 14);
        Text(label, r.x + (r.width - width) / 2, r.y + (r.height - 14) / 2, 14, Color{9, 25, 20, 255});
        return IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
    }
    return GuiButton(r, label) != 0;
}

bool NavItem(Rectangle r, const char* icon, const char* title, const char* subtitle, bool active) {
    if (active) {
        DrawRectangleRec(r, Color{23, 55, 39, 255});
        DrawRectangle(int(r.x), int(r.y), 3, int(r.height), kAccent);
    } else if (CheckCollisionPointRec(GetMousePosition(), r)) {
        DrawRectangleRec(r, kPanel2);
    }
    Text(icon, r.x + 16, r.y + 18, 15, active ? kAccent : kCyan);
    Text(title, r.x + 45, r.y + 10, 17);
    Text(subtitle, r.x + 45, r.y + 32, 11, kMuted);
    return IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r);
}

void Header(float width) {
    DrawRectangle(0, 0, int(width), 58, Color{4, 16, 18, 255});
    DrawLine(0, 58, int(width), 58, kLine);
    DrawRectangle(18, 15, 30, 30, kAccent);
    Text("G", 29, 23, 16, Color{9, 25, 20, 255});
    Text("G U B S Y   S H E L L", 60, 14, 13);
    Text("raygui native utility trial", 60, 32, 10, kMuted);
    Text("*  OFFLINE     |     VEGA", width * 0.55f, 22, 13);
    Text("NATIVE VIEWPORT", width - 145, 12, 10, kMuted);
    Text(TextFormat("%d x %d", GetScreenWidth(), GetScreenHeight()), width - 106, 29, 11, kCyan);
}

void BottomNav(App& app, float width, float height) {
    static constexpr std::array labels{"Play", "Players", "Settings", "Controls", "Progress", "Mods"};
    const float h = 58;
    DrawRectangle(0, int(height - h), int(width), int(h), Color{4, 16, 18, 255});
    const float item = width / labels.size();
    for (int i = 0; i < int(labels.size()); ++i) {
        Rectangle r{item * i, height - h, item, h};
        if (int(app.screen) == i) DrawRectangleRec(r, Color{23, 55, 39, 255});
        Text(labels[i], r.x + (r.width - MeasureText(labels[i], 12)) / 2, r.y + 23, 12,
             int(app.screen) == i ? kAccent : kText);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) app.screen = Screen(i);
    }
}

void SideNav(App& app, Rectangle area) {
    DrawRectangleRec(area, Color{5, 19, 21, 255});
    DrawLine(int(area.x + area.width), int(area.y), int(area.x + area.width), int(area.y + area.height), kLine);
    DrawRectangle(int(area.x + 22), int(area.y + 24), 36, 36, Color{13, 45, 51, 255});
    Text("VE", area.x + 31, area.y + 36, 12, kCyan);
    Text("ACTIVE PROFILE", area.x + 70, area.y + 25, 10, kMuted);
    Text("Vega", area.x + 70, area.y + 42, 14);
    DrawLine(int(area.x + 16), int(area.y + 78), int(area.x + area.width - 16), int(area.y + 78), kLine);
    static constexpr std::array titles{"Play", "Players", "Settings", "Controls", "Progress", "Mods"};
    static constexpr std::array subtitles{"Continue or start", "Profiles & devices", "Game preferences", "Bindings & input", "Campaigns & checkpoints", "Installed content"};
    static constexpr std::array icons{">", "+", "*", "~", "#", "o"};
    for (int i = 0; i < 6; ++i) {
        Rectangle r{area.x + 10, area.y + 92 + i * 60.0f, area.width - 20, 54};
        if (NavItem(r, icons[i], titles[i], subtitles[i], int(app.screen) == i)) app.screen = Screen(i);
    }
    Text("x", area.x + 25, area.y + area.height - 48, 15, kDanger);
    Text("Quit", area.x + 52, area.y + area.height - 53, 16);
    Text("Return to desktop", area.x + 52, area.y + area.height - 32, 10, kMuted);
}

int Tabs(Rectangle r, const std::vector<const char*>& labels, int current) {
    float x = r.x;
    for (int i = 0; i < int(labels.size()); ++i) {
        const float w = std::max(84.0f, float(MeasureText(labels[i], 13) + 28));
        Rectangle tab{x, r.y, w, r.height};
        if (i == current) {
            DrawRectangleRec(tab, kPanel2);
            DrawRectangle(int(tab.x), int(tab.y + tab.height - 2), int(tab.width), 2, kAccent);
        }
        Text(labels[i], tab.x + 14, tab.y + 13, 13, i == current ? kText : kMuted);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), tab)) current = i;
        x += w;
    }
    DrawLine(int(r.x), int(r.y + r.height), int(r.x + r.width), int(r.y + r.height), kLine);
    return current;
}

void Row(Rectangle r, const char* title, const char* sub, const char* value = nullptr, bool selected = false) {
    if (selected) DrawRectangleRec(r, Color{21, 54, 40, 255});
    DrawLine(int(r.x), int(r.y + r.height), int(r.x + r.width), int(r.y + r.height), kLine);
    Text(title, r.x + 12, r.y + 11, 15);
    Text(sub, r.x + 12, r.y + 31, 10, kMuted);
    if (value) Text(value, r.x + r.width - MeasureText(value, 11) - 12, r.y + 18, 11, kAccent);
}

bool Select(App& app, int id, Rectangle r, const char* options, int* active) {
    bool edit = app.dropdown_open && app.open_dropdown == id;
    if (GuiDropdownBox(r, options, active, edit)) {
        if (edit) { app.dropdown_open = false; app.open_dropdown = -1; }
        else { app.dropdown_open = true; app.open_dropdown = id; }
        return true;
    }
    return false;
}

void SliderRow(Rectangle r, const char* title, const char* sub, float* value, float min, float max, const char* suffix) {
    Row(r, title, sub);
    Rectangle slider{r.x + r.width * .48f, r.y + 20, r.width * .34f, 18};
    GuiSliderBar(slider, nullptr, nullptr, value, min, max);
    Text(TextFormat("%.0f%s", *value, suffix), r.x + r.width - 58, r.y + 21, 11, kAccent);
}

void Play(App& app, Rectangle body) {
    Label("SPLONKS / PLAY", body.x, body.y);
    Text("Play", body.x, body.y + 18, 42);
    Rectangle left{body.x, body.y + 72, body.width * .64f, body.height - 72};
    Rectangle right{left.x + left.width + 12, left.y, body.width - left.width - 12, left.height};
    Panel(left); Panel(right);
    Label(app.play_activity == 0 ? "CONTINUE QUEST" : "NEW EXPEDITION", left.x + 18, left.y + 15);
    Text(app.play_activity == 0 ? "The Violet Reach" : "Choose a quest", left.x + 18, left.y + 36, 22);
    Text(app.play_activity == 0 ? "The Glass Caverns · Latest checkpoint · Vega" : "Start from stage one with a clean checkpoint history", left.x + 18, left.y + 64, 11, kMuted);
    float y = left.y + 92;
    Row({left.x + 12, y, left.width - 24, 56}, "Activity", "Continue a checkpoint or start another quest");
    Select(app, 1, {left.x + left.width * .55f, y + 10, left.width * .4f, 36}, "Continue expedition;New expedition", &app.play_activity);
    y += 58;
    Row({left.x + 12, y, left.width - 24, 56}, app.play_activity == 0 ? "Resume point" : "Quest", app.play_activity == 0 ? "Latest checkpoint · The Violet Reach" : "The Violet Reach · 8 stages · normal route", app.play_activity == 0 ? "CHOOSE CHECKPOINT ›" : "CHOOSE QUEST ›");
    y += 58;
    Row({left.x + 12, y, left.width - 24, 56}, "Play with", "Who may occupy the remaining slots");
    Select(app, 2, {left.x + left.width * .55f, y + 10, left.width * .4f, 36}, "Solo;Friends can join;Public", &app.join_mode);
    y += 58;
    Row({left.x + 12, y, left.width - 24, 56}, "Host using", "Automatic chooses direct or relay hosting");
    Select(app, 3, {left.x + left.width * .55f, y + 10, left.width * .4f, 36}, "Automatic;Direct;Relay", &app.host_mode);
    y += 58;
    Row({left.x + 12, y, left.width - 24, 52}, "Expedition rules", "Standard · 4 lives · ghost at 180s", "EDIT ALL ›"); y += 54;
    Row({left.x + 12, y, left.width - 24, 52}, "Session mods", "7 active · dependency set valid", "MANAGE ›");
    Button({left.x + 16, left.y + left.height - 53, 150, 38}, "Pause preview");
    Button({left.x + left.width - 265, left.y + left.height - 53, 245, 38}, app.play_activity == 0 ? "> Resume latest checkpoint" : "> Begin expedition", true);

    Label("PLAYERS", right.x + 16, right.y + 14);
    Text(app.join_mode == 0 ? "Solo player" : "Your party", right.x + 16, right.y + 34, 18);
    Row({right.x + 12, right.y + 66, right.width - 24, 52}, "P1   Moss", "Xbox Wireless Controller", "READY");
    if (app.join_mode != 0) {
        Row({right.x + 12, right.y + 120, right.width - 24, 52}, "+   Open slot", "Invite a friend or add locally");
        Row({right.x + 12, right.y + 174, right.width - 24, 52}, "+   Open slot", "Invite a friend or add locally");
    }
    Button({right.x + 14, right.y + right.height - 96, right.width - 28, 34}, "Invite / copy link");
    Text("CONTENT", right.x + 16, right.y + right.height - 47, 10, kMuted);
    Text("7 mods", right.x + right.width - 64, right.y + right.height - 47, 10);
    Text("NETWORK", right.x + 16, right.y + right.height - 29, 10, kMuted);
    Text(app.host_mode == 0 ? "Automatic" : app.host_mode == 1 ? "Direct" : "Relay", right.x + right.width - 70, right.y + right.height - 29, 10);
}

void Players(App& app, Rectangle body) {
    Label("SPLONKS / PLAYERS", body.x, body.y); Text("Players", body.x, body.y + 18, 42);
    app.players_tab = Tabs({body.x, body.y + 72, body.width, 44}, {"Local players", "Profiles", "Devices"}, app.players_tab);
    Rectangle panel{body.x, body.y + 128, body.width, body.height - 128}; Panel(panel);
    if (app.players_tab == 0) {
        Label("LOCAL PARTY", panel.x + 18, panel.y + 16); Text("Assign devices and profiles", panel.x + 18, panel.y + 36, 20);
        Row({panel.x + 16, panel.y + 72, panel.width * .56f, 72}, "P1  Moss", "Profile: Moss · Xbox Wireless Controller", "READY", true);
        Row({panel.x + 16, panel.y + 146, panel.width * .56f, 72}, "P2  Open slot", "Add a local player or invite a friend");
        Rectangle d{panel.x + panel.width * .6f, panel.y + 72, panel.width * .37f, panel.height - 94}; Panel(d);
        Label("SELECTED PLAYER", d.x + 16, d.y + 14); Text("Moss", d.x + 16, d.y + 35, 25);
        Text("Associated devices", d.x + 16, d.y + 78, 12, kMuted);
        Row({d.x + 12, d.y + 96, d.width - 24, 55}, "Xbox Wireless Controller", "Gamepad 0 · connected", "ASSIGNED");
        Row({d.x + 12, d.y + 153, d.width - 24, 55}, "Keyboard + mouse", "Shared desktop input", "AVAILABLE");
        Button({d.x + 14, d.y + d.height - 50, 135, 34}, "Assign device", true); Button({d.x + 157, d.y + d.height - 50, 130, 34}, "Mark not ready");
    } else if (app.players_tab == 1) {
        Label("PROFILE LIBRARY", panel.x + 18, panel.y + 16); Text("3 profiles · profile data is not a save", panel.x + 18, panel.y + 34, 11, kMuted);
        Button({panel.x + panel.width - 126, panel.y + 16, 108, 34}, "+ New profile", true);
        const char* names[] = {"Moss", "Vega", "Guest"}; const char* stats[] = {"38h 22m · 84 runs · 21 wins", "14h 11m · 31 runs · 4 wins", "Never · 0 runs · 0 wins"};
        for (int i=0;i<3;i++) { Rectangle c{panel.x + 16 + i*(panel.width-44)/3, panel.y+70, (panel.width-56)/3, 128}; Panel(c); Text(names[i], c.x+18,c.y+18,20); Text(stats[i],c.x+18,c.y+48,11,kMuted); Text(i==0?"ACTIVE":"SELECT",c.x+18,c.y+88,11,kAccent); }
        Text("Moss's history", panel.x + 18, panel.y + 226, 23);
        const char* labels[] = {"84\nPLAYS","21\nWINS","138\nDEATHS","2.4M\nSCORE","42\nREPLAYS"};
        for(int i=0;i<5;i++) Text(labels[i], panel.x+40+i*panel.width/5, panel.y+276, 16, i==0?kAccent:kText);
    } else {
        Label("CONNECTED INPUT", panel.x + 18, panel.y + 16); Text("Devices may belong to more than one local player", panel.x + 18, panel.y + 34, 11, kMuted);
        Row({panel.x+16,panel.y+68,panel.width*.55f,66},"Xbox Wireless Controller","Gamepad 0 · battery 82%","MOSS",true);
        Row({panel.x+16,panel.y+136,panel.width*.55f,66},"Keyboard + mouse","Desktop aggregate","AVAILABLE");
        Row({panel.x+16,panel.y+204,panel.width*.55f,66},"T.16000M Joystick","14 axes · 32 buttons","UNASSIGNED");
        Rectangle d{panel.x+panel.width*.59f,panel.y+68,panel.width*.38f,panel.height-90}; Panel(d);
        Label("LIVE INPUT EXPLORER",d.x+16,d.y+14); Text("Xbox Wireless Controller",d.x+16,d.y+35,20);
        Text("A / Button South       idle\nLeft X                  +0.04\nRight Trigger           0.00\nD-pad                   centered",d.x+16,d.y+78,12,kMuted);
    }
}

void Settings(App& app, Rectangle body) {
    Label("SPLONKS / SETTINGS", body.x, body.y); Text("Settings", body.x, body.y + 18, 42);
    app.settings_tab = Tabs({body.x,body.y+72,body.width,44},{"Display","Audio","Accessibility","Gameplay"},app.settings_tab);
    Rectangle left{body.x,body.y+128,body.width*.51f,body.height-128}; Rectangle right{left.x+left.width+12,left.y,body.width-left.width-12,left.height}; Panel(left);Panel(right);
    float y=left.y+18;
    if(app.settings_tab==0){
        Row({left.x+12,y,left.width-24,60},"Fullscreen","Use the entire selected display",app.fullscreen?"ON":"OFF"); if(GuiToggle({left.x+left.width-98,y+15,66,28},"",&app.fullscreen)){} y+=62;
        Row({left.x+12,y,left.width-24,60},"Display resolution","Output resolution for this display"); Select(app,10,{left.x+left.width*.52f,y+12,left.width*.42f,36},"1280 × 720;1920 × 1080;2560 × 1440",&app.resolution); y+=62;
        SliderRow({left.x+12,y,left.width-24,66},"Render scale","Internal 3D resolution",&app.render_scale,50,150,"%");y+=68;
        Row({left.x+12,y,left.width-24,60},"Frame cap","Maximum simulation frames per second");Select(app,11,{left.x+left.width*.52f,y+12,left.width*.42f,36},"60 FPS;120 FPS;144 FPS;Unlimited",&app.frame_cap);y+=62;
        SliderRow({left.x+12,y,left.width-24,66},"Brightness","Fine-tune scene visibility",&app.brightness,0,100,"%");
    } else if(app.settings_tab==1){ SliderRow({left.x+12,y,left.width-24,72},"Master volume","Overall output level",&app.master_volume,0,100,"%"); y+=74; SliderRow({left.x+12,y,left.width-24,72},"Music volume","Score and ambient layers",&app.music_volume,0,100,"%"); y+=74; Row({left.x+12,y,left.width-24,62},"Dynamic range","Compress loud and quiet sounds"); int range=1; Select(app,12,{left.x+left.width*.52f,y+12,left.width*.42f,36},"Night;Balanced;Wide",&range); }
    else if(app.settings_tab==2){ Row({left.x+12,y,left.width-24,64},"Subtitles","Dialogue and important callouts","ON",true);y+=66;Row({left.x+12,y,left.width-24,64},"Color distinction","Shape and label reinforcement","ON");y+=66;Row({left.x+12,y,left.width-24,64},"Reduced motion","Limit camera and menu animation","OFF");y+=66;Row({left.x+12,y,left.width-24,64},"Text scale","Interface and subtitle size","110%"); }
    else { Row({left.x+12,y,left.width-24,64},"Pause when unfocused","Suspend local simulation","ON",true);y+=66;Row({left.x+12,y,left.width-24,64},"Tutorial prompts","Context-sensitive hints","ON");y+=66;Row({left.x+12,y,left.width-24,64},"Damage numbers","Combat feedback","OFF"); }
    Label("SELECTED SETTING",right.x+18,right.y+16); Text(app.settings_tab==0?"Render scale":app.settings_tab==1?"Master volume":app.settings_tab==2?"Subtitles":"Pause when unfocused",right.x+18,right.y+38,26);
    Text("This value is stored in the active profile. The game provides the\nsetting schema; Gubsy provides stable native controls and serialization.",right.x+18,right.y+79,12,kMuted);
    Panel({right.x+18,right.y+130,right.width-36,80}); Label("CURRENT VALUE",right.x+32,right.y+146); Text(app.settings_tab==0?TextFormat("%.0f%%",app.render_scale):app.settings_tab==1?TextFormat("%.0f%%",app.master_volume):"Enabled",right.x+32,right.y+170,25,kAccent);
    Label("SETTING CONTEXT",right.x+18,right.y+242); Row({right.x+14,right.y+260,right.width-28,44},"Scope","Current machine");Row({right.x+14,right.y+306,right.width-28,44},"Profile","Vega");Row({right.x+14,right.y+352,right.width-28,44},"Restart","Not required");
    Button({right.x+18,right.y+right.height-50,180,34},"Apply settings",true); Button({right.x+206,right.y+right.height-50,140,34},"Reset category");
}

void Controls(App& app, Rectangle body) {
    Label("SPLONKS / CONTROLS",body.x,body.y);Text("Controls",body.x,body.y+18,42);
    app.controls_tab=Tabs({body.x,body.y+72,body.width,44},{"Bindings","Devices","Input tuning"},app.controls_tab);
    Rectangle left{body.x,body.y+128,body.width*.5f,body.height-128};Rectangle right{left.x+left.width+12,left.y,body.width-left.width-12,left.height};Panel(left);Panel(right);
    if(app.controls_tab==0){
        GuiTextBox({left.x+14,left.y+14,left.width-28,36},const_cast<char*>("Filter actions..."),64,false);
        const char* actions[]={"Menu Up","Menu Down","Menu Left","Menu Right","Activate","Cancel","Move","Look"}; const char* binds[]={"D-Pad Up","D-Pad Down","D-Pad Left","D-Pad Right","Gamepad A","Gamepad B","Left Stick","Right Stick"};
        for(int i=0;i<8;i++){Rectangle r{left.x+14,left.y+62+i*46,left.width-28,44};Row(r,actions[i],i<6?"Button action":"Analog action",binds[i],i==app.selected_row);if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),r))app.selected_row=i;}
        Label("SELECTED ACTION",right.x+18,right.y+16);Text(actions[app.selected_row],right.x+18,right.y+38,26);Text("Each action may have multiple bindings, including transformed axes.",right.x+18,right.y+76,12,kMuted);
        Row({right.x+14,right.y+108,right.width-28,54},"1   Gamepad binding",binds[app.selected_row],"REPLACE");Row({right.x+14,right.y+164,right.width-28,54},"2   Keyboard binding","Keyboard W / arrow key","REPLACE");
        Button({right.x+18,right.y+235,130,36},"Listen for input",true);Button({right.x+156,right.y+235,138,36},"Browse controls");
        Text("Manual browse supports unusual mappings such as trigger-to-button,\nwheel pedals, joysticks, and macro pads.",right.x+18,right.y+292,11,kMuted);
    } else if(app.controls_tab==1){
        Label("DEVICE ASSIGNMENT",left.x+16,left.y+16);Row({left.x+14,left.y+50,left.width-28,66},"Xbox Wireless Controller","Gamepad 0 · connected","MOSS",true);Row({left.x+14,left.y+118,left.width-28,66},"Keyboard + mouse","Desktop aggregate","AVAILABLE");Row({left.x+14,left.y+186,left.width-28,66},"T.16000M Joystick","14 axes · 32 buttons","MOSS");
        Label("INPUT EXPLORER",right.x+18,right.y+16);Text("Press controls to identify Gubsy input IDs",right.x+18,right.y+38,12,kMuted);Panel({right.x+18,right.y+76,right.width-36,190});Text("Button South             idle\nButton East              idle\nAxis Left X              +0.04\nAxis Right Trigger       0.00\nHat 0                    centered",right.x+34,right.y+96,13);
    } else {
        Label("INPUT PROFILE",left.x+16,left.y+16);Text("Standard · Xbox Wireless Controller",left.x+16,left.y+34,11,kMuted);float y=left.y+70;SliderRow({left.x+12,y,left.width-24,66},"Look sensitivity","Horizontal and vertical camera speed",&app.look_sensitivity,0,100,"%");y+=68;SliderRow({left.x+12,y,left.width-24,66},"Stick deadzone","Ignore small movement near center",&app.stick_deadzone,0,40,"%");y+=68;SliderRow({left.x+12,y,left.width-24,66},"Vibration strength","Controller rumble output",&app.vibration,0,100,"%");y+=68;SliderRow({left.x+12,y,left.width-24,66},"Trigger deadzone","Minimum trigger travel",&app.trigger_deadzone,0,30,"%");
        Label("DEVICE RESPONSE",right.x+18,right.y+16);Text("Live response",right.x+18,right.y+38,24);DrawCircleLines(int(right.x+right.width/2),int(right.y+190),86,kLine);DrawCircle(int(right.x+right.width/2+25),int(right.y+190-14),13,kAccent);Text("Raw and qualified output are shown together.",right.x+18,right.y+300,11,kMuted);
    }
}

void Progress(App& app, Rectangle body) {
    (void)app;Label("SPLONKS / PROGRESS",body.x,body.y);Text("Progress",body.x,body.y+18,42);Label("GAME-PROVIDED PROGRESSION",body.x,body.y+75);Text("3 campaigns · automatic checkpoints",body.x,body.y+92,11,kMuted);
    Rectangle left{body.x,body.y+122,body.width*.5f,body.height-122};Rectangle right{left.x+left.width+12,left.y,body.width-left.width-12,left.height};Panel(left);Panel(right);
    Row({left.x+14,left.y+26,left.width-28,66},"The Glass Caverns","Moss · Temple Depths · 42%","12h 48m  READY",true);Row({left.x+14,left.y+94,left.width-28,66},"A Quiet Beginning","Vega · Green Valley · 11%","3h 06m  READY");Row({left.x+14,left.y+162,left.width-28,66},"Old Expedition","Moss · Version 0.7 data","INCOMPATIBLE");
    Label("ASSOCIATED PROFILE",right.x+16,right.y+16);Row({right.x+14,right.y+38,right.width-28,56},"MO   Moss","Ownership recorded by progression provider");Label("RECORDED MOD SET · 7 PACKAGES",right.x+16,right.y+112);Row({right.x+14,right.y+130,right.width-28,38},"Base Content","Required package","v1.4.0");Row({right.x+14,right.y+170,right.width-28,38},"Cartographer's Desk","Quest dependency","v0.8.2");Row({right.x+14,right.y+210,right.width-28,38},"Old Lanterns","Installed package","UPDATE");Label("CHECKPOINT HISTORY",right.x+16,right.y+270);Row({right.x+14,right.y+288,right.width-28,48},"Temple safe room","Today, 06:52 · Stage 4","RESUME");Row({right.x+14,right.y+338,right.width-28,48},"Flooded archive","Yesterday · Stage 3","BACKUP");
    Button({right.x+right.width-270,right.y+right.height-48,155,34},"> Resume campaign",true);Button({right.x+right.width-108,right.y+right.height-48,92,34},"Export");
}

void Mods(App& app, Rectangle body) {
    Label("SPLONKS / MODS",body.x,body.y);Text("Mods",body.x,body.y+18,42);app.mods_tab=Tabs({body.x,body.y+72,body.width,44},{"Installed","Browse catalog"},app.mods_tab);
    Rectangle search{body.x,body.y+128,body.width*.64f,38};GuiTextBox(search,const_cast<char*>("Search the Gubsy mod catalog..."),64,false);GuiCheckBox({body.x+body.width*.66f,body.y+135,22,22},"Compatible only",&app.compatible_only);Button({body.x+body.width-100,body.y+128,100,38},"↻ Refresh");
    Rectangle left{body.x,body.y+180,body.width*.5f,body.height-180};Rectangle right{left.x+left.width+12,left.y,body.width-left.width-12,left.height};Panel(left);Panel(right);
    const char* names[]={"Mycelium Below","Brassline Grapple Kit","Skybreak Caverns","Abyssal Tide","Old Lanterns","Pocket Expedition","Temple Weather","Mirror Depths"};const char* deps[]={"Requires Base Content","Requires Cartographer's Desk","Requires game 1.4","Requires Underground Rivers","Update available","2 dependencies","Required by 2 mods","Requires Gubsy Mod API 0.2"};
    for(int i=0;i<8;i++){Rectangle r{left.x+14,left.y+18+i*48,left.width-28,46};Row(r,names[i],deps[i],i==7?"INCOMPATIBLE":i==4?"UPDATE":"✓ 93%",i==app.selected_row);if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),r))app.selected_row=i;}
    Label(app.mods_tab?"CATALOG ENTRY":"INSTALLED PACKAGE",right.x+18,right.y+16);Text(names[app.selected_row],right.x+18,right.y+38,25);Text("A substantial content package with carefully integrated rooms, mechanics,\nartwork, and native co-op synchronization.",right.x+18,right.y+76,11,kMuted);
    Label("COMPATIBILITY & DEPENDENCIES",right.x+18,right.y+122);Row({right.x+14,right.y+140,right.width-28,42},"↳ Base Content ≥ 1.4.0","Core dependency","INSTALLED");Row({right.x+14,right.y+184,right.width-28,42},"↳ Underground Rivers ≥ 2.2.0","Automatic dependency","WILL INSTALL");Label("REQUIRED BY",right.x+18,right.y+246);Row({right.x+14,right.y+264,right.width-28,42},"↑ Temple Weather","Dependent package","ACTIVE");Row({right.x+14,right.y+308,right.width-28,42},"↑ Pocket Expedition","Dependent package","ACTIVE");
    Text(app.selected_row==7?"This mod cannot be installed on the current game/API version.":"The full dependency change plan is shown before mutation.",right.x+18,right.y+374,11,app.selected_row==7?kDanger:kMuted);
    Button({right.x+18,right.y+right.height-48,220,34},app.mods_tab?"Install & add to session":"Update",true);Button({right.x+246,right.y+right.height-48,110,34},app.mods_tab?"Install only":"Open files");
}

void DrawApp(App& app) {
    const float w=float(GetScreenWidth()), h=float(GetScreenHeight());
    ClearBackground(kBg); Header(w);
    const bool compact = w < 900 || h < 620;
    Rectangle body;
    if(compact){ BottomNav(app,w,h); body={20,76,w-40,h-150}; }
    else { const float nav=std::clamp(w*.195f,220.0f,280.0f); SideNav(app,{0,58,nav,h-58}); body={nav+42,84,w-nav-68,h-112}; }
    switch(app.screen){case Screen::Play:Play(app,body);break;case Screen::Players:Players(app,body);break;case Screen::Settings:Settings(app,body);break;case Screen::Controls:Controls(app,body);break;case Screen::Progress:Progress(app,body);break;case Screen::Mods:Mods(app,body);break;}
}

} // namespace

int main(int argc, char** argv) {
    const Args args=ParseArgs(argc,argv);
    unsigned int window_flags = FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIDDEN;
    if (args.benchmark == 0 && args.capture.empty()) window_flags |= FLAG_VSYNC_HINT;
    SetConfigFlags(window_flags);
    InitWindow(args.width,args.height,"Gubsy raygui trial");
    MarkWindowUtility("Gubsy raygui trial");
    if (args.benchmark == 0 && args.capture.empty()) {
        const int monitor = 0;
        const Vector2 origin = GetMonitorPosition(monitor);
        SetWindowPosition(int(origin.x) + (GetMonitorWidth(monitor) - args.width) / 2,
                          int(origin.y) + (GetMonitorHeight(monitor) - args.height) / 2);
        ClearWindowState(FLAG_WINDOW_HIDDEN);
    }
    SetWindowMinSize(720,480);
    SetTargetFPS(args.benchmark > 0 ? 0 : 144);
    GuiSetStyle(DEFAULT,TEXT_SIZE,args.height>=900?16:13);
    GuiSetStyle(DEFAULT,BACKGROUND_COLOR,ColorToInt(kPanel));
    GuiSetStyle(DEFAULT,BASE_COLOR_NORMAL,ColorToInt(kPanel2));
    GuiSetStyle(DEFAULT,BORDER_COLOR_NORMAL,ColorToInt(kLine));
    GuiSetStyle(DEFAULT,TEXT_COLOR_NORMAL,ColorToInt(kText));
    GuiSetStyle(DEFAULT,BASE_COLOR_FOCUSED,ColorToInt(Color{27,67,48,255}));
    GuiSetStyle(DEFAULT,BORDER_COLOR_FOCUSED,ColorToInt(kAccent));
    GuiSetStyle(DEFAULT,TEXT_COLOR_FOCUSED,ColorToInt(kText));
    GuiSetStyle(DEFAULT,BASE_COLOR_PRESSED,ColorToInt(kAccent));
    GuiSetStyle(DEFAULT,TEXT_COLOR_PRESSED,ColorToInt(Color{9,25,20,255}));
    App app;app.screen=Screen(std::clamp(args.screen,0,5));app.benchmark_frames=args.benchmark;app.capture=args.capture;
    while(!WindowShouldClose()){
        for(int i=0;i<6;i++) if(IsKeyPressed(KEY_F1+i)) app.screen=Screen(i);
        const auto start=std::chrono::steady_clock::now();
        BeginDrawing();DrawApp(app);EndDrawing();
        const auto stop=std::chrono::steady_clock::now();
        app.ui_ms_total+=std::chrono::duration<double,std::milli>(stop-start).count();app.frame_index++;
        if(!app.capture.empty()&&app.frame_index==3){Image image=LoadImageFromScreen();ExportImage(image,app.capture.c_str());UnloadImage(image);break;}
        if(app.benchmark_frames>0&&app.frame_index>=app.benchmark_frames){
            std::printf("backend=raygui frames=%d viewport=%dx%d ui_and_render_cpu_ms=%.4f fps_equivalent=%.1f rss_kib=%ld\n",app.frame_index,GetScreenWidth(),GetScreenHeight(),app.ui_ms_total/app.frame_index,1000.0/(app.ui_ms_total/app.frame_index),RssKiB());break;
        }
    }
    CloseWindow();return 0;
}
