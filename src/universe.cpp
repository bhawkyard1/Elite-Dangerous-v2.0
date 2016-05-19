#include "common.hpp"
#include "file.hpp"
#include "sfx.hpp"
#include "shapes.hpp"
#include "sim_time.hpp"
#include "universe.hpp"
#include "util.hpp"

universe::universe()
    :
      m_drawer(g_WIN_WIDTH, g_WIN_HEIGHT),
      m_ply( {0.0f, 0.0f}, m_drawer.getTextureRadius(PLAYER_SHIP) )
{
    createFactions();

    m_sounds.sfxInit();
    m_sounds.loadSounds();

    showUI = true;
    m_time_elapsed = 0.0;
    m_pos = {0.0f, 0.0f};
    setVel({0,0});

    m_factionCounts.assign(8, 0);
    m_factionMaxCounts.assign(8, 0);

    m_factionMaxCounts[GALACTIC_FEDERATION] = 1;
    m_factionMaxCounts[SPACE_COMMUNISTS] = 1;
    m_factionMaxCounts[ALLIANCE] = 5;
    m_maxMiners = 0;
    m_maxMiners = 0;
    m_minerCount = 0;
    m_wingmenCount = 0;

    m_ply.setPos({g_WIN_WIDTH/2.0f,g_WIN_HEIGHT/2.0f});
    m_ply.setPPos({g_WIN_WIDTH/2.0f,g_WIN_HEIGHT/2.0f});

    m_tCol[0] = 255.0f;
    m_tCol[1] = 200.0f;
    m_tCol[2] = 50.0f;

    m_cCol[0] = 255.0f;
    m_cCol[1] = 200.0f;
    m_cCol[2] = 50.0f;

#if RENDER_MODE == 0
    for(int i = 0; i < 2048 * pow(2, g_GRAPHICAL_DETAIL) ; i++)
    {
        m_dots.push_back(stardust(m_cCol));
    }

    for(int i = 0; i < 1024 * pow(2, g_GRAPHICAL_DETAIL); i++)
    {
        std::string id;
        switch(rand()%4)
        {
        case 0:
            id = "STARDUST_1";
            break;
        case 1:
            id = "STARDUST_2";
            break;
        case 2:
            id = "STARDUST_3";
            break;
        case 3:
            id = "STARDUST_4";
            break;
        default:
            break;
        }

        int w = 0;
        int h = 0;
        m_drawer.queryTexture(id, 0, &w, &h);
        m_sparkles.push_back(stardust_sprite(id, m_cCol, w, h));
    }
#endif

    initUI();
    if(g_DEV_MODE) setScore(100000);
    else setScore(16);

    m_paused = false;
    m_mouse_state = -1;

    loadShips();
    m_escMenuShown = false;

    if(g_BEAST_MODE) m_enemySpawnRate = 8;
    else m_enemySpawnRate = 36000;

    m_balanceOfPower.assign(m_factions.size(), 0.0f);

    m_mapExpanded = false;

    m_contextShip = nullptr;
}

void universe::addShot(
        const vec3 _p,
        const vec3 _v,
        const float _angle,
        const std::array<float, WEAPS_W> _weap,
        const aiTeam _team,
        const long int _owner
        )
{
    int temp_angle = _angle + 90;
    for(int i = 0; i < _weap[0]; ++i)
    {
        //vec2 vec = vec(temp_angle);
        laser temp( _p + _v, _v, temp_angle, _weap, _team, _owner);
        temp.setWVel({0.0f, 0.0f, 0.0f});
        m_shots.push_back(temp);
    }
}

void universe::addDebris(
        const vec3 _pos,
        const vec3 _vel,
        resourceType _type
        )
{
    debris temp (_pos, _vel, m_vel, _type);
    m_resources.push_back(temp);
}

void universe::addMissile(
        const vec3 _p,
        const vec3 _v,
        const float _angle,
        const aiTeam _team
        )
{
    missile m(_p, m_drawer.getTextureRadius(ION_MISSILE_MKI));
    m.setVel(_v + tovec3(vec(_angle + 90)) * 5);
    m.setWVel(m_vel);
    m.setAng(_angle);

    int mx = 0, my = 0;
    SDL_GetMouseState(&mx,&my);
    mx -= g_HALFWIN.m_x;
    my -= g_HALFWIN.m_y;

    m.setTarget(closestEnemy({ mx * g_ZOOM_LEVEL, my * g_ZOOM_LEVEL}, _team));

    m_missiles.push_back(m);
}

