#ifndef SHIP_HPP
#define SHIP_HPP

#include <array>
#include <unordered_map>
#include <vector>

#include "base.hpp"
#include "common.hpp"
#include "inventory.hpp"
#include "slotMap.hpp"
#include "util.hpp"
#include "weapons.hpp"

#define UPGRADES_LEN 4

//----------------------------------------------------------------------------------------------------------------------
/// \file ship.hpp
/// \brief This file contains the ship class, and a small, related, utility struct.
/// \author Ben Hawkyard
/// \version 1.0
/// \date 11/04/16
/// Revision History :
/// This is an initial version used for the game, an alternative SDL based renderer is available in renderer.hpp
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/// \brief Represents ship types.
//----------------------------------------------------------------------------------------------------------------------
enum ship_spec {
    COMMUNIST_1, COMMUNIST_2, COMMUNIST_CAPITAL, COMMUNIST_TURRET,
    FEDERATION_MKI, FEDERATION_MKII, FEDERATION_MKIII, FEDERATION_MKIV, FEDERATION_GUNSHIP, FEDERATION_CAPITAL, FEDERATION_TURRET,
    PIRATE_GNAT, PIRATE_CRUISER, PIRATE_WRANGLER, PIRATE_MARAUDER, PIRATE_GUNSHIP, PIRATE_CAPITAL, PIRATE_TURRET,
    ALLIANCE_SCOUT, ALLIANCE_TRACKER, ALLIANCE_PHOENIX, ALLIANCE_DRAGON, ALLIANCE_TRADER, ALLIANCE_GUNSHIP, ALLIANCE_TURRET,
    PLAYER_MINER_DROID, PLAYER_HUNTER, PLAYER_DEFENDER, PLAYER_DESTROYER, PLAYER_CAPITAL, PLAYER_TURRET,
    PLAYER_STATION, PLAYER_GRAVWELL, PLAYER_BARRACKS,
    PLAYER_SHIP,
    ION_MISSILE_MKI,
    ASTEROID_SMALL, ASTEROID_MID, ASTEROID_LARGE,
    SHIPS_END
};

ship_spec operator+(const ship_spec &_lhs, const ship_spec &_rhs);

ship_spec operator+(const ship_spec &_lhs, const int &_rhs);
ship_spec operator+(const int &_lhs, const ship_spec &_rhs);

ship_spec operator+(const ship_spec &_lhs, const size_t &_rhs);
ship_spec operator+(const size_t &_lhs, const ship_spec &_rhs);

ship_spec& operator++(ship_spec &_lhs);

//bool operator<(const ship_spec &_lhs, const ship_spec &_rhs);


//----------------------------------------------------------------------------------------------------------------------
/// \brief What does the ship DO?
//----------------------------------------------------------------------------------------------------------------------
enum ship_type {
    SHIP_TYPE_FIGHTER, SHIP_TYPE_CAPITAL, SHIP_TYPE_TRADER, SHIP_TYPE_MINER,
    SHIP_TYPE_TURRET, SHIP_TYPE_STRUCTURE,
    SHIP_TYPE_NONE
};

