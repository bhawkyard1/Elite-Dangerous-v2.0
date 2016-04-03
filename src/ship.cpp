#include "ship.hpp"
#include "weapons.hpp"
#include "vectors.hpp"

std::vector<tinfo> g_texture_keys = {
    {"FEDERATION_MKI", 32}, {"FEDERATION_MKII", 32}, {"FEDERATION_MKIII", 32}, {"FEDERATION_MKIV", 32}, {"FEDERATION_GUNSHIP", 64},
    {"PIRATE_GNAT", 32}, {"PIRATE_CRUISER", 32}, {"PIRATE_WRANGLER", 40}, {"PIRATE_MARAUDER", 40}, {"PIRATE_GUNSHIP", 64},
    {"PLAYER_MINER_DROID", 16}, {"PLAYER_TURRET", 16}, {"PLAYER_STATION", 1024}, {"PLAYER_GRAVWELL", 256}, {"PLAYER_BARRACKS", 512},
    {"PLAYER_SHIP", 32},
    {"PLAYER_HUNTER", 32}, {"PLAYER_DEFENDER", 32}, {"PLAYER_DESTROYER", 32},
    {"ION_MISSILE_MKI", 16},
    {"ASTEROID_SMALL", 32}, {"ASTEROID_MID", 64}, {"ASTEROID_LARGE", 128},
    {"SHIPS_END", 0}
};

std::vector<ship> g_ship_templates;

std::string getTextureKey(ship_spec _s) {return g_texture_keys[_s].m_name;}
ship_spec decrSpec(ship_spec _s) {return static_cast<ship_spec>( static_cast<int>(_s) - 1 );}

