#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX
#pragma comment(lib, "Urlmon.lib")
#include "Main.hpp"
#include <D3dx9tex.h>
#include <tchar.h>
#include "auth.hpp"
#include <CommCtrl.h>
#include <future>
#include <random>
#include <chrono>

char Licence[50] = "";
char PassWord[20] = "";
//char Licence[50] = "";
char UserName[20] = "";
char RgPassWord[20] = "";
char RgUserName[20] = "";

static int Tabs = 1;
bool MenuColorEditor = false;
//#include "Features.hpp"
#pragma comment(lib, "D3dx9")
float dpi_scale = 1.f;
ImDrawList* draw;
ImVec2 pos;
//int MenuColor[3] = { 254, 0, 125 };
bool SpoofLaunchMenu = false;
int currentColumn = 0;
using namespace KeyAuth;

std::string name = XorStr("pandemonium"); //Application name found in application settings
std::string ownerid = XorStr("9rvw91Sitf"); //Owner ID Found in user settings
std::string secret = XorStr("2e1824f0449d9ef467937162aad85e9e05023db8f3e4de0d7fea3dbaf8d4b795"); //Application secret found in Application settings
std::string version = XorStr("1.0"); // Version can be changed but is not really important
std::string url = XorStr("https://keyauth.win/api/1.1/"); // change if you're self-hosting
std::string sslPin = XorStr("ssl pin key (optional)"); // don't change unless you intend to pin public certificate key. you can get here in the "Pin SHA256" field https://www.ssllabs.com/ssltest/analyze.html?d=ke
api KeyAuthApp(name, ownerid, secret, version, url, sslPin);


typedef NTSTATUS(WINAPI* lpQueryInfo)(HANDLE, LONG, PVOID, ULONG, PULONG);

void createfile() {
    std::filesystem::create_directories("C:\\pandemonium\\Data");
}


namespace ImGui {

    
    bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);

        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;

        const float a = speed * 0;
        const float b = speed * 0.333f;
        const float c = speed * 0.666f;

        const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
        const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
        const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        // Render
        window->DrawList->PathClear();

        int num_segments = 30;
        int start = abs(ImSin(g.Time * 0.2f) * (num_segments - 5));

        const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
                centre.y + ImSin(a + g.Time * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
    }

}



bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    // Load texture from disk
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;
    return true;
}

void DownloadData()
{
    std::string dwnld_URL = "https://cdn.discordapp.com/attachments/972524086267875368/972959277960552498/unknown.png"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepath = "C:\\pandemonium\\Data\\unknown.png";
    URLDownloadToFile(NULL, dwnld_URL.c_str(), savepath.c_str(), 0, NULL);

    std::string dwnld_URLwarzone = "https://cdn.discordapp.com/attachments/972524086267875368/972978159911829566/Warzone.png"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathwarzone = "C:\\pandemonium\\Data\\Warzone.png";
    URLDownloadToFile(NULL, dwnld_URLwarzone.c_str(), savepathwarzone.c_str(), 0, NULL);

    std::string dwnld_URLvalorant = "https://cdn.discordapp.com/attachments/950728214215553054/973362921205796895/Valorant.png"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathvalorant = "C:\\pandemonium\\Data\\Valorant.png";
    URLDownloadToFile(NULL, dwnld_URLvalorant.c_str(), savepathvalorant.c_str(), 0, NULL);

    std::string dwnld_URLapex = "https://media.discordapp.net/attachments/972524086267875368/973367077199028234/Apex.png?width=94&height=94"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathapex = "C:\\pandemonium\\Data\\Apex.png";
    URLDownloadToFile(NULL, dwnld_URLapex.c_str(), savepathapex.c_str(), 0, NULL);

    std::string dwnld_URLfortnut = "https://store-images.s-microsoft.com/image/apps.30614.70702278257994163.e5c727b5-ca54-4dba-a1c0-7866c2cdb37c.4943548f-2cdf-4640-8d30-72d550b5591c?w=300&h=300"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathfortnut = "C:\\pandemonium\\Data\\Fortnite.png";
    URLDownloadToFile(NULL, dwnld_URLfortnut.c_str(), savepathfortnut.c_str(), 0, NULL);

    std::string dwnld_URLrust = "https://media.discordapp.net/attachments/971474003397079160/975142847256875088/Rust.png?width=71&height=71"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathrust = "C:\\pandemonium\\Data\\Rust.png";
    URLDownloadToFile(NULL, dwnld_URLrust.c_str(), savepathrust.c_str(), 0, NULL);

    std::string dwnld_URLtarkov = "https://media.discordapp.net/attachments/971474003397079160/975142846896160868/Tarkov.jpg?width=71&height=71"; // This isnt a rat this is the logo since yall retards dont know how to follow instructions.
    std::string savepathtarkov = "C:\\pandemonium\\Data\\Tarkov.png";
    URLDownloadToFile(NULL, dwnld_URLtarkov.c_str(), savepathtarkov.c_str(), 0, NULL);


}