void universe::update(const float _dt)
{
    m_ui.update(m_factions[TEAM_PLAYER].m_wealth, getMousePos());

    //If m_paused, we do not update the game.
    if(m_paused) return;

    calcPowerBalance();

    m_drawer.update(_dt);

    m_pos += m_vel * g_PIXEL_UNIT_CONVERSION * _dt;
    m_time_elapsed += _dt;
    //Interpolate towards desired background colour.
    m_cCol[0] += clamp(m_tCol[0] - m_cCol[0], -0.01f, 0.01f);
    m_cCol[1] += clamp(m_tCol[1] - m_cCol[1], -0.01f, 0.01f);
    m_cCol[2] += clamp(m_tCol[2] - m_cCol[2], -0.01f, 0.01f);

    //Randomly change background colour.
    if(rand()%1024 == 0)
    {
        float p0 = randNum(0.0f, 1.0f);
        float p1 = randNum(0.0f, 1.0f);
        float p2 = randNum(0.0f, 1.0f);
        float total = p0 + p1 + p2;

        m_tCol[0] = ( p0 / total ) * 250.0f;
        m_tCol[1] = ( p1 / total ) * 250.0f;
        m_tCol[2] = ( p2 / total ) * 250.0f;
    }

    if(rand()%10000 == 0) g_BG_DENSITY = randNum(1.0f,10.0f);
    if(rand()%10000 == 0) m_gameplay_intensity = randNum(0.0f, 2.2f);

    if(!g_GAME_OVER)
    {
        m_ply.ctrlUpdate();
        m_ply.update(_dt);
    }

    m_vel = -m_ply.getVel();

    //If player health is below 25%, emit smoke.
    if(m_ply.getHealth() < m_ply.getMaxHealth() / 4.0f and m_ply.getHealth() > 0.0f) addParticleSprite(m_ply.getPos(), m_ply.getVel(), m_ply.getRadius(), "SMOKE");

    if(m_ply.isFiring() and m_ply.getCooldown() <= 0.0f and m_ply.getEnergy() > m_ply.getCurWeapStat( ENERGY_COST ))
    {
        m_sounds.playSnd( static_cast<sound>(m_ply.getCurWeap()) );
        m_ply.shoot();
        addShot( m_ply.getPos() - m_ply.getVel(), m_ply.getVel(), m_ply.getAng(), m_ply.getWeap(), TEAM_PLAYER, m_ply.getUniqueID() );
        m_ply.setEnergy( m_ply.getEnergy() - m_ply.getCurWeapStat(ENERGY_COST) );
        m_ply.setCooldown( m_ply.getCurWeapStat(COOLDOWN) );
        m_drawer.addShake(m_ply.getCurWeapStat(STOPPING_POWER) * 1000.0f);
    }

    if(!g_DEV_MODE and m_ply.getHealth() <= 0.0f and !g_GAME_OVER)
    {
        for(int p = 0; p < rand()%5 + 10; ++p)
        {
            vec3 pos = {randNum(-16.0f,16.0f), randNum(-16.0f,16.0f), 0.0f};
            pos += m_ply.getPos();
            addpfx(pos, m_ply.getVel(), rand()%50 + 20, rand()%50 + 8);
        }
        m_ply.setMaxHealth(0, true);
        m_ply.setMaxEnergy(0, true);
        m_ply.setMaxShield(0, true);
        playSnd(EXPLOSION_SND);

        m_ply.setPos({F_INF, F_INF});
        m_drawer.addShake(20.0f);

        m_ui.clear();

        g_GAME_OVER = true;
    }

#if RENDER_MODE == 0
    for(auto &i : m_dots)
    {
        i.setWVel(m_vel);
        i.updatePos(_dt);
        if(isOffScreen(i.getPos(), clamp(g_MAX_DIM * g_BG_DENSITY / g_ZOOM_LEVEL, g_MAX_DIM, F_INF)))
        {
            i.gen(true, m_cCol);
        }
    }

    for(auto &i : m_sparkles)
    {
        i.setWVel(m_vel);
        i.updateSprite(_dt);

        std::string temp = i.getIdentifier();
        int w = 0;
        int h = 0;
        m_drawer.queryTexture(temp, 0, &w, &h);

        if(isOffScreen(i.getPos(), clamp((g_MAX_DIM + std::max(w, h)) * g_BG_DENSITY * i.getZ() / g_ZOOM_LEVEL, (g_MAX_DIM + std::max(w, h)), F_INF )))
        {
            i.spriteGen(m_cCol, w, h);
        }
    }
#endif

    for(int i = m_shots.size() - 1; i >= 0; i--)
    {
        if(m_shots[i].getPower() < 0.0f)
        {
            swapnpop(&m_shots, i);
        }
        else
        {
            m_shots[i].setWVel(m_vel);
            m_shots[i].update(_dt);
        }
    }

#if RENDER_MODE == 0
    m_partitions.rects.clear();
#endif
    m_partitions.clear();

    std::vector<enemy*> init_ship;
    for(auto &i : m_agents) init_ship.push_back(&i);
    std::vector<laser*> init_laser;
    for(auto &i : m_shots) init_laser.push_back(&i);
    std::vector<missile*> init_missile;
    for(auto &i : m_missiles) init_missile.push_back(&i);
    std::vector<ship*> init_asteroid;
    for(auto &i : m_asteroids) init_asteroid.push_back(&i);
    std::vector<debris*> init_resources;
    for(auto &i : m_resources) init_resources.push_back(&i);


    std::vector<SDL_Rect> testRects;
    if(m_agents.size() > 0) testRects.push_back(enclose(m_agents));
    if(m_asteroids.size() > 0) testRects.push_back(enclose(m_asteroids));
    if(m_shots.size() > 0) testRects.push_back(enclose(m_shots));
    if(m_resources.size() > 0) testRects.push_back(enclose(m_resources));
    testRects.push_back( {static_cast<int>(m_ply.getPos().m_x), static_cast<int>(m_ply.getPos().m_y), 0, 0} );

    SDL_Rect ir = maxRect(testRects);

    detectCollisions(ir, init_ship, init_laser, init_missile, init_asteroid, init_resources, 0);

    checkCollisions();

    for(int i = m_missiles.size() - 1; i >= 0; i--)
    {
        m_missiles[i].updatePos(_dt);
        bool ofscr = isOffScreen(m_missiles[i].getPos(), 80000.0f);
        if(ofscr or m_missiles[i].getHealth() <= 0 or m_missiles[i].detonate())
        {
            if(!ofscr)
            {
                for(int p = 0; p < rand()%2 + 4; p++)
                {
                    vec3 pos = {randNum(-16.0f,16.0f), randNum(-16.0f,16.0f), 0.0f};
                    pos += m_missiles[i].getPos();
                    addpfx(pos, m_missiles[i].getVel(), rand()%50 + 50, rand()%50 + 8);
                }

                for(size_t j = 0; j < m_agents.size(); j++)
                {
                    vec3 ep = m_agents[j].getPos();
                    vec3 mp = m_missiles[i].getPos();

                    float dmg = magns(mp-ep);

                    if(dmg > 300*300) continue;

                    dmg = 1/mag(mp - ep) * 30000;
                    m_agents[j].damage(dmg);
                    addDamagePopup(dmg, m_agents[j].getTeam(), ep, -m_vel + randVec3(2.0f));
                }

                vec3 pdiff = m_missiles[i].getPos() - m_ply.getPos();
                if(magns(pdiff) < 300*300)
                {
                    float dmg = invMag(pdiff) * 30000;
                    m_ply.damage(dmg);
                    addDamagePopup(dmg, TEAM_PLAYER, m_ply.getPos(), randVec3(2.0f));
                }
            }
            swapnpop(&m_missiles, i);
        }
        else
        {
            if(m_missiles[i].getTarget() == nullptr) m_missiles[i].setTarget(closestEnemy(m_missiles[i].getPos(), m_missiles[i].getTeam()));
            m_missiles[i].setWVel(m_vel);
            m_missiles[i].update(_dt);
            m_missiles[i].steering();
        }
    }

    for(int i = m_asteroids.size() - 1; i >= 0; i--)
    {
        m_asteroids[i].updatePos(_dt);
        vec3 p = m_asteroids[i].getPos();
        vec3 v = m_asteroids[i].getVel();

        if((isOffScreen(p,60000.0f) or m_asteroids[i].getHealth() <= 0.0f))
        {
            if(m_asteroids[i].getHealth() <= 0.0f)
            {
                for(int fx = 0; fx < rand() % 5 + 1; fx++)
                {
                    vec3 pos = {randNum(-16.0f,16.0f), randNum(-16.0f,16.0f)};
                    pos += m_asteroids[i].getPos();
                    addpfx(pos, m_asteroids[i].getVel(), rand()%20 + 50, rand()%30 + 2);
                    for(int q = 0; q < 50; ++q) addParticleSprite(pos, m_asteroids[i].getVel() + m_vel + tovec3(randVec2(1.0f)), m_asteroids[i].getRadius(), "SMOKE");
                }
                if(m_asteroids[i].getClassification() == ASTEROID_SMALL)
                {
                    for(int i = 0; i < rand() % 3 + 3; ++i)
                    {
                        addDebris(p, v + tovec3(randVec2(1.0f)), RESOURCE_IRON);
                    }
                    addScore( 10 );
                }
                else
                {
                    for(int q = 0; q < 4; ++q)
                    {
                        ship a(g_ship_templates.at( static_cast<int>(m_asteroids[i].getClassification()) - 1 ), p + tovec3(randVec2( m_asteroids[i].getRadius()) ));
                        a.setVel( m_asteroids[i].getVel() + tovec3(randVec2(1.0f)) );
                        a.update(_dt);
                        m_asteroids.push_back(a);
                    }
                }
                playSnd(EXPLOSION_SND);
                m_drawer.addShake(12000.0f / (1.0f + mag(m_asteroids[i].getPos() - m_ply.getPos())));
            }
            swapnpop(&m_asteroids, i);
        }
        else
        {
            m_asteroids[i].setWVel(m_vel);
            m_asteroids[i].update(_dt);
        }
    }

    for(int i = m_resources.size() - 1; i >= 0; --i)
    {
        if((isOffScreen(m_resources[i].getPos(),60000.0f)))
        {
            swapnpop(&m_resources, i);
        }
        else
        {
            m_resources[i].setWVel(m_vel);
            m_resources[i].updatePos(_dt);
        }
    }

    //Cull dead m_agents.
    for(int i = m_agents.size() - 1; i >= 0; i--)
    {
        vec3 p = m_agents[i].getPos();

        bool isOffscreen = isOffScreen(p, 80000.0f);
        bool isDead = m_agents[i].getHealth() <= 0.0f;
        bool isPlayerOwned = friendshipCheck(m_agents[i].getTeam(), TEAM_PLAYER);
        bool isSmall = m_agents[i].getType() == SHIP_TYPE_FIGHTER;

        //Offscreen elimination, health-based elimination
        if( isDead or ( isOffscreen and (!isPlayerOwned or isSmall) ) )
        {
            if(isDead)
            {
                for(int p = 0; p < rand() % 5 + 1; p++)
                {
                    vec3 pos = {randNum(-16.0f,16.0f), randNum(-16.0f,16.0f), 0.0f};
                    pos += m_agents[i].getPos();
                    addpfx(pos, m_agents[i].getVel(), randNum(5, 7), m_agents[i].getMaxHealth() / randNum(2.0f, 4.0f));
                }

                //Dump inventory.
                for(auto &d : *(m_agents[i].getCargo()->getItems()))
                {
                    addDebris(p, m_agents[i].getVel() + tovec3(randVec2(1.0f)), RESOURCE_IRON);
                }

                addScore( m_agents[i].getScore() );
                addFrag( m_agents[i].getLastAttacker() );
                if( (m_agents[i].getTeam() == GALACTIC_FEDERATION or m_agents[i].getTeam() == SPOOKY_SPACE_PIRATES) and rand() % 8 <= g_DIFFICULTY ) m_factionMaxCounts[GALACTIC_FEDERATION] += g_DIFFICULTY + 1;
                else if( m_agents[i].getTeam() == SPACE_COMMUNISTS and rand() % 6 <= g_DIFFICULTY ) m_factionMaxCounts[SPACE_COMMUNISTS] += g_DIFFICULTY + 1;
                else if( m_agents[i].getTeam() == ALLIANCE and rand() % 10 <= g_DIFFICULTY ) m_factionMaxCounts[ALLIANCE] += g_DIFFICULTY + 1;

                playSnd(EXPLOSION_SND);
                m_drawer.addShake(10000.0f / (1.0f + mag(m_agents[i].getPos() - m_ply.getPos())));
            }
            if(m_agents[i].getTeam() == GALACTIC_FEDERATION or m_agents[i].getTeam() == SPOOKY_SPACE_PIRATES) m_factionCounts[GALACTIC_FEDERATION]--;
            else m_factionCounts[m_agents[i].getTeam()]--;

            long int la = m_agents[i].getLastAttacker();
            if(la != -1)
            {
                for(auto &j : m_agents)
                {
                    if(j.getUniqueID() == la and j.getTeam() == TEAM_PLAYER)
                    {
                        addPopup( getRandomEntry(&g_fragRemarks), POPUP_NEUTRAL, 4.0f, j.getPos(), -m_vel + randVec3(2.0f) );
                        break;
                    }
                }
                if(la == m_ply.getUniqueID())
                {
                    addPopup( getRandomEntry(&g_fragRemarks), POPUP_NEUTRAL, 4.0f, m_ply.getPos(), randVec3(2.0f) );
                }
            }
            if(m_contextShip == &m_agents[i]) m_contextShip = nullptr;
            swapnpop(&m_agents, i);
        }
    }

    //Set squad variables
    //Get the average position for each squad.
    for(auto &s : m_squads) {s.m_centerPoint = {0.0f, 0.0f}; s.m_averageVel = {0.0f, 0.0f};}
    for(auto &e : m_agents)
    {
        //std::cout << e.getSquadID() << std::endl;
        if(e.getSquadID() >= 0) {m_squads[e.getSquadID()].m_centerPoint += e.getPos(); m_squads[e.getSquadID()].m_averageVel += e.getVel();}
    }
    for(auto &s : m_squads)
    {
        float size = static_cast<float>(s.m_size);
        if(s.m_size > 0) {s.m_centerPoint /= size; s.m_averageVel /= size;}
    }

    //Update live m_agents.
    for(size_t e = 0; e < m_agents.size(); ++e)
    {
        if(!m_agents[e].hasParent())
        {
            m_agents[e].updatePos(_dt);
            m_agents[e].setWVel(m_vel);
        }
        else if(m_agents[e].hasParent())
        {
            ship * parent = nullptr;
            for(auto &q : m_agents)
            {
                if(q.getUniqueID() == m_agents[e].getParent()) parent = &q;
            }
            if(parent == nullptr)
            {
                m_agents[e].setHealth(-1.0f);
                continue;
            }

            float angle = parent->getAng();
            float s = sin(rad(angle));
            float c = cos(rad(angle));

            vec3 epos = m_agents[e].getParentOffset();
            float xn = epos.m_x * c - epos.m_y * s;
            float yn = epos.m_x * s + epos.m_y * c;
            vec3 comb = {xn, yn, 0.0f};

            m_agents[e].setPos( parent->getPos() + comb );
            m_agents[e].setPPos( m_agents[e].getPos() );

            m_agents[e].setWVel(m_vel);
            m_agents[e].setVel(parent->getVel());
        }

        //Setting energy priorities----------------------------------------------------------------------------//
        if(m_agents[e].getHealth() < m_agents[e].getConfidence()) m_agents[e].setEnergyPriority(2);
        else if(m_agents[e].getHealth() < m_agents[e].getMaxHealth() * 0.75f) m_agents[e].setEnergyPriority(1);
        else m_agents[e].setEnergyPriority(0);
        //-----------------------------------------------------------------------------------------------------//
        m_agents[e].update(_dt);

        vec3 p = m_agents[e].getPos();

        //If the agent is damaged, add smoke.
        if(m_agents[e].getHealth() < m_agents[e].getMaxHealth()) addParticleSprite(p, m_agents[e].getVel(), m_agents[e].getRadius(), "SMOKE");

        float minDist = F_MAX;

        //Reset target.
        m_agents[e].setTarget(nullptr);

        if(m_agents[e].getClassification() == PLAYER_MINER_DROID)
        {
            std::cout << "TYPE IS " << m_agents[e].getType() << '\n';
        }

        if(m_agents[e].getType() == SHIP_TYPE_MINER) //Set miner targets
        {
            std::cout << "MINER TARGETS SET!\n";
            //Find the closest asteroid.
            for(auto &k : m_asteroids)
            {
                float nd = magns(p - k.getPos());
                if(nd < minDist)
                {
                    m_agents[e].setTarget( &k );
                    m_agents[e].setGoal(GOAL_ATTACK);
                    minDist = nd;
                }
            }
        }
        else if(m_agents[e].getClassification() == PLAYER_GRAVWELL) //Gravwell attraction
        {
            //Attract m_asteroids based on distancm_agents[e].
            for(auto &k : m_asteroids)
            {
                vec3 incr = p - k.getPos();
                float dist = fabs( magns( incr ) );
                dist /= sqr( 32.0f );

                if(dist > 10000.0f) k.addVel( incr / dist );
                else k.addVel( -k.getVel() * 0.0125f );
            }
        }
        else if(m_agents[e].getClassification() == PLAYER_BARRACKS)
        {
            //Spawn wingman.
            if(rand() % 2048 == 0 and m_factionCounts[TEAM_PLAYER] < 20) spawnShip(getRandomShipType(TEAM_PLAYER), TEAM_PLAYER, p);
        }
        else if(m_agents[e].getCanShoot()) //Default m_target acquisition
        {
            if(m_agents[e].getType() == SHIP_TYPE_MINER) std::cout << "MINER TARGETS INCORRECTLY SET!\n";
            //Get closest enemy.
            for(auto &k : m_agents)
            {
                //Do not target self.
                if(&m_agents[e] == &k) continue;

                float nd = magns(p - k.getPos()) - sqr(k.getRadius()) + sqr(m_agents[e].getRadius());
                if(nd < minDist and emnityCheck(m_agents[e].getTeam(), k.getTeam()))
                {
                    m_agents[e].setTarget( (enemy*)&k );
                    if(m_agents[e].getClassification() != PLAYER_TURRET) m_agents[e].setGoal(GOAL_ATTACK);
                    else m_agents[e].setGoal(GOAL_TURRET);
                    minDist = nd;
                }
            }
        }

        float nd = magns(m_ply.getPos() - m_agents[e].getPos());

        if(emnityCheck( m_agents[e].getTeam(), TEAM_PLAYER ) and nd < minDist and !g_GAME_OVER )
        {
            //If the given agent is hostile, and the players distance is the closest ship.
            m_agents[e].setTarget( &m_ply );
            m_agents[e].setGoal( GOAL_ATTACK );
            minDist = nd;
        }

        //Setting the follow distances of the different units.
        float fd = 0.0f;
        if(m_agents[e].getTeam() == TEAM_PLAYER)
        {
            if(m_agents[e].getType() != SHIP_TYPE_MINER) fd = 15000.0f;
            else fd = 20000.0f;
        }

        if(m_agents[e].getTarget() != nullptr) fd /= 10.0f;

        //If the agent can move, is friendly to the player, and close by, and not in combat.
        if(m_agents[e].getCanMove() and friendshipCheck( m_agents[e].getTeam(), TEAM_PLAYER ) and ( nd > fd * fd ) and !m_agents[e].inCombat())
        {
            m_agents[e].setTarget( (player*)&m_ply );
            m_agents[e].setGoal( GOAL_CONGREGATE );
        }
        else if( m_agents[e].getTarget() == nullptr )
        {
            //If the agent has no m_target, it becomes idle.
            m_agents[e].setGoal( GOAL_WANDER );
        }

        if(emnityCheck( m_agents[e].getTeam(), TEAM_PLAYER ) and m_agents[e].getHealth() < m_agents[e].getConfidence() and m_agents[e].getCanMove())
        {
            //If the enemy can move and is scared, runs away.
            removeFromSquad(&m_agents[e], getSquadFromID(m_agents[e].getSquadID()));
            m_agents[e].setGoal(GOAL_FLEE);
        }

        if(m_agents[e].isFiring() and m_agents[e].getCooldown() <= 0)
        {
            //If the agent is shooting, add lasers.
            m_agents[e].shoot();
            addShot(m_agents[e].getPos() - m_agents[e].getVel(), m_agents[e].getVel(), m_agents[e].getAng(), m_agents[e].getWeap(), m_agents[e].getTeam(), m_agents[e].getUniqueID());
            m_agents[e].setCooldown( (m_agents[e].getCurWeapStat(COOLDOWN)) );
            m_agents[e].setFiring(false);
        }

        //If too far from group center, congregate at center.
        if(m_agents[e].getSquadID() >= 0 and !m_agents[e].inCombat() and magns(p - m_squads[m_agents[e].getSquadID()].m_centerPoint) > sqr(m_squads[m_agents[e].getSquadID()].m_regroupDist) )
        {
            m_agents[e].setTarget(nullptr);
            m_agents[e].setTPos( m_squads[m_agents[e].getSquadID()].m_centerPoint );
            m_agents[e].setTVel( m_squads[m_agents[e].getSquadID()].m_averageVel );
            m_agents[e].setGoal(GOAL_CONGREGATE);
        }

        //Update behaviour, steer towards m_target.
        m_agents[e].behvrUpdate(_dt);
        m_agents[e].steering();
    }

    //Ship spawning functions.
    for(int i = m_particles.size() - 1; i >= 0; i--)
    {
        m_particles[i].setWVel(m_vel);
        m_particles[i].update(_dt);
        if(m_particles[i].done()) swapnpop(&m_particles, i);

    }

    for(int i = m_passiveSprites.size() - 1; i >= 0; --i)
    {
        float alph = m_passiveSprites[i].getCol(3);

        m_passiveSprites[i].setWVel(m_vel);
        m_passiveSprites[i].updateSprite(_dt);

        int w = 32;
        int h = 32;

        if(alph <= 0.1f or isOffScreen(m_passiveSprites[i].getPos(), (g_MAX_DIM + std::max(w, h)) * g_BG_DENSITY * m_passiveSprites[i].getZ() / g_ZOOM_LEVEL) or m_passiveSprites[i].getDim() <= 0.0f)
        {
            //std::cout <<"popit" << std::endl;
            swapnpop(&m_passiveSprites, i);
            continue;
        }
        alph *= 0.9f;
        alph -= 0.02f;
        m_passiveSprites[i].setCol(3, alph);
        m_passiveSprites[i].incrDim(_dt);
    }

    for(int i = m_popups.size() - 1; i >= 0; --i)
    {
        if(m_popups[i].getCol(3) <= 0.0f)
        {
            swapnpop(&m_popups, i);
            continue;
        }
        m_popups[i].update(_dt);
    }

    if(rand() % 256 == 0 and m_wingmenCount < m_maxWingmen)
    {
        spawnShip(TEAM_PLAYER);
    }
    if(rand() % 256 == 0 and m_minerCount < m_maxMiners)
    {
        spawnShip(PLAYER_MINER_DROID, TEAM_PLAYER, tovec3(randVec2(10000.0f, 20000.0f)));
    }

    if(g_DIFFICULTY == 0) return;

    if(rand() % m_enemySpawnRate <= g_DIFFICULTY * m_gameplay_intensity and m_factionCounts[GALACTIC_FEDERATION] < clamp(m_factionMaxCounts[GALACTIC_FEDERATION],0,100))
    {
        int reps = clamp(rand() % (g_DIFFICULTY * 5) + 1, 1, clamp(m_factionMaxCounts[GALACTIC_FEDERATION],0,80) - m_factionCounts[GALACTIC_FEDERATION]);
        aiTeam pteam;
        if(rand() % 100 < 50) pteam = SPOOKY_SPACE_PIRATES;
        else pteam = GALACTIC_FEDERATION;
        spawnSquad(pteam, 10000.0f, 20000.0f, reps);
    }

    if(rand() % m_enemySpawnRate <= g_DIFFICULTY * m_gameplay_intensity and m_factionCounts[SPACE_COMMUNISTS] < clamp(m_factionMaxCounts[SPACE_COMMUNISTS],0,100))
    {
        int reps = clamp(rand() % (g_DIFFICULTY * 20) + 1, 1, clamp(m_factionMaxCounts[SPACE_COMMUNISTS],0,80) - m_factionCounts[SPACE_COMMUNISTS]);
        aiTeam pteam;
        pteam = SPACE_COMMUNISTS;
        spawnSquad(pteam, 10000.0f, 20000.0f, reps);
    }

    if(rand() % m_enemySpawnRate <= g_DIFFICULTY * m_gameplay_intensity and m_factionCounts[ALLIANCE] < clamp(m_factionMaxCounts[ALLIANCE],0,100))
    {
        int reps = clamp(rand() % (g_DIFFICULTY * 20) + 1, 1, clamp(m_factionMaxCounts[ALLIANCE],0,80) - m_factionCounts[ALLIANCE]);
        aiTeam pteam;
        pteam = ALLIANCE;
        spawnSquad(pteam, 10000.0f, 20000.0f, reps);
    }

    if(rand() % 1500 == 0 and m_asteroids.size() < 10)
    {
        ship_spec size = ASTEROID_MID;
        int prob = rand() % 100;

        if(prob > 50 and prob <= 80) size = ASTEROID_SMALL;
        else if(prob > 80 and prob <= 99) size = ASTEROID_LARGE;

        vec3 ppos;
        int side = rand() %4 ;
        if(side == 0) ppos = {randNum(-20000.0f,20000.0f), -20000.0f, 0.0f};
        else if(side == 1) ppos = {randNum(-20000.0f,20000.0f), 20000.0f, 0.0f};
        else if(side == 2) ppos = {-20000.0f, randNum(-20000.0f,20000.0f), 0.0f};
        else if(side == 3) ppos = {20000.0f, randNum(-20000.0f,20000.0f), 0.0f};
        ship a(ppos, size, m_drawer.getTextureRadius( size ));
        a.setVel( tovec3(randVec2(64.0f)) );
        a.update(_dt);
        m_asteroids.push_back(a);
    }

    if(rand() == 0) m_factionMaxCounts[GALACTIC_FEDERATION] = clamp(m_factionMaxCounts[GALACTIC_FEDERATION] - 1, 0, I_MAX);
    if(rand() == 0) m_factionMaxCounts[SPACE_COMMUNISTS] = clamp(m_factionMaxCounts[SPACE_COMMUNISTS] - 1, 0, I_MAX);
}