ship::ship(
        vec2 _p,
        ship_spec _type,
        float _radius
        )
{
    m_coolDown = 0.0f;
    m_shooting = false;

    setPos(_p);
    setPPos(_p);

    setVel({0.0f,0.0f});
    setWVel({0.0f,0.0f});
    m_angle = 0.0f;
    m_targetAngle = 0.0f;

    m_engineGlow = 0.0f;
    m_steeringGlow = 0.0f;
    m_shieldGlow = 0.0f;
    m_drawShot = 0.0f;
    m_angVel = 0.0f;

    m_canMove = true;
    m_canShoot = true;

    m_curWeap = 0;

    switch(_type)
    {
    case FEDERATION_MKI:
        m_identifier = "FEDERATION_MKI";
        setMaxHealth(50.0f,true);
        setMaxShield(30.0f,true);
        setMaxEnergy(100.0f,true);
        m_inertia = 0.1f;
        m_enginePower = 4.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 4] );
        m_curWeap = 0;
        break;
    case FEDERATION_MKII:
        m_identifier = "FEDERATION_MKII";
        setMaxHealth(90.0f,true);
        setMaxShield(60.0f,true);
        setMaxEnergy(100.0f,true);
        m_inertia = 0.09f;
        m_enginePower = 5.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 4] );
        m_curWeap = 0;
        break;
    case FEDERATION_MKIII:
        m_identifier = "FEDERATION_MKIII";
        setMaxHealth(120.0f,true);
        setMaxShield(90.0f,true);
        setMaxEnergy(150.0f,true);
        m_inertia = 0.08f;
        m_enginePower = 6.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 4] );
        m_curWeap = 0;
        break;
    case FEDERATION_MKIV:
        m_identifier = "FEDERATION_MKIV";
        setMaxHealth(150.0f,true);
        setMaxShield(120.0f,true);
        setMaxEnergy(200.0f,true);
        m_inertia = 0.15f;
        m_enginePower = 7.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 4] );
        m_curWeap = 0;
        break;
    case FEDERATION_GUNSHIP:
        m_identifier = "FEDERATION_GUNSHIP";
        setMaxHealth(500.0f,true);
        setMaxShield(200.0f,true);
        setMaxEnergy(500.0f,true);
        m_inertia = 0.02f;
        m_enginePower = 8.0f;
        m_weapons.push_back( g_weapons[rand() % 2 + 7] );
        m_curWeap = 0;
        break;
    case PIRATE_GNAT:
        m_identifier = "PIRATE_GNAT";
        setMaxHealth(80.0f,true);
        setMaxShield(40.0f,true);
        setMaxEnergy(90.0f,true);
        m_inertia = 0.15f;
        m_enginePower = 4.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 14] );
        m_curWeap = 0;
        break;
    case PIRATE_CRUISER:
        m_identifier = "PIRATE_CRUISER";
        setMaxHealth(100.0f,true);
        setMaxShield(60.0f,true);
        setMaxEnergy(100.0f,true);
        m_inertia = 0.1f;
        m_enginePower = 5.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 14] );
        m_curWeap = 0;
        break;
    case PIRATE_WRANGLER:
        m_identifier = "PIRATE_WRANGLER";
        setMaxHealth(140.0f,true);
        setMaxShield(70.0f,true);
        setMaxEnergy(120.0f,true);
        m_inertia = 0.05f;
        m_enginePower = 6.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 14] );
        m_curWeap = 0;
        break;
    case PIRATE_MARAUDER:
        m_identifier = "PIRATE_MARAUDER";
        setMaxHealth(120.0f,true);
        setMaxShield(120.0f,true);
        setMaxEnergy(120.0f,true);
        m_inertia = 0.15f;
        m_enginePower = 7.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 14] );
        m_curWeap = 0;
        break;
    case PIRATE_GUNSHIP:
        m_identifier = "PIRATE_GUNSHIP";
        setMaxHealth(200.0f,true);
        setMaxShield(120.0f,true);
        setMaxEnergy(150.0f,true);
        m_inertia = 0.05f;
        m_enginePower = 8.0f;
        m_weapons.push_back( g_weapons[17] );
        m_curWeap = 0;
        break;
    case PLAYER_SHIP:
        m_identifier = "PLAYER_SHIP";
        setMaxHealth(100.0f,true);
        setMaxShield(100.0f,true);
        setMaxEnergy(100.0f,true);
        m_inertia = 0.07f;
        m_enginePower = 5.0f;
        m_weapons.push_back( g_weapons[0] );
        m_weapons.push_back( g_weapons[1] );
        m_weapons.push_back( g_weapons[2] );
        m_curWeap = 0;
        break;
    case PLAYER_HUNTER:
        m_identifier = "PLAYER_HUNTER";
        setMaxHealth(100.0f,true);
        setMaxShield(100.0f,true);
        setMaxEnergy(100.0f,true);
        m_inertia = 0.2f;
        m_enginePower = 4.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 9] );
        m_curWeap = 0;
        break;
    case PLAYER_DEFENDER:
        m_identifier = "PLAYER_DEFENDER";
        setMaxHealth(125.0f,true);
        setMaxShield(125.0f,true);
        setMaxEnergy(125.0f,true);
        m_inertia = 0.1f;
        m_enginePower = 8.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 9] );
        m_curWeap = 0;
        break;
    case PLAYER_DESTROYER:
        m_identifier = "PLAYER_DESTROYER";
        setMaxHealth(150.0f,true);
        setMaxShield(150.0f,true);
        setMaxEnergy(150.0f,true);
        m_inertia = 0.07f;
        m_enginePower = 10.0f;
        m_weapons.push_back( g_weapons[rand() % 3 + 9] );
        m_curWeap = 0;
        break;
    case PLAYER_MINER_DROID:
        m_identifier = "PLAYER_MINER_DROID";
        setMaxHealth(10.0f,true);
        setMaxShield(20.0f,true);
        setMaxEnergy(200.0f,true);
        m_inertia = 0.3f;
        m_enginePower = 2.0f;
        m_weapons.push_back( g_weapons[12] );
        m_curWeap = 0;
        break;
    case ION_MISSILE_MKI:
        m_identifier = "ION_MISSILE_MKI";
        m_inertia = 0.1f;
        m_enginePower = 6.0f;
        m_canShoot = false;
        break;
    case ASTEROID_SMALL:
        m_identifier = "ASTEROID_SMALL";
        setMaxHealth(500.0f,true);
        setMaxShield(0.0f,true);
        setMaxEnergy(0.0f,true);
        m_angVel = randFloat(-1.0f, 1.0f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case ASTEROID_MID:
        m_identifier = "ASTEROID_MID";
        setMaxHealth(1000.0f,true);
        setMaxShield(0.0f,true);
        setMaxEnergy(0.0f,true);
        m_angVel = randFloat(-1.0f, 1.0f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case ASTEROID_LARGE:
        m_identifier = "ASTEROID_LARGE";
        setMaxHealth(2000.0f,true);
        setMaxShield(0.0f,true);
        setMaxEnergy(0.0f,true);
        m_angVel = randFloat(-1.0f, 1.0f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case PLAYER_TURRET:
        m_identifier = "PLAYER_TURRET";
        setMaxHealth(100.0f,true);
        setMaxShield(200.0f,true);
        setMaxEnergy(300.0f,true);
        m_inertia = 0.1f;
        m_enginePower = 0.0f;
        m_weapons.push_back( g_weapons[13] );
        m_curWeap = 0;
        m_canMove = false;
        break;
    case PLAYER_STATION:
        m_identifier = "PLAYER_STATION";
        setMaxHealth(10000.0f,true);
        setMaxShield(10000.0f,true);
        setMaxEnergy(100000.0f,true);
        m_angVel = randFloat(-0.1f, 0.1f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case PLAYER_GRAVWELL:
        m_identifier = "PLAYER_GRAVWELL";
        setMaxHealth(5000.0f,true);
        setMaxShield(5000.0f,true);
        setMaxEnergy(50000.0f,true);
        m_angVel = randFloat(-0.1f, 0.1f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case PLAYER_BARRACKS:
        m_identifier = "PLAYER_BARRACKS";
        setMaxHealth(8000.0f,true);
        setMaxShield(8000.0f,true);
        setMaxEnergy(80000.0f,true);
        m_angVel = randFloat(-0.1f, 0.1f);
        m_inertia = 0.0f;
        m_enginePower = 0.0f;
        m_canMove = false;
        m_canShoot = false;
        break;
    case SHIPS_END:
        break;
    }

    m_priority = NONE;

    for(short unsigned int i = 0; i < UPGRADES_LEN; i++) m_upgrades[i] = 0;
    m_shieldMul = 1.0f;
    m_generatorMul = 1.0f;

    m_classification = _type;

    m_radius = _radius;
}

ship::ship(
        const ship &_src,
        const vec2 _p
        )
{
    m_coolDown = 0.0f;
    m_shooting = false;

    setPos(_p);
    setPPos(_p);

    setVel({0.0f,0.0f});
    setWVel({0.0f,0.0f});
    m_angle = 0.0f;
    m_targetAngle = 0.0f;

    m_engineGlow = 0.0f;
    m_steeringGlow = 0.0f;
    m_shieldGlow = 0.0f;
    m_drawShot = 0.0f;

    m_canMove = _src.getCanMove();
    m_canShoot = _src.getCanShoot();

    setMaxHealth( _src.getMaxHealth(), true );
    setMaxShield( _src.getMaxShield(), true );
    setMaxEnergy( _src.getMaxEnergy(), true );

    m_inertia = _src.getInertia();
    m_enginePower = _src.getEnginePower();
    m_angVel = _src.getAngVel();

    m_identifier = _src.getIdentifier();

    m_priority = NONE;

    for(short unsigned int i = 0; i < UPGRADES_LEN; i++) m_upgrades[i] = 0;
    m_shieldMul = 1.0f;
    m_generatorMul = 1.0f;

    m_classification = _src.getClassification();

    m_radius = _src.getRadius();

    m_curWeap = 0;

    for(size_t i = 0; i < _src.getWeaps().size(); ++i)
    {
        m_weapons.push_back(_src.getWeaps()[i]);
    }

    switch(_src.getClassification())
    {
    case FEDERATION_MKI:
        m_curWeap = rand() % 3 + 4;
        break;
    case FEDERATION_MKII:
        m_curWeap = rand() % 3 + 4;
        break;
    case FEDERATION_MKIII:
        m_curWeap = rand() % 3 + 4;
        break;
    case FEDERATION_MKIV:
        m_curWeap = rand() % 3 + 4;
        break;
    case FEDERATION_GUNSHIP:
        m_curWeap = rand() % 2 + 7;
        break;
    case PIRATE_GNAT:
        m_curWeap = rand() % 3 + 14;
        break;
    case PIRATE_CRUISER:
        m_curWeap = rand() % 3 + 14;
        break;
    case PIRATE_WRANGLER:
        m_curWeap = rand() % 3 + 14;
        break;
    case PIRATE_MARAUDER:
        m_curWeap = rand() % 3 + 14;
        break;
    case PIRATE_GUNSHIP:
        m_curWeap = 17;
        break;
    case PLAYER_SHIP:
        m_curWeap = 0;
        break;
    case PLAYER_HUNTER:
        m_curWeap = rand() % 3 + 9;
        break;
    case PLAYER_DEFENDER:
        m_curWeap = rand() % 3 + 9;
        break;
    case PLAYER_DESTROYER:
        m_curWeap = rand() % 3 + 9;
        break;
    case PLAYER_MINER_DROID:
        m_curWeap = 12;
        break;
    case PLAYER_TURRET:
        m_curWeap = 13;
    default:
        break;
    }
}

ship::ship(ship_spec _s)
{
    *this = g_ship_templates[_s];
}

void ship::accelerate(const float _mult)
{
    float energyLoss = 0.6f, accelMult = 1.0f;

    if(m_priority == ENGINES)
    {
        energyLoss = 1.2f;
        accelMult = 2.0f;
    }
    else if(m_priority == GUNS)
    {
        energyLoss = 0.6f;
        accelMult = 0.8f;
    }

    if(m_energy <= energyLoss) return;
    vec2 add = vec(getAng() + 90.0f) * accelMult * m_inertia * m_enginePower;
    addVel(add * _mult);
    m_energy -= energyLoss;
    m_engineGlow = clamp(m_engineGlow + 40.0f * accelMult,0.0f,255.0f);

    setAccelerating(true);
}

void ship::accelerate(
        const vec2 _dir,
        const float _mult
        )
{
    float energyLoss = 0.6f, accelMult = 1.0f;

    if(m_priority == ENGINES)
    {
        energyLoss = 1.2f;
        accelMult = 2.0f;
    }
    else if(m_priority == GUNS)
    {
        energyLoss = 0.6f;
        accelMult = 0.8f;
    }

    if(m_energy <= energyLoss) return;
    addVel(_dir * _mult * m_inertia * m_enginePower);
    m_energy -= energyLoss;
    m_engineGlow = clamp(m_engineGlow + 40.0f * accelMult,0.0f,255.0f);

    setAccelerating(true);
}

void ship::dodge(const float _side)
{
    float energyLoss = 0.2, accelMult = 1;

    if(m_priority == ENGINES)
    {
        energyLoss = 0.4;
        accelMult = 2;
    }
    else if(m_priority == GUNS)
    {
        energyLoss = 0.08;
        accelMult = 0.8;
    }

    if(m_energy <= energyLoss) return;
    vec2 vec = computeVector(getAng());
    addVel(vec * _side * accelMult * m_inertia * m_enginePower);
    m_energy -= energyLoss * fabs(_side);
}

void ship::update(const float _dt)
{
    float angDiff = clampRoll(m_targetAngle - m_angle, -180.0f, 180.0f);
    float turnConst = 0.2f;

    if(m_angVel != 0.0f) setAng( clampRoll( m_angle + m_angVel, -180.0f, 180.0f ) );
    else if(angDiff < -1.0f)
    {
        setAng(clampRoll(m_angle + clamp(angDiff * m_inertia * m_enginePower * turnConst, -9999.0f, -1.0f) * _dt * g_PIXEL_UNIT_CONVERSION, -180.0f, 180.0f));
        m_steeringGlow = clamp(m_steeringGlow + 20.0f, 0.0f, 255.0f);
    }
    else if(angDiff > 1.0f)
    {
        setAng(clampRoll(m_angle + clamp(angDiff * m_inertia * m_enginePower * turnConst, 1.0f, 9999.0f) * _dt * g_PIXEL_UNIT_CONVERSION, -180.0f, 180.0f));
        m_steeringGlow = clamp(m_steeringGlow + 20.0f, 0.0f, 255.0f);
    }

    float energy_loss = 0.05f, shield_add = 0.05f;

    if(m_priority == SHIELDS)
    {
        energy_loss = 0.3f;
        shield_add = 0.4f;
        if(m_shield >= 0.0f) m_shieldGlow = 255.0f;
    }
    else if(m_priority == GUNS or m_priority == ENGINES)
    {
        energy_loss = 0.01f;
        shield_add = 0.03f;
    }

    if(m_energy >= energy_loss and m_shield < m_maxShield)
    {
        m_shield = clamp(m_shield + shield_add * m_shieldMul, 0.0f, m_maxShield);
        m_energy -= energy_loss;
    }

    //if(!accelerating)
    m_energy = clamp(m_energy + 0.1f * m_generatorMul, 0.0f, m_maxEnergy);

    if(rand()%999 == 0) m_health = clamp(m_health + 0.5f, 0.0f, m_maxHealth);

    m_steeringGlow = clamp(m_steeringGlow - 10.0f, 0.0f, 255.0f);
    m_shieldGlow = clamp(m_shieldGlow - 10.0f, 0.0f, 255.0f);
    m_engineGlow = clamp(m_engineGlow - 20.0f, 0.0f, 255.0f);

    m_coolDown = clamp(m_coolDown - _dt, 0.0f, 999.0f);
    m_damageTimer = clamp(m_damageTimer - _dt, 0.0f, 10.0f);

    m_drawShot /= 1.5f;

    addVel(-getVel() * 0.00001f);

    m_accelerating = false;
}

void ship::setFiring(const bool _v)
{
    m_shooting = _v;
    if(_v)
    {
        m_damageTimer = 3.0f;
    }
}

void ship::damage(float _d)
{           
    if(getShield() - _d > 0) m_shieldGlow = 255.0f;

    float shieldDmg = clamp(_d, 0.0f, getShield());
    _d -= shieldDmg;
    setShield(getShield() - shieldDmg);

    float healthDmg = clamp(_d, 0.0f, getHealth());
    _d -= healthDmg;
    setHealth(getHealth()-healthDmg);

    m_damageTimer = 10.0f;
}

void ship::damage(float _d, const vec2 _v)
{
    //std::cout << " dot " << dotProd1( vec(m_angle + 90), unit(_v) ) + 1.5f << std::endl;
    //Shots to the rear do more damage.
    if(m_canMove)
    {
        _d *= (dotProd1( vec(m_angle + 90), unit(_v) ) / 2.0f) + 1.5f;
        addVel(_v);
    }

    if(getShield() - _d > 0) m_shieldGlow = 255.0f;

    float shieldDmg = clamp(_d, 0.0f, getShield());
    _d -= shieldDmg;
    setShield(getShield() - shieldDmg);

    float healthDmg = clamp(_d, 0.0f, getHealth());
    _d -= healthDmg;
    setHealth(getHealth()-healthDmg);

    m_damageTimer = 10.0f;
}

int ship::upgrade(const int _i)
{
    if(_i < 4) m_upgrades[_i]++;
    else return 0;

    switch(_i)
    {
    case 0:
        /*
            0)lasers per shot
            1)spread
            2)dmg
            3)speed
            4)red
            5)green
            6)blue
            7)energy cost
            8)cooldown.
            */
        if(m_weapons.size() < 1) break;
        m_weapons[0][1] *= 0.9f;
        m_weapons[0][2] *= 1.2f;
        m_weapons[0][3] *= 1.12f;
        m_weapons[0][7] *= 1.1f;
        m_weapons[0][8] *= 0.95f;

        if(m_weapons.size() < 2) break;
        m_weapons[1][0] *= 1.1f;
        m_weapons[1][1] *= 0.9f;
        m_weapons[1][2] *= 1.1f;
        m_weapons[1][3] *= 1.12f;
        m_weapons[1][7] *= 1.1f;
        m_weapons[1][8] *= 0.95f;

        if(m_weapons.size() < 3) break;
        m_weapons[2][0] *= 1.1f;
        m_weapons[2][1] *= 0.9f;
        m_weapons[2][3] *= 1.12f;
        m_weapons[1][7] *= 0.95f;
        m_weapons[2][8] *= 0.95f;
        break;
    case 1:
        m_shieldMul *= 1.2f;
        break;
    case 2:
        m_generatorMul *= 1.2f;
        break;
    case 3:
        m_enginePower *= 1.2f;
        break;
    case 4:
        m_missiles++;
        break;
    case 5:
        break;
    default:
        break;
    }
    return m_upgrades[_i];
}

void ship::setGrade(
        const int _i,
        const int _v
        )
{
    for(int k = 0; k < _v; ++k) upgrade(_i);
}

int ship::getScore() const
{
    return static_cast<int>((m_maxHealth + m_maxShield + m_maxEnergy) / 100.0f);
}

void ship::setCooldown(const float _f)
{
    m_coolDown = _f;
    addVelS(-vec(m_angle + 90.0f) * getCurWeapStat(STOPPING_POWER) * 5.0f);
}

std::array<float, 4> ship::getCurWeapCol() const
{
    if(m_canShoot) return {getCurWeapStat(COLOUR_RED) / 255.0f, getCurWeapStat(COLOUR_GREEN) / 255.0f, getCurWeapStat(COLOUR_BLUE) / 255.0f, getLaserGlow()};
    return {0.0f, 0.0f, 0.0f, 0.0f};
}
