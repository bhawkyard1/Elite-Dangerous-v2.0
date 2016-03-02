#include <string>
#include "renderer.hpp"
#include "util.hpp"

#include "enemy.hpp"
#include "laser.hpp"
#include "missile.hpp"

renderer::renderer(int _w, int _h)
{
  init();

  m_w = _w;
  m_h = _h;

  m_window = SDL_CreateWindow("Elite: Dangerous v2.0",
                                          WIN_POS_X, WIN_POS_Y,
                                          WIN_HEIGHT, WIN_WIDTH,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED );

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
}

renderer::~renderer()
{
  SDL_DestroyWindow( m_window );
}

renderer::init()
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    std::cerr << "SDL_Init() failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return EXIT_FAILURE;
  }

  if(TTF_Init() != 0)
  {
    std::cerr << "TTF_Init() failed: " << TTF_GetError() << std::endl;
    SDL_Quit();
    return EXIT_FAILURE;
  }
}

void renderer::loadTextures()
{
  loadTextureSet("FEDERATION_MKI",      "enemy_1");
  loadTextureSet("FEDERATION_MKII",     "enemy_2");
  loadTextureSet("FEDERATION_MKIII",    "enemy_3");
  loadTextureSet("FEDERATION_MKIV",     "enemy_4");
  loadTextureSet("FEDERATION_GUNSHIP",  "enemy_5");

  loadTextureSet("PIRATE_GNAT",         "pirate_1");
  loadTextureSet("PIRATE_CRUISER",      "pirate_2");
  loadTextureSet("PIRATE_WRANGLER",     "pirate_3");
  loadTextureSet("PIRATE_MAURAUDER",    "pirate_4");
  loadTextureSet("PIRATE_GUNSHIP",      "pirate_5");

  loadTextureSet("PLAYER_SHIP",         "player");
  loadTextureSet("PLAYER_HUNTER",       "wingman_1");
  loadTextureSet("PLAYER_DEFENDER",     "wingman_2");
  loadTextureSet("PLAYER_DESTROYER",    "wingman_3");
  loadTextureSet("PLAYER_MINER_DROID",  "droid_1");
  loadTextureSet("PLAYER_TURRET",       "turret_1");
  loadTextureSet("PLAYER_STATION",      "station_1");
  loadTextureSet("PLAYER_GRAVWELL",     "well_1");
  loadTextureSet("PLAYER_BARRACKS",     "barracks_1");

  loadTextureSet("ION_MISSILE_MKI",     "missile");

  loadTextureSet("ASTEROID_SMALL",      "asteroid_1");
  loadTextureSet("ASTEROID_MID",        "asteroid_2");
  loadTextureSet("ASTEROID_LARGE",      "asteroid_3");

  loadTexture("STARDUST_1", "stardust_1");
  loadTexture("STARDUST_2", "stardust_2");
  loadTexture("STARDUST_3", "stardust_3");
  loadTexture("STARDUST_4", "stardust_4");

  loadTexture("EXPLOSION", "explosion_1");
}

void renderer::loadTexture(std::string _key, std::string _path)
{
  std::vector<SDL_Texture*> temp;
  temp.push_back( SDL_CreateTextureFromSurface(m_renderer, IMG_Load( "../resources/textures/" + _path + "/" + _path + ".png" ) ) );
  m_textures[_key] = temp;
}

void renderer::loadTextureSet(std::string key, std::string _set)
{
  vector<SDL_Surface*> temp_surf;
  temp_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + ".png") );
  temp_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + "_engines.png") );
  temp_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + "_steering.png") );
  temp_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + "_shoot.png") );
  temp_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + "_shield.png") );
  ship_surf.push_back( IMG_Load("../resources/textures/" + _set + "/" + _set + "_static.png") );

  vector<SDL_Texture*> temp_tex;
  for(size_t i = 0; i < temp_surf.size(); ++i)
  {
    if(temp_surf.at(i) != nullptr)
    {
      temp_tex.push_back( SDL_CreateTextureFromSurface( m_renderer, temp_surf.at(i) ) );
      SDL_FreeSurface(temp_surf.at(i));
    }
    else
    {
      std::cout << SDL_GetError() << std::endl;
      temp_tex.push_back( nullptr );
    }
  }
  m_textures[_key] = temp_tex;
}

