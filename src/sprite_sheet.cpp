#include "sprite_sheet.hpp"

sprite_sheet create(SDL_Renderer * _renderer, std::string _path, int _w, int _h)
{
  sprite_sheet sheet;
  sheet.m_w = _w;
  sheet.m_h = _h;
  sheet.m_sheet = SDL_CreateTextureFromSurface( _renderer, IMG_Load( path ) )
}

void sprite_sheet::destroy()
{
  SDL_DestroyTexture(m_sheet);
}