#if RENDER_MODE == 0
void universe::draw(float _dt)
{
    //std::cout << 1/_dt << " fps" << std::endl;
    //std::cout << m_passiveSprites.size() << std::endl;
    if(m_paused) _dt = 0.0f;

    m_drawer.clear();

    for(auto i = m_dots.begin(); i != m_dots.end(); ++i)
    {
        if(i->getZ() > 1) continue;

        vec3 ipos = i->getInterpolatedPosition(_dt);
        vec3 ivel = (i->getVel() + i->getWVel()) * i->getZ();
        std::array<float, 4> icol = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
        m_drawer.drawLine(tovec2(ipos), tovec2(ipos + ivel), icol);
    }

    m_drawer.setBlendMode(SDL_BLENDMODE_ADD);
    for(auto i = m_sparkles.begin(); i != m_sparkles.end(); ++i)
    {
        if(i->getZ() <= 1)
        {
            vec3 ipos = i->getInterpolatedPosition(_dt);
            std::array<float, 4> col = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
            m_drawer.drawTexture( i->getTex(), 0, tovec2(ipos), i->getAng(), col );
        }
    }
    m_drawer.setBlendMode(SDL_BLENDMODE_BLEND);

    for(auto i = m_passiveSprites.begin(); i != m_passiveSprites.end(); ++i)
    {
        if(!m_paused) i->incrDim();
        vec3 ipos = i->getInterpolatedPosition(_dt);
        std::array<float, 4> col = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
        m_drawer.drawTexture( i->getTex(), 0, tovec2(ipos), i->getAng(), col );
    }

    for(auto i = m_shots.begin(); i != m_shots.end(); ++i)
    {
        vec3 ipos = i->getInterpolatedPosition(_dt);
        vec3 ivel = (i->getVel() + i->getWVel()) * 3;
        std::array<float, 4> iscol = {i->getCol(0), i->getCol(1), i->getCol(2), 255};
        std::array<float, 4> iecol = {iscol[0] / 2, iscol[1] / 2, iscol[2] / 2, 20};
        m_drawer.drawLineGr(tovec2(ipos), tovec2(ipos + ivel), iecol, iscol);
    }

    for(auto i = m_asteroids.begin(); i != m_asteroids.end(); ++i)
    {
        vec3 ipos = i->getInterpolatedPosition(_dt);
        std::array<float, 4> icol = {255, 255, 255, 255};
        m_drawer.drawTexture(i->getIdentifier(), 0, tovec2(ipos), i->getAng(), icol);
    }

    for(auto i = m_agents.begin(); i != m_agents.end(); ++i)
    {
        vec3 ipos = i->getInterpolatedPosition(_dt);
        std::array<float, 4> ialpha = i->getAlphaStats();
        m_drawer.drawTextureSet(i->getIdentifier(), tovec2(ipos), i->getAng(), ialpha);
    }

    if(!g_GAME_OVER)
    {
        vec3 ppos = m_ply.getInterpolatedPosition(_dt);
        std::array<float, 4> palpha = m_ply.getAlphaStats();
        m_drawer.drawTextureSet(m_ply.getIdentifier(), tovec2(ppos), m_ply.getAng(), palpha);
    }

    for(auto i = m_missiles.begin(); i != m_missiles.end(); ++i)
    {
        vec3 ipos = i->getInterpolatedPosition(_dt);
        std::array<float, 4> ialpha = i->getAlphaStats();
        m_drawer.drawTextureSet(i->getIdentifier(), tovec2(ipos), i->getAng(), ialpha);
    }

    for(auto i = m_particles.begin(); i != m_particles.end(); ++i)
    {
        vec3 ipos = i->getPos();
        std::array<float, 4> col = {i->getCol(0), i->getCol(1), i->getCol(2), i->getAlpha()};

        m_drawer.drawTexture(i->getIdentifier(), 0, tovec2(ipos), 0, col);
        int k = 0;
        for(auto j = i->getParticles()->begin(); j != i->getParticles()->end(); ++j)
        {
            vec3 jpos = j->getInterpolatedPosition(_dt);
            col[3] = i->getAlpha(k);
            m_drawer.drawLine(tovec2(jpos), tovec2(jpos + j->getVel()), col);
            k++;
        }
    }

    for(auto i = m_dots.begin(); i != m_dots.end(); ++i)
    {
        if(i->getZ() <= 1) continue;
        vec3 ipos = i->getInterpolatedPosition(_dt);
        vec3 ivel = (i->getVel() + i->getWVel()) * i->getZ();
        std::array<float, 4> icol = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
        m_drawer.drawLine(tovec2(ipos), tovec2(ipos + ivel), icol);
    }

    m_drawer.setBlendMode(SDL_BLENDMODE_ADD);
    for(auto i = m_sparkles.begin(); i != m_sparkles.end(); ++i)
    {
        if(i->getZ() > 1)
        {
            vec3 ipos = i->getInterpolatedPosition(_dt);
            std::array<float, 4> icol = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
            m_drawer.drawTexture( i->getTex(), 0, tovec2(ipos), i->getAng(), icol );

        }
    }
    m_drawer.setBlendMode(SDL_BLENDMODE_BLEND);

    /*if(g_DEV_MODE)
            {
                for(auto i = m_partitions.rects.begin(); i != m_partitions.rects.end(); ++i)
                {
                    std::array<int, 4> col = {255, 0, 0, 255};
                    m_drawer.drawRect({static_cast<float>(i->x), static_cast<float>(i->y)}, {static_cast<float>(i->w), static_cast<float>(i->h)}, col, true);
                }
            }*/

    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    vec2 dpos = {static_cast<float>(mx), static_cast<float>(my)};
    dpos -= g_HALFWIN;
    dpos /= g_ZOOM_LEVEL;

    switch(m_mouse_state)
    {
    case 7:
        m_drawer.drawTexture("PLAYER_TURRET", 0, dpos, 0, {255, 255, 255, 100});
        m_drawer.drawTexture("PLAYER_TURRET", 5, dpos, 0, {255, 255, 255, 100});
        break;
    case 8:
        m_drawer.drawTexture("PLAYER_GRAVWELL", 0, dpos, 0, {255, 255, 255, 100});
        break;
    case 9:
        m_drawer.drawTexture("PLAYER_BARRACKS", 0, dpos, 0, {255, 255, 255, 100});
        break;
    case 10:
        m_drawer.drawTexture("PLAYER_STATION", 0, dpos, 0, {255, 255, 255, 100});
        break;
    default:
        break;
    }

    //Draw the ui
    if(showUI) drawUI();

    m_drawer.finalise();

}

