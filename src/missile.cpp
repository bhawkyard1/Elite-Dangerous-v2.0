#include "missile.hpp"

missile::missile(vec2 p, float _r): ship(p, ION_MISSILE_MKI, _r)
{
  setPos(p);
  setMaxHealth(10.0f, true);
  setMaxShield(5.0f, true);
  setMaxEnergy(10000.0f, true);
  setTAng(0.0f);
  det = false;
}

void missile::steering()
{
  if(m_target != nullptr)
  {
    vec2 m_tPos = m_target->getPos();

    float dist = mag(m_tPos - getPos());

    float cSpd = mag(getVel() - m_target->getVel());

    float frames = dist/cSpd;

    vec2 tPPos = m_target->getPos() + m_target->getVel() * frames;
    float tPPDist = mag(tPPos - getPos());

    float vecMul = dotProd1(unit(getVel()),computeVector(getTAng() + 90));

    float stoppingDistance = (cSpd*cSpd)/2;
    if(vecMul < 0) stoppingDistance *= -1;

    float accelMul = clamp(((tPPDist-stoppingDistance)),-3.0f,1.5f);

    float angleMul = (shortestAngle(getAng(), getTAng())+90)/90.0;

    setTAng(clampRoll(computeAngle(getPos() - m_tPos),-180.0f,180.0f));

    if(fabs(shortestAngle(getAng(), getTAng())) <= 2.0f)
    {
      accelerate(accelMul*angleMul);
    }

    if(magns(getPos() - m_target->getPos()) < 50*50*cSpd)
    {
      det = true;
    }
  }
  else
  {
    setTAng(getAng());
    //accelerate(1);
  }
}
