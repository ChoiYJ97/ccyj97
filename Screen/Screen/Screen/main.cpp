// Screen.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include "Utils.h"

using namespace std;

class Screen {
	int width;
	char* canvas;

	static Screen* instance;
	Screen(int width = 40) 
		: width(width),
		canvas(new char[(width+1)])
	{
		Borland::initialize();
	}
public:
	static Screen& getInstance() {
		if (instance == nullptr) {
			instance = new Screen();
		}
		return *instance;
	}

	~Screen() {
		if (instance) {
			delete[] canvas;
			instance = nullptr;
		}
	}

	void draw(const char* shape, int w, const Position& pos)
	{
		if (!shape) return;
		for (int i = 0; i < 1; i++)
		{
			strncpy(&canvas[pos.x + (pos.y+i)*(width+1)], &shape[i*w], w);
		}
	}

	void render()
	{
		for (int i = 0; i < width; i++)
			canvas[i] = 'O';
		Borland::gotoxy(0, 0);
		cout << canvas;
	}

	void clear()
	{
		memset(canvas, ' ', (width + 1));
		canvas[width+1] = '\0';
	}

};

Screen* Screen::instance = nullptr;

class GameObject {
	char* shape;
	int width;
	Position pos;
	Screen& screen;

public:
	GameObject(const char* shape, int width) 
		: width(width), shape(nullptr), pos(0, 0),
		screen(Screen::getInstance())
	{
		if (!shape || strlen(shape) == 0 || width == 0)
		{
			this->shape = new char[1];
			this->shape[0] = 'x';
			width = 1;
		} else {
			this->shape = new char[width];
			strncpy(this->shape, shape, width);
		}
		this->width = width;
	}
	virtual ~GameObject() {
		if (shape) { delete[] shape; }
		width = 0;
	}

	void setPos(int x)
	{ 
		this->pos.x = x;
	}

	void draw() {
		screen.draw(shape, width, pos);
	}

	virtual void update(const Position& pos) {
		
	}

};

int main()
{
	GameObject mine("0", 3);
	
	Screen&	 screen = Screen::getInstance();
	INPUT_RECORD InputRecord;
	DWORD Events;

	screen.clear(); screen.render();
	while (true)
	{
		mine.draw();
		screen.render();		
		Sleep(30);
		screen.clear();

		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &InputRecord, 1, &Events);
		if (InputRecord.EventType == MOUSE_EVENT) {
			if (InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
				COORD coord;
				coord.X = InputRecord.Event.MouseEvent.dwMousePosition.X;
				coord.Y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
				Position pos;
				pos.x = InputRecord.Event.MouseEvent.dwMousePosition.X;
				pos.y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
				mine.update(pos);
			}
		}
		
	}

	return 0;
}