void universe::drawUI()
{
    if(!g_GAME_OVER)
    {
        m_drawer.drawText("SCORE: " + std::to_string( m_factions[TEAM_PLAYER].m_wealth ),"pix",{260, 2}, false, 1.0f);
        m_drawer.drawText("MISSILES: " + std::to_string( m_ply.getMissiles() ),"pix",{260, 20}, false, 1.0f);

        m_drawer.drawMap(&m_missiles, &m_agents, &m_asteroids, &m_shots, &m_factions);
        m_drawer.ss(&m_ply);
        m_drawer.drawWeaponStats(&m_ply);
    }

    for(auto i = m_ui.getElements()->begin(); i != m_ui.getElements()->end(); ++i)
    {
        if(!i->isVisible()) continue;
        for(auto j = i->getButtons()->begin(); j != i->getButtons()->end(); ++j)
        {
            std::array<int, 8> col = j->getCol();
            if(!j->isSelected())
            {
                m_drawer.drawRect(j->getPos(), j->getDim(), {col[0], col[1], col[2], col[3]}, false);
            }
            else
            {
                m_drawer.drawRect(j->getPos(), j->getDim(), {col[4], col[5], col[6], col[7]}, false);
            }
            m_drawer.drawText(j->getLabel(), "pix", j->getPos(), false, 1.0f);

            if(j->isDark())
            {
                m_drawer.drawRect(j->getPos(), j->getDim(), {0, 0 ,0 , 200}, false);
            }
        }
    }
}

#elif RENDER_MODE == 1
void universe::draw(float _dt)
{
    m_drawer.update(_dt);

    m_drawer.drawBackground(m_time_elapsed, tovec2(m_pos), tovec2(m_vel), m_cCol);

    for(auto i = m_passiveSprites.begin(); i != m_passiveSprites.end(); ++i)
    {
        if(i->getIdentifier() != "SMOKE") continue;
        vec3 ipos = i->getInterpolatedPosition(_dt);
        vec2 idim = {i->getDim(), i->getDim()};
        std::array<float, 4> col = {i->getCol(0), i->getCol(1), i->getCol(2), i->getCol(3)};
        col = col255to1(col);
        //col[3] *= 255.0f;
        //std::cout << col[0] << ", " << col[1] << ", " << col[2] << ", " << col[3] << std::endl;
        m_drawer.addRect(ipos, idim, 0.0f, col);
    }
    m_drawer.drawSmoke(m_time_elapsed);
    m_drawer.clearVectors();

    m_drawer.useShader("flame");
    float stat = (m_ply.getAlphaStats()[0] * m_ply.getEnginePower() * m_ply.getRadius()) / 1000.0f;

    //std::cout << m_ply.getAlphaStats()[0] << " " << stat << std::endl;
    if(stat > 0.05f and !g_GAME_OVER)
    {
        m_drawer.drawFlames(
                    m_ply.getInterpolatedPosition(_dt) + tovec3(back(rad(m_ply.getAng()))) * m_ply.getRadius() * clamp(stat * 0.055f, 1.0f, 20.0f),
        {m_ply.getRadius() * 0.85f, stat},
                    m_ply.getAng(),
        {0.1f, 0.4f, 1.0f, 1.0f},
                    m_time_elapsed,
                    m_ply.getAlphaStats()[0] * m_ply.getEnginePower()
                );
    }

    for(auto &i : m_agents)
    {
        float stat = (i.getAlphaStats()[0] * i.getEnginePower() * i.getRadius()) / 1000.0f;
        std::array<float, 4> col = i.getCurWeapCol();
        col[3] = 1.0f;
        if(stat > 0.05f)
        {
            m_drawer.drawFlames(
                        i.getInterpolatedPosition(_dt) + tovec3(back(rad(i.getAng()))) * i.getRadius() * clamp(stat * 0.055f, 1.0f, 20.0f),
            {i.getRadius() * 0.85f, stat},
                        i.getAng(),
                        col,
                        m_time_elapsed,
                        i.getAlphaStats()[0] * i.getEnginePower()
                    );
        }
    }

    for(auto &i : m_missiles)
    {
        float stat = (i.getAlphaStats()[0] * i.getEnginePower()) / 25.0f;

        if(stat > 0.05f)
        {
            m_drawer.drawFlames(
                        i.getInterpolatedPosition(_dt) + tovec3(back(rad(i.getAng()))) * (i.getRadius() + stat),
            {i.getRadius(), stat},
                        i.getAng(),
            {0.1f, 0.4f, 1.0f, 1.0f},
                        m_time_elapsed,
                        i.getAlphaStats()[0] * i.getEnginePower()
                    );
        }
    }

    m_drawer.enableDepthSorting();
    m_drawer.useShader("ship");
    if(!g_GAME_OVER) m_drawer.drawShip(m_ply.getInterpolatedPosition(_dt), m_ply.getAng(), m_ply.getIdentifier(), m_ply.getCurWeapCol());
    for(auto &i : m_agents)
    {
        m_drawer.drawShip(i.getInterpolatedPosition(_dt), i.getAng(), i.getIdentifier(), i.getCurWeapCol());
    }

    for(auto &i : m_missiles)
    {
        m_drawer.drawShip(i.getInterpolatedPosition(_dt), i.getAng(), i.getIdentifier(), i.getCurWeapCol());
    }
    for(auto &i : m_asteroids)
    {
        m_drawer.drawShip(i.getInterpolatedPosition(_dt), i.getAng(), i.getIdentifier(), {0.0f, 0.0f, 0.0f, 0.0f});
    }
    for(auto &i : m_resources)
    {
        m_drawer.drawShip(i.getInterpolatedPosition(_dt), i.getAng(), i.getIdentifier(), {0.0f, 0.0f, 0.0f, 0.0f});
    }

    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);

    vec2 dpos = {static_cast<float>(mx), static_cast<float>(my)};
    dpos -= g_HALFWIN;
    dpos /= g_ZOOM_LEVEL;

    if(m_mouse_state != -1) m_drawer.drawAsset(dpos, 0.0f, g_ship_templates[m_mouse_state].getIdentifier(), 0.5f);

    m_drawer.useShader("laser");
    for(auto &i : m_shots)
    {
        vec3 ipos = i.getInterpolatedPosition(_dt);
        vec3 ivel = (i.getVel() + i.getWVel()) * 8.0f;
        std::array<float, 4> icol = i.getCol();

        icol = col255to1(icol);
        icol[3] *= clamp(i.getPower() / 25.0f, 0.0f, 1.0f);

        m_drawer.addLine(ipos, ipos + ivel, icol);
    }
    m_drawer.drawLasers();

    m_drawer.clearVectors();

    m_drawer.disableDepthSorting();

    if(m_ply.getShieldGlow() > 1 and !g_GAME_OVER) m_drawer.drawShield(m_ply.getInterpolatedPosition(_dt), m_ply.getRadius(), m_time_elapsed / m_ply.getRadius(), m_ply.getShieldGlow() / 255.0f, {0.1f, 0.4f, 1.0f, 1.0f});
    for(auto &i : m_agents)
    {
        if(i.getShieldGlow() <= 1) continue;
        m_drawer.drawShield(i.getInterpolatedPosition(_dt), i.getRadius(), m_time_elapsed / i.getRadius(), i.getShieldGlow() / 255.0f, i.getShieldCol());
    }
    for(auto &i : m_missiles)
    {
        if(i.getShieldGlow() <= 1) continue;
        m_drawer.drawShield(i.getInterpolatedPosition(_dt), i.getRadius(), m_time_elapsed / i.getRadius(), i.getShieldGlow() / 255.0f, {0.1f, 0.4f, 1.0f, 1.0f});
    }

    m_drawer.clearVectors();
    if(m_ply.getCurWeapCol()[3] > 0.05f and !g_GAME_OVER)
    {
        float dim = m_ply.getCurWeapStat(DAMAGE) * 4.0f;
        m_drawer.addRect(m_ply.getPos() + tovec3(front(rad(m_ply.getAng()))) * m_ply.getRadius(),
        {dim, dim},
                         0.0f,
                         m_ply.getCurWeapCol());
    }
    for(auto &i : m_agents)
    {
        if(i.getCurWeapCol()[3] > 0.05f)
        {
            float dim = i.getCurWeapStat(DAMAGE) * 4.0f;m_drawer.addRect(
                        i.getPos() + tovec3(front(rad(i.getAng()))) * i.getRadius(),
            {dim, dim},
                        0.0f,
                        i.getCurWeapCol());
        }
    }
    m_drawer.useShader("explosion");
    m_drawer.drawRects(true);

    m_drawer.clearVectors();
    for(auto &i : m_particles)
    {
        std::array<float, 4> col = {i.getCol(0), i.getCol(1), i.getCol(2), i.getAlpha()};
        col = col255to1(col);
        vec3 ipos = i.getPos();
        float idim = i.getForce() * 20.0f;

        if(col[3] > 0.05f)
        {
            m_drawer.addRect(ipos, {idim, idim}, 0.0f, col);
        }
    }
    m_drawer.useShader("explosion");
    m_drawer.drawRects(true);

    if(g_GRAPHICAL_DETAIL > 1)
    {
        m_drawer.clearVectors();
        for(auto &i : m_particles)
        {
            std::array<float, 4> col = {i.getCol(0), i.getCol(1), i.getCol(2), i.getAlpha()};
            col = col255to1(col);
            int k = 0;
            for(auto j = i.getParticles()->begin(); j != i.getParticles()->end(); ++j)
            {
                vec3 jpos = j->getInterpolatedPosition(_dt);
                vec3 jvel = (j->getVel()) * 3;
                col[3] = i.getAlpha(k) / 255.0f;

                m_drawer.addLine(jpos, jpos + jvel, col);
                ++k;
            }
        }
        m_drawer.useShader("plain");
        m_drawer.drawLines(1.0f);
    }

    if(showUI)
    {
        for(auto &i : m_popups)
        {
            m_drawer.drawText(i.getLabel(), "pix", tovec2(i.getPos()), true, i.getSize(), col255to1(i.getCol()));
        }
        drawUI(_dt);
    }
}

