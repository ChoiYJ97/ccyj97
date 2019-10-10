// Screen.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <ctime>
#include "Utils.h"

using namespace std;

class GameObject {
	char*		shape;
	int			width;
	int			height;
	Position	pos;
	Screen&		screen;

	vector<GameObject *> children;


public:
	GameObject(const char* shape, int width, int height, const Position& pos = Position{ 0, 0 } )
		: height(height), width(width), 
		shape(nullptr), pos(pos), 
		screen(Screen::getInstance()) {
		if (!shape || strlen(shape) == 0 || width == 0 || height == 0)
		{
			this->shape = new char[1];
			this->shape[0] = 'x';
			width = 1;
			height = 1;
		} else {
			this->shape = new char[width*height];
			strncpy(this->shape, shape, width*height);
		}
		this->width = width;
		this->height = height;
	}

	virtual ~GameObject() {
		if (shape) { delete[] shape; }
		width = 0, height = 0;
	}

	void add(GameObject* child) {
		if (!child) return;
		children.push_back(child);
	}

	void setShape(const char* shape) {
		if (!shape) return;
		strncpy(this->shape, shape, width*height);
	}

	void setPos(int x, int y) { this->pos.x = x; this->pos.y = y; }

	Position& getPos() { return pos; }

	void internalDraw(const Position& accumulatedPos = Position{ 0,0 })
	{
		draw(accumulatedPos);
		for (auto child : children) 
			child->internalDraw( pos + accumulatedPos );
	}
		
	virtual void draw(const Position& accumulatedPos = Position{ 0,0 })
	{		
		screen.draw(shape, width, height, pos + accumulatedPos);
	}

	void internalUpdate()
	{
		update();
		for (auto child : children)
			child->internalUpdate();
	}

	virtual void update() 
	{	
	}

	vector<GameObject *>& getChildren() { return children;  }
};

class Block : public GameObject {	

	vector<string> sprites;
	char array[50][70];
	int current;

public:
	Block(vector<string>& sprites, int w, int h, 
		const Position& pos = Position{ 0,0 } )
		: sprites(sprites), current(0), 
		GameObject(sprites[current].c_str(), w, h, pos) {}

	void update() {
		if (getPos().y != 36) {
			if (Input::GetKeyDown(KeyCode::Right)) {
			getPos().x++;
				}
			if (Input::GetKeyDown(KeyCode::Left)) {
				getPos().x--;
			}
			if (Input::GetKeyDown(KeyCode::Up)) {
				current = (current + 1) % sprites.size();
				setShape(sprites[current].c_str());
			}
			if (Input::GetKeyDown(KeyCode::Down)) {
				getPos().y = Screen::getInstance().getHeight();
			}
			if (Input::GetKeyDown(KeyCode::A)) {
				current = (current + 1) % sprites.size();
				setShape(sprites[current].c_str());
			}
			if (Input::GetKeyDown(KeyCode::D)) {
				current = (current + 3) % sprites.size();
				setShape(sprites[current].c_str());
			}
			getPos().y = (getPos().y + 1) % Screen::getInstance().getHeight();
		}
		else {  //2차원 배열로 위치에 저장시켜놓기
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++)
				{
					if (array[getPos().y + i][getPos().x + j] != ' ')
						array[getPos().y + i][getPos().x + j] = '\xB2';
				}
			}
		}
	}
};

int main()
{
	clock_t t1, t2;
	int plus = 0;
	Position Gamemap = { 0,0 };
	Position Nextblock = {40, 0};
	vector<string> sprites { "\xB2\xB2  \xB2  \xB2 ",
		"     \xB2\xB2\xB2\xB2", 
		" \xB2  \xB2  \xB2\xB2", 
		"   \xB2\xB2\xB2\xB2  ",
		"    \xB2\xB2 \xB2\xB2",
		"      \xB2\xB2\xB2",
		"  \xB2  \xB2  \xB2",
		"    \xB2 \xB2\xB2\xB2",
		"  \xB2 \xB2\xB2  \xB2",
		"   \xB2\xB2\xB2 \xB2 ",
		"\xB2  \xB2\xB2 \xB2  "
	};
	vector<string> sprites2{ " ", " " };

	Screen&	 screen = Screen::getInstance();
	vector<GameObject *> gameObjects;

	srand(time(nullptr));

	string mode{ "mode con cols="
		+ to_string(screen.getWidth() + 4)
		+ " lines=" + to_string(screen.getHeight() + 5) };
	system(mode.c_str());
	system("chcp 437");

	auto parent = new Block{ sprites, 3,3,
		Position{screen.getWidth()/10, 1} };;

	auto child = new Block{ sprites2, 1,1, Position{0, 2} };
	parent->add(child);
	gameObjects.push_back(parent);
	
	screen.clear(); screen.render();		

	while (true)
	{	
		screen.clear();
		screen.drawRect(Gamemap,30,40);
		screen.drawRect(Nextblock, 10,5);
		t1 = clock();

		for (auto obj : gameObjects) obj->internalUpdate();

		for (auto it = gameObjects.cbegin(); 
			it != gameObjects.cend(); it++)
			(*it)->internalDraw();
		
		screen.render();




		t2 = clock();
		if ((t2 - t1) / 1000 > 10)
			plus++;
		Sleep(150 - plus);

		Input::EndOfFrame();		
	}

	return 0;
}