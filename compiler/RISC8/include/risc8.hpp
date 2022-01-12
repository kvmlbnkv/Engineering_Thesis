#ifndef INC_RISC8_CPP_RISC8_HPP
#define INC_RISC8_CPP_RISC8_HPP

#include "assembly.hpp"

struct risc8 : ASMLine
{
  enum class OpCode {
    unknown,

    HLT,
    NOP,
    ADL,
    ADH,
    AND,
    OR,
    XOR,
    NOT,
    CLC,
    SEC,
    SHL,
    SHR,
    JMP,
    JNE,
    LDA,
    STA,
    MAC,
    MBA,
    MAM,
    MMA,
    MAP,
    IXR,
    IXN,
    LIX,
    LIL_0,
    LIL_1,
    LIL_2,
    LIL_3,
    LIL_4,
    LIL_5,
    LIL_6,
    LIL_7,
    LIL_8,
    LIL_9,
    LIL_A,
    LIL_B,
    LIL_C,
    LIL_D,
    LIL_E,
    LIL_F,
    LIH_0,
    LIH_1,
    LIH_2,
    LIH_3,
    LIH_4,
    LIH_5,
    LIH_6,
    LIH_7,
    LIH_8,
    LIH_9,
    LIH_A,
    LIH_B,
    LIH_C,
    LIH_D,
    LIH_E,
    LIH_F,
  };

    static bool get_is_branch(const OpCode o)
  {
    switch (o) {
    case OpCode::HLT:
    case OpCode::NOP:
    case OpCode::ADL:
    case OpCode::ADH:
    case OpCode::AND:
    case OpCode::OR:
    case OpCode::XOR:
    case OpCode::NOT:
    case OpCode::CLC:
    case OpCode::SEC:
    case OpCode::SHL:
    case OpCode::SHR:
    case OpCode::JMP:
    case OpCode::JNE:
    case OpCode::LDA:
    case OpCode::STA:
    case OpCode::MAC:
    case OpCode::MBA:
    case OpCode::MAM:
    case OpCode::MMA:
    case OpCode::MAP:
    case OpCode::IXR:
    case OpCode::IXN:
    case OpCode::LIX:
    case OpCode::LIL_0:
    case OpCode::LIL_1:
    case OpCode::LIL_2:
    case OpCode::LIL_3:
    case OpCode::LIL_4:
    case OpCode::LIL_5:
    case OpCode::LIL_6:
    case OpCode::LIL_7:
    case OpCode::LIL_8:
    case OpCode::LIL_9:
    case OpCode::LIL_A:
    case OpCode::LIL_B:
    case OpCode::LIL_C:
    case OpCode::LIL_D:
    case OpCode::LIL_E:
    case OpCode::LIL_F:
    case OpCode::LIH_0:
    case OpCode::LIH_1:
    case OpCode::LIH_2:
    case OpCode::LIH_3:
    case OpCode::LIH_4:
    case OpCode::LIH_5:
    case OpCode::LIH_6:
    case OpCode::LIH_7:
    case OpCode::LIH_8:
    case OpCode::LIH_9:
    case OpCode::LIH_A:
    case OpCode::LIH_B:
    case OpCode::LIH_C:
    case OpCode::LIH_D:
    case OpCode::LIH_E:
    case OpCode::LIH_F:
    case OpCode::unknown:
      break;
    }
    return false;
  }

  static bool get_is_comparison(const OpCode o)
  {
    switch (o) {
    case OpCode::HLT:
    case OpCode::NOP:
    case OpCode::ADL:
    case OpCode::ADH:
    case OpCode::AND:
    case OpCode::OR:
    case OpCode::XOR:
    case OpCode::NOT:
    case OpCode::CLC:
    case OpCode::SEC:
    case OpCode::SHL:
    case OpCode::SHR:
    case OpCode::JMP:
    case OpCode::JNE:
    case OpCode::LDA:
    case OpCode::STA:
    case OpCode::MAC:
    case OpCode::MBA:
    case OpCode::MAM:
    case OpCode::MMA:
    case OpCode::MAP:
    case OpCode::IXR:
    case OpCode::IXN:
    case OpCode::LIX:
    case OpCode::LIL_0:
    case OpCode::LIL_1:
    case OpCode::LIL_2:
    case OpCode::LIL_3:
    case OpCode::LIL_4:
    case OpCode::LIL_5:
    case OpCode::LIL_6:
    case OpCode::LIL_7:
    case OpCode::LIL_8:
    case OpCode::LIL_9:
    case OpCode::LIL_A:
    case OpCode::LIL_B:
    case OpCode::LIL_C:
    case OpCode::LIL_D:
    case OpCode::LIL_E:
    case OpCode::LIL_F:
    case OpCode::LIH_0:
    case OpCode::LIH_1:
    case OpCode::LIH_2:
    case OpCode::LIH_3:
    case OpCode::LIH_4:
    case OpCode::LIH_5:
    case OpCode::LIH_6:
    case OpCode::LIH_7:
    case OpCode::LIH_8:
    case OpCode::LIH_9:
    case OpCode::LIH_A:
    case OpCode::LIH_B:
    case OpCode::LIH_C:
    case OpCode::LIH_D:
    case OpCode::LIH_E:
    case OpCode::LIH_F:
    case OpCode::unknown:
      break;
    }
    return false;
  }