void universe::drawUI(const float _dt)
{
    m_drawer.clearVectors();
    if(!g_GAME_OVER)
    {
        m_drawer.drawText("SCORE: " + std::to_string( m_factions[TEAM_PLAYER].m_wealth ),"pix",{280, 56}, false, 1.0f);
        m_drawer.drawText("MISSILES: " + std::to_string( m_ply.getMissiles() ),"pix",{280, 80}, false, 1.0f);

        m_drawer.statusBars(&m_ply);
        m_drawer.drawWeaponStats(&m_ply);
        m_drawer.drawMap(&m_missiles, &m_agents, &m_asteroids, &m_shots, &m_factions, m_mapExpanded);

        m_drawer.clearVectors();
        float cumulative = 0.0f;
        int i = 0;
        for(auto &b : m_balanceOfPower)
        {
            cumulative += b * 128.0f;
            m_drawer.addRect({g_WIN_WIDTH - 256.0f + cumulative, 272.0f}, {b * 256.0f, 16.0f}, 0.0f, col255to1(m_factions[i].m_colour));
            cumulative += b * 128.0f;
            ++i;
        }
        m_drawer.useShader("plain");
        m_drawer.drawRects(false);
        m_drawer.clearVectors();

        //DRAWING CONTEXT-SELECTED SHIP STATS
        selection * infoCard = &(*m_ui.getElements())[3];

        if(m_contextShip != nullptr)
        {
            vec3 csp = m_contextShip->getPos();
            vec2 offset = {256.0f, 256.0f};
            vec2 min = (-g_HALFWIN + offset) / g_ZOOM_LEVEL;
            vec2 max = (g_HALFWIN - offset) / g_ZOOM_LEVEL;
            csp = clamp(csp, tovec3(min), tovec3(max));

            float health = 128.0f * m_contextShip->getHealth() / m_contextShip->getMaxHealth();
            float shield = 128.0f * m_contextShip->getShield() /  m_contextShip->getMaxShield();
            float energy = 128.0f * m_contextShip->getEnergy() / m_contextShip->getMaxEnergy();

            infoCard->setVisible(true);

            infoCard->getAt(0)->setLabel(m_contextShip->getIdentifier());
            infoCard->getAt(1)->setLabel("KILLS: " + std::to_string(m_contextShip->getKills()));
            infoCard->getAt(2)->setLabel("DISTANCE: " + std::to_string(static_cast<int>(mag(m_contextShip->getPos()) / 4.0f)));
            for(auto &i : *(infoCard->getButtons()))
            {
                vec2 pos = i.getPos();
                pos += tovec2( csp );
                i.setPos( pos );
            }

            csp.m_x += 64.0f;
            csp.m_y += 32.0f;

            //Health base
            m_drawer.addRect(csp, {128.0f, 16.0f}, 0.0f, {0.4f, 0.08f, 0.08f, 0.5f});
            //Health
            m_drawer.addRect({csp.m_x - 64.0f + health / 2.0f, csp.m_y}, {health, 16.0f}, 0.0f, {0.9f, 0.2f, 0.2f, 0.5f});

            csp.m_y += 16.0f;
            //Shield base
            m_drawer.addRect(csp, {128.0f, 16.0f}, 0.0f, {0.1f, 0.1f, 0.4f, 0.5f});
            //Shield
            m_drawer.addRect({csp.m_x - 64.0f + shield / 2.0f, csp.m_y}, {shield, 16.0f}, 0.0f, {0.2f, 0.2f, 0.9f, 0.5f});

            csp.m_y += 16.0f;
            //Energy base
            m_drawer.addRect(csp, {128.0f, 16.0f}, 0.0f, {0.08f, 0.4f, 0.08f, 0.5f});
            //Energy
            m_drawer.addRect({csp.m_x - 64.0f + energy / 2.0f, csp.m_y}, {energy, 16.0f}, 0.0f, {0.2f, 0.9f, 0.2f, 0.5f});

            m_drawer.drawRects(true);
            m_drawer.clearVectors();
        }
        else
        {
            infoCard->setVisible(false);
        }

        if(m_ply.getCargo()->isVisible())
        {
            vec2 dim = m_ply.getCargo()->getDim();
            m_drawer.addRect({0.0f, 0.0f, 0.0f}, dim, 0.0f, {0.8f, 0.8f, 0.8f, 0.8f});
            m_drawer.drawRects(true);
            m_drawer.clearVectors();

            m_drawer.useShader("ship");
            for(auto &i : *(m_ply.getCargo()->getItems()))
            {
                m_drawer.drawShip(i.getInterpolatedPosition(_dt), i.getAng(), i.getIdentifier(), {0.0f, 0.0f, 0.0f, 0.0f});
            }
            m_drawer.useShader("plain");
        }
    }

    for(auto i = m_ui.getElements()->begin(); i != m_ui.getElements()->end(); ++i)
    {
        if(!i->isVisible()) continue;
        for(auto j = i->getButtons()->begin(); j != i->getButtons()->end(); ++j)
        {
            std::array<float, 4> col = j->getDrawCol();

            vec2 jdim = j->getDim();
            vec2 jpos = j->getPos();

            jpos += jdim * 0.5f;

            m_drawer.addRect(tovec3(jpos), jdim, 0.0f, col);
        }

        m_drawer.useShader("plain");
        m_drawer.drawRects(i->inWorldSpace());
        m_drawer.clearVectors();

        for(auto k = i->getButtons()->begin(); k != i->getButtons()->end(); ++k)
        {
            std::array<float, 4> col = col255to1(k->getTCol());

            vec2 kdim = k->getDim();
            vec2 kpos = k->getPos();

            kpos.m_x += kdim.m_x * 0.25f;
            kpos.m_y += kdim.m_y * 0.5f;

            m_drawer.drawText(k->getLabel(), "pix", kpos, i->inWorldSpace(), k->getTextSizeMul(), col);
        }
    }
}
#endif

void universe::detectCollisions(
        SDL_Rect _box,
        std::vector<enemy*> _ships,
        std::vector<laser*> _lasers,
        std::vector<missile*> _rockets,
        std::vector<ship*> _rocks,
        std::vector<debris*> _resources,
        unsigned short int _lvl
        )
{
    size_t count = 0;
    std::vector<enemy*> pships;
    std::vector<laser*> plasers;
    std::vector<missile*> prockets;
    std::vector<ship*> procks;
    std::vector<debris*> presources;

    vec2 boxPos = {static_cast<float>(_box.x), static_cast<float>(_box.y)};
    vec2 boxDim = {static_cast<float>(_box.w), static_cast<float>(_box.h)};

    //Check through all input vectors.
    for(auto& i: _ships)
    {
        if( circleIntersectRect(tovec2(i->getPos()), i->getRadius(), boxPos, boxDim ) )
        {
            pships.push_back(i);
            count++;
        }
    }

    for(auto &i : _rockets)
    {
        if( circleIntersectRect(tovec2(i->getPos()), i->getRadius(), boxPos, boxDim ) )
        {
            prockets.push_back(i);
            count++;
        }
    }

    for(auto &i : _rocks)
    {
        if( circleIntersectRect(tovec2(i->getPos()), i->getRadius(), boxPos, boxDim ) )
        {
            procks.push_back(i);
            count++;
        }
    }

    for(auto &i : _lasers)
    {
        vec2 p = tovec2(i->getPos());
        vec2 v = p + tovec2(i->getVel());

        int sx = p.m_x, sy = p.m_y, ex = v.m_x, ey = v.m_y;
        if( SDL_IntersectRectAndLine(&_box, &sx, &sy, &ex, &ey ) )
        {
            plasers.push_back(i);
            count++;
        }
    }

    for(auto &i : _resources)
    {
        if( circleIntersectRect(tovec2(i->getPos()), i->getRadius(), boxPos, boxDim) )
        {
            presources.push_back(i);
            count++;
        }
    }

    //Return if count is too low, or nothing can interact.
    if(count < 16 or _lvl > 8)
    {
        m_partitions.push_back({
                                   pships,
                                   plasers,
                                   prockets,
                                   procks,
                                   presources,
                               });

#if RENDER_MODE == 0
        m_partitions.rects.push_back(_box);
#endif
    }
    //Subdivide otherwise.
    else
    {
        int x = _box.x, y = _box.y, w = _box.w / 2, h = _box.h / 2;
        SDL_Rect r1 = {x, 		y, 		w, 		h};
        SDL_Rect r2 = {x + w, 	y, 		w, 		h};
        SDL_Rect r3 = {x, 		y + h, 	w, 		h};
        SDL_Rect r4 = {x + w, 	y + h, 	w, 		h};

        detectCollisions(r1, pships, plasers, prockets, procks, presources, _lvl + 1);
        detectCollisions(r2, pships, plasers, prockets, procks, presources, _lvl + 1);
        detectCollisions(r3, pships, plasers, prockets, procks, presources, _lvl + 1);
        detectCollisions(r4, pships, plasers, prockets, procks, presources, _lvl + 1);
    }
}

