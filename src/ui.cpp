#include "ui.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>

namespace {

constexpr ImVec4 Accent{0.59f, 0.94f, 0.45f, 1.0f};
constexpr ImVec4 Muted{0.56f, 0.65f, 0.63f, 1.0f};
constexpr ImVec4 Cyan{0.36f, 0.83f, 0.88f, 1.0f};
UiAssets Assets{};
int RowId=0;

void Label(const char* text) { ImGui::TextColored(Accent, "%s", text); }

void Sub(const char* text) { ImGui::TextColored(Muted, "%s", text); }

void Title(const char* text) {
  ImGui::PushFont(nullptr, 30.0f);
  ImGui::TextUnformatted(text);
  ImGui::PopFont();
}

bool PrimaryButton(const char* label, ImVec2 size = {0, 0}) {
  ImGui::PushStyleColor(ImGuiCol_Button, Accent);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.68f, 1.0f, 0.55f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.03f, 0.10f, 0.08f, 1.0f});
  const bool result = ImGui::Button(label, size);
  ImGui::PopStyleColor(3);
  return result;
}

bool NavItem(int index, int& current, const char* icon, const char* title, const char* subtitle, float width) {
  ImGui::PushID(index);
  const ImVec2 start = ImGui::GetCursorScreenPos();
  bool active = current == index;
  if (active) ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{0.10f, 0.27f, 0.18f, 1.0f});
  const bool pressed = ImGui::Selectable("##nav", active, ImGuiSelectableFlags_None, {width, 52});
  if (active) ImGui::PopStyleColor();
  auto* dl = ImGui::GetWindowDrawList();
  if (active) dl->AddRectFilled(start, {start.x + 3, start.y + 52}, ImGui::ColorConvertFloat4ToU32(Accent));
  (void)icon;const ImU32 icon_color=ImGui::ColorConvertFloat4ToU32(active?Accent:Cyan);const ImVec2 c{start.x+20,start.y+22};
  if(index==0)dl->AddTriangleFilled({c.x-5,c.y-6},{c.x-5,c.y+6},{c.x+6,c.y},icon_color);
  else if(index==1)dl->AddQuadFilled({c.x,c.y-6},{c.x+6,c.y},{c.x,c.y+6},{c.x-6,c.y},icon_color);
  else if(index==2){dl->AddCircle(c,6,icon_color,8,2);dl->AddCircleFilled(c,2,icon_color);}
  else if(index==3){dl->AddLine({c.x-6,c.y-2},{c.x+6,c.y+2},icon_color,2);dl->AddLine({c.x-3,c.y-5},{c.x+3,c.y+5},icon_color,1);}
  else if(index==4){dl->AddRect({c.x-6,c.y-6},{c.x+6,c.y+6},icon_color,0,0,2);dl->AddRectFilled({c.x-2,c.y-2},{c.x+2,c.y+2},icon_color);}
  else dl->AddNgon(c,6,icon_color,6,2);
  dl->AddText({start.x + 42, start.y + 7}, IM_COL32(224,234,232,255), title);
  dl->AddText({start.x + 42, start.y + 29}, IM_COL32(145,164,161,255), subtitle);
  if (pressed) current = index;
  ImGui::PopID();
  return pressed;
}

bool Row(const char* title, const char* sub, const char* value = nullptr, bool selected = false, float height = 52) {
  ImGui::PushID(RowId++);
  if (selected) ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{0.08f,0.22f,0.16f,1});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{12,5});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,{8,1});
  ImGui::BeginChild("row", {0,height}, ImGuiChildFlags_Borders,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::TextUnformatted(title); Sub(sub);
  if (value) {
    ImVec2 size=ImGui::CalcTextSize(value);
    ImGui::SetCursorPos({ImGui::GetWindowWidth()-size.x-12,13});
    ImGui::TextColored(Accent,"%s",value);
  }
  const bool pressed=ImGui::IsWindowHovered()&&ImGui::IsMouseReleased(ImGuiMouseButton_Left);
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  if (selected) ImGui::PopStyleColor();
  ImGui::PopID();
  return pressed;
}

