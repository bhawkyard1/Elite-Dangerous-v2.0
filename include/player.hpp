#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "ship.hpp"

class player: public ship
{
public:
    player(vec2 _p, float _r);
	void ctrlUpdate();
};

#endif
