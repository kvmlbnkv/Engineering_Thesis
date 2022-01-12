#ifndef INC_RISC8_C_R8_HPP
#define INC_RISC8_C_R8_HPP

#include "../personality8.hpp"

struct R8 : Personality8
{

  void insert_autostart_sequence(std::vector<risc8> &new_instructions) const override
  { 
    new_instructions.emplace_back(ASMLine::Type::Directive, "GOTO main\n.aznak:\n.asgn:\nDB 0\n.aexp:\nDB 0\n.aman:\nDB 0\n.aman1:\nDB 0\n.aman2:\nDB 0\n.bsgn:\nDB 0\n.bexp:\nDB 0\n.bman:\nDB 0\n.bman1:\nDB 0\n.bman2:\nDB 0\n.csgn:\nDB 0\n.cexp:\nDB 0\n.cman:\nDB 0\n.cman1:\nDB 0\n.cman2:\nDB 0\n.mulaman_3:\nDB 0\n.mulaman_2:\nDB 0\n.mulaman_1:\nDB 0\n.mulman:\nDB 0\n.mulman1:\nDB 0\n.mulman2:\nDB 0\n.mulman3:\nDB 0\n.mulman4:\nDB 0\n.mulman5:\nDB 0\n.compare_diff:\nDB 0\n.compare_b:\nDB 0\n.exp_diff:\nDB 0\n.shift_carry:\nDB 0\n.mul_exp_check:\nDB 0\n.rcall_addr:\nDB 0\nDBB");
  
  }

  [[nodiscard]] Operand get_register(const int reg_num) const override
  {
    switch (reg_num) {
      //  http://sta.c64.org/cbm64mem.html
    case 0: return Operand(Operand::Type::literal, "79");// unused, int->fp routine pointer
    case 1: return Operand(Operand::Type::literal, "78");
    case 2: return Operand(Operand::Type::literal, "77");// unused
    case 3: return Operand(Operand::Type::literal, "76");// unused
    case 4: return Operand(Operand::Type::literal, "75");// bit buffer for rs232
    case 5: return Operand(Operand::Type::literal, "74");// counter for rs232
    case 6: return Operand(Operand::Type::literal, "73");// unused
    case 7: return Operand(Operand::Type::literal, "72");// unused
    case 8: return Operand(Operand::Type::literal, "71");// unused
    case 9: return Operand(Operand::Type::literal, "70");// unused
    case 10: return Operand(Operand::Type::literal, "69");// Current BASIC line number
    case 11: return Operand(Operand::Type::literal, "68");// Current BASIC line number
    case 12: return Operand(Operand::Type::literal, "67");// arithmetic register #3
    case 13: return Operand(Operand::Type::literal, "66");
    case 14: return Operand(Operand::Type::literal, "65");
    case 15: return Operand(Operand::Type::literal, "64");
    case 16: return Operand(Operand::Type::literal, "63");
    case 17: return Operand(Operand::Type::literal, "62");
    case 18: return Operand(Operand::Type::literal, "61");
    case 19: return Operand(Operand::Type::literal, "60");
    case 20: return Operand(Operand::Type::literal, "59");
    case 21: return Operand(Operand::Type::literal, "58");
    case 22: return Operand(Operand::Type::literal, "57");
    case 23: return Operand(Operand::Type::literal, "56");
    case 24: return Operand(Operand::Type::literal, "55");
    case 25: return Operand(Operand::Type::literal, "54");
    case 26: return Operand(Operand::Type::literal, "53");
    case 27: return Operand(Operand::Type::literal, "52");
    case 28: return Operand(Operand::Type::literal, "51");
    case 29: return Operand(Operand::Type::literal, "50");
    case 30: return Operand(Operand::Type::literal, "49");
    case 31: return Operand(Operand::Type::literal, "48");
    }
    throw std::runtime_error("Unhandled register number: " + std::to_string(reg_num));
  }
};

#endif// INC_RISC8_C_R8_HPP