void universe::checkCollisions()
{
    for(size_t p = 0; p < m_partitions.size(); ++p)
    {
        bool done = false;
        int harm = 0;

        //LASERS
        //Pew pew
        for(int l = m_partitions[p].m_lasers.size() - 1; l >= 0; --l)
        {
            vec3 sp = m_partitions[p].m_lasers[l]->getPos();
            vec3 sv = m_partitions[p].m_lasers[l]->getVel() * 5.0f;
            vec3 spv = sp + sv * 1.5;
            float stop = m_partitions[p].m_lasers[l]->getStop();
            long int so = m_partitions[p].m_lasers[l]->getOwner();

            vec3 ep;
            vec3 ev;
            float er;
            float ei;

            vec3 d_dir;

            //cout << "ENEMY CHECK" << endl;
            for(int s = m_partitions[p].m_ships.size() - 1; s >= 0; s--)
            {
                if(neutralityCheck(m_partitions[p].m_lasers[l]->getTeam(), m_partitions[p].m_ships[s]->getTeam())) continue;
                harm = 0;

                ep = m_partitions[p].m_ships[s]->getPos();
                ev = m_partitions[p].m_ships[s]->getVel();
                er = m_partitions[p].m_ships[s]->getRadius();
                ei = m_partitions[p].m_ships[s]->getInertia();

                //if(lineIntersectCircle(tovec2(sp), tovec2(spv), tovec2(ep), er))
                if(lineIntersectSphere(sp, spv, ep, er))
                {
                    addpfx(ep + randVec3(er), ev, randNum(3.0f, 8.0f), randNum(3.0f, 8.0f));
                    harm = m_partitions[p].m_lasers[l]->getDmg();

                    d_dir = m_partitions[p].m_lasers[l]->getVel();
                    //Delete m_shots if they match the ones in the universe vector.
                    for(int i = m_shots.size() - 1; i >= 0; --i) if(&m_shots[i] == m_partitions[p].m_lasers[l]) swapnpop(&m_shots, i);
                    //Pop the partition pointer.
                    swapnpop(&m_partitions[p].m_lasers, l);
                    done = true;
                }
                if(harm > 0)
                {
                    m_partitions[p].m_ships[s]->damage(harm, d_dir * stop * ei, so);
                    addDamagePopup(harm, m_partitions[p].m_ships[s]->getTeam(), ep, -m_vel + randVec3(2.0f));
                    break;
                }
            }
            if(done) continue;

            for(int r = m_partitions[p].m_rocks.size() - 1; r >= 0; r--)
            {
                //cout << "R is " << r << ") " << m_partitions[p].m_rocks[r] << endl;
                harm = 0;

                ep = m_partitions[p].m_rocks[r]->getPos();
                ev = m_partitions[p].m_rocks[r]->getVel();
                er = m_partitions[p].m_rocks[r]->getRadius();
                ei = m_partitions[p].m_rocks[r]->getInertia();

                //if(lineIntersectCircle(tovec2(sp), tovec2(spv), tovec2(ep), er))
                if(lineIntersectSphere(sp, spv, ep, er))
                {
                    addpfx(ep + randVec3(er), ev, randNum(3.0f, 8.0f), randNum(3.0f, 8.0f));
                    for(int q = 0; q < 20; ++q) addParticleSprite(ep, ev + tovec3(randVec2(1.0f)), er, "SMOKE");
                    harm = m_partitions[p].m_lasers[l]->getDmg();

                    d_dir = m_partitions[p].m_lasers[l]->getVel();
                    for(int i = m_shots.size() - 1; i >= 0; --i) if(&m_shots[i] == m_partitions[p].m_lasers[l]) swapnpop(&m_shots, i);//m_shots.erase(m_shots.begin() + i);
                    swapnpop(&m_partitions[p].m_lasers, l);

                    done = true;
                }
                if(harm > 0)
                {
                    m_partitions[p].m_rocks[r]->damage(harm, d_dir * stop * ei, so);
                    break;
                }
            }
            if(done) continue;

            //cout << "ENEMY CHECK" << endl;
            for(int m = m_partitions[p].m_rockets.size() - 1; m >= 0; m--)
            {
                harm = 0;

                ep = m_partitions[p].m_rockets[m]->getPos();
                ev = m_partitions[p].m_rockets[m]->getVel();
                er = m_partitions[p].m_rockets[m]->getRadius();
                ei = m_partitions[p].m_rockets[m]->getInertia();

                //if(lineIntersectCircle(tovec2(sp), tovec2(spv), tovec2(ep), er))
                if(lineIntersectSphere(sp, spv, ep, er))
                {
                    addpfx(ep + randVec3(er), ev, randNum(3.0f, 8.0f), randNum(3.0f, 8.0f));

                    harm = m_shots[l].getDmg();

                    d_dir = m_partitions[p].m_lasers[l]->getVel();
                    for(int i = m_shots.size() - 1; i >= 0; --i) if(&m_shots[i] == m_partitions[p].m_lasers[l]) m_shots.erase(m_shots.begin() + i);
                    swapnpop(&m_partitions[p].m_lasers, l);

                    done = true;
                }
                if(harm > 0)
                {
                    m_partitions[p].m_rockets[m]->damage(harm, d_dir * stop * ei, so);
                    addDamagePopup(harm, TEAM_PLAYER, ep, -m_vel + randVec3(2.0f));
                    break;
                }
            }
            if(done) continue;

            harm = 0;

            vec3 dp = sp - m_ply.getPos();
            vec3 dv = sv + m_ply.getVel();
            if(fabs(dv.m_x) > fabs(dp.m_x) - 32 and fabs(dv.m_y) > fabs(dp.m_y) - 32 and m_partitions[p].m_lasers[l]->getTeam() != TEAM_PLAYER)
            {
                vec3 spv = sp + sv;

                //if(lineIntersectCircle(tovec2(sp), tovec2(spv), tovec2(dp), m_ply.getRadius()))
                if(lineIntersectSphere(sp, spv, dp, m_ply.getRadius()))
                {
                    playSnd(RICOCHET_SND);
                    m_drawer.addShake(5.0f);
                    addpfx(sp + randVec3(32.0f), m_ply.getVel(), randNum(3.0f, 8.0f), randNum(3.0f, 8.0f));
                    harm = m_partitions[p].m_lasers[l]->getDmg();

                    d_dir = m_partitions[p].m_lasers[l]->getVel();
                    for(int i = m_shots.size() - 1; i >= 0; --i) if(&m_shots[i] == m_partitions[p].m_lasers[l]) swapnpop(&m_shots, i);
                    swapnpop(&m_partitions[p].m_lasers, l);

                    done = true;
                }
                if(harm > 0)
                {
                    m_ply.damage(harm, d_dir * stop * m_ply.getInertia(), so);
                    //std::cout << "ADDING VEL (" << sv.m_x << "," << sv.m_y << " - " << ev.m_x << "," << ev.m_y << ") * " << stop << " = (" << ( (sv - ev) * stop ).m_x << ", " << ( (sv - ev) * stop ).m_y << ")" << std::endl;
                    setVel(-m_ply.getVel());
                }
            }

            //Space-station healing check.
            for(size_t i = 0; i < m_partitions[p].m_ships.size(); ++i)
            {
                enemy * a = m_partitions[p].m_ships[i];
                for(size_t j = 0; j < m_partitions[p].m_ships.size(); ++j)
                {
                    enemy * b = m_partitions[p].m_ships[j];

                    //If a is a station, and b the two ships are on the same team...
                    if( a->getClassification() == PLAYER_STATION and friendshipCheck( a->getTeam(), b->getTeam() ) and a != b )
                    {
                        float dist = magns( b->getPos() - a->getPos() );
                        if(dist < 160000.0f) b->incrHealth(0.01f);
                    }
                }

                if( a->getClassification() == PLAYER_STATION and friendshipCheck( a->getTeam(), TEAM_PLAYER ) )
                {
                    float dist = magns( m_ply.getPos() - a->getPos() );
                    if(dist < 160000.0f) m_ply.incrHealth(0.01f);
                }
            }
        }

        //COLLISIONS
        //It made sense at the time...
        for(auto &i : m_partitions[p].m_ships)
        {
            //Ships vs ships
            for(auto &j : m_partitions[p].m_ships)
            {
                if(emnityCheck(i->getTeam(), j->getTeam()) and i != j)
                {
                    resolveCollision(reinterpret_cast<ship *>(i), reinterpret_cast<ship *>(j));
                }
            }

            //Ships vs rocks
            for(auto &j : m_partitions[p].m_rocks)
            {
                if(i->getType() != SHIP_TYPE_MINER) resolveCollision(reinterpret_cast<ship *>(i), reinterpret_cast<ship *>(j));
            }

            //Ships vs player
            if(emnityCheck(i->getTeam(), TEAM_PLAYER))resolveCollision(reinterpret_cast<ship *>(i), reinterpret_cast<ship *>(&m_ply));
        }

        for(auto &i : m_partitions[p].m_rocks)
        {
            resolveCollision(reinterpret_cast<ship *>(i), reinterpret_cast<ship *>(&m_ply));
            for(auto &j : m_partitions[p].m_rocks)
            {
                if(i != j) resolveCollision(reinterpret_cast<ship *>(i), reinterpret_cast<ship *>(j));
            }
        }

        //RESOURCE PICKUPS
        //Get rich
        for(int i = m_partitions[p].m_resources.size() - 1; i >= 0; --i)
        {
            debris * resource = m_partitions[p].m_resources[i];
            for(int j = 0; j < m_partitions[p].m_ships.size(); ++j)
            {
                ship * agent = m_partitions[p].m_ships[j];
                if(circleIntersectCircle(tovec2(resource->getPos()), resource->getRadius(), tovec2(agent->getPos()), agent->getRadius()))
                {
                    if(!agent->addItem(*resource)) continue;

                    for(int k = m_resources.size() - 1; k >= 0; --k) if(&m_resources[k] == resource) swapnpop(&m_resources, k);
                    swapnpop(&m_partitions[p].m_resources, i);
                    break;
                }
                else
                {
                    vec3 add = agent->getPos() - resource->getPos();
                    add = add / (magns(add) * 200.0f);
                    resource->addVel(add);
                }
            }

            if(circleIntersectCircle(tovec2(resource->getPos()), resource->getRadius(), tovec2(m_ply.getPos()), m_ply.getRadius()))
            {
                if(!m_ply.addItem(*resource)) continue;

                m_sounds.playSnd(PLACE_SND);

                for(int k = m_resources.size() - 1; k >= 0; --k) if(&m_resources[k] == resource) swapnpop(&m_resources, k);
                swapnpop(&m_partitions[p].m_resources, i);
            }
            else
            {
                vec3 add = m_ply.getPos() - resource->getPos();
                add = add / (magns(add) * 200.0f);
                resource->addVel(add);
            }
        }
    }
}

void universe::resolveCollision(ship *_a, ship *_b)
{
    if(_a == _b or
            _a->getParent() == _b->getUniqueID() or
            _b->getParent() == _a->getUniqueID()) return;

    float dist = magns(_a->getPos() - _b->getPos());

    if(dist > sqr(_a->getRadius() + _b->getRadius())) return;

    dist = mag(_b->getPos() - _a->getPos());
    vec3 normal = (_b->getPos() - _a->getPos()) / dist;
    normal.m_z = 0.0f;

    //Spooky boolean math here we go
    float ainvmass = _a->getInertia() * _a->getCanMove();
    float binvmass = _b->getInertia() * _b->getCanMove();
    float cinvmass = ainvmass + binvmass;

    if(cinvmass == 0.0f) return;

    _a->damage(0.0f);
    _b->damage(0.0f);

    //Positional correction.
    float diff = (_a->getRadius() + _b->getRadius()) - dist;
    vec3 correction = diff * normal;
    correction.m_z = 0.0f;

    _a->addPos( -correction * (ainvmass / cinvmass) );
    _b->addPos( correction * (binvmass / cinvmass) );

    vec3 rv = _a->getVel() - _b->getVel();
    rv.m_z = 0.0f;
    float separation = dotProd(rv, normal);
    if(separation < 0.0f) return;

    float force = -1.6f * separation;
    force /= cinvmass;

    vec3 impulse = force * normal;
    impulse.m_z = 0.0f;

    float admg = fabs(force) * fmax(ainvmass, 0.05f);
    float bdmg = fabs(force) * fmax(binvmass, 0.05f);

    //addDamagePopup(admg, m_partitions[p].m_ships[s]->getTeam(), ep, ev);
    //addDamagePopup(bdmg, m_partitions[p].m_ships[s]->getTeam(), ep, ev);

    _a->damage(admg, _b->getVel());
    _b->damage(bdmg, _a->getVel());

    if(_a->getHealth() > 0.0f) _a->addVel( ainvmass * impulse );
    if(_b->getHealth() > 0.0f) _b->addVel( -binvmass * impulse );
}

void universe::addpfx(
        const vec3 _p,
        const vec3 _v,
        const int _no,
        const float _f
        )
{
    pfx pf(_p, _v, m_vel, _no, _f, "EXPLOSION");
    m_particles.push_back(pf);
}