bool spoofing = false;
bool cleaning = false;
bool Spoofermenumain = true;
int spooferpage = 1;
int cleanerpage = 1;
bool ALLHWID = false;
bool ProductID = false;
bool CpuID = false;
bool GpuID = false;
bool MacID = false;

bool RAMID = false;
bool SMBIOSID = false;
bool MOTHERBOARDID = false;
bool UUUID = false;

// Main code
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    
    
    createfile();
    DownloadData(); // Downloads all t
    
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, LOADER_BRAND, NULL };
    RegisterClassEx(&wc);
    main_hwnd = CreateWindow(wc.lpszClassName, LOADER_BRAND, WS_POPUP,  0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);
    
    // Initialize Direct3D
    if (!CreateDeviceD3D(main_hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }



    ShowWindow(main_hwnd, SW_HIDE);
    UpdateWindow(main_hwnd);

    HWND Stealth;
    AllocConsole();
    Stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(Stealth, 1);
    printf("\x1B[31m[\033[0m\x1B[32m!\033[0m\x1B[31m]\033[0m Initializing GUI");
    SetWindowPos(main_hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = ImGui::GetStyle().Colors;
    io.KeyMap;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style->WindowRounding = 0.0f;
        style->Colors[ImGuiCol_WindowBg].w = 1.0f;
    }



    ImGui::StyleColorsDark();


    
    style->Alpha = 1.0f;
    style->WindowRounding = 5;
    style->FramePadding = ImVec2(4, 3);
    style->WindowPadding = ImVec2(0, 0);
    style->ItemInnerSpacing = ImVec2(6, 6);
    style->ItemSpacing = ImVec2(8, 10);
    style->FrameRounding = 30;
    style->PopupRounding = 1.f;
    style->Rounding = 15.f;
    style->FrameRounding = 1.0f;

    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 4.5f;
    
    
    




    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF,
        0x0400, 0x044F,
        0,
    };

    
    
    
    
    ImFont* f6 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 24.0f);
    ImFont* f7 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 13.0f);
    ImFont* f8 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 16.0f);
    
    int Page = 1;
    ImGui_ImplWin32_Init(main_hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    DWORD window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;

    RECT screen_rect;
    GetWindowRect(GetDesktopWindow(), &screen_rect);
    auto x = float(screen_rect.right - WINDOW_WIDTH) / 2.f;
    auto y = float(screen_rect.bottom - WINDOW_HEIGHT) / 2.f;
    //Logo shit below is commented out due to the new color changes coming out
    int my_image_width = 5;
    int my_image_height = 5;
    PDIRECT3DTEXTURE9 my_texture = NULL;
    bool Result = LoadTextureFromFile("C:\\pandemonium\\Data\\unknown.png", &my_texture, &my_image_width, &my_image_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result);

    int my_warzoneimage_width = 5;
    int my_warzoneimage_height = 5;
    PDIRECT3DTEXTURE9 my_warzonetexture = NULL;
    bool Result1 = LoadTextureFromFile("C:\\pandemonium\\Data\\Warzone.png", &my_warzonetexture, &my_warzoneimage_width, &my_warzoneimage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result1);

    int my_valorantimage_width = 5;
    int my_valorantimage_height = 5;
    PDIRECT3DTEXTURE9 my_valoranttexture = NULL;
    bool Result2 = LoadTextureFromFile("C:\\pandemonium\\Data\\Valorant.png", &my_valoranttexture, &my_valorantimage_width, &my_valorantimage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result2);

    int my_apeximage_width = 5;
    int my_apeximage_height = 5;
    PDIRECT3DTEXTURE9 my_apextexture = NULL;
    bool Result3 = LoadTextureFromFile("C:\\pandemonium\\Data\\Apex.png", &my_apextexture, &my_apeximage_width, &my_apeximage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result3);

    int my_fortniteimage_width = 5;
    int my_fortniteimage_height = 5;
    PDIRECT3DTEXTURE9 my_fortnitetexture = NULL;
    bool Result4 = LoadTextureFromFile("C:\\pandemonium\\Data\\Fortnite.png", &my_fortnitetexture, &my_fortniteimage_width, &my_fortniteimage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result4);

    int my_rustimage_width = 5;
    int my_rustimage_height = 5;
    PDIRECT3DTEXTURE9 my_rusttexture = NULL;
    bool Result5 = LoadTextureFromFile("C:\\pandemonium\\Data\\Rust.png", &my_rusttexture, &my_rustimage_width, &my_rustimage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result5);

    int my_tarkovimage_width = 5;
    int my_tarkovimage_height = 5;
    PDIRECT3DTEXTURE9 my_tarkovtexture = NULL;
    bool Result6 = LoadTextureFromFile("C:\\pandemonium\\Data\\Tarkov.png", &my_tarkovtexture, &my_tarkovimage_width, &my_tarkovimage_height); // This loads the logo to memory so imgui can load it 
    IM_ASSERT(Result6);

    bool InfWindow = false;
    RECT rc = { 0 };
    POINT mouse;



    KeyAuthApp.init(); ///initalize keyauth





    //MessageBox(NULL, TEXT("This source is the following source code for Meta Spoofer, and will only be used for meta spoofer and possible rebrands in the future, if anything close to token grabbers or rats are added to this source I (Null#1124) am not responsible since I was only the UI developer!"), TEXT("ALERT!"), MB_OK);
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        


        ///Styles

        style->Colors[ImGuiCol_TitleBg] = ImColor(0, 0, 0);
        style->Colors[ImGuiCol_TitleBgActive] = ImColor(0, 0, 0);
        style->Colors[ImGuiCol_Button] = ImColor(29, 31, 39);
        style->Colors[ImGuiCol_ButtonActive] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_ButtonHovered] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_FrameBgActive] = ImColor(29, 31, 38);
        style->Colors[ImGuiCol_FrameBg] = ImColor(10, 10, 10);
        style->Colors[ImGuiCol_FrameBgHovered] = ImColor(29, 31, 38); //dont need custom colors here
        style->Colors[ImGuiCol_SliderGrab] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_Slider] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_SliderGrabActive] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_Header] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_HeaderHovered] = ImColor(51, 51, 255);
        style->Colors[ImGuiCol_HeaderActive] = ImColor(51, 51, 255);



        
        

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
           
            ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT));
            ImGui::SetNextWindowBgAlpha(1.0f);
            
            
            
                auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ WINDOW_WIDTH, WINDOW_HEIGHT };
                
                ImGui::Begin("pandemonium spoofer", &loader_active, window_flags);
                {
                    if (Spoofermenumain)
                    {

                    
                    
                        {//Draw Line
                            static int x = 450 * dpi_scale, y = 288 * dpi_scale;
                            pos = ImGui::GetWindowPos();
                            draw = ImGui::GetWindowDrawList();

                            draw->AddRectFilled(pos, ImVec2(pos.x + 545, pos.y + 355), ImColor(38, 38, 42), 9);
                            draw->AddRectFilled(pos, ImVec2(pos.x + 545, pos.y + 355), ImGui::GetColorU32(ImGuiCol_WindowBg), 9);
                            draw->AddRectFilled(ImVec2(pos.x, pos.y + 30), ImVec2(pos.x + 130, pos.y + 355), ImGui::GetColorU32(ImGuiCol_TabBar), 9, 4);
                            draw->AddLine(ImVec2(pos.x, pos.y + 38), ImVec2(pos.x + 545, pos.y + 38), ImColor(51, 51, 255), 2.0f);
                        }

                        {//tab area
                            ImGui::BeginGroup();
                            {
                                ImGui::PushFont(f7);
                                
                                if (Tabs == 1)
                                {
                                    ImGui::SetCursorPos(ImVec2(215.5, 8));
                                    ImGui::Text("   Pande");
                                    ImGui::SameLine();
                                    ImGui::TextColored(ImColor(51, 51, 255), "monium");
                                    ImGui::SetCursorPos(ImVec2(205, 85));
                                    ImGui::Text("LICENSE KEY HERE");

                                    ImGui::Spacing();

                                     

                                    ImGui::SetCursorPos(ImVec2(95, 125));
                                    ImGui::InputText("##Licence", Licence, IM_ARRAYSIZE(Licence));

                                    ImGui::SetCursorPos(ImVec2(245, 150));
                                    if (ImGui::Button("Login"))
                                    {
                                        KeyAuthApp.license(Licence);

                                        if (!KeyAuthApp.data.success)
                                        {
                                            MessageBox(NULL, TEXT("Error: 0x1 - Create ticket on our discord."), TEXT("Invalid Key or Loader"), MB_OK);
                                            exit(0);
                                        }

                                        MessageBox(NULL, TEXT("Succes; 0x42 - Welcome back"), TEXT("Login"), MB_OK);

                                        InfWindow = true;

                                        Tabs = 2;

                                    }
                                }

                                
                                

                                

                                ImGui::PopFont();

                                if (InfWindow == true)
                                {

                                
                                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 4));

                                ImGui::PushFont(f8);
                                ImGui::SetCursorPos(ImVec2(25, -10));
                                if (ImGui::Tab(("Spoofer"), currentColumn == 1)) currentColumn = 1;
                                ImGui::SetCursorPos(ImVec2(125, -10));
                                if (ImGui::Tab(("Cleaner"), currentColumn == 2)) currentColumn = 2; 
                                ImGui::SetCursorPos(ImVec2(215.5, 8));
                                ImGui::Text("Pande");
                                ImGui::SameLine();
                                ImGui::TextColored(ImColor(51, 51, 255), "monium");
                                ImGui::SetCursorPos(ImVec2(325, -10));
                                if (ImGui::Tab(("Settings"), currentColumn == 3)) currentColumn = 3;
                                ImGui::SetCursorPos(ImVec2(425, -10));
                                if (ImGui::Tab(("User Info"), currentColumn == 4)) currentColumn = 4;
                                ImGui::SetCursorPos(ImVec2(18, 334));
                                ImGui::Text("Pande");
                                ImGui::SameLine(50);
                                ImGui::TextColored(ImColor(51, 51, 255), "monium");
                                ImGui::PopFont();

                                ImGui::PopStyleVar();
                            }
                            ImGui::EndGroup();
                        }
                        {//Main Area

                            switch (currentColumn) {
                                break;
                            case 1:


                                
                                


                                ImGui::SetCursorPos(ImVec2(16, 45));

                                ImGui::BeginChildss("##Spoofer", ImVec2(494.5, 260 + 25)); ImColor(0, 115, 230);
                                {
                                    ImGui::SetCursorPos(ImVec2(10, 16));
                                    ImGui::BeginGroup();

                                    ImGui::PushFont(f7);
                                    
                                    
                                    
                                    
                                    if (spooferpage == 1)
                                    {

                                        
                                        ImGui::SameLine(22.5);
                                        ImGui::Image((void*)my_warzonetexture, ImVec2(my_warzoneimage_width, my_warzoneimage_height));

                                        ImGui::SameLine(170);
                                        ImGui::Image((void*)my_valoranttexture, ImVec2(my_valorantimage_width, my_valorantimage_height));
                                        ImGui::SameLine(320);
                                        ImGui::Image((void*)my_apextexture, ImVec2(my_apeximage_width, my_apeximage_height));
                                        ImGui::SetCursorPos(ImVec2(32, 163));

                                        if (ImGui::Button("            LAUNCH            "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }
                                        ImGui::SetCursorPos(ImVec2(180, 163));
                                        if (ImGui::Button("             LAUNCH           "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }
                                        ImGui::SetCursorPos(ImVec2(330, 163));
                                        if (ImGui::Button("            LAUNCH             "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }


                                        ImGui::SetCursorPos(ImVec2(225, 215));
                                        ImGui::Text("STATUS");
                                        ImGui::SetCursorPos(ImVec2(78, 238.6));
                                        ImGui::Text("Warzone:"); ImGui::SetCursorPos(ImVec2(125, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "Undetected");
                                        ImGui::SameLine(185);
                                        ImGui::Text("Valorant:"); ImGui::SetCursorPos(ImVec2(239, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "Undetected");
                                        ImGui::SameLine(300);
                                        ImGui::Text("Apex:"); ImGui::SetCursorPos(ImVec2(338, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "Undetected");

                                    }
                                    


                                    if (spooferpage == 2)
                                    {



                                        //using sameLine here doesnt really help cause it fucks with positions but its just something quick.
                                        ImGui::SameLine(22.5);
                                        ImGui::Image((void*)my_rusttexture, ImVec2(my_rustimage_width, my_rustimage_height));

                                        ImGui::SameLine(170);
                                        ImGui::Image((void*)my_tarkovtexture, ImVec2(my_tarkovimage_width, my_tarkovimage_height));
                                        ImGui::SameLine(320);
                                        ImGui::Image((void*)my_fortnitetexture, ImVec2(my_fortniteimage_width, my_fortniteimage_height));
                                        ImGui::SetCursorPos(ImVec2(32, 163));

                                        if (ImGui::Button("            LAUNCH            "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }
                                        ImGui::SetCursorPos(ImVec2(180, 163));
                                        if (ImGui::Button("             LAUNCH           "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }
                                        ImGui::SetCursorPos(ImVec2(330, 163));
                                        if (ImGui::Button("             LAUNCH            "))
                                        {
                                            spoofing = true;
                                            Spoofermenumain = false;
                                        }


                                        ImGui::SetCursorPos(ImVec2(225, 215));
                                        ImGui::Text("STATUS");
                                        ImGui::SetCursorPos(ImVec2(78, 238.6));
                                        ImGui::Text("      Rust:"); ImGui::SetCursorPos(ImVec2(125, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "Undetected");
                                        ImGui::SameLine(185);
                                        ImGui::Text("  FiveM:"); ImGui::SetCursorPos(ImVec2(232, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "  Undetected");
                                        ImGui::SameLine(300);
                                        ImGui::Text("Fortnite:                 "); ImGui::SetCursorPos(ImVec2(338, 238.6));  ImGui::TextColored(ImColor(42, 163, 20), "    Undetected");



                                    }




                                    ImGui::SetCursorPos(ImVec2(8, 90));
                                    if (ImGui::Button("<"))
                                    {
                                        spooferpage = 1;
                                    }
                                    ImGui::SetCursorPos(ImVec2(468, 90));
                                    if (ImGui::Button(">"))
                                    {
                                        spooferpage = 2;
                                    }

                                    ImGui::PopFont();

                                    ImGui::EndGroup();
                                }
                                ImGui::EndChild();



                                break;
                            case 2:
                                
                                    ImGui::SetCursorPos(ImVec2(16, 45));
                                    ImGui::BeginChildss("##cleaner", ImVec2(494.5, 260 + 25));
                                    {
                                        ImGui::PushFont(f7);
                                        ImGui::BeginGroup();
                                        

                                        if (cleanerpage == 1)
                                        {


                                            ImGui::SameLine(22.5);
                                            ImGui::Image((void*)my_warzonetexture, ImVec2(my_warzoneimage_width, my_warzoneimage_height));

                                            ImGui::SameLine(170);
                                            ImGui::Image((void*)my_valoranttexture, ImVec2(my_valorantimage_width, my_valorantimage_height));
                                            ImGui::SameLine(320);
                                            ImGui::Image((void*)my_apextexture, ImVec2(my_apeximage_width, my_apeximage_height));
                                            ImGui::SetCursorPos(ImVec2(32, 163));

                                            if (ImGui::Button("             CLEAN             "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }
                                            ImGui::SetCursorPos(ImVec2(180, 163));
                                            if (ImGui::Button("              CLEAN            "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }
                                            ImGui::SetCursorPos(ImVec2(330, 163));
                                            if (ImGui::Button("             CLEAN             "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }


                                            ImGui::SetCursorPos(ImVec2(210, 215));
                                            ImGui::Text("CLEANER INFO");
                                            ImGui::SetCursorPos(ImVec2(78, 238.6));
                                            ImGui::Text("Cleaners are used to clean traces to avoid bans on games so use it plz.");

                                        }



                                        if (cleanerpage == 2)
                                        {



                                            //using sameLine here doesnt really help cause it fucks with positions but its just something quick.
                                            ImGui::SameLine(22.5);
                                            ImGui::Image((void*)my_rusttexture, ImVec2(my_rustimage_width, my_rustimage_height));

                                            ImGui::SameLine(170);
                                            ImGui::Image((void*)my_tarkovtexture, ImVec2(my_tarkovimage_width, my_tarkovimage_height));
                                            ImGui::SameLine(320);
                                            ImGui::Image((void*)my_fortnitetexture, ImVec2(my_fortniteimage_width, my_fortniteimage_height));
                                            ImGui::SetCursorPos(ImVec2(32, 163));

                                            if (ImGui::Button("             CLEAN             "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }
                                            ImGui::SetCursorPos(ImVec2(180, 163));
                                            if (ImGui::Button("             CLEAN             "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }
                                            ImGui::SetCursorPos(ImVec2(330, 163));
                                            if (ImGui::Button("             CLEAN             "))
                                            {
                                                cleaning = true;
                                                Spoofermenumain = false;
                                            }



                                            ImGui::SetCursorPos(ImVec2(210, 215));
                                            ImGui::Text("CLEANER INFO");
                                            ImGui::SetCursorPos(ImVec2(78, 238.6));
                                            ImGui::Text("Cleaners are used to clean traces to avoid bans on games so use it plz.");
                                            



                                        }




                                        ImGui::SetCursorPos(ImVec2(8, 90));
                                        if (ImGui::Button("<"))
                                        {
                                            cleanerpage = 1;
                                        }
                                        ImGui::SetCursorPos(ImVec2(468, 90));
                                        if (ImGui::Button(">"))
                                        {
                                            cleanerpage = 2;
                                        }

                                        ImGui::PopFont();

                                        ImGui::EndGroup();
                                    }
                                    ImGui::EndChild();
                                    break;
                            case 3:
                               
                                ImGui::SetCursorPos(ImVec2(16, 45));
                                ImGui::BeginChildss("##Settings", ImVec2(494.5, 260 + 25));
                                {
                                    ImGui::BeginGroup();

                                    
                                    ImGui::PushFont(f7);

                                    ImGui::Text("       pandemonium.store | yaeow & imanj       ");
                                    ImGui::Checkbox("All Hwids", &ALLHWID);

                                    ImGui::SameLine();
                                    ImGui::Checkbox("MacID", &MacID);

                                    ImGui::Checkbox("ProductID", &ProductID);

                                    ImGui::SameLine();
                                    ImGui::Checkbox("UuID", &UUUID); //should be 2 u's here but I cannot do that due to a variable already being named that in windows.

                                    ImGui::Checkbox("CpuID", &CpuID);

                                    ImGui::SameLine();
                                    ImGui::Checkbox("RamID", &RAMID);

                                    ImGui::Checkbox("GpuID", &GpuID);

                                    ImGui::SameLine();
                                    ImGui::Checkbox("SmbiosID", &SMBIOSID);
                                    ImGui::SameLine();
                                    ImGui::Checkbox("MotherboardID", &MOTHERBOARDID);
                                    //ImGui::SameLine();


                                    ImGui::PopFont();

                                    ImGui::EndGroup();
                                }

                                ImGui::EndChild();
                                break;
                            case 4:

                                ImGui::SetCursorPos(ImVec2(16, 45));
                                ImGui::BeginChildss("##UserInfo", ImVec2(494.5, 260 + 25));
                                {
                                    ImGui::BeginGroup();

                                    ImGui::PushFont(f7);

                                    ImGui::Text("Time Left On Subscription");
                                    
                                    ImGui::PopFont();

                                    ImGui::EndGroup();
                                }

                                ImGui::EndChild();

                            }
                        }
                    }
                    }
                    if (spoofing == true)
                    {
                        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
                        ImGui::SetCursorPos(ImVec2(245.5, 145));
                        ImGui::Spinner("##spinner", 16, 4, col);
                        ImGui::SetCursorPos(ImVec2(220.5, 200));
                        ImGui::Text("Spoofing! %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                    }
                    if (cleaning)
                    {
                        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
                        ImGui::SetCursorPos(ImVec2(245.5, 145));
                        ImGui::Spinner("##spinner", 16, 4, col);
                        ImGui::SetCursorPos(ImVec2(220.5, 200));
                        ImGui::Text("Cleaning! %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
                        
                    }
                }
                ImGui::End();
            
        }
        ImGui::EndFrame();

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ResetDevice();
        }
        if (!loader_active) 
        {
            msg.message = WM_QUIT;
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(main_hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}