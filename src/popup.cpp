#include "popup.hpp"

std::array< std::array<float, 4>, 3> g_popupCols = {{
    {0.0f, 255.0f, 0.0f, 255.0f},
    {255.0f, 0.0f, 0.0f, 255.0f},
    {200.0f, 200.0f, 200.0f, 255.0f}
}};

popup::popup(const std::string _label, vec3 _pos, vec3 _vel, float _smul, std::array<float, 4> _col)
{
    setPos(_pos);
    setPPos(_pos);
    setVel(_vel);

    m_label    = _label;
    m_smul = _smul;
    m_col = _col;
}

void popup::update(float _dt)
{
    m_col[3] -= 250.0f * _dt;
    updatePos(_dt);
}