ship * universe::closestEnemy(vec3 p, aiTeam t)
{
    float best_dist = F_MAX;
    ship * r = nullptr;
    for(size_t i = 0; i < m_agents.size(); i++)
    {
        if(!emnityCheck(m_agents[i].getTeam(), t)) continue;
        vec3 pe = m_agents[i].getPos();
        float dist = magns(pe-p);

        if(dist < best_dist)
        {
            best_dist = dist;
            r = &m_agents[i];
        }
    }
    return r;
}

void universe::addParticleSprite(
        const vec3 _p,
        const vec3 _v,
        const float _dim,
        const std::string _tex
        )
{
    float col;
    if( _tex == "SMOKE" ) col = randNum(200.0f, 220.0f);
    else if( _tex == "EXPLOSION" ) col = 255.0f;

    int w = 0, h = 0;
    stardust_sprite sm (_tex, col, _dim, _dim);
    sm.setPos(_p);
    sm.setWVel(m_vel);
    if( _tex == "SMOKE" ) sm.setVel(_v + tovec3(randVec2(1.0f)));
    else if( _tex == "EXPLOSION" ) sm.setVel(_v);
    sm.setZ(1.0f);
    m_passiveSprites.push_back(sm);
}

void universe::spawnShip(const aiTeam _t)
{
    int side = rand()%4;
    vec2 pass;

    if(side == 0) pass = {randNum(-20000.0f,20000.0f), -20000.0f};
    else if(side == 1) pass = {-20000.0f,randNum(-20000.0f, 20000.0f)};
    else if(side == 2) pass = {randNum(-20000.0f, 20000.0f), 20000.0f};
    else if(side == 3) pass = {20000.0f, randNum(-20000.0f, 20000.0f)};

    spawnShip(getRandomShipType(_t), _t, tovec3(pass));
}

ship_spec universe::getRandomShipType(const aiTeam _t)
{
    int prob = rand()%1000;

    ship_spec type;

    if( _t == SPACE_COMMUNISTS )
    {
        type = COMMUNIST_1;
        if(prob > 500 and prob <= 990) type = COMMUNIST_2;
        else if(prob > 990) type = COMMUNIST_CAPITAL;
    }
    else if( _t == SPOOKY_SPACE_PIRATES )
    {
        type = PIRATE_GNAT;
        if(prob > 750 and prob <= 800) type = PIRATE_CRUISER;
        else if(prob > 800 and prob <= 900) type = PIRATE_WRANGLER;
        else if(prob > 900 and prob <= 980) type = PIRATE_MARAUDER;
        else if(prob > 980 and prob <= 995) type = PIRATE_GUNSHIP;
        else if(prob > 995) type = PIRATE_CAPITAL;
    }
    else if( _t == GALACTIC_FEDERATION )
    {
        type = FEDERATION_MKI;
        if(prob > 750 and prob <= 800) type = FEDERATION_MKII;
        else if(prob > 800 and prob <= 900) type = FEDERATION_MKIII;
        else if(prob > 900 and prob <= 980) type = FEDERATION_MKIV;
        else if(prob > 980 and prob <= 995) type = FEDERATION_GUNSHIP;
        else if(prob > 995 and prob <= 999) type = FEDERATION_CAPITAL;
    }
    else if( _t == TEAM_PLAYER )
    {
        type = PLAYER_HUNTER;
        if(prob > 500 and prob <= 800) type = PLAYER_DEFENDER;
        if(prob > 800 and prob <= 999) type = PLAYER_DESTROYER;
        m_wingmenCount++;
    }
    else if( _t == ALLIANCE )
    {
        type = ALLIANCE_SCOUT;
        if(prob > 400 and prob <= 700) type = ALLIANCE_TRACKER;
        else if(prob > 700 and prob <= 800) type = ALLIANCE_PHOENIX;
        else if(prob > 800 and prob <= 900) type = ALLIANCE_DRAGON;
        else if(prob > 900 and prob <= 950) type = ALLIANCE_GUNSHIP;
        else if(prob > 950) type = ALLIANCE_TRADER;
    }

    return type;
}

