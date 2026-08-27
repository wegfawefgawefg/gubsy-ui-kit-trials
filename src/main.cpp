#include "ui.h"

#include "SDL3/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <unistd.h>

namespace {
using Clock=std::chrono::steady_clock;
double Ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
long RssKiB(){FILE* f=std::fopen("/proc/self/statm","r");long total=0,resident=0;if(f){std::fscanf(f,"%ld %ld",&total,&resident);std::fclose(f);}return resident*long(sysconf(_SC_PAGESIZE))/1024;}
struct Args{int width=1280,height=720,screen=0,frames=0;std::string capture;};
Args Parse(int argc,char** argv){Args a;for(int i=1;i<argc;i++){std::string_view v=argv[i];if(v=="--resolution"&&i+1<argc)std::sscanf(argv[++i],"%dx%d",&a.width,&a.height);else if(v=="--screen"&&i+1<argc){std::string n=argv[++i];const char* names[]={"play","players","settings","controls","progress","mods"};a.screen=std::atoi(n.c_str());for(int j=0;j<6;j++)if(n==names[j])a.screen=j;}else if(v=="--benchmark"&&i+1<argc)a.frames=std::max(121,std::atoi(argv[++i]));else if(v=="--capture"&&i+1<argc){a.capture=argv[++i];a.frames=5;}}return a;}

SDL_GPUTexture* LoadTexture(SDL_GPUDevice* gpu, const char* path) {
  int width=0,height=0,channels=0;
  unsigned char* pixels=stbi_load(path,&width,&height,&channels,4);
  if(!pixels){std::fprintf(stderr,"texture %s: %s\n",path,stbi_failure_reason());return nullptr;}
  SDL_GPUTextureCreateInfo texture_info{};texture_info.type=SDL_GPU_TEXTURETYPE_2D;texture_info.format=SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;texture_info.usage=SDL_GPU_TEXTUREUSAGE_SAMPLER;texture_info.width=Uint32(width);texture_info.height=Uint32(height);texture_info.layer_count_or_depth=1;texture_info.num_levels=1;texture_info.sample_count=SDL_GPU_SAMPLECOUNT_1;
  SDL_GPUTexture* texture=SDL_CreateGPUTexture(gpu,&texture_info);
  SDL_GPUTransferBufferCreateInfo transfer_info{};transfer_info.usage=SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;transfer_info.size=Uint32(width*height*4);
  SDL_GPUTransferBuffer* transfer=SDL_CreateGPUTransferBuffer(gpu,&transfer_info);
  if(!texture||!transfer){if(texture)SDL_ReleaseGPUTexture(gpu,texture);if(transfer)SDL_ReleaseGPUTransferBuffer(gpu,transfer);stbi_image_free(pixels);return nullptr;}
  void* mapped=SDL_MapGPUTransferBuffer(gpu,transfer,false);std::memcpy(mapped,pixels,size_t(width)*height*4);SDL_UnmapGPUTransferBuffer(gpu,transfer);stbi_image_free(pixels);
  SDL_GPUCommandBuffer* commands=SDL_AcquireGPUCommandBuffer(gpu);SDL_GPUCopyPass* copy=SDL_BeginGPUCopyPass(commands);SDL_GPUTextureTransferInfo source{};source.transfer_buffer=transfer;source.pixels_per_row=Uint32(width);source.rows_per_layer=Uint32(height);SDL_GPUTextureRegion destination{};destination.texture=texture;destination.w=Uint32(width);destination.h=Uint32(height);destination.d=1;SDL_UploadToGPUTexture(copy,&source,&destination,false);SDL_EndGPUCopyPass(copy);SDL_SubmitGPUCommandBuffer(commands);SDL_WaitForGPUIdle(gpu);SDL_ReleaseGPUTransferBuffer(gpu,transfer);return texture;
}
}

