/*
	구현내용
	1. 배경 이미지 추가						- 구현
	2. 총알 격추 시 사라짐					- 구현
	3. 적 피격 시 사라짐						- 구현
	4. hp 생성								- 구현
	5. 적과 주인공 부딪히면 피 -1				- 구현
	6. 피 3개 다 사용시 게임 종료				- 구현
	7. 총알 여러 발을 쏘는 알고리즘			- 구현 실패
	8. 점수 및 점수판 구현					- 구현 실패
	9. 몬스터 스폰 수를 특정 점수 마다 증가	- 구현
	10. 필살기(스킬) 구현						- 구현 실패
	11. 주인공 벽 밖으로 못나가게 하기			- 구현
	12. 몬스터 움직임의 방향을 랜덤하게 주기	- 구현
*/

// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <tchar.h>

// define the screen resolution and keyboard macros
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#define ENEMY_NUM 30 

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPD3DXSPRITE d3dspt;    // the pointer to our Direct3D Sprite interface
LPD3DXFONT dxfont;    // the pointer to the font object

// sprite declarations
LPDIRECT3DTEXTURE9 sprite;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_hero;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_enemy;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_bullet;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_space;    // the pointer to the sprite
LPDIRECT3DTEXTURE9 sprite_hp;    // the pointer to the sprite

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);		// closes Direct3D and releases memory

void init_game(void);
void do_game_logic(void); 


// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

using namespace std; 


enum {MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT}; 


//기본 클래스 
class entity{

public: 
float x_pos; 
float y_pos; 
int status; 

virtual void fire() {}
virtual void hit() {}
};

//주인공 클래스 
class Hero:public entity{

public: 
	bool hitted;

	void fire() {}
	void super_fire() {}
	void move(int i) {
		switch (i)
		{
		case MOVE_UP:
			y_pos -= 5;
			break;
		case MOVE_DOWN:
			y_pos += 5;
			break;
		case MOVE_LEFT:
			x_pos -= 5;
			break;
		case MOVE_RIGHT:
			x_pos += 5; 
			break;
		}
	}
	void init(float x, float y) { x_pos = x;	y_pos = y; }
	void hit() {}
}; 

// 적 클래스 
class Enemy:public entity{
	
public:

	//void fire(); 
	void init(float x, float y) { x_pos = x;	y_pos = y; }
	void move(int i){
		switch (i)
		{
		case 0:
			y_pos += 3;
			break;
		case 1:
			y_pos += 3;
			x_pos += 3;
			break;
		case 2:
			y_pos += 3;
			x_pos -= 3;
			break;
		case 3:
			y_pos += 0;
			x_pos += 2 - i;
			break;
		}
	}
	void hit() { x_pos = (float)(rand() % 600); y_pos = rand() % 200 - 300; }
}; 

// 총알 클래스 
class Bullet:public entity{
	
public: 
bool bShow; 

void init(float x, float y) { x_pos = x;	y_pos = y; }
void move() { y_pos -= 8; }
bool show() { return bShow; }
void hide() { bShow = false; }
void active(){ bShow = true; }
}; 

//주인공 hp객체
class Hp:public Hero {
public:
	bool show;
	void init(float x, float y) { x_pos = x; y_pos = y; }
	void erase() { x_pos = -70; }
};

//객체 생성 
Hero hero;					//주인공
Enemy enemy[ENEMY_NUM];		//적
Bullet bullet;				//총알
Hp hp[3];					//체력
int Hitted_count = 0;		//게임 끝내기 위한 전역변수
int Score = 0;				//점수

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL, L"WindowClass", L"Our Direct3D Program",
                          WS_EX_TOPMOST | WS_POPUP, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    // set up and initialize Direct3D
    initD3D(hWnd);


	//게임 오브젝트 초기화 
	init_game();

    // enter the main loop:

    MSG msg;

    while(TRUE)
    {
        DWORD starting_point = GetTickCount();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		do_game_logic();


        render_frame();

        // check the 'escape' key
        if(KEY_DOWN(VK_ESCAPE))
            PostMessage(hWnd, WM_DESTROY, 0, 0);




        while ((GetTickCount() - starting_point) < 25);
		if (Hitted_count >= 3) break;
    }

    // clean up DirectX and COM
    cleanD3D();

    return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}


// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = FALSE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;


    // create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

    D3DXCreateSprite(d3ddev, &d3dspt);    // create the Direct3D Sprite object

    D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"Panel3.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite);    // load to sprite


    D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"hero.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_hero);    // load to sprite

    D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"enemy.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_enemy);    // load to sprite


	D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
                                L"bullet.png",    // the file name
                                D3DX_DEFAULT,    // default width
                                D3DX_DEFAULT,    // default height
                                D3DX_DEFAULT,    // no mip mapping
                                NULL,    // regular usage
                                D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                                D3DPOOL_MANAGED,    // typical memory handling
                                D3DX_DEFAULT,    // no filtering
                                D3DX_DEFAULT,    // no mip filtering
                                D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
                                NULL,    // no image info struct
                                NULL,    // not using 256 colors
                                &sprite_bullet);    // load to sprite

	D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
								L"space.png",    // the file name
								D3DX_DEFAULT,    // default width
								D3DX_DEFAULT,    // default height
								D3DX_DEFAULT,    // no mip mapping
								NULL,    // regular usage
								D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
								D3DPOOL_MANAGED,    // typical memory handling
								D3DX_DEFAULT,    // no filtering
								D3DX_DEFAULT,    // no mip filtering
								D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
								NULL,    // no image info struct
								NULL,    // not using 256 colors
								&sprite_space);    // load to sprite

	D3DXCreateTextureFromFileEx(d3ddev,    // the device pointer
								L"hp.png",    // the file name
								D3DX_DEFAULT,    // default width
								D3DX_DEFAULT,    // default height
								D3DX_DEFAULT,    // no mip mapping
								NULL,    // regular usage
								D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
								D3DPOOL_MANAGED,    // typical memory handling
								D3DX_DEFAULT,    // no filtering
								D3DX_DEFAULT,    // no mip filtering
								D3DCOLOR_XRGB(255, 0, 255),    // the hot-pink color key
								NULL,    // no image info struct
								NULL,    // not using 256 colors
								&sprite_hp);    // load to sprite

	D3DXCreateFont(d3ddev,    // the D3D Device
		40,    // font height of 30
		0,    // default font width
		FW_NORMAL,    // font weight
		1,    // not using MipLevels
		true,    // italic font
		DEFAULT_CHARSET,    // default character set
		OUT_DEFAULT_PRECIS,    // default OutputPrecision,
		DEFAULT_QUALITY,    // default Quality
		DEFAULT_PITCH | FF_DONTCARE,    // default pitch and family
		L"Arial",    // use Facename Arial
		&dxfont);    // the font object

    return;
}

//게임 케릭터 및 오브젝트 초기화
void init_game(void)
{
	//객체 초기화 
	hero.init(SCREEN_WIDTH/2, SCREEN_HEIGHT-64);
	
	//적들 초기화 
	for (int i=0; i<ENEMY_NUM-(20 - Score/5); i++)
	{
		enemy[i].init((float) (rand()%500 +50), rand()%200 - 300); 
	}

	//총알 초기화 
	bullet.init(-10, -10);

	for (int i = 0; i < 3; i++) {
		hp[i].init(0, SCREEN_HEIGHT - (i+1) * 32);
	}
}

//게임 로직 함수
void do_game_logic(void)
{
	srand((unsigned)time(NULL));
	int num = rand() % 4;

	//주인공 움직임 처리 
	if (KEY_DOWN(VK_UP)) {
		hero.move(MOVE_UP);
		if (hero.y_pos < 0)
			hero.y_pos = 0;
	}
	if (KEY_DOWN(VK_DOWN)) {
		hero.move(MOVE_DOWN);
		if (hero.y_pos > SCREEN_HEIGHT - 64)
			hero.y_pos = SCREEN_HEIGHT - 64;
	}
	if (KEY_DOWN(VK_LEFT)) {
		hero.move(MOVE_LEFT);
		if (hero.x_pos < 0)
			hero.x_pos = 0;
	}
	if (KEY_DOWN(VK_RIGHT)) {
		hero.move(MOVE_RIGHT);
		if (hero.x_pos > SCREEN_WIDTH - 64)
			hero.x_pos = SCREEN_WIDTH - 64;
	}

	//적들 처리 - 움직임  
	for (int i = 0; i < ENEMY_NUM - (20 - Score / 5); i++) {
		if(enemy[i].y_pos > 500)
			enemy[i].init((float) (rand()%600 - 10*i +50), rand()%200 - 300 + 10 * i);
		else{
			enemy[i].move((num+i)%4);
			if (enemy[i].x_pos < 32) { enemy[i].x_pos = 32; enemy[i].move(1);}
			if (enemy[i].x_pos > SCREEN_WIDTH - 32) { enemy[i].x_pos = SCREEN_WIDTH - 32; enemy[i].move(2);}
		}
	}

	//적들 및 주인공, HP처리 - 피격처리, 주인공 피격처리 
	for (int i=0; i< ENEMY_NUM - (20 - Score / 5); i++)
	{
		//총알 격돌시 파괴
		if (enemy[i].x_pos <= bullet.x_pos + 66 && enemy[i].y_pos <= bullet.y_pos + 66) {
			if (enemy[i].x_pos >= bullet.x_pos - 2 && enemy[i].y_pos >= bullet.y_pos - 2) {
				enemy[i].hit();
				bullet.bShow = false; //적 적중 시 사라짐
				bullet.init(-100, -100); //맞춘 현 위치에 남지 않게 맵 밖으로 보내서 없앤다.
				Score++;
			}
		}
		//주인공 피격 처리
		if (enemy[i].x_pos <= hero.x_pos + 64 && enemy[i].y_pos <= hero.y_pos + 64) {
			if (enemy[i].x_pos >= hero.x_pos && enemy[i].y_pos >= hero.y_pos){
				hp[Hitted_count].erase();
				Hitted_count += 1;
				hero.init(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 64);
			}

		}
	}

	//총알 처리 - 여러발 쏘는 알고리즘 - 실패
	/*if (bullet.show() == false)
	{
		if (KEY_DOWN(VK_SPACE))
		{
			bullet.active();
			bullet.init(hero.x_pos, hero.y_pos);
		}
	}
	if (bullet.show() == true)
	{
		if (bullet.y_pos < -70)
			bullet.hide();
		else
			bullet.move();
	}
	*/

	//총알 처리 - 한발 쏘는 알고리즘
	if (bullet.show() == false)
	{
		if (KEY_DOWN(VK_SPACE))
		{
			bullet.active();
			bullet.init(hero.x_pos, hero.y_pos);
		}
	}
	if (bullet.show() == true)
	{
		if (bullet.y_pos < -70)
			bullet.hide();
		else
			bullet.move();
	}
}