void renderer::clear()
{
    //Clear renderer.
    SDL_SetRenderDrawColor( renderer, 4, 1, 8, 255);
    SDL_RenderClear(renderer);
}

void renderer::drawTextureSet(std::string key, vec2 pos, float orient, float * alphaMod)
{
  int w, h;
  SDL_QueryTexture(m_textures.at(key).at(0), NULL, NULL, &w, &h);

  pos *= ZOOM_LEVEL;
  pos += HALFWIN;

  w *= ZOOM_LEVEL;
  h *= ZOOM_LEVEL;

  SDL_Rect drawRect;
  drawRect.x = p.x - (w/2);
  drawRect.y = p.y - (h/2);
  drawRect.w = w;
  drawRect.h = h;

  SDL_SetTextureAlphaMod(m_textures.at(key).at(1), alphaMod[1]);
  SDL_SetTextureAlphaMod(m_textures.at(key).at(2), alphaMod[2]);
  SDL_SetTextureAlphaMod(m_textures.at(key).at(3), alphaMod[3]);
  SDL_SetTextureAlphaMod(m_textures.at(key).at(4), alphaMod[4]);
  SDL_SetTextureColorMod(m_textures.at(key).at(3), weapons[curWeap][4], weapons[curWeap][5], weapons[curWeap][6]);

  for(int i = 0; i < 5; ++i) SDL_RenderCopyEx(m_renderer, m_textures.at(key).at(i), NULL, &drawRect, orient, NULL, SDL_FLIP_NONE);
  SDL_RenderCopyEx(m_renderer, m_textures.at(key).at(5), NULL, &drawRect, 0, NULL, SDL_FLIP_NONE);
}

void renderer::drawTexture(std::string key, size_t index, vec2 pos, float orient, int col[])
{
  SDL_SetTextureColorMod(m_textures.at(key).at(index), col[0], col[1], col[2]);
  SDL_SetTextureAlphaMod(m_textures.at(key).at(index), col[3]);

  SDL_RenderCopyEx(m_renderer, m_textures.at(key).at(index), NULL, &drawRect, orient, NULL, SDL_FLIP_NONE);
}

void renderer::drawLine(vec2 _start, vec2 _end, int _col[])
{
  SDL_SetRenderDrawColor(m_renderer, _col[0], _col[1], _col[2], _col[3]);
  SDL_RenderDrawLine(m_renderer, _start.x, _start.y, _end.x, _end.y);
}

void renderer::drawLineGr(vec2 _start, vec2 _end, int _scol[], int _ecol[])
{
  SDL_SetRenderDrawColor(renderer, _scol[0], _scol[1], _scol[2], _scol[3]);
  int p0[2] = {static_cast<int>(_start.x), static_cast<int>(_start.y)};
  int p1[2] = {static_cast<int>(_end.x), static_cast<int>(_end.y)};
  int dx = p1[0] - p0[0], dy = p1[1] - p0[1];
  int cur[2] = {0, 0};

  int octant = 0;
  int M, m;

  if(abs(dx) > abs(dy))
  {
    M = abs(dx);
    m = abs(dy);

    if(dx > 0 and dy >= 0) octant = 0;
    else if(dx > 0 and dy < 0) octant = 7;
    else if(dx < 0 and dy > 0) octant = 3;
    else if(dx < 0 and dy <= 0) octant = 4;
  }
  else
  {
    M = abs(dy);
    m = abs(dx);

    if(dx > 0 and dy > 0) octant = 1;
    else if(dx >= 0 and dy < 0) octant = 6;
    else if(dx <= 0 and dy > 0) octant = 2;
    else if(dx < 0 and dy < 0) octant = 5;
  }

  float len = abs(dx) + abs(dy);

  int d = 2 * m - M;

  int count = 0;
  for(int i = 0; i < M; ++i)
  {
    float t = static_cast<float>(count) / len;

    int draw[2] = {cur[0], cur[1]};
    toOctant(&draw[0], &draw[1], octant);
    SDL_SetRenderDrawColor(m_renderer, t * _ecol[0] + (1 - t) * _scol[0], t * _ecol[1] + (1 - t) * _scol[1], t * _ecol[2] + (1 - t) * _scol[2], t * _ecol[3] + (1 - t) * _scol[3]);
    SDL_RenderDrawPoint(m_renderer, draw[0] + p0[0], draw[1] + p0[1]);

    d = d + 2 * m;
    cur[0]++;
    count++;
    if(d > 0)
    {
      cur[1]++;
      count++;
      d = d - (2 * M);
    }
  }
}