void Tabs(const char* id, const std::array<const char*,4>& names, int count, int& active) {
  if (ImGui::BeginTabBar(id, ImGuiTabBarFlags_None)) {
    for(int i=0;i<count;i++) {
      ImGuiTabItemFlags flags = i==active ? ImGuiTabItemFlags_SetSelected : 0;
      if(ImGui::BeginTabItem(names[i],nullptr,flags)){ if(ImGui::IsItemActivated()) active=i; ImGui::EndTabItem(); }
    }
    ImGui::EndTabBar();
  }
}

void SettingRow(const char* title, const char* sub, auto widget) {
  ImGui::PushID(title);ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{12,6});ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,{8,2}); ImGui::BeginChild("setting",{0,58},ImGuiChildFlags_Borders,
                                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
  ImGui::BeginGroup();ImGui::TextUnformatted(title);Sub(sub);ImGui::EndGroup();
  ImGui::SameLine(ImGui::GetWindowWidth()*0.52f);ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-12);widget();
  ImGui::EndChild();ImGui::PopStyleVar(2);ImGui::PopID();
}

void Play(UiState& s) {
  Label("SPLONKS / PLAY");Title("Play");
  const float right=320, gap=12;
  ImGui::BeginChild("setup",{ImGui::GetContentRegionAvail().x-right-gap,0},ImGuiChildFlags_Borders,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
  Label(s.play_activity==0?"CONTINUE QUEST":"NEW EXPEDITION");
  ImGui::Text("%s",s.play_activity==0?"The Violet Reach":"Choose a quest");
  Sub(s.play_notice==1?"Checkpoint picker opened — select a recorded resume point":s.play_notice==2?"Expedition rules editor opened — changes apply to this lobby":s.play_activity==0?"The Glass Caverns · Latest checkpoint · Vega":"Start a clean quest from stage one");ImGui::Separator();
  SettingRow("Activity","Continue a checkpoint or start another quest",[&]{const char* v[]={"Continue expedition","New expedition"};ImGui::Combo("##activity",&s.play_activity,v,2);});
  if(Row(s.play_activity==0?"Resume point":"Quest",s.play_activity==0?"Latest checkpoint · The Violet Reach":"The Violet Reach · 8 stages",s.play_activity==0?"CHOOSE CHECKPOINT >":"CHOOSE QUEST >"))s.play_notice=1;
  SettingRow("Play with","Who may occupy the remaining slots",[&]{const char* v[]={"Solo","Friends can join","Public"};ImGui::Combo("##join",&s.join_mode,v,3);});
  SettingRow("Host using","Automatic chooses direct or relay hosting",[&]{const char* v[]={"Automatic","Direct","Relay"};ImGui::Combo("##host",&s.host_mode,v,3);});
  if(Row("Expedition rules","Standard · 4 lives · ghost at 180s","EDIT ALL >"))s.play_notice=2;
  if(Row("Session mods","7 active · dependency set valid","MANAGE >"))s.screen=5;
  ImGui::SetCursorPosY(ImGui::GetWindowHeight()-52);ImGui::Button("Pause preview",{150,36});ImGui::SameLine(ImGui::GetWindowWidth()-280);PrimaryButton(s.play_activity==0?"Resume latest checkpoint":"Begin expedition",{264,36});
  ImGui::EndChild();ImGui::SameLine();
  ImGui::BeginChild("party",{right,0},ImGuiChildFlags_Borders,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);Label("PLAYERS");ImGui::TextUnformatted(s.join_mode==0?"Solo player":"Your party");
  Row("P1   Moss","Xbox Wireless Controller","READY");if(s.join_mode!=0){Row("+   Open slot","Invite a friend or add locally");Row("+   Open slot","Invite a friend or add locally");}
  ImGui::SetCursorPosY(ImGui::GetWindowHeight()-90);ImGui::Button("Invite / copy link",{-1,34});Sub("CONTENT                         7 mods");Sub(s.host_mode==0?"NETWORK                       Automatic":s.host_mode==1?"NETWORK                       Direct":"NETWORK                       Relay");
  ImGui::EndChild();
}

void Players(UiState& s) {
  Label("SPLONKS / PLAYERS");Title("Players");std::array<const char*,4> tabs{"Local players","Profiles","Devices",nullptr};Tabs("players-tabs",tabs,3,s.players_tab);
  ImGui::BeginChild("players-body",{0,0},ImGuiChildFlags_Borders);
  if(s.players_tab==0){Label("LOCAL PARTY");Sub("Assign devices and profiles");ImGui::Columns(2,nullptr,false);Row("P1  Moss","Profile: Moss · Xbox Wireless Controller","READY",true,72);Row("P2  Open slot","Add a local player or invite a friend",nullptr,false,72);ImGui::NextColumn();Label("SELECTED PLAYER");ImGui::TextUnformatted("Moss");Row("Xbox Wireless Controller","Gamepad 0 · connected","ASSIGNED");Row("Keyboard + mouse","Shared desktop input","AVAILABLE");PrimaryButton("Assign device",{140,36});ImGui::SameLine();ImGui::Button("Mark not ready",{140,36});ImGui::Columns(1);}
  else if(s.players_tab==1){Label("PROFILE LIBRARY");Sub("Profile data is not a save");ImGui::SameLine(ImGui::GetWindowWidth()-130);PrimaryButton("+ New profile",{110,34});ImGui::Columns(3,nullptr,false);Row("Moss","38h 22m · 84 runs · 21 wins","ACTIVE",true,120);ImGui::NextColumn();Row("Vega","14h 11m · 31 runs · 4 wins","SELECT",false,120);ImGui::NextColumn();Row("Guest","Never · 0 runs · 0 wins","SELECT",false,120);ImGui::Columns(1);ImGui::Separator();ImGui::TextUnformatted("Moss's history");ImGui::Columns(5,nullptr,false);const char* stats[]={"84\nPLAYS","21\nWINS","138\nDEATHS","2.4M\nSCORE","42\nREPLAYS"};for(auto* v:stats){ImGui::TextColored(Accent,"%s",v);ImGui::NextColumn();}ImGui::Columns(1);}
  else {Label("CONNECTED INPUT");Sub("Devices may belong to more than one local player");ImGui::Columns(2,nullptr,false);Row("Xbox Wireless Controller","Gamepad 0 · battery 82%","MOSS",true,66);Row("Keyboard + mouse","Desktop aggregate","AVAILABLE",false,66);Row("T.16000M Joystick","14 axes · 32 buttons","UNASSIGNED",false,66);ImGui::NextColumn();Label("LIVE INPUT EXPLORER");ImGui::TextUnformatted("Xbox Wireless Controller");Row("Button South","idle");Row("Left X","+0.04");Row("Right Trigger","0.00");ImGui::Columns(1);}
  ImGui::EndChild();
}

void Settings(UiState& s) {
  Label("SPLONKS / SETTINGS");Title("Settings");std::array<const char*,4> tabs{"Display","Audio","Accessibility","Gameplay"};Tabs("settings-tabs",tabs,4,s.settings_tab);
  ImGui::BeginChild("settings-list",{ImGui::GetContentRegionAvail().x*.52f,0},ImGuiChildFlags_Borders);
  if(s.settings_tab==0){SettingRow("Fullscreen","Use the entire selected display",[&]{ImGui::Checkbox("##full",&s.fullscreen);});SettingRow("Display resolution","Output resolution for this display",[&]{static int v=1;const char* a[]={"1280 x 720","1920 x 1080","2560 x 1440"};ImGui::Combo("##res",&v,a,3);});SettingRow("Render scale","Internal 3D resolution",[&]{ImGui::SliderFloat("##scale",&s.render_scale,50,150,"%.0f%%");});SettingRow("Frame cap","Maximum simulation frames per second",[&]{static int v=2;const char* a[]={"60 FPS","120 FPS","144 FPS","Unlimited"};ImGui::Combo("##cap",&v,a,4);});SettingRow("Brightness","Fine-tune scene visibility",[&]{ImGui::SliderFloat("##bright",&s.brightness,0,100,"%.0f%%");});}
  else if(s.settings_tab==1){SettingRow("Master volume","Overall output level",[&]{ImGui::SliderFloat("##master",&s.master_volume,0,100,"%.0f%%");});SettingRow("Music volume","Score and ambient layers",[&]{ImGui::SliderFloat("##music",&s.music_volume,0,100,"%.0f%%");});SettingRow("Dynamic range","Compress loud and quiet sounds",[&]{static int v=1;const char* a[]={"Night","Balanced","Wide"};ImGui::Combo("##range",&v,a,3);});}
  else if(s.settings_tab==2){SettingRow("Subtitles","Dialogue and important callouts",[&]{ImGui::Checkbox("##sub",&s.subtitles);});Row("Color distinction","Shape and label reinforcement","ON");Row("Reduced motion","Limit camera and menu animation","OFF");Row("Text scale","Interface and subtitle size","110%");}
  else {Row("Pause when unfocused","Suspend local simulation","ON",true);Row("Tutorial prompts","Context-sensitive hints","ON");Row("Damage numbers","Combat feedback","OFF");}
  ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("setting-detail",{0,0},ImGuiChildFlags_Borders);Label("SELECTED SETTING");ImGui::TextUnformatted(s.settings_tab==0?"Render scale":s.settings_tab==1?"Master volume":s.settings_tab==2?"Subtitles":"Pause when unfocused");ImGui::TextWrapped("The game supplies this schema. Gubsy supplies stable native controls, profile storage, serialization, and synchronization.");ImGui::Separator();Label("CURRENT VALUE");ImGui::TextColored(Accent,s.settings_tab==0?"%.0f%%":s.settings_tab==1?"%.0f%%":"Enabled",s.settings_tab==0?s.render_scale:s.master_volume);Row("Scope","Current machine");Row("Profile","Vega");Row("Restart","Not required");PrimaryButton("Apply settings",{170,36});ImGui::SameLine();ImGui::Button("Reset category",{140,36});ImGui::EndChild();
}

void Controls(UiState& s) {
  Label("SPLONKS / CONTROLS");Title("Controls");std::array<const char*,4> tabs{"Bindings","Devices","Input tuning",nullptr};Tabs("controls-tabs",tabs,3,s.controls_tab);
  ImGui::BeginChild("controls-left",{ImGui::GetContentRegionAvail().x*.5f,0},ImGuiChildFlags_Borders);
  if(s.controls_tab==0){static char filter[80]{};ImGui::InputTextWithHint("##filter","Filter actions...",filter,sizeof filter);const char* a[]={"Menu Up","Menu Down","Menu Left","Menu Right","Activate","Cancel","Move","Look"};const char* b[]={"D-Pad Up","D-Pad Down","D-Pad Left","D-Pad Right","Gamepad A","Gamepad B","Left Stick","Right Stick"};for(int i=0;i<8;i++){if(ImGui::Selectable(a[i],s.selected_action==i,0,{0,44}))s.selected_action=i;ImGui::SameLine(ImGui::GetWindowWidth()-100);ImGui::TextColored(Accent,"%s",b[i]);}}
  else if(s.controls_tab==1){Label("DEVICE ASSIGNMENT");Row("Xbox Wireless Controller","Gamepad 0 · connected","MOSS",true,66);Row("Keyboard + mouse","Desktop aggregate","AVAILABLE",false,66);Row("T.16000M Joystick","14 axes · 32 buttons","MOSS",false,66);}
  else {Label("INPUT PROFILE");Sub("Standard · Xbox Wireless Controller");SettingRow("Look sensitivity","Camera speed",[&]{ImGui::SliderFloat("##sense",&s.sensitivity,0,100,"%.0f%%");});SettingRow("Stick deadzone","Ignore center noise",[&]{ImGui::SliderFloat("##dead",&s.deadzone,0,40,"%.0f%%");});SettingRow("Vibration strength","Rumble output",[&]{ImGui::SliderFloat("##vibe",&s.vibration,0,100,"%.0f%%");});SettingRow("Trigger deadzone","Minimum travel",[&]{ImGui::SliderFloat("##trigger",&s.trigger_deadzone,0,30,"%.0f%%");});}
  ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("controls-detail",{0,0},ImGuiChildFlags_Borders);
  if(s.controls_tab==0){const char* a[]={"Menu Up","Menu Down","Menu Left","Menu Right","Activate","Cancel","Move","Look"};Label("SELECTED ACTION");ImGui::TextUnformatted(a[s.selected_action]);Sub("Multiple device bindings and transformed axes are supported.");Row("1   Gamepad binding","D-pad / face / analog input","REPLACE");Row("2   Keyboard binding","Keyboard key","REPLACE");PrimaryButton("Listen for input",{140,36});ImGui::SameLine();ImGui::Button("Browse controls",{140,36});ImGui::TextWrapped("Manual browse supports trigger-to-button, wheel pedals, joysticks, and macro pads.");}
  else if(s.controls_tab==1){Label("INPUT EXPLORER");ImGui::TextUnformatted("Press controls to identify Gubsy input IDs");Row("Button South","idle");Row("Axis Left X","+0.04");Row("Axis Right Trigger","0.00");}
  else {Label("DEVICE RESPONSE");ImGui::TextUnformatted("Live response");ImGui::ProgressBar(s.sensitivity/100,{ImGui::GetContentRegionAvail().x,30},"qualified stick magnitude");Row("Device","Xbox Wireless Controller");Row("Curve","Smooth");}
  ImGui::EndChild();
}

void Progress(UiState&) {
  Label("SPLONKS / PROGRESS");Title("Progress");Label("GAME-PROVIDED PROGRESSION");Sub("3 campaigns · automatic checkpoints");
  ImGui::BeginChild("campaigns",{ImGui::GetContentRegionAvail().x*.5f,0},ImGuiChildFlags_Borders);Row("The Glass Caverns","Moss · Temple Depths · 42%","12h 48m  READY",true,66);Row("A Quiet Beginning","Vega · Green Valley · 11%","3h 06m  READY",false,66);Row("Old Expedition","Moss · Version 0.7 data","INCOMPATIBLE",false,66);ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("progress-detail",{0,0},ImGuiChildFlags_Borders);Label("ASSOCIATED PROFILE");Row("MO   Moss","Ownership recorded by progression provider");Label("RECORDED MOD SET · 7 PACKAGES");Row("Base Content","Required package","v1.4.0");Row("Cartographer's Desk","Quest dependency","v0.8.2");Row("Old Lanterns","Installed package","UPDATE");Label("CHECKPOINT HISTORY");Row("Temple safe room","Today, 06:52 · Stage 4","RESUME");Row("Flooded archive","Yesterday · Stage 3","BACKUP");PrimaryButton("Resume campaign",{160,36});ImGui::SameLine();ImGui::Button("Export",{90,36});ImGui::EndChild();
}

void Mods(UiState& s) {
  Label("SPLONKS / MODS");Title("Mods");std::array<const char*,4> tabs{"Installed","Browse catalog",nullptr,nullptr};Tabs("mods-tabs",tabs,2,s.mods_tab);
  static char search[100]{};ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x*.62f);ImGui::InputTextWithHint("##search","Search the Gubsy mod catalog...",search,sizeof search);ImGui::SameLine();ImGui::Checkbox("Compatible only",&s.compatible_only);ImGui::SameLine();ImGui::Button("Refresh");
  ImGui::BeginChild("mods-list",{ImGui::GetContentRegionAvail().x*.5f,0},ImGuiChildFlags_Borders);const char* names[]={"Mycelium Below","Brassline Grapple Kit","Skybreak Caverns","Abyssal Tide","Old Lanterns","Pocket Expedition","Temple Weather","Mirror Depths","Clockwork Orchard","Lantern Cartography","Fungal Friends","Deep Relay","Cave Radio","Run History+","Accessible Traps","Quiet Ghost","Shared Wallet","Daily Seed Lab","Rope Physics","Vanilla Plus"};const char* deps[]={"Requires Base Content","Requires Cartographer's Desk","Requires game 1.4","Requires Underground Rivers","Update available","2 dependencies","Required by 2 mods","Requires Gubsy Mod API 0.2"};for(int i=0;i<20;i++){ImGui::PushID(i);const ImVec2 start=ImGui::GetCursorScreenPos();if(ImGui::Selectable("##mod",s.selected_mod==i,0,{0,62}))s.selected_mod=i;auto* dl=ImGui::GetWindowDrawList();if(Assets.mod_sheet!=ImTextureID_Invalid){const float u0=float(i%5)/5.0f,u1=float(i%5+1)/5.0f;dl->AddImage(Assets.mod_sheet,{start.x+8,start.y+7},{start.x+56,start.y+55},{u0,0},{u1,1});}else dl->AddRectFilled({start.x+8,start.y+7},{start.x+56,start.y+55},IM_COL32(20,55,60,255));dl->AddText({start.x+66,start.y+9},IM_COL32(224,234,232,255),names[i]);dl->AddText({start.x+66,start.y+33},IM_COL32(145,164,161,255),i<8?deps[i]:"Catalog content package");const char* status=i==7?"INCOMPATIBLE":i==4?"UPDATE":"93%";const ImVec2 status_size=ImGui::CalcTextSize(status);dl->AddText({start.x+ImGui::GetContentRegionAvail().x-status_size.x-10,start.y+19},ImGui::ColorConvertFloat4ToU32(i==7?ImVec4{1,.4f,.4f,1}:Accent),status);ImGui::PopID();}ImGui::EndChild();ImGui::SameLine();ImGui::BeginChild("mod-detail",{0,0},ImGuiChildFlags_Borders);if(Assets.mod_sheet!=ImTextureID_Invalid){const float u0=float(s.selected_mod%5)/5.0f,u1=float(s.selected_mod%5+1)/5.0f;ImGui::Image(Assets.mod_sheet,{ImGui::GetContentRegionAvail().x,118},{u0,.12f},{u1,.88f});}Label(s.mods_tab?"CATALOG ENTRY":"INSTALLED PACKAGE");ImGui::TextUnformatted(names[s.selected_mod]);ImGui::TextWrapped("A substantial content package with carefully integrated rooms, mechanics, artwork, and native co-op synchronization.");Label("COMPATIBILITY & DEPENDENCIES");Row("Base Content >= 1.4.0","Core dependency","INSTALLED");Row("Underground Rivers >= 2.2.0","Automatic dependency","WILL INSTALL");Label("REQUIRED BY");Row("Temple Weather","Dependent package","ACTIVE");Row("Pocket Expedition","Dependent package","ACTIVE");ImGui::TextColored(s.selected_mod==7?ImVec4{1,.4f,.4f,1}:Muted,"%s",s.selected_mod==7?"This mod cannot be installed on the current API version.":"The full dependency change plan is shown before mutation.");PrimaryButton(s.mods_tab?"Install & add to session":"Update",{210,36});ImGui::SameLine();ImGui::Button(s.mods_tab?"Install only":"Open files",{110,36});ImGui::EndChild();
}

} // namespace