int main(int argc,char** argv){
  Args args=Parse(argc,argv);const bool hidden=args.frames>0;
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
  if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)){std::fprintf(stderr,"SDL_Init: %s\n",SDL_GetError());return 1;}
  SDL_WindowFlags wf=SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY|SDL_WINDOW_UTILITY;if(hidden)wf|=SDL_WINDOW_HIDDEN;
  SDL_Window* window=SDL_CreateWindow("Gubsy Dear ImGui trial",args.width,args.height,wf);if(!window){std::fprintf(stderr,"SDL_CreateWindow: %s\n",SDL_GetError());return 1;}
  SDL_DisplayID display=SDL_GetPrimaryDisplay();SDL_SetWindowPosition(window,SDL_WINDOWPOS_CENTERED_DISPLAY(display),SDL_WINDOWPOS_CENTERED_DISPLAY(display));
  SDL_GPUDevice* gpu=SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV|SDL_GPU_SHADERFORMAT_DXIL|SDL_GPU_SHADERFORMAT_MSL,true,nullptr);
  if(!gpu||!SDL_ClaimWindowForGPUDevice(gpu,window)){std::fprintf(stderr,"SDL_GPU: %s\n",SDL_GetError());return 1;}
  SDL_SetGPUSwapchainParameters(gpu,window,SDL_GPU_SWAPCHAINCOMPOSITION_SDR,hidden?SDL_GPU_PRESENTMODE_IMMEDIATE:SDL_GPU_PRESENTMODE_VSYNC);
  IMGUI_CHECKVERSION();ImGui::CreateContext();ImGuiIO& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_NavEnableGamepad;io.Fonts->AddFontFromFileTTF(GUBSY_IMGUI_FONT_PATH,16.0f);ConfigureUiStyle(1.0f);
  ImGui_ImplSDL3_InitForSDLGPU(window);ImGui_ImplSDLGPU3_InitInfo info{};info.Device=gpu;info.ColorTargetFormat=SDL_GetGPUSwapchainTextureFormat(gpu,window);info.MSAASamples=SDL_GPU_SAMPLECOUNT_1;info.SwapchainComposition=SDL_GPU_SWAPCHAINCOMPOSITION_SDR;info.PresentMode=hidden?SDL_GPU_PRESENTMODE_IMMEDIATE:SDL_GPU_PRESENTMODE_VSYNC;ImGui_ImplSDLGPU3_Init(&info);
  SDL_GPUTexture* mod_sheet=LoadTexture(gpu,GUBSY_IMGUI_MOD_SHEET_PATH);SetUiAssets({mod_sheet?(ImTextureID)(intptr_t)mod_sheet:ImTextureID_Invalid});
  int pixel_w=args.width,pixel_h=args.height;SDL_GetWindowSizeInPixels(window,&pixel_w,&pixel_h);
  SDL_GPUTexture* offscreen=nullptr;SDL_GPUTransferBuffer* transfer=nullptr;
  if(hidden){SDL_GPUTextureCreateInfo ti{};ti.type=SDL_GPU_TEXTURETYPE_2D;ti.format=info.ColorTargetFormat;ti.usage=SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;ti.width=pixel_w;ti.height=pixel_h;ti.layer_count_or_depth=1;ti.num_levels=1;ti.sample_count=SDL_GPU_SAMPLECOUNT_1;offscreen=SDL_CreateGPUTexture(gpu,&ti);if(!args.capture.empty()){SDL_GPUTransferBufferCreateInfo bi{};bi.usage=SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;bi.size=Uint32(pixel_w*pixel_h*4);transfer=SDL_CreateGPUTransferBuffer(gpu,&bi);}}
  UiState state;state.screen=std::clamp(args.screen,0,5);bool running=true;int frame=0;double build_total=0,record_total=0,submit_total=0,wait_total=0,frame_total=0;
  while(running){auto fs=Clock::now();SDL_Event e;while(SDL_PollEvent(&e)){ImGui_ImplSDL3_ProcessEvent(&e);if(e.type==SDL_EVENT_QUIT)running=false;if(e.type==SDL_EVENT_KEY_DOWN&&e.key.key>=SDLK_F1&&e.key.key<=SDLK_F6)state.screen=int(e.key.key-SDLK_F1);}
    int w=0,h=0;SDL_GetWindowSize(window,&w,&h);ImGui_ImplSDLGPU3_NewFrame();ImGui_ImplSDL3_NewFrame();ImGui::NewFrame();auto bs=Clock::now();DrawGubsyUi(state,w,h);ImGui::Render();auto be=Clock::now();build_total+=Ms(bs,be);
    SDL_GPUCommandBuffer* cb=SDL_AcquireGPUCommandBuffer(gpu);SDL_GPUTexture* swap=nullptr;Uint32 sw=0,sh=0;if(!hidden&&!SDL_WaitAndAcquireGPUSwapchainTexture(cb,window,&swap,&sw,&sh)){SDL_CancelGPUCommandBuffer(cb);break;}SDL_GPUTexture* target=hidden?offscreen:swap;if(!target){SDL_CancelGPUCommandBuffer(cb);continue;}
    auto rs=Clock::now();ImDrawData* dd=ImGui::GetDrawData();ImGui_ImplSDLGPU3_PrepareDrawData(dd,cb);SDL_GPUColorTargetInfo ct{};ct.texture=target;ct.clear_color={.008f,.025f,.03f,1};ct.load_op=SDL_GPU_LOADOP_CLEAR;ct.store_op=SDL_GPU_STOREOP_STORE;SDL_GPURenderPass* pass=SDL_BeginGPURenderPass(cb,&ct,1,nullptr);ImGui_ImplSDLGPU3_RenderDrawData(dd,cb,pass);SDL_EndGPURenderPass(pass);
    if(transfer&&frame+1>=args.frames){SDL_GPUTextureRegion src{};src.texture=target;src.w=pixel_w;src.h=pixel_h;src.d=1;SDL_GPUTextureTransferInfo dst{};dst.transfer_buffer=transfer;dst.pixels_per_row=pixel_w;dst.rows_per_layer=pixel_h;SDL_GPUCopyPass* cp=SDL_BeginGPUCopyPass(cb);SDL_DownloadFromGPUTexture(cp,&src,&dst);SDL_EndGPUCopyPass(cp);}record_total+=Ms(rs,Clock::now());auto ss=Clock::now();SDL_SubmitGPUCommandBuffer(cb);submit_total+=Ms(ss,Clock::now());if(hidden){auto ws=Clock::now();SDL_WaitForGPUIdle(gpu);wait_total+=Ms(ws,Clock::now());}frame_total+=Ms(fs,Clock::now());frame++;if(args.frames&&frame>=args.frames)running=false;
  }
  SDL_WaitForGPUIdle(gpu);
  if(transfer){void* pixels=SDL_MapGPUTransferBuffer(gpu,transfer,false);SDL_PixelFormat pf=(info.ColorTargetFormat==SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM||info.ColorTargetFormat==SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB)?SDL_PIXELFORMAT_BGRA32:SDL_PIXELFORMAT_RGBA32;SDL_Surface* surf=SDL_CreateSurfaceFrom(pixel_w,pixel_h,pf,pixels,pixel_w*4);if(!SDL_SaveBMP(surf,args.capture.c_str()))std::fprintf(stderr,"capture: %s\n",SDL_GetError());SDL_DestroySurface(surf);SDL_UnmapGPUTransferBuffer(gpu,transfer);SDL_ReleaseGPUTransferBuffer(gpu,transfer);}
  if(args.frames>5){const int samples=std::max(1,frame);std::printf("backend=dear-imgui frames=%d viewport=%dx%d ui_build_ms=%.4f render_record_ms=%.4f submit_ms=%.4f gpu_wait_ms=%.4f frame_cpu_ms=%.4f rss_kib=%ld\n",frame,args.width,args.height,build_total/samples,record_total/samples,submit_total/samples,wait_total/samples,frame_total/samples,RssKiB());}
  if(offscreen)SDL_ReleaseGPUTexture(gpu,offscreen);
  if(mod_sheet)SDL_ReleaseGPUTexture(gpu,mod_sheet);
  ImGui_ImplSDLGPU3_Shutdown();ImGui_ImplSDL3_Shutdown();ImGui::DestroyContext();SDL_ReleaseWindowFromGPUDevice(gpu,window);SDL_DestroyGPUDevice(gpu);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
