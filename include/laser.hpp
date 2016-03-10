#ifndef LASER_HPP
#define LASER_HPP

#include "weapons.hpp"
#include "base.hpp"
#include "enemy.hpp"

class laser: public base
{
    int m_dmg;
    int m_spd;
    aiTeam m_team;
    float m_ang;
    float m_col[3];
    float m_power;
    float m_stop;
public:
    laser(vec2 _p, vec2 _v, float _ang, std::array<float, WEAPS_W> _data, aiTeam _team);
    int getDmg() const {return m_dmg;}
    float getStop() const {return m_stop;}
    void update(float _dt);
    aiTeam getTeam() const  {return m_team;}
    float getCol(int _index) const {return m_col[_index];}
    float getPower() const {return m_power;}
};

#endif