// this is the function used to render a single frame
void render_frame(void)
{
    // clear the window to a deep blue
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    d3ddev->BeginScene();    // begins the 3D scene

    d3dspt->Begin(D3DXSPRITE_ALPHABLEND);    // // begin sprite drawing with transparency

	//UI 창 렌더링 
	
	/*
	static int frame = 21;    // start the program on the final frame
    if(KEY_DOWN(VK_SPACE)) frame=0;     // when the space key is pressed, start at frame 0
    if(frame < 21) frame++;     // if we aren't on the last frame, go to the next frame

    // calculate the x-position
    int xpos = frame * 182 + 1;

	RECT part;
    SetRect(&part, xpos, 0, xpos + 181, 128);
    D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);    // center at the upper-left corner
    D3DXVECTOR3 position(150.0f, 50.0f, 0.0f);    // position at 50, 50 with no depth
    d3dspt->Draw(sprite, &part, &center, &position, D3DCOLOR_ARGB(127, 255, 255, 255));
	*/

	//배경 이미지
	RECT space;
	SetRect(&space, 0, 0, 650, 500);
	D3DXVECTOR3 center_space(0.0f, 0.0f, -2.0f);  
	D3DXVECTOR3 position_space(0.0f, 0.0f, 0.0f);  
	d3dspt->Draw(sprite_space, &space, &center_space, &position_space, D3DCOLOR_ARGB(255, 255, 255, 255));

	//주인공 
	RECT Hero;
	SetRect(&Hero, 0, 0, 64, 64);
    D3DXVECTOR3 center_hero(0.0f, 0.0f, 0.0f);   
	D3DXVECTOR3 position_hero(hero.x_pos, hero.y_pos, 0.0f);   
	d3dspt->Draw(sprite_hero, &Hero, &center_hero, &position_hero, D3DCOLOR_ARGB(255, 255, 255, 255));

	//총알 
	RECT Bullet;
	SetRect(&Bullet, 0, 0, 64, 64);
	D3DXVECTOR3 center_bullet(0.0f, 0.0f, 0.0f);   
	if (bullet.bShow == true)
	{
		D3DXVECTOR3 position_bullet(bullet.x_pos, bullet.y_pos, 0.0f);    
		d3dspt->Draw(sprite_bullet, &Bullet, &center_bullet, &position_bullet, D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	//적 
	RECT Enemy;
	SetRect(&Enemy, 0, 0, 64, 64);
    D3DXVECTOR3 center_enemy(32.0f, 32.0f, -1.0f);    
	for (int i=0; i<ENEMY_NUM; i++)
	{
		D3DXVECTOR3 position_enemy(enemy[i].x_pos, enemy[i].y_pos, 0.0f);    
		d3dspt->Draw(sprite_enemy, &Enemy, &center_enemy, &position_enemy, D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	//체력 게이지
	RECT Hp;
	SetRect(&Hp, 0, 0, 32, 32);
	D3DXVECTOR3 center_hp(0.0f, 0.0f, 0.0f);    
	for (int i = 0; i < 3; i++)
	{
		D3DXVECTOR3 position_hp(hp[i].x_pos, hp[i].y_pos, 0.0f);    
		d3dspt->Draw(sprite_hp, &Hp, &center_hp, &position_hp, D3DCOLOR_ARGB(255, 255, 255, 255));
	}

	char print[20];
	_itoa(Score, print, 10);
	// 점수 출력 - 구현 실패
	RECT textbox;
	SetRect(&textbox, 32, SCREEN_HEIGHT-40, 100, SCREEN_HEIGHT);
	dxfont->DrawTextA(NULL,
		print,
		3,
		&textbox,
		DT_LEFT,
		D3DCOLOR_ARGB(255, 255, 255, 255));
	
    d3dspt->End();    // end sprite drawing

    d3ddev->EndScene();    // ends the 3D scene

    d3ddev->Present(NULL, NULL, NULL, NULL);

    return;
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
    sprite->Release();
    d3ddev->Release();
    d3d->Release();

	//객체 해제 
    sprite_hero->Release();
	sprite_enemy->Release();
	sprite_bullet->Release();
	sprite_space->Release();
	sprite_hp->Release();
    return;
}