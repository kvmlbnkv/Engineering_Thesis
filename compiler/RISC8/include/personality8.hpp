#ifndef INC_RISC8_CPP_PERSONALITY_HPP
#define INC_RISC8_CPP_PERSONALITY_HPP

#include <vector>
#include "risc8.hpp"

class Personality8
{
public:
  virtual void                  insert_autostart_sequence(std::vector<risc8> &new_instructions) const = 0;
  [[nodiscard]] virtual Operand get_register(const int reg_num) const                                   = 0;

  virtual ~Personality8()           = default;
  Personality8(const Personality8 &) = delete;
  Personality8(Personality8 &&)      = delete;
  Personality8 &operator=(const Personality8 &) = delete;
  Personality8 &operator=(Personality8 &&) = delete;

protected:
  Personality8() = default;
};

#endif//INC_RISC8_CPP_PERSONALITY_HPP
