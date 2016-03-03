#include "ui/button.hpp"

button::button(std::string _txt, int _b_col[], int _t_col[], vec2 _pos, vec2 _dim)
{
  dark = false;
  selected = false;

  m_initLabel = _txt;
  m_label = _txt;

  for(size_t i = 0; i < 8; i++)
  {
    col[i] = _b_col[i];
  }
  for(size_t i = 0; i < 8; i++)
  {
    tcol[i] = _t_col[i];
  }

  m_pos.x = _pos.x;
  m_pos.y = _pos.y;
  m_dim.x = _dim.x;
  m_dim.y = _dim.y;

  m_initCost = 0;
  cost = 0;
}

button::button(std::string _txt, int _b_col[], int _t_col[], vec2 _pos, vec2 _dim, int _pcost)
{
  dark = true;
  selected = false;

  m_initLabel = _txt;
  m_label = _txt;

  for(size_t i = 0; i < 8; i++)
  {
    col[i] = _b_col[i];
  }
  for(size_t i = 0; i < 8; i++)
  {
    tcol[i] = _t_col[i];
  }

  m_pos.x = _pos.x;
  m_pos.y = _pos.y;
  m_dim.x = _dim.x;
  m_dim.y = _dim.y;

  m_initCost = pcost;
  cost = _pcost;
}

void button::updateText(std::string _text)
{
  m_text = _text;
}

void button::select()
{
  if(selected)
  {
    selected = false;
    SDL_SetTextureColorMod(texture,tcol[0],tcol[1],tcol[2]);
    SDL_SetTextureAlphaMod(texture,tcol[3]);
  }
  else
  {
    selected = true;
    SDL_SetTextureColorMod(texture,tcol[4],tcol[5],tcol[6]);
    SDL_SetTextureAlphaMod(texture,tcol[7]);
  }
}

void button::reset()
{
  m_label = m_initLabel;
  cost = m_initCost;
}