  explicit risc8(const OpCode o)
    : ASMLine(Type::Instruction, std::string{ to_string(o) }), opcode(o), is_branch(get_is_branch(o)), is_comparison(get_is_comparison(o))
  {
  }

  risc8(const Type t, std::string s)
    : ASMLine(t, std::move(s))
  {
  }


  risc8(const OpCode o, Operand t_o)
    : ASMLine(Type::Instruction, std::string{ to_string(o) }), opcode(o), op(std::move(t_o)), is_branch(get_is_branch(o)), is_comparison(get_is_comparison(o))
  {
  }

  constexpr static std::string_view to_string(const OpCode o)
  {
    switch (o) {
    case OpCode::HLT: return "HLT";
    case OpCode::NOP: return "NOP";
    case OpCode::ADL: return "ADL";
    case OpCode::ADH: return "ADH";
    case OpCode::AND: return "AND";
    case OpCode::OR: return "OR";
    case OpCode::XOR: return "XOR";
    case OpCode::NOT: return "NOT";
    case OpCode::CLC: return "CLC";
    case OpCode::SEC: return "SEC";
    case OpCode::SHL: return "SHL";
    case OpCode::SHR: return "SHR";
    case OpCode::JMP: return "JMP";
    case OpCode::JNE: return "JNE";
    case OpCode::LDA: return "LDA";
    case OpCode::STA: return "STA";
    case OpCode::MAC: return "MAC";
    case OpCode::MBA: return "MBA";
    case OpCode::MAM: return "MAM";
    case OpCode::MAP: return "MAP";
    case OpCode::MMA: return "MMA";
    case OpCode::IXR: return "IXR";
    case OpCode::IXN: return "IXN";
    case OpCode::LIX: return "LIX";
    case OpCode::LIL_0: return "LIL 0x0";
    case OpCode::LIL_1: return "LIL 0x1";
    case OpCode::LIL_2: return "LIL 0x2";
    case OpCode::LIL_3: return "LIL 0x3";
    case OpCode::LIL_4: return "LIL 0x4";
    case OpCode::LIL_5: return "LIL 0x5";
    case OpCode::LIL_6: return "LIL 0x6";
    case OpCode::LIL_7: return "LIL 0x7";
    case OpCode::LIL_8: return "LIL 0x8";
    case OpCode::LIL_9: return "LIL 0x9";
    case OpCode::LIL_A: return "LIL 0xA";
    case OpCode::LIL_B: return "LIL 0xB";
    case OpCode::LIL_C: return "LIL 0xC";
    case OpCode::LIL_D: return "LIL 0xD";
    case OpCode::LIL_E: return "LIL 0xE";
    case OpCode::LIL_F: return "LIL 0xF";
    case OpCode::LIH_0: return "LIH 0x0";
    case OpCode::LIH_1: return "LIH 0x1";
    case OpCode::LIH_2: return "LIH 0x2";
    case OpCode::LIH_3: return "LIH 0x3";
    case OpCode::LIH_4: return "LIH 0x4";
    case OpCode::LIH_5: return "LIH 0x5";
    case OpCode::LIH_6: return "LIH 0x6";
    case OpCode::LIH_7: return "LIH 0x7";
    case OpCode::LIH_8: return "LIH 0x8";
    case OpCode::LIH_9: return "LIH 0x9";
    case OpCode::LIH_A: return "LIH 0xA";
    case OpCode::LIH_B: return "LIH 0xB";
    case OpCode::LIH_C: return "LIH 0xC";
    case OpCode::LIH_D: return "LIH 0xD";
    case OpCode::LIH_E: return "LIH 0xE";
    case OpCode::LIH_F: return "LIH 0xF";
    case OpCode::unknown: return "";
    }

    return "";
  }

  [[nodiscard]] std::string to_string() const
  {
    switch (type) {
    case ASMLine::Type::Label:
      return text;// + ':';
    case ASMLine::Type::Directive:
    case ASMLine::Type::Instruction: {
      return fmt::format("\t{} {:15}\t; {}", text, op.value, comment);
    }
    }
    throw std::runtime_error("Unable to render: " + text);
  }


  OpCode      opcode = OpCode::unknown;
  Operand     op;
  std::string comment;
  bool        is_branch     = false;
  bool        is_comparison = false;
};

#endif//INC_RISC8_CPP_RISC8_HPP