void ConfigureUiStyle(float density) {
  ImGui::StyleColorsDark(); auto& st=ImGui::GetStyle();st.ScaleAllSizes(density);st.WindowRounding=0;st.ChildRounding=0;st.FrameRounding=0;st.WindowBorderSize=0;st.FrameBorderSize=1;st.ItemSpacing={8,7};
  st.Colors[ImGuiCol_WindowBg]={.025f,.065f,.075f,1};st.Colors[ImGuiCol_ChildBg]={.04f,.11f,.12f,1};st.Colors[ImGuiCol_Border]={.18f,.31f,.31f,1};st.Colors[ImGuiCol_FrameBg]={.045f,.12f,.13f,1};st.Colors[ImGuiCol_Header]={.08f,.20f,.17f,1};st.Colors[ImGuiCol_HeaderHovered]={.11f,.28f,.21f,1};st.Colors[ImGuiCol_HeaderActive]={.15f,.36f,.25f,1};st.Colors[ImGuiCol_CheckMark]=Accent;st.Colors[ImGuiCol_SliderGrab]=Accent;st.Colors[ImGuiCol_NavHighlight]=Cyan;st.Colors[ImGuiCol_TabSelected]={.10f,.27f,.18f,1};
}

void SetUiAssets(UiAssets assets) { Assets=assets; }