void universe::spawnShip(
        const ship_spec _type,
        const aiTeam _t,
        const vec3 _p
        )
{
    enemy newShip(_p, -m_vel, _type, _t);

    if(_type == PLAYER_MINER_DROID) m_minerCount++;

    if( _t == TEAM_PLAYER ) m_factionCounts[TEAM_PLAYER]++;
    else if( _t == SPOOKY_SPACE_PIRATES or _t == GALACTIC_FEDERATION) m_factionCounts[GALACTIC_FEDERATION]++;
    else if(_t == SPACE_COMMUNISTS )  m_factionCounts[SPACE_COMMUNISTS]++;
    else if(_t == ALLIANCE) m_factionCounts[ALLIANCE]++;

    newShip.setWVel({0.0f, 0.0f, 0.0f});

    m_agents.push_back(newShip);

    //Do not assign squad if enemy cannot move and shoot.
    if(!newShip.getCanMove() or !newShip.getCanShoot() or newShip.getType() == SHIP_TYPE_MINER) return;

    std::vector<enemy> temp;
    //Turrets.
    if(_type == FEDERATION_CAPITAL)
    {
        for(int q = 0; q < 10; ++q)
        {
            enemy temp1({0.0f, 0.0f}, {0.0f, 0.0f}, FEDERATION_TURRET, GALACTIC_FEDERATION);
            enemy temp2({0.0f, 0.0f}, {0.0f, 0.0f}, FEDERATION_TURRET, GALACTIC_FEDERATION);

            shipAddParent(&m_agents.back(), &temp1, {200.0f, q * 200.0f - 960.0f, 0.0f});
            shipAddParent(&m_agents.back(), &temp2, {-200.0f, q * 200.0f - 960.0f, 0.0f});

            temp.push_back(temp1);
            temp.push_back(temp2);
        }
    }
    else if(_type == PIRATE_CAPITAL)
    {
        for(int p = 1; p < 3; ++p)
        {
            for(int pang = 0; pang < 360; pang += 60)
            {
                vec3 pos = {static_cast<float>(cos(rad(pang))) * p * 150.0f, static_cast<float>(sin(rad(pang))) * p * 150.0f, 0.0f};
                enemy temp1({0.0f, 0.0f}, {0.0f, 0.0f}, PIRATE_TURRET, SPOOKY_SPACE_PIRATES);

                shipAddParent(&m_agents.back(), &temp1, pos);

                temp.push_back(temp1);
            }
        }
    }
    else if(_type == COMMUNIST_CAPITAL)
    {
        vec3 base = {-184.0f, -604.0f, 0.0f};

        for(int i = 0; i < 3; ++i)
        {
            for(int j = 0; j < 4; ++j)
            {
                vec3 pos = {120.0f * i, 120.0f * j, 0.0f};
                pos += base;

                enemy temp1( {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, COMMUNIST_TURRET, SPACE_COMMUNISTS);

                shipAddParent(&m_agents.back(), &temp1, pos);

                temp.push_back(temp1);
            }
        }
    }
    else if(_type == PLAYER_CAPITAL)
    {
        vec3 base;
        for(int i = 0; i < 3; ++i)
        {
            base = {-164.0f, -424.0f};
            for(int j = 0; j < 3; ++j)
            {
                vec3 pos = {164.0f * i, 80.0f * j, 0.0f};
                pos += base;

                enemy temp1( {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, PLAYER_TURRET, TEAM_PLAYER);

                shipAddParent(&m_agents.back(), &temp1, pos);

                temp.push_back(temp1);
            }
        }

        for(int j = 1; j < 3; ++j)
        {
            base = {0.0f, 143.0f};
            for(int i = 0; i < 360; i += 60)
            {
                vec3 pos = {static_cast<float>(cos(rad(i))), static_cast<float>(sin(rad(i))), 0.0f};
                pos *= j * 100.0f;
                pos += base;

                enemy temp1( {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, PLAYER_TURRET, TEAM_PLAYER);

                shipAddParent(&m_agents.back(), &temp1, pos);

                temp.push_back(temp1);
            }
        }
    }

    bool quit = false;
    //Assign squad if enemy can move and shoot.
    for(auto &s : m_squads)
    {
        if(s.m_team == _t and s.m_size < s.m_max_size and rand() % 100 < 90)
        {
            addToSquad(&m_agents.back(), &s);
            quit = true;
        }
    }

    if(!quit)
    {
        //Create a new squad if no suitable one exists.
        squad tempSquad = createSquad(_t);
        tempSquad.m_id = m_squads.size();
        addToSquad(&m_agents.back(), &tempSquad);
        m_squads.push_back(tempSquad);
    }

    for(auto &q : temp) m_agents.push_back(q);
}

void universe::spawnSquad(
        const aiTeam _t,
        const float _min,
        const float _max,
        const int _i)
{
    vec3 p = tovec3(randVec2(_min, _max));
    for(int i = 0; i < _i; ++i)
    {
        spawnShip(getRandomShipType(_t), _t, p + tovec3(randVec2(512.0f)));
    }
}

bool universe::emnityCheck(
        aiTeam _a,
        aiTeam _b
        )
{
    return m_factions[_a].m_relations[_b] == DIPLOMACY_ENEMY and m_factions[_b].m_relations[_a] == DIPLOMACY_ENEMY;
}

bool universe::friendshipCheck(
        aiTeam _a,
        aiTeam _b
        )
{
    return m_factions[_a].m_relations[_b] == DIPLOMACY_FRIEND and m_factions[_b].m_relations[_a] == DIPLOMACY_FRIEND;
}

bool universe::neutralityCheck(
        aiTeam _a,
        aiTeam _b
        )
{
    return m_factions[_a].m_relations[_b] >= DIPLOMACY_NEUTRAL and m_factions[_b].m_relations[_a] >= DIPLOMACY_NEUTRAL;
}

void universe::reload(const bool _newGame)
{
    m_agents.clear();
    m_missiles.clear();
    m_shots.clear();
    m_asteroids.clear();
    m_passiveSprites.clear();
    m_particles.clear();

    m_vel = {0.0f, 0.0f};
    m_pos = {0.0f, 0.0f};
    m_ply.setHealth( m_ply.getMaxHealth() );
    m_ply.setShield( m_ply.getMaxShield() );
    m_ply.setEnergy( m_ply.getMaxEnergy() );
    m_ply.setVel({0, 0});

    m_ui.clear();
    initUI();
    m_ui.reset();
    m_paused = false;

    m_partitions.clear();

#if RENDER_MODE == 0
    m_partitions.rects.clear();
#endif

    m_time_elapsed = 0.0;
    setVel({0,0});

    m_factionCounts.assign(8, 0);
    m_factionMaxCounts.assign(8, 0);

    m_mouse_state = -1;

    m_escMenuShown = false;

    m_factionMaxCounts[GALACTIC_FEDERATION] = 1;
    m_factionMaxCounts[SPACE_COMMUNISTS] = 1;
    m_factionMaxCounts[ALLIANCE] = 5;
    m_maxMiners = 0;
    m_maxMiners = 0;
    m_minerCount = 0;
    m_wingmenCount = 0;

    m_ply.setWeapData(0,0);
    m_ply.setWeapData(1,1);
    m_ply.setWeapData(2,2);

    m_ply.setEnginePower(5.0f);
    m_ply.setGeneratorMul(1.0f);

    m_contextShip = nullptr;

    m_ply.getCargo()->getItems()->clear();

    ship newAsteroid({0.0f, 200.0f, 0.0f}, ASTEROID_MID, m_drawer.getTextureRadius(ASTEROID_MID));
    m_asteroids.push_back(newAsteroid);

    if(!_newGame)
    {
        return;
    }

    m_sounds.playSnd(SAVE_SND);
    for(int i = 0; i < UPGRADES_LEN; ++i)
    {
        m_ply.setGradeArr(i, 0);
    }

    m_ply.setMissiles(3);
    setScore(16);
}

void universe::addBuild(const ship_spec _type, const aiTeam _team)
{
    vec2 pass;

    int side = rand() % 4;
    if(side == 0) pass = {randNum(-20000.0f,20000.0f), -20000.0f};
    else if(side == 1) pass = {-20000.0f,randNum(-20000.0f, 20000.0f)};
    else if(side == 2) pass = {randNum(-20000.0f, 20000.0f), 20000.0f};
    else if(side == 3) pass = {20000.0f, randNum(-20000.0f, 20000.0f)};

    addBuild(tovec3(pass), _type, _team);
}

void universe::addBuild(
        const vec3 _p,
        const ship_spec _type,
        const aiTeam _team
        )
{
    enemy newShip(_p, {0.0f, 0.0f}, _type, _team);

    switch(_type)
    {
    case PLAYER_STATION:
        m_factionMaxCounts[GALACTIC_FEDERATION] += 25;
        m_factionMaxCounts[SPACE_COMMUNISTS] += 1;
        m_factionMaxCounts[ALLIANCE] += 25;
        break;
    case PLAYER_GRAVWELL:
        m_factionMaxCounts[GALACTIC_FEDERATION] += 5;
        m_factionMaxCounts[SPACE_COMMUNISTS] += 1;
        m_factionMaxCounts[ALLIANCE] += 2;
        break;
    case PLAYER_TURRET:
    {
        m_factionMaxCounts[GALACTIC_FEDERATION] += 1;
        m_factionMaxCounts[SPACE_COMMUNISTS] += 1;

        //Attach to a player capital ship, if any is close enough.
        for(auto &i : m_agents)
        {
            if(i.getClassification() == PLAYER_CAPITAL and magns(_p - i.getPos()) < sqr(i.getRadius()))
            {
                vec3 pos = _p - i.getPos();
                float angle = -i.getAng();
                float s = sin(rad(angle));
                float c = cos(rad(angle));

                float xn = pos.m_x * c - pos.m_y * s;
                float yn = pos.m_x * s + pos.m_y * c;
                vec3 comb = {xn, yn, 0.0f};

                shipAddParent(&i, &newShip, comb);
            }
        }
        break;
    }
    case PLAYER_BARRACKS:
        m_factionMaxCounts[GALACTIC_FEDERATION] += 20;
        m_factionMaxCounts[SPACE_COMMUNISTS] += 1;
        m_factionMaxCounts[ALLIANCE] += 2;
        break;
    case PLAYER_CAPITAL:
        m_factionMaxCounts[GALACTIC_FEDERATION] += 15;
        m_factionMaxCounts[SPACE_COMMUNISTS] += 1;
        break;
    default:
        break;
    }

    m_agents.push_back(newShip);

    addpfx(_p, {0.0f, 0.0f, 0.0f}, rand()%20 + 50, rand()%30 + 2);
    for(int q = 0; q < 50; ++q) addParticleSprite(_p, tovec3(randVec2(1.0f)), 128.0f, "SMOKE");
    m_sounds.playSnd(PLACE_SND);
    m_sounds.playSnd(CLUNK_SND);
}

void universe::initUI()
{
    //Initialise the two selection menus.
    selection energy_menu = loadSelection("priorityMenu.txt");
    selection upgrades_menu = loadSelection("upgradesMenu.txt");
    selection buy_menu = loadSelection("buyMenu.txt");

    selection info_menu;
    info_menu.setVisible(false);
    info_menu.setInWorldSpace(true);
    button name("", {0,0,0,0}, {255,255,255,255}, {-32.0f, 0.0f}, {128.0f,32.0f});
    button kills("KILLS: ", {0,0,0,0}, {255,255,255,255}, {-32.0f, 80.0f}, {128.0f,32.0f});
    button distance("DISTANCE: ", {0,0,0,0}, {255,255,255,255}, {-32.0f, 96.0f}, {128.0f,32.0f});
    button trade("TRADE", {100,100,100,255}, {255,255,255,255}, {0.0f, 128.0f}, {64.0f,32.0f});
    button close("CLOSE", {255,10,20,255}, {255,255,255,255}, {64.0f, 128.0f}, {64.0f,32.0f});
    info_menu.add(name);
    info_menu.add(kills);
    info_menu.add(distance);
    info_menu.add(trade);
    info_menu.add(close);

    m_ui.add(energy_menu);
    m_ui.add(upgrades_menu);
    m_ui.add(buy_menu);
    m_ui.add(info_menu);
}

bool universe::upgradeCallback(
        int _sel,
        int _btn
        )
{
    //This function takes the selected button, looks at the cost vs the score, updates relevant values,
    //then returns a bool representing whether the upgrade was successful or unsuccessful.

    button * selectedbutton = &m_ui.getElements()->at(_sel).getButtons()->at(_btn);

    //The current upgrade level, -1 if it is a buy attempt.
    int lvl;
    if(_sel == 1) lvl = m_ply.getUpgrade( _btn );
    else if(_sel == 2) lvl = -1;

    selectedbutton->set(false);

    if(selectedbutton->getCost() > m_factions[TEAM_PLAYER].m_wealth or lvl > 9)
    {
        m_sounds.playSnd(MENU_FAIL_SND);
        return false;
    }

    if(lvl < 9)
    {
        addScore( -selectedbutton->getCost() );
        if(_btn < 4 and _sel == 1) selectedbutton->setCost(selectedbutton->getCost() * 2);
        else return true;
    }

    m_ply.upgrade(_btn);
    upgradeSetLabels(_sel, _btn, -1);

    return true;
}

void universe::upgradeSetLabels(
        int _sel,
        int _btn,
        int _plvl
        )
{
    if(_sel == 2) return;
    button * selectedbutton = &m_ui.getElements()->at(_sel).getButtons()->at(_btn);

    std::string s1;
    int lvl;
    if(_plvl < 0) lvl = m_ply.getUpgrade( _btn );
    else lvl = _plvl;

    switch(_btn)
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
    default:
        return;
        break;
    }
    s1 += g_ROMAN_NUMS[clamp(lvl, 0, 9)];

    if(lvl < 8)
    {
        selectedbutton->setCost(pow(2.0, lvl + 1) * 2);
        s1 += " (";
        std::stringstream ss;
        ss << selectedbutton->getCost();
        s1 += ss.str();
        s1 += ")";
    }

    selectedbutton->updateText(s1);

    addScore(0);

    std::cout << "UPDATING... " << s1 << "(" << lvl << ")" << std::endl;
}

//This function loads all the ships in the game into a vector that we can copy from later.
void universe::loadShips()
{
    for(int i = 0; i < static_cast<int>(SHIPS_END); ++i)
    {
        ship insert( {0.0f, 0.0f}, static_cast<ship_spec>(i), m_drawer.getTextureRadius(static_cast<ship_spec>(i)) );
        g_ship_templates.push_back(insert);
    }
    std::cout << "No of ship types: " << g_ship_templates.size() << std::endl;
}

void universe::createFactions()
{
    faction player;
    player.m_team = TEAM_PLAYER;
    player.m_colour = {0, 255, 0, 255};
    player.m_relations = {DIPLOMACY_FRIEND, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_ENEMY};
    player.m_wealth = 16;
    m_factions.push_back(player);

    faction galactic_fed;
    galactic_fed.m_team = GALACTIC_FEDERATION;
    galactic_fed.m_colour = {165, 14, 226, 255};
    galactic_fed.m_relations = {DIPLOMACY_ENEMY, DIPLOMACY_FRIEND, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_NEUTRAL, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY};
    galactic_fed.m_wealth = 36000;
    m_factions.push_back(galactic_fed);

    faction spooky_pirates;
    spooky_pirates.m_team = SPOOKY_SPACE_PIRATES;
    spooky_pirates.m_colour = {240, 211, 10, 255};
    spooky_pirates.m_relations = {DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_FRIEND, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY};
    spooky_pirates.m_wealth = 5000;
    m_factions.push_back(spooky_pirates);

    faction space_communists;
    space_communists.m_team = SPACE_COMMUNISTS;
    space_communists.m_colour = {255, 0, 0, 255};
    space_communists.m_relations = {DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_FRIEND, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY};
    space_communists.m_wealth = 20000;
    m_factions.push_back(space_communists);

    faction alliance;
    alliance.m_team = ALLIANCE;
    alliance.m_colour = {0, 255, 255, 255};
    alliance.m_relations = {DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_FRIEND, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY};
    alliance.m_wealth = 30000;
    m_factions.push_back(alliance);

    faction neutral;
    neutral.m_team = NEUTRAL;
    neutral.m_colour = {200, 200, 200, 255};
    neutral.m_relations = {DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL, DIPLOMACY_NEUTRAL};
    neutral.m_wealth = 0;
    m_factions.push_back(neutral);

    faction none;
    none.m_team = NONE;
    none.m_colour = {0, 0, 0, 255};
    none.m_relations = {DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY, DIPLOMACY_ENEMY};
    none.m_wealth = 0;
    m_factions.push_back(none);
}

squad * universe::getSquadFromID(int _id)
{
    for(auto &s : m_squads)
    {
        if(s.m_id == _id)
        {
            return &s;
        }
    }
    return nullptr;
}

void universe::shipAddParent(ship * _parent, ship * _child, vec3 _offset)
{
    _child->setParent(_parent->getUniqueID());
    _child->setParentOffset(_offset);
}

void universe::addPopup(std::string _label, popup_type _type, float _smul, vec3 _pos, vec3 _vel)
{
    popup temp (_label, _pos, randVec3(2.0f), _smul, g_popupCols[_type]);
    m_popups.push_back(temp);
}

void universe::addDamagePopup(int _dmg, aiTeam _team, vec3 _pos, vec3 _vel)
{
    popup_type p = POPUP_GOOD;
    if(friendshipCheck(_team, TEAM_PLAYER)) p = POPUP_BAD;
    else if(neutralityCheck(_team, TEAM_PLAYER)) p = POPUP_NEUTRAL;

    addPopup(std::to_string(_dmg), p, 2.0f, _pos, _vel);
}

void universe::calcPowerBalance()
{
    m_balanceOfPower.assign(m_factions.size(), 0.0f);
    for(auto &i : m_agents)
    {
        m_balanceOfPower[i.getTeam()] += i.getHealth() + i.getShield();
    }
    float total = 0.0f;
    for(auto &i : m_balanceOfPower) total += i;
    for(auto &i : m_balanceOfPower) i /= total;
}

selectionReturn universe::handleInput(vec2 _mouse)
{
    selectionReturn ret = m_ui.handleInput(_mouse);
    if(ret.m_sel_val > -1) m_sounds.playSnd(MENU_SELECT_SND);
    else
    {
        _mouse = toWorldSpace(_mouse);
        m_contextShip = nullptr;
        for(auto &i : m_agents)
        {
            if(magns(_mouse - tovec2(i.getPos())) < sqr(i.getRadius() + 32.0f))
            {
                m_contextShip = &i;
                break;
            }
        }
        //std::cout << "END FUNCTION" << m_contextShip << ", " << (m_contextShip == nullptr) << '\n';
    }
    return ret;
}

ship * universe::getByID(const unsigned long _i)
{
    for(auto &i : m_agents)
    {
        if(i.getUniqueID() == _i)
            return &i;
    }
    return nullptr;
}

void universe::addFrag(const unsigned long _i)
{
    ship * killer = getByID(_i);
    if(killer != nullptr) killer->addKill();
}
