#ifndef UI_HPP
#define UI_HPP

#include "SDL2/SDL.h"
#include "common.hpp"
#include <vector>
#include "ui_classes.hpp"


void drawMap();
void statusBars();
void drawCircle(int x, int y, int radius);
void drawText(int score, int missiles);
void energyChoice();
void drawUpgrades(int score);
void drawWeap();

std::vector<enemy>* ep;
std::vector<laser>* lp;
std::vector<missile>* mp;
std::vector<ship>* ap;
player* ply;
int* scorept;

selection energy_menu;
selection upgrades_menu;

std::string roman_nums[] = {"I","II","III","IV","V","VI","VII","IX","X"};

struct selectionReturn
{
	bool sel;
	int val;
};

void energyChoice()
{
	energy_menu.draw();
}

selectionReturn energy_clicked(vec2 p)
{
	bool ret1 = energy_menu.click(p);
	return {ret1, energy_menu.getSelected()};
}

void drawUpgrades(int score)
{
  std::vector<button> * vec = upgrades_menu.getButtons();
	for(size_t i = 0; i < vec->size(); i++)
	{
		if(vec->at(i).getCost() > score or (i < 4 and ply->getUpgrade(i) > 7))
		{ 
			vec->at(i).setDark(true);
		}
		else vec->at(i).setDark(false);
	}
	
	upgrades_menu.draw();
}

selectionReturn upgrades_clicked(vec2 p)
{
	bool ret1 = upgrades_menu.click(p);
	return {ret1, upgrades_menu.getSelected()};
}

void setUpgradeTextures(int lvl, int type)
{
	if(type > 3) return;
	
  std::string s1;
	
	button * up = upgrades_menu.getAt(type);
	up->setCost(pow(4, lvl + 1));
	
	switch(type)
	{
		case 0:
			s1 = "LASERS ";
			break;
		case 1:
			s1 = "SHIELDS ";
			break;
		case 2:
			s1 = "GENERATORS ";
			break;
		case 3:
			s1 = "THRUSTERS ";
			break;
	}

	s1 += roman_nums[lvl];
	
	if(lvl < 8)
	{
		s1 += " (";
    std::stringstream ss;
		ss << up->getCost();
		s1 += ss.str();
		s1 += ")";
	}
	
	if(lvl < 9) up->updateText(s1);
}

void playerUpgrade(int lvl)
{
	int type = upgrades_menu.getSelected();
	button * up = upgrades_menu.getAt(type);
	up->set(false);
	
	if(lvl < 9)
	{
		*scorept -= up->getCost();
		if(type < 4) up->setCost(up->getCost() * 2);
	}
	
	if(type > 3) return;
	
  std::string s1;
	
	switch(type)
	{
		case 0:
			s1 = "LASERS ";
			break;
		case 1:
			s1 = "SHIELDS ";
			break;
		case 2:
			s1 = "GENERATORS ";
			break;
		case 3:
			s1 = "THRUSTERS ";
			break;
	}
	
	s1 += roman_nums[lvl];
	
	if(lvl < 8)
	{
		s1 += " (";
    std::stringstream ss;
		ss << up->getCost();
		s1 += ss.str();
		s1 += ")";
	}
	
	if(lvl < 9) up->updateText(s1);
}

void drawWeap()
{
	SDL_Rect weap;
	weap.w = WIN_WIDTH * 0.1;
	weap.h = WIN_HEIGHT * 0.2;
	weap.x = WIN_WIDTH - weap.w;
	weap.y = WIN_HEIGHT - 1.4 * weap.h;
	
	int wn = ply->getWeap();
	
	SDL_SetRenderDrawColor(renderer,weapons[wn][4],weapons[wn][5],weapons[wn][6],255);
	SDL_RenderFillRect(renderer,&weap);
	
	
  TTF_Font * fnt = TTF_OpenFont("../resources/fonts/minimal.otf", 25);
	
  std::string weapText = "Damage ";

  weapText += std::to_string(weapons[wn][2]);
	
	weapText += "\nLasers ";
  weapText += std::to_string(weapons[wn][0]);
	
	weapText += "\nSpread ";
  weapText += std::to_string(weapons[wn][1]);
	
	weapText += "\nRate ";
	float rate = 1.0f / weapons[wn][8];
  weapText += std::to_string( static_cast<int>( rate ) );
		
	SDL_Texture * weapsTexture = renderText(
		weapText.c_str(),
		fnt,
		{255,255,255,255},
		renderer,
		WIN_WIDTH * 0.1
		);
	
	SDL_RenderCopy(renderer, weapsTexture, NULL, &weap);
	
	TTF_CloseFont(fnt);
	SDL_DestroyTexture(weapsTexture);
}

#endif