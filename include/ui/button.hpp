#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <array>
#include <string>
#include "vectors.hpp"

namespace ui
{

class button
{
  bool m_selected = false;
  std::array<int, 8> m_col;
  std::array<int, 8> m_tcol;
  //Variable to track actual current colour.
  std::array<float, 4> m_dcol;
  std::string m_initLabel;
  std::string m_label;
  int m_initCost;
  int m_cost;
  bool m_dark;
  vec2 m_pos;
  vec2 m_dim;
  float m_smul;
public:
  button(const std::string _label,const std::array<int, 8> _pcol,const std::array<int, 8> _tcol,const vec2 _pos,const vec2 _dim);
  button(const std::string _label,const std::array<int, 8> _pcol,const std::array<int, 8> _tcol,const vec2 _pos,const vec2 _dim, const float _smul);
  button(std::string,std::array<int, 8>,std::array<int, 8>,vec2,vec2,int);
  void select();
  bool isSelected() {return m_selected;}
  void updateText(std::string);
  void set(bool s) {m_selected = s;}
  bool on() {return m_selected;}
  void update(int _s);

  int getCost() {return m_cost;}
  void setCost(int pcost) {m_cost = pcost;}

  void setDark(bool b) {m_dark = b;}
  bool isDark() {return m_dark;}

  vec2 getPos() {return m_pos;}
  vec2 getDim() {return m_dim;}

  std::array<float, 4> getDrawCol() {return m_dcol;}
  std::array<int, 8> getCol() {return m_col;}
  std::array<int, 8> getTCol() {return m_tcol;}

  std::string getLabel() {return m_label;}

  void reset();

  float getTextSizeMul() {return m_smul;}
};

}
#endif