void DrawGubsyUi(UiState& s, int width, int height) {
  RowId=0;
  ImGui::SetNextWindowPos({0,0});ImGui::SetNextWindowSize({float(width),float(height)});
  ImGui::Begin("Gubsy shell",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
  const float header=48;ImGui::BeginChild("header",{0,header},ImGuiChildFlags_Borders);ImGui::TextColored(Accent,"G");ImGui::SameLine();ImGui::TextUnformatted("G U B S Y   S H E L L");ImGui::SameLine(ImGui::GetWindowWidth()*.58f);ImGui::TextUnformatted("●  OFFLINE     |     VEGA");ImGui::SameLine(ImGui::GetWindowWidth()-125);ImGui::TextColored(Cyan,"%d x %d",width,height);ImGui::EndChild();
  const bool compact=width<900||height<620;const float footer=compact?52:0;const float nav=compact?0:230;
  const float remaining=ImGui::GetContentRegionAvail().y;
  if(!compact){ImGui::BeginChild("nav",{nav,remaining},ImGuiChildFlags_Borders,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);ImGui::TextUnformatted("VE   ACTIVE PROFILE");Sub("     Vega");ImGui::Separator();const char* t[]={"Play","Players","Settings","Controls","Progress","Mods"};const char* u[]={"Continue or start","Profiles & devices","Game preferences","Bindings & input","Campaigns & checkpoints","Installed content"};const char* i[]={">","◆","*","~","#","o"};for(int n=0;n<6;n++)NavItem(n,s.screen,i[n],t[n],u[n],nav-16);ImGui::SetCursorPosY(ImGui::GetWindowHeight()-52);ImGui::TextUnformatted("×   Quit");Sub("     Return to desktop");ImGui::EndChild();ImGui::SameLine();}
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,{22,12});ImGui::BeginChild("content",{0,compact?remaining-footer:remaining},ImGuiChildFlags_AlwaysUseWindowPadding,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);switch(s.screen){case 0:Play(s);break;case 1:Players(s);break;case 2:Settings(s);break;case 3:Controls(s);break;case 4:Progress(s);break;case 5:Mods(s);break;}ImGui::EndChild();ImGui::PopStyleVar();
  if(compact){ImGui::BeginChild("bottom-nav",{0,footer},ImGuiChildFlags_Borders);const char* n[]={"Play","Players","Settings","Controls","Progress","Mods"};for(int i=0;i<6;i++){if(i)ImGui::SameLine();if(ImGui::Selectable(n[i],s.screen==i,0,{(ImGui::GetContentRegionAvail().x-(5-i)*8)/(6-i),footer-8}))s.screen=i;}ImGui::EndChild();}
  ImGui::End();
}