void renderer::drawCircle(int x, int y, int radius)
{
  for(int j = y - radius; j < y + radius; j++)
  {
    for(int i = x - radius; i < x + radius; i++)
    {
      if((i-x)*(i-x)+(j-y)*(j-y)<radius*radius)
      {
        SDL_RenderDrawPoint(m_renderer,i,j);
      }
    }
  }
}

void renderer::drawText(std::string text, )
{
  TTF_Font * Font_Pixelade = TTF_OpenFont("../resources/fonts/pix.TTF", 18);
  if(!Font_Pixelade) std::cout << "Font not loaded! " << TTF_GetError() << std::endl;

  std::string scoreText = "SCORE : ";

  scoreText += std::to_string(score);

  SDL_Texture * scoreTxt = renderText(
    scoreText.c_str(),
    Font_Pixelade,
    {255,255,255,255},
    renderer,
    I_MAX
    );

  SDL_Rect sdst = {260,2,86,32};

  SDL_RenderCopy(renderer, scoreTxt, NULL, &sdst);

  std::string missilesText = "MISSILES : ";

  missilesText += std::to_string(missiles);

  SDL_Texture * missilesTxt = renderText(
    missilesText.c_str(),
    Font_Pixelade,
    {255,255,255,255},
    renderer,
    I_MAX
    );

  SDL_Rect mdst = {260,40,86,32};

  SDL_RenderCopy(renderer, missilesTxt, NULL, &mdst);

  TTF_CloseFont(Font_Pixelade);
  SDL_DestroyTexture(scoreTxt);
  SDL_DestroyTexture(missilesTxt);
}

void renderer::finalise()
{
  //Show the contents of the renderer.
  SDL_RenderPresent(m_renderer);
}