//----------------------------------------------------------------------------------------------------------------------
/// \struct tinfo
/// \brief Contains the dimensions for each ship, based off of its texture.
//----------------------------------------------------------------------------------------------------------------------
struct tinfo
{
    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The identifier, matches up with a texture id stored in the renderer.
    //----------------------------------------------------------------------------------------------------------------------
    std::string m_name;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The radius of said texture.
    //----------------------------------------------------------------------------------------------------------------------
    float m_radius;
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief A global long unsigned int, increments on each ship creation, gives a new ID to use.
//----------------------------------------------------------------------------------------------------------------------
//extern long unsigned int g_shipIDCounter;

//----------------------------------------------------------------------------------------------------------------------
/// \brief A global vector, containing texture identifiers and their radii.
//----------------------------------------------------------------------------------------------------------------------
extern std::vector<tinfo> g_texture_keys;

//----------------------------------------------------------------------------------------------------------------------
/// \brief Returns an identifier from a ship type, which we can look up the radius for in g_texture_keys.
/// \param _s ship type
//----------------------------------------------------------------------------------------------------------------------
std::string getTextureKey(ship_spec _s);

//----------------------------------------------------------------------------------------------------------------------
/// \class ship
/// \brief Inherits from ship, contains data such as weapon type, engine power, shield strength. It does not, however, contain any functionality to behave autonomously.
//----------------------------------------------------------------------------------------------------------------------
class ship: public base
{
public:
    //----------------------------------------------------------------------------------------------------------------------
    /// \brief ctor for the ship class.
    /// \param _p position
    /// \param _type ship type
    /// \param _radius ship size
    //----------------------------------------------------------------------------------------------------------------------
    ship(const vec3 _p, const ship_spec _type, const float _radius);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Copy ctor for the ship class.
    /// \param _src source object
    /// \param _p position
    //----------------------------------------------------------------------------------------------------------------------
    ship(const ship& _src, const vec3 _p);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Adds velocity to the ship.
    /// \param _v base vector
    //----------------------------------------------------------------------------------------------------------------------
    void addVelS(const vec3 _v) {if(m_canMove) addVel(_v * m_inertia * m_enginePower);}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Accelerates the ship, in the direction that it is facing.
    /// \param _mult multiplier
    //----------------------------------------------------------------------------------------------------------------------
    void accelerate(const float _mult);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Adds velocity to the ship.
    /// \param _dir base vector
    /// \param _mult multiplier
    //----------------------------------------------------------------------------------------------------------------------
    void accelerate(const vec3 _dir, const float _mult);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Setter for m_accelerating.
    /// \param _b input
    //----------------------------------------------------------------------------------------------------------------------
    void setAccelerating(const bool _b) {m_accelerating = _b;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Adds velocity to the ship, perpendicular to the direction in which it is facing.
    /// \param _side multiplier
    //----------------------------------------------------------------------------------------------------------------------
    void dodge(const float _side);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Updates the ship.
    /// \param _dt time-step
    //----------------------------------------------------------------------------------------------------------------------
    void update(const float _dt);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_targetAngle.
    //----------------------------------------------------------------------------------------------------------------------
    void setTAng(const float _ang) {m_targetAngle = _ang;}
    float getTAng() const {return m_targetAngle;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_angle.
    //----------------------------------------------------------------------------------------------------------------------
    void setAng(const float _ang) {m_angle = _ang;}
    float getAng() const {return m_angle;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Sets weapon data for a weapon slot.
    /// \param _val weapon slot to fill
    /// \param _src source index from global weapon data array (weapons.hpp)
    //----------------------------------------------------------------------------------------------------------------------
    void setWeapData(const int _val, const int _src) {m_weapons[_val] = g_weapons[_src];}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Sets, increments the current weapon.
    //----------------------------------------------------------------------------------------------------------------------
    void setWeap(const int _val) {m_curWeap = _val;}
    void incrWeap(const int _val) {m_curWeap = clampRoll(m_curWeap + _val, 0, static_cast<int>(m_weapons.size()) - 1 );}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Gets a stat from the active weapon.
    /// \param _stat weapon stat to retrieve (enum defined in weapons.hpp)
    //----------------------------------------------------------------------------------------------------------------------
    float getCurWeapStat(WEAPON_STAT _stat) const;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns the colour of the current weapon.
    //----------------------------------------------------------------------------------------------------------------------
    std::array<float, 4> getCurWeapCol() const;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns the colour of the shield.
    //----------------------------------------------------------------------------------------------------------------------
    std::array<float, 4> getShieldCol() const;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns array of all of the ships weapons.
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<std::array<float, 10>> getWeaps() const {return m_weapons;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns all data concerning a specific weapon.
    //----------------------------------------------------------------------------------------------------------------------
    std::array<float, 10> getWeap() const {return m_weapons[m_curWeap];}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns current weapon index.
    //----------------------------------------------------------------------------------------------------------------------
    int getCurWeap() const {return m_curWeap;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Sets m_drawShot to 255.
    //----------------------------------------------------------------------------------------------------------------------
    void shoot() {m_drawShot = 255.0f;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_shooting.
    //----------------------------------------------------------------------------------------------------------------------
    void setFiring(const bool _v);
    bool isFiring() const {return m_shooting;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_maxHealth.
    //----------------------------------------------------------------------------------------------------------------------
    void setMaxHealth(const float _h, const bool _match) {m_maxHealth = _h; if(_match) m_health = _h;}
    float getMaxHealth() const {return m_maxHealth;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_maxShield.
    //----------------------------------------------------------------------------------------------------------------------
    void setMaxShield(const float _s, const bool _match) {m_maxShield = _s; if(_match) m_shield = _s;}
    float getMaxShield() const {return m_maxShield;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_maxEnergy.
    //----------------------------------------------------------------------------------------------------------------------
    void setMaxEnergy(const float _e, const bool _match) {m_maxEnergy = _e; if(_match) m_energy = _e;}
    float getMaxEnergy() const {return m_maxEnergy;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getters and setters for m_health.
    //----------------------------------------------------------------------------------------------------------------------
    void setHealth(const float _h) {m_health = _h;}
    float getHealth() const {return m_health;}
    void incrHealth(const float _v) {m_health += _v;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_health.
    //----------------------------------------------------------------------------------------------------------------------
    void setShield(const float _s) {m_shield = _s;}
    float getShield() const {return m_shield;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_energy.
    //----------------------------------------------------------------------------------------------------------------------
    void setEnergy(const float _e) {m_energy = _e;}
    float getEnergy() const {return m_energy;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_cooldown.
    //----------------------------------------------------------------------------------------------------------------------
    float getCooldown() const {return m_coolDown;}
    void setCooldown(const float _f);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getters and setters for m_missiles.
    //----------------------------------------------------------------------------------------------------------------------
    void setMissiles(const int _m) {m_missiles = _m;}
    void incrMissiles(const int _m) {m_missiles += _m;}
    int getMissiles() const {return m_missiles;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_priority.
    //----------------------------------------------------------------------------------------------------------------------
    void setEnergyPriority(const int _v) {m_priority = static_cast<energyPriority>(_v);}
    int getEnergyPriority() const {return static_cast<int>(m_priority);}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Damages the ship.
    /// \param _d damage
    //----------------------------------------------------------------------------------------------------------------------
    void damage(const float _d);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Damages the ship.
    /// \param _d damage, _v knockback vector
    //----------------------------------------------------------------------------------------------------------------------
    void damage(const float _d, const vec3 _v);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Damages the ship.
    /// \param _d damage, _v knockback vector, _id attacker id
    //----------------------------------------------------------------------------------------------------------------------
    void damage(const float _d, const vec3 _v, uniqueID _id);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getters and setters for m_upgrades.
    //----------------------------------------------------------------------------------------------------------------------
    int getUpgrade(const int _index) {if(_index < UPGRADES_LEN) return m_upgrades[_index]; return -1;}
    void setGradeArr(const int _i, const int _v) {m_upgrades[_i] = _v;}
    void setGrade(const int _i, const int _v);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Upgrades the ship.
    /// \param _i slot to upgrade
    //----------------------------------------------------------------------------------------------------------------------
    int upgrade(const int _i);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_inertia.
    //----------------------------------------------------------------------------------------------------------------------
    void setInertia(const float _in) {m_inertia = _in;}
    float getInertia() const {return m_inertia;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Setter for generator multiplier.
    //----------------------------------------------------------------------------------------------------------------------
    void setGeneratorMul(float _g) {m_generatorMul = _g;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for m_enginePower.
    //----------------------------------------------------------------------------------------------------------------------
    void setEnginePower(const float _val) {m_enginePower = _val;}
    float getEnginePower() const {return m_enginePower;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief m_classification getter.
    //----------------------------------------------------------------------------------------------------------------------
    ship_spec getClassification() const {return m_classification;}

    ship_type getType() const {return m_type;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief m_radius getter.
    //----------------------------------------------------------------------------------------------------------------------
    float getRadius() const {return m_radius;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Returns the score value for the ship, how many points the player gets for blowing it up.
    //----------------------------------------------------------------------------------------------------------------------
    int getScore() const;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for m_angVel.
    //----------------------------------------------------------------------------------------------------------------------
    float getAngVel() const {return m_angVel;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for m_canMove.
    //----------------------------------------------------------------------------------------------------------------------
    bool getCanMove() const {return m_canMove;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for m_canShoot.
    //----------------------------------------------------------------------------------------------------------------------
    bool getCanShoot() const {return m_canShoot;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Whether the ship can be considered to be in combat.
    //----------------------------------------------------------------------------------------------------------------------
    bool inCombat() {return m_damageTimer > 0.0f;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for the ship type id.
    //----------------------------------------------------------------------------------------------------------------------
    std::string getIdentifier() const {return m_identifier;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Gets the alpha values for various colours associated with the ship.
    //----------------------------------------------------------------------------------------------------------------------
    std::array<float, 4> getAlphaStats() const {std::array<float, 4> ret = {static_cast<float>(m_engineGlow), static_cast<float>(m_steeringGlow), static_cast<float>(m_drawShot), static_cast<float>(m_shieldGlow)}; return ret;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for m_shieldGlow.
    //----------------------------------------------------------------------------------------------------------------------
    float getShieldGlow() const {return m_shieldGlow;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for m_drawShot, converts it to the range 0-1.
    //----------------------------------------------------------------------------------------------------------------------
    float getLaserGlow() const {return m_drawShot / 255.0f;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getters and setters for parents.
    //----------------------------------------------------------------------------------------------------------------------
    bool hasParent() const {return m_hasParent;}
    uniqueID getParent() const {return m_parent;}
    void setParent(uniqueID _p);

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getters and setters for parent offsets.
    //----------------------------------------------------------------------------------------------------------------------
    vec3 getParentOffset() const {return m_parentOffset;}
    void setParentOffset(vec3 _v) {m_parentOffset = _v;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter for unique ID.
    //----------------------------------------------------------------------------------------------------------------------
    //long int getUniqueID() {return m_uniqueID;}
    //void setUniqueID(const unsigned long  _id) {m_uniqueID = _id;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Getter and setter for last attacker.
    //----------------------------------------------------------------------------------------------------------------------
    uniqueID getLastAttacker() {return m_lastAttacker;}
    void setLastAttacker(uniqueID _id) {m_lastAttacker = _id;}

    void toggleInventory() {m_cargo.toggleVisible();}
    bool isInventoryVisible() {return m_cargo.isVisible();}

    void addKill() {m_kills++; m_maxEnergy += 2.0f;}
    unsigned long getKills() const {return m_kills;}
    void setKills(unsigned long _kills) {m_kills = _kills;}

    bool addItem(debris _in) {return m_cargo.addItem(_in);}
    inventory * getCargo() {return &m_cargo;}

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Calculate the cost for an AI faction to buy.
    //----------------------------------------------------------------------------------------------------------------------
    float calcAICost();
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The target angle of the ship.
    //----------------------------------------------------------------------------------------------------------------------
    float m_targetAngle;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The current weapon index.
    //----------------------------------------------------------------------------------------------------------------------
    int m_curWeap;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Current number of missiles.
    //----------------------------------------------------------------------------------------------------------------------
    int m_missiles;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Angular velocity.
    //----------------------------------------------------------------------------------------------------------------------
    float m_angVel;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Inertia, slows down the ship when turning and moving.
    //----------------------------------------------------------------------------------------------------------------------
    float m_inertia;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief A multiplier used to compute movement and turning speed.
    //----------------------------------------------------------------------------------------------------------------------
    float m_enginePower;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Current health.
    //----------------------------------------------------------------------------------------------------------------------
    float m_health;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Maximum health.
    //----------------------------------------------------------------------------------------------------------------------
    float m_maxHealth;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Current shield level.
    //----------------------------------------------------------------------------------------------------------------------
    float m_shield;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Maximum shield level.
    //----------------------------------------------------------------------------------------------------------------------
    float m_maxShield;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Current angle.
    //----------------------------------------------------------------------------------------------------------------------
    float m_angle;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Current energy level.
    //----------------------------------------------------------------------------------------------------------------------
    float m_energy;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Max energy level.
    //----------------------------------------------------------------------------------------------------------------------
    float m_maxEnergy;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The ship may only fire when this is below zero.
    //----------------------------------------------------------------------------------------------------------------------
    float m_coolDown;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Size of ship, for collisions.
    //----------------------------------------------------------------------------------------------------------------------
    float m_radius;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Time since the ship was last damaged.
    //----------------------------------------------------------------------------------------------------------------------
    float m_damageTimer;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Ship type id.
    //----------------------------------------------------------------------------------------------------------------------
    std::string m_identifier;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Vector containing weapons data.
    //----------------------------------------------------------------------------------------------------------------------
    std::vector< std::array<float, 10> > m_weapons;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Ship type.
    //----------------------------------------------------------------------------------------------------------------------
    ship_spec m_classification;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief What the ship does.
    //----------------------------------------------------------------------------------------------------------------------
    ship_type m_type;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Whether the ship is shooting.
    //----------------------------------------------------------------------------------------------------------------------
    bool m_shooting;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Whether the ship is accelerating.
    //----------------------------------------------------------------------------------------------------------------------
    bool m_accelerating;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Whether the ship can move. Turrets cannot, for example.
    //----------------------------------------------------------------------------------------------------------------------
    bool m_canMove;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Whether the ship is able to shoot.
    //----------------------------------------------------------------------------------------------------------------------
    bool m_canShoot;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The system the ship is prioritising.
    //----------------------------------------------------------------------------------------------------------------------
    enum energyPriority {PRIORITY_NONE, PRIORITY_SHIELDS, PRIORITY_ENGINES, PRIORITY_GUNS};
    energyPriority m_priority;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Glowing of the engines.
    //----------------------------------------------------------------------------------------------------------------------
    size_t m_engineGlow;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Glowing of the steering surfaces.
    //----------------------------------------------------------------------------------------------------------------------
    size_t m_steeringGlow;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Muzzle flash.
    //----------------------------------------------------------------------------------------------------------------------
    float m_drawShot;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Glowing of the shield.
    //----------------------------------------------------------------------------------------------------------------------
    size_t m_shieldGlow;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Tracks ship upgrades.
    //----------------------------------------------------------------------------------------------------------------------
    std::array<size_t, UPGRADES_LEN> m_upgrades;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Shield strength multiplier.
    //----------------------------------------------------------------------------------------------------------------------
    float m_shieldMul;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Generator power multiplier.
    //----------------------------------------------------------------------------------------------------------------------
    float m_generatorMul;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Does the ship have a parent?
    //----------------------------------------------------------------------------------------------------------------------
    bool m_hasParent;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The parent ships unique ID.
    //----------------------------------------------------------------------------------------------------------------------
    uniqueID m_parent;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The offset from the parent ship.
    //----------------------------------------------------------------------------------------------------------------------
    vec3 m_parentOffset;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The unique id of the ship, used for parenting because pointers and.
    //----------------------------------------------------------------------------------------------------------------------
    //long int m_uniqueID;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The id of the ship which last damaged this ship.
    //----------------------------------------------------------------------------------------------------------------------
    uniqueID m_lastAttacker;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief The inventory of the ship.
    //----------------------------------------------------------------------------------------------------------------------
    inventory m_cargo;

    //----------------------------------------------------------------------------------------------------------------------
    /// \brief Number of defeated ships.
    //----------------------------------------------------------------------------------------------------------------------
    long unsigned int m_kills;
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief A global vector of ships, which we will copy from, rather than constructing from scratch.
//----------------------------------------------------------------------------------------------------------------------
extern std::vector<ship> g_ship_templates;

float calcAICost(const ship_spec _spec);

#endif