//UI CODE
void renderer::drawMap(std::vector<missile> * mp, std::vector<ship> * ap, std::vector<laser> * lp)
{
  SDL_Rect map;
  map.w = 256;
  map.h = 256;
  map.x = WIN_WIDTH - 256;
  map.y = 0;

  SDL_SetRenderDrawColor(renderer,200,200,255,100);
  SDL_RenderFillRect(renderer,&map);

  SDL_SetRenderDrawColor(renderer,0,0,0,255);
  SDL_RenderDrawLine(renderer,WIN_WIDTH-128,124,WIN_WIDTH-128,132);
  SDL_RenderDrawLine(renderer,WIN_WIDTH-124,128,WIN_WIDTH-132,128);

  SDL_SetRenderDrawColor(renderer,0,0,255,255);
  for(unsigned int i = 0; i < lp->size(); i++)
  {
    vec2 lpp = lp->at(i).getPos();

    double x = clamp(lpp.x / 156.0f + WIN_WIDTH - 128.0f, WIN_WIDTH - 256.0f, static_cast<float>(WIN_WIDTH));
    double y = clamp(lpp.y / 156.0f + 128.0f, 0.0f, 256.0f);

    SDL_RenderDrawPoint(renderer,x,y);
  }

  SDL_SetRenderDrawColor(renderer,255,0,0,255);
  for(unsigned int i = 0; i < mp->size(); i++)
  {
    vec2 mpp = mp->at(i).getPos();

    double x = clamp(mpp.x / 156.0f + WIN_WIDTH - 128.0f, WIN_WIDTH - 256.0f, static_cast<float>(WIN_WIDTH));
    double y = clamp(mpp.y / 156.0f + 128.0f, 0.0f, 256.0f);

    SDL_RenderDrawPoint(renderer,x,y);
  }

  SDL_SetRenderDrawColor(renderer,200,200,200,255);
  for(unsigned int i = 0; i < ap->size(); i++)
  {
    vec2 app = ap->at(i).getPos();

    double x = clamp(app.x / 156.0f + WIN_WIDTH - 128.0f, WIN_WIDTH - 256.0f, static_cast<float>(WIN_WIDTH));
    double y = clamp(app.y / 156.0f + 128.0f, 0.0f, 256.0f);

    int radius = 1;
    if(ap->at(i).getClassification() == ASTEROID_MID) radius = 2;
    else if(ap->at(i).getClassification() == ASTEROID_LARGE) radius = 3;

    drawCircle(x,y,radius);
  }

  for(unsigned int i = 0; i < ep->size(); i++)
  {
    vec2 epp = ep->at(i).getPos();
    int radius = clamp( ep->at(i).getRadius() / 16.0f, 1.0f, 5.0f );

    if(ep->at(i).getTeam() == TEAM_PLAYER) SDL_SetRenderDrawColor(renderer,0,255,0,255);
    else if(ep->at(i).getTeam() == TEAM_PLAYER_MINER) SDL_SetRenderDrawColor(renderer,0,255,0,255);
    else if(ep->at(i).getTeam() == GALACTIC_FEDERATION or ep->at(i).getTeam() == SPOOKY_SPACE_PIRATES) SDL_SetRenderDrawColor(renderer,255,0,0,255);

    float x = clamp(epp.x / 156.0f + WIN_WIDTH - 128.0f, WIN_WIDTH - 256.0f, static_cast<float>(WIN_WIDTH));
    float y = clamp(epp.y / 156.0f + 128.0f, 0.0f, 256.0f);

    drawCircle(x,y,radius);
  }
}

void renderer::statusBars(player * ply)
{
  SDL_Rect health_base;
  health_base.w = 256;
  health_base.h = 16;
  health_base.x = 0;
  health_base.y = 0;//WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,100,20,20,255);
  SDL_RenderFillRect(renderer,&health_base);

  SDL_Rect health;
  health.w = ply->getHealth()*2.56;
  health.h = 16;
  health.x = 0;
  health.y =0;// WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,230,50,50,255);
  SDL_RenderFillRect(renderer,&health);

  SDL_Rect shield_base;
  shield_base.w = 256;
  shield_base.h = 16;
  shield_base.x = 0;
  shield_base.y = 16;//WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,20,20,100,255);
  SDL_RenderFillRect(renderer,&shield_base);

  SDL_Rect shield;
  shield.w = ply->getShield()*2.56;
  shield.h = 16;
  shield.x = 0;
  shield.y =16;// WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,50,50,230,255);
  SDL_RenderFillRect(renderer,&shield);

  SDL_Rect energy_base;
  energy_base.w = 256;
  energy_base.h = 16;
  energy_base.x = 0;
  energy_base.y = 32;//WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,20,100,20,255);
  SDL_RenderFillRect(renderer,&energy_base);

  SDL_Rect energy;
  energy.w = ply->getEnergy()*2.56;
  energy.h = 16;
  energy.x = 0;
  energy.y =32;// WIN_HEIGHT-32;

  SDL_SetRenderDrawColor(renderer,50,230,50,255);
  SDL_RenderFillRect(renderer,&energy);
}