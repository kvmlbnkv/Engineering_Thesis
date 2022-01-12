#include <cassert>
#include <cctype>
#include <charconv>
#include <ctre.hpp>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "include/risc8.hpp"
#include "include/personalities/r8.hpp"
#include "include/6502.hpp"
#include "include/assembly.hpp"
#include "include/lib1funcs.hpp"
#include "include/optimizer.hpp"
#include "include/personalities/c64.hpp"
#include "include/personalities/x16.hpp"
#include "include/risc8funcs.hpp"


int to_int(const std::string_view sv)
{
  int result{};
  std::from_chars(sv.begin(), sv.end(), result);
  return result;
}

int parse_8bit_literal(const std::string_view s) { return to_int(s.substr(1)); }

std::string_view strip_lo_hi(std::string_view s)
{
  const auto matcher = ctre::match<R"(^(lo|hi)8\((.*)\)$)">;

  if (const auto results = matcher(s); results) { return results.get<2>(); }

  return s;
}

std::string_view strip_negate(std::string_view s)
{
  const auto matcher = ctre::match<R"(^-\((.*)\)$)">;

  if (const auto results = matcher(s); results) { return results.get<1>(); }

  return s;
}

std::string_view strip_gs(std::string_view s)
{
  const auto matcher = ctre::match<R"(gs\((.*)\))">;

  if (const auto results = matcher(s); results) { return results.get<1>(); }

  return s;
}

std::string_view strip_offset(std::string_view s)
{
  const auto matcher = ctre::match<R"(^(.*)(\+|-)[0-9]+$)">;

  if (const auto results = matcher(s); results) { return results.get<1>(); }

  return s;
}


std::string fixup_8bit_literal(const std::string &s)
{
  if (s[0] == '$') { return std::to_string(static_cast<uint8_t>(parse_8bit_literal(s))); }

  if (s.starts_with("0x")) { return s.substr(2); }

  if (s.starts_with("lo8(")) { return fmt::format("{}",strip_gs(strip_lo_hi(s))); }
  if (s.starts_with("hi8(")) { return fmt::format("{}", strip_gs(strip_lo_hi(s))); }

  const auto is_num = std::all_of(begin(s), end(s), [](const auto c) { return (c >= '0' && c <= '9') || c == '-'; });

  if (is_num) { return s; }

  return s;
}

std::string fixup_8bit_literal_subi(const std::string &s)
{
  if (s[0] == '$') { return std::to_string(static_cast<uint8_t>(parse_8bit_literal(s))); }
  if (s.starts_with("0x")) { return s.substr(2); }

  if (s.starts_with("lo8(-(-")) {
	  if (s.size() == 11) {
		return fmt::format("{}",s.substr(7,2));
	  }
	  return fmt::format("{}",s.substr(7,1)); 
  }
  if (s.starts_with("lo8(-(")) { 
	  if (s.size() == 10) {
		return fmt::format("-{}",s.substr(6,2));
	  }
	  return fmt::format("-{}",s.substr(6,1)); 
  }
  if (s.starts_with("lo8(")) { return fmt::format("{}",strip_gs(strip_lo_hi(s))); }
  if (s.starts_with("hi8(")) { return fmt::format("{}", strip_gs(strip_lo_hi(s))); }

  const auto is_num = std::all_of(begin(s), end(s), [](const auto c) { return (c >= '0' && c <= '9') || c == '-'; });

  if (is_num) { return s; }

  return s;
}


struct AVR : ASMLine
{
  enum class OpCode {
    unknown,

    adc,
    adiw,
    add,
    andi,
    asr,

    breq,
    brge,
    brlt,
    brlo,
    brne,
    brsh,

    call,
    clr,
    com,
    cp,
    cpc,
    cpi,
    cpse,
    dec,

    eor,

    in,
    inc,
    icall,

    jmp,

    ld,
    ldd,
    ldi,
    lds,
    lsl,
    lsr,

    mov,

    nop,

    OR,
    ori,
    out,

    pop,
    push,

    rcall,
    ret,
    rjmp,
    rol,
    ror,

    sbc,
    sbci,
    sbiw,
    sbrc,
    sbrs,
    st,
    std,
    sts,
    sub,
    subi,
    swap,

    tst,
  };

  [[nodiscard]] static constexpr OpCode parse_opcode(Type t, std::string_view o)
  {
    switch (t) {
    case Type::Label:
    case Type::Directive: return OpCode::unknown;
    case Type::Instruction: {
      if (o == "ldi") { return OpCode::ldi; }
      if (o == "sts") { return OpCode::sts; }
      if (o == "ret") { return OpCode::ret; }
      if (o == "mov") { return OpCode::mov; }
      if (o == "lsl") { return OpCode::lsl; }
      if (o == "rol") { return OpCode::rol; }
      if (o == "ror") { return OpCode::ror; }
      if (o == "rcall") { return OpCode::rcall; }
      if (o == "icall") { return OpCode::icall; }
      if (o == "call") { return OpCode::call; }
      if (o == "ld") { return OpCode::ld; }
      if (o == "sub") { return OpCode::sub; }
      if (o == "subi") { return OpCode::subi; }
      if (o == "sbc") { return OpCode::sbc; }
      if (o == "sbci") { return OpCode::sbci; }
      if (o == "st") { return OpCode::st; }
      if (o == "std") { return OpCode::std; }
      if (o == "ldd") { return OpCode::ldd; }
      if (o == "lds") { return OpCode::lds; }
      if (o == "lsr") { return OpCode::lsr; }
      if (o == "andi") { return OpCode::andi; }
      if (o == "asr") { return OpCode::asr; }
      if (o == "eor") { return OpCode::eor; }
      if (o == "sbrc") { return OpCode::sbrc; }
      if (o == "rjmp") { return OpCode::rjmp; }
      if (o == "sbrs") { return OpCode::sbrs; }
      if (o == "brne") { return OpCode::brne; }
      if (o == "dec") { return OpCode::dec; }
      if (o == "adiw") { return OpCode::adiw; }
      if (o == "sbiw") { return OpCode::sbiw; }
      if (o == "push") { return OpCode::push; }
      if (o == "pop") { return OpCode::pop; }
      if (o == "com") { return OpCode::com; }
      if (o == "swap") { return OpCode::swap; }
      if (o == "clr") { return OpCode::clr; }
      if (o == "cpse") { return OpCode::cpse; }
      if (o == "cpi") { return OpCode::cpi; }
      if (o == "brlo") { return OpCode::brlo; }
      if (o == "add") { return OpCode::add; }
      if (o == "adc") { return OpCode::adc; }
      if (o == "cpc") { return OpCode::cpc; }
      if (o == "cp") { return OpCode::cp; }
      if (o == "brsh") { return OpCode::brsh; }
      if (o == "breq") { return OpCode::breq; }
      if (o == "in") { return OpCode::in; }
      if (o == "out") { return OpCode::out; }
      if (o == "inc") { return OpCode::inc; }
      if (o == "nop") { return OpCode::nop; }
      if (o == "jmp") { return OpCode::jmp; }
      if (o == "tst") { return OpCode::tst; }
      if (o == "brge") { return OpCode::brge; }
      if (o == "brlt") { return OpCode::brlt; }
      if (o == "or") { return OpCode::OR; }
      if (o == "ori") { return OpCode::ori; }
    }
    }
    throw std::runtime_error(fmt::format("Unknown opcode: {}", o));
  }

  static int get_register_number(const char reg_name)
  {
    if (reg_name == 'X') { return 26; }
    if (reg_name == 'Y') { return 28; }
    if (reg_name == 'Z') { return 30; }

    throw std::runtime_error("Unknown register name");
  }

  static Operand parse_operand(std::string_view o)
  {
    if (o.empty()) { return Operand(); }

    if (o[0] == 'r' && o.size() > 1) {
      return Operand(Operand::Type::reg, to_int(o.substr(1)));
    } else {
      return Operand(Operand::Type::literal, std::string{ o });
    }
  }

  AVR(const int t_line_num,
    std::string_view t_line_text,
    Type t,
    std::string_view t_opcode,
    std::string_view o1 = "",
    std::string_view o2 = "")
    : ASMLine(t, std::string(t_opcode)), line_num(t_line_num), line_text(std::string(t_line_text)),
      opcode(parse_opcode(t, t_opcode)), operand1(parse_operand(o1)), operand2(parse_operand(o2))
  {}

  int line_num;
  std::string line_text;
  OpCode opcode;
  Operand operand1;
  Operand operand2;
};

void indirect_load(std::vector<mos6502> &instructions,
  const std::string &from_address_low_byte,
  const std::string &to_address,
  const int offset = 0)
{
  instructions.emplace_back(mos6502::OpCode::ldy, Operand(Operand::Type::literal, fmt::format("#{}", offset)));
  instructions.emplace_back(
    mos6502::OpCode::lda, Operand(Operand::Type::literal, "(" + from_address_low_byte + "), Y"));
  instructions.emplace_back(mos6502::OpCode::sta, Operand(Operand::Type::literal, to_address));
}

void indirect_store(std::vector<risc8> &instructions)
{
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    int L = (79 - 0) & 0x0F;
    int H = (79 - 0) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);

}


// returns the "s_set","s_clear" labels to use for later
[[nodiscard]] std::pair<std::string, std::string> setup_S_flag(std::vector<mos6502> &instructions)
{
  const auto location = instructions.size();
  std::string n_set = fmt::format("n_set_{}", location);
  std::string s_set = fmt::format("s_set_{}", location);
  std::string s_clear = fmt::format("s_clear_{}", location);

  instructions.emplace_back(ASMLine::Type::Directive, "; Handle N / S flags from AVR");
  instructions.emplace_back(mos6502::OpCode::bmi, Operand(Operand::Type::literal, n_set));
  instructions.emplace_back(mos6502::OpCode::bvs, Operand(Operand::Type::literal, s_set));
  instructions.emplace_back(mos6502::OpCode::jmp, Operand(Operand::Type::literal, s_clear));
  instructions.emplace_back(ASMLine::Type::Label, n_set);
  instructions.emplace_back(mos6502::OpCode::bvs, Operand(Operand::Type::literal, s_clear));
  instructions.emplace_back(mos6502::OpCode::jmp, Operand(Operand::Type::literal, s_set));
  return { s_set, s_clear };
}

void fixup_16_bit_N_Z_flags(std::vector<mos6502> &instructions)
{

  // need to get both Z and N set appropriately
  // assuming A contains higher order byte and X contains lower order byte
  instructions.emplace_back(ASMLine::Type::Directive, "; BEGIN remove if next is lda");
  instructions.emplace_back(ASMLine::Type::Directive, "; set CPU flags assuming A holds the higher order byte already");
  std::string set_flag_label = "fixup_16_bit_op_flags" + std::to_string(instructions.size());
  // if high order is negative, we know it's not 0 and it is negative
  instructions.emplace_back(mos6502::OpCode::bmi, Operand(Operand::Type::literal, set_flag_label));
  // if it's not 0, then branch down, we know the result is not 0 and not negative
  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal, set_flag_label));
  // if the higher order byte is 0, test the lower order byte, which was saved for us in X
  instructions.emplace_back(mos6502::OpCode::txa);
  // if low order is not negative, we know it's 0 or not 0
  instructions.emplace_back(mos6502::OpCode::bpl, Operand(Operand::Type::literal, set_flag_label));
  // if low order byte is negative, just load 1, this will properly set the Z flag and leave C correct
  instructions.emplace_back(mos6502::OpCode::lda, Operand(Operand::Type::literal, "#1"));
  instructions.emplace_back(ASMLine::Type::Label, set_flag_label);
  instructions.emplace_back(ASMLine::Type::Directive, "; END remove if next is lda, bcc, bcs, ldy, inc, clc, sec");
}

void add_16_bit(const Personality &personality,
  std::vector<mos6502> &instructions,
  int reg,
  const std::string_view value)
{
  instructions.emplace_back(mos6502::OpCode::clc);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::adc, Operand(Operand::Type::literal, fmt::format("#({})", value)));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::tax);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg + 1));
  instructions.emplace_back(mos6502::OpCode::adc, Operand(Operand::Type::literal, "#0"));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg + 1));

  fixup_16_bit_N_Z_flags(instructions);
}

void add_16_bit(const Personality &personality, std::vector<mos6502> &instructions, int reg, const std::uint16_t value)
{
  instructions.emplace_back(mos6502::OpCode::clc);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg));
  instructions.emplace_back(
    mos6502::OpCode::adc, Operand(Operand::Type::literal, "#" + std::to_string((value & 0xFFu))));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::tax);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg + 1));
  instructions.emplace_back(
    mos6502::OpCode::adc, Operand(Operand::Type::literal, "#" + std::to_string((value >> 8u) & 0xFFu)));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg + 1));

  fixup_16_bit_N_Z_flags(instructions);
}

void subtract_16_bit(const Personality &personality,
  std::vector<mos6502> &instructions,
  int reg,
  const std::string_view value)
{
  instructions.emplace_back(mos6502::OpCode::sec);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::sbc, Operand(Operand::Type::literal, fmt::format("#({})", value)));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::tax);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg + 1));
  instructions.emplace_back(mos6502::OpCode::sbc, Operand(Operand::Type::literal, "#0"));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg + 1));

  fixup_16_bit_N_Z_flags(instructions);
}

void subtract_16_bit(const Personality &personality,
  std::vector<mos6502> &instructions,
  int reg,
  const std::uint16_t value)
{
  instructions.emplace_back(mos6502::OpCode::sec);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg));
  instructions.emplace_back(
    mos6502::OpCode::sbc, Operand(Operand::Type::literal, "#" + std::to_string((value & 0xFFu))));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::tax);
  instructions.emplace_back(mos6502::OpCode::lda, personality.get_register(reg + 1));
  instructions.emplace_back(
    mos6502::OpCode::sbc, Operand(Operand::Type::literal, "#" + std::to_string((value >> 8u) & 0xFFu)));
  instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(reg + 1));

  fixup_16_bit_N_Z_flags(instructions);
}

void increment_16_bit(const Personality &personality, std::vector<mos6502> &instructions, int reg)
{
  std::string skip_high_byte_label = "skip_inc_high_byte_" + std::to_string(instructions.size()) + "__optimizable";
  instructions.emplace_back(mos6502::OpCode::inc, personality.get_register(reg));
  instructions.emplace_back(mos6502::OpCode::bne, Operand(Operand::Type::literal, skip_high_byte_label));
  instructions.emplace_back(mos6502::OpCode::inc, personality.get_register(reg + 1));
  instructions.emplace_back(ASMLine::Type::Label, skip_high_byte_label);
}

void decrement_16_bit(const Personality &personality, std::vector<mos6502> &instructions, int reg)
{
  subtract_16_bit(personality, instructions, reg, 1);
}


void translate_instruction(const Personality8 &personality,
  std::vector<risc8> &instructions,
  const AVR::OpCode op,
  const Operand &o1,
  const Operand &o2, bool needs[], int rd_rr_lastlabel[], int flags[], std::vector<std::pair<std::string,int>> &label_lix)
{
  const auto translate_register_number = [](const Operand &reg) {
    if (reg.value == "__zero_reg__") {
      return 1;
    } else if (reg.value == "__temp_reg__") {
      return 0;
    } else {
      return reg.reg_num;
    }
  };

  const auto o1_reg_num = translate_register_number(o1);
  const auto o2_reg_num = translate_register_number(o2);
  int L;
  int H;
  
  switch (op) {
  case AVR::OpCode::brge: {
    return; 
  }
  case AVR::OpCode::OR: {
    return;
  }
  case AVR::OpCode::ori: {
    return;
  }
  case AVR::OpCode::jmp: return;
  case AVR::OpCode::tst: {
    rd_rr_lastlabel[0] = (79 - o1_reg_num);
    flags[1] = 1;
    return;
  }
  case AVR::OpCode::dec:  return;
  case AVR::OpCode::ldi: {
    Operand o = personality.get_register(o1_reg_num);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    L = std::stoi(fixup_8bit_literal(o2.value)) & 0x0F;
    H = std::stoi(fixup_8bit_literal(o2.value)) & 0xF0;
    switch(L){
	    case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
	    case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
	    case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
     switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    instructions.emplace_back(risc8::OpCode::STA);

    label_lix.front().second += 11;
    return;
  }
  case AVR::OpCode::sts:
    return;
  case AVR::OpCode::ret: 
    instructions.emplace_back(risc8::OpCode::HLT);
    label_lix.front().second += 1;
    return;
  case AVR::OpCode::mov:
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o2_reg_num) & 0x0F;
    H = (79 - o2_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::OR);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 16;
    return;
  case AVR::OpCode::lsl:  return;
  case AVR::OpCode::rol:  return;
  case AVR::OpCode::ror:  return;
  case AVR::OpCode::call:
    if (o1.value != ".") {
      char * label = new char[o1.value.size() +1];
      std::copy(o1.value.begin(), o1.value.end(), label);
      label[o1.value.size()] = '\0';
      if (o1.value == "__mulqi3"){
        needs[0] = true;
	sprintf(label, ".mulqi3");
      }
      if (o1.value == "__subsf3"){
        needs[1] = true;
	sprintf(label, ".subf");
      }
      if (o1.value == "__addsf3"){
        needs[2] = true;
	sprintf(label, ".addf");
      }
      if (o1.value == "__mulsf3"){
        needs[3] = true;
	sprintf(label, ".mulf");
      }
      rd_rr_lastlabel[2]++;

      instructions.emplace_back(risc8::OpCode::IXN);
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::IXR);
      instructions.emplace_back(risc8::OpCode::LIL_E);
      instructions.emplace_back(risc8::OpCode::LIH_F);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::LIL_F);
      instructions.emplace_back(risc8::OpCode::LIH_F);
      instructions.emplace_back(risc8::OpCode::CLC);
      instructions.emplace_back(risc8::OpCode::ADL);
      instructions.emplace_back(risc8::OpCode::ADH);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      instructions.emplace_back(risc8::OpCode::MMA);
     
      char buff[64];
      sprintf(buff, ".CALL%d",rd_rr_lastlabel[2]);

      instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
      instructions.emplace_back(risc8::OpCode::STA);

      
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::LIL_E);
      instructions.emplace_back(risc8::OpCode::LIH_F);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::LIL_F);
      instructions.emplace_back(risc8::OpCode::LIH_F);
      instructions.emplace_back(risc8::OpCode::CLC);
      instructions.emplace_back(risc8::OpCode::ADL);
      instructions.emplace_back(risc8::OpCode::ADH);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      instructions.emplace_back(risc8::OpCode::MMA); 

      instructions.emplace_back(risc8::OpCode::IXN);
      instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
      instructions.emplace_back(risc8::OpCode::STA);
      
      instructions.emplace_back(risc8::OpCode::IXR);
      instructions.emplace_back(risc8::OpCode::IXN);
      std::cout << "\n";
      std::cout << instructions.size();
      std::cout << "\n";
      instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, label));
      std::cout << instructions.size();
      std::cout << "\n";
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::IXR);
      instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, label));
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::JMP);

      label_lix.front().second += 52;
      label_lix.emplace_back(std::make_pair(fmt::format(".CALL{}", rd_rr_lastlabel[2]), label_lix.front().second));
      sprintf(buff, ".CALL%d:",rd_rr_lastlabel[2]);
      instructions.emplace_back(risc8::Type::Label, buff);
      instructions.emplace_back(risc8::OpCode::IXN);
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::IXR);
      instructions.emplace_back(risc8::OpCode::LIL_E);
      instructions.emplace_back(risc8::OpCode::LIH_F);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::SEC);
      instructions.emplace_back(risc8::OpCode::ADL);
      instructions.emplace_back(risc8::OpCode::ADH);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      label_lix.front().second += 15;
      return;
    }
    throw std::runtime_error("Unhandled call");
  case AVR::OpCode::icall: {
    return;
  }
  case AVR::OpCode::rcall:
    if (o1.value != ".") {
    } else {
      // just push in 2 bytes
    }

    return;
  case AVR::OpCode::ld: {
    if (o2.value == "Z" || o2.value == "X" || o2.value == "Y") {
      return;
    }
    if (o2.value == "Z+" || o2.value == "X+" || o2.value == "Y+") {
      return;
    }
    throw std::runtime_error("Unknown ld indexing");
  }
  case AVR::OpCode::ldd: {
    if (o2.value[1] == '+') {
      instructions.emplace_back(risc8::OpCode::IXN);
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::IXR);
     
      instructions.emplace_back(risc8::OpCode::LIL_3);
      instructions.emplace_back(risc8::OpCode::LIH_3);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::MBA);

      L = (std::stoi(o2.value.substr(2))) & 0x0F;
      H = (std::stoi(o2.value.substr(2))) & 0xF0;
      switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
      }
      switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
      }
      instructions.emplace_back(risc8::OpCode::NOT);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::SEC);
      instructions.emplace_back(risc8::OpCode::ADL);
      instructions.emplace_back(risc8::OpCode::ADH);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::OR);
      L = (79 - o1_reg_num) & 0x0F;
      H = (79 - o1_reg_num) & 0xF0;
      switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
      }
      switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
      }  
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      label_lix.front().second += 30;
      return;
    }

    throw std::runtime_error("Unhandled 'ldd'");
  }
  case AVR::OpCode::std: {
    if (o1.value[1] == '+') {
      instructions.emplace_back(risc8::OpCode::IXN);
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::MBA);
      instructions.emplace_back(risc8::OpCode::IXR);
     
      instructions.emplace_back(risc8::OpCode::LIL_3);
      instructions.emplace_back(risc8::OpCode::LIH_3);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::MBA);

      L = (std::stoi(o1.value.substr(2))) & 0x0F;
      H = (std::stoi(o1.value.substr(2))) & 0xF0;
      switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
      }
      switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
      }
      instructions.emplace_back(risc8::OpCode::NOT);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::SEC);
      instructions.emplace_back(risc8::OpCode::ADL);
      instructions.emplace_back(risc8::OpCode::ADH);
      instructions.emplace_back(risc8::OpCode::LIL_3);
      instructions.emplace_back(risc8::OpCode::LIH_1);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      
      instructions.emplace_back(risc8::OpCode::LIL_0);
      instructions.emplace_back(risc8::OpCode::LIH_0);
      instructions.emplace_back(risc8::OpCode::MBA);
      L = (79 - o2_reg_num) & 0x0F;
      H = (79 - o2_reg_num) & 0xF0;
      switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
      }
      switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
      }
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::OR);

      instructions.emplace_back(risc8::OpCode::LIL_3);
      instructions.emplace_back(risc8::OpCode::LIH_1);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::LDA);
      instructions.emplace_back(risc8::OpCode::MMA);
      instructions.emplace_back(risc8::OpCode::MAC);
      instructions.emplace_back(risc8::OpCode::STA);
      
      label_lix.front().second += 38; 
      return;
    }

    throw std::runtime_error("Unhandled 'std'");
  }
  case AVR::OpCode::brlt: {
    return;
  }
  case AVR::OpCode::sub: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o2_reg_num) & 0x0F;
    H = (79 - o2_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::NOT);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::MBA);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::SEC);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 21;
    //    fixup_16_bit_N_Z_flags(instructions);
    return;
  }
  case AVR::OpCode::sbc: {
    // we want to utilize the carry flag, however it was set previously
    // (it's really a borrow flag on the 6502)
    return;
  }
  case AVR::OpCode::sbci: {
    // we want to utilize the carry flag, however it was set previously
    // (it's really a borrow flag on the 6502)
    return;
  }
  case AVR::OpCode::inc: return;

  case AVR::OpCode::subi: {
    if (o1_reg_num == 28){
    	instructions.emplace_back(risc8::OpCode::IXN);
    	instructions.emplace_back(risc8::OpCode::LIL_0);
    	instructions.emplace_back(risc8::OpCode::LIH_0);
    	instructions.emplace_back(risc8::OpCode::MMA);

    	L = std::stoi(fixup_8bit_literal_subi(o2.value)) & 0x0F;
    	H = std::stoi(fixup_8bit_literal_subi(o2.value)) & 0xF0;
    	switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    	}
    	switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    	}
    	instructions.emplace_back(risc8::OpCode::MBA);
    	instructions.emplace_back(risc8::OpCode::CLC);
    	instructions.emplace_back(risc8::OpCode::IXR); 
    	L = (79 - o1_reg_num) & 0x0F;
    	H = (79 - o1_reg_num) & 0xF0;
    	switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    	}
    	switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    	}


    	instructions.emplace_back(risc8::OpCode::MMA);
    	instructions.emplace_back(risc8::OpCode::LDA);
     
    	instructions.emplace_back(risc8::OpCode::ADL);
    	instructions.emplace_back(risc8::OpCode::ADH);

    	instructions.emplace_back(risc8::OpCode::MAC);
    	instructions.emplace_back(risc8::OpCode::STA);
        label_lix.front().second += 17;
    	return;
    	}
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);

    L = std::stoi(fixup_8bit_literal_subi(o2.value)) & 0x0F;
    H = std::stoi(fixup_8bit_literal_subi(o2.value)) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    
    instructions.emplace_back(risc8::OpCode::NOT);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::SEC);
    instructions.emplace_back(risc8::OpCode::IXR); 
    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
     
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);

    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 19;
    return;
  }
  case AVR::OpCode::st: {
    if (o1.value == "Z" || o1.value == "Y" || o1.value == "X") {
      return;
    }
    if (o1.value == "Z+" || o1.value == "Y+" || o1.value == "X+") {
      return;
    }
    if (o1.value == "-Z" || o1.value == "-Y" || o1.value == "-X") {
      return;
    }
    throw std::runtime_error("Unhandled st");
  }
  case AVR::OpCode::lds: {
    return;
  }
  case AVR::OpCode::lsr: {
    return;
  }
  case AVR::OpCode::asr: {
    return;
  }
  case AVR::OpCode::andi: {
    return;
  }
  case AVR::OpCode::eor: {
    return;
  }
  case AVR::OpCode::cpse: {
    return;
  }
  case AVR::OpCode::sbrc: {
    return;
  }
  case AVR::OpCode::sbrs: {
    return;
  }
  case AVR::OpCode::brne: {

    if (o1.value == "0b") {
      return;
    } else if (o1.value == "1b") {
      return;
    } else if (o1.value == ".+2") {
      // assumes 6502 'borrow' for Carry flag instead of carry, so bcc instead of bcs
      return;
    }
    else if(flags[0] == 1){
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o2_reg_num) & 0x0F;
    H = (79 - o2_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::XOR);
    
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);
    label_lix.front().second += 22;
    flags[0]=0;
    return;
    }
    else if(flags[1] == 1){
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = rd_rr_lastlabel[0] & 0x0F;
    H = rd_rr_lastlabel[0] & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);


    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::XOR);

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);
    label_lix.front().second += 23;
    flags[1]=0;
    return;
    }

    throw std::runtime_error("Unhanled brne format");
  }

  case AVR::OpCode::rjmp: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JMP);
    label_lix.front().second += 9;
    return;
  }

  case AVR::OpCode::sbiw: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);

    L = (std::stoi(o2.value.substr(0,2)) - std::stoi(o2.value.substr(3,2))) & 0x0F;
    H = (std::stoi(o2.value.substr(0,2)) - std::stoi(o2.value.substr(3,2))) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::CLC);
    instructions.emplace_back(risc8::OpCode::IXR); 
    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
     
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);

    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 17;
    return;
  }

  case AVR::OpCode::adiw: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);

    L = (std::stoi(o2.value.substr(0,2)) - std::stoi(o2.value.substr(3,2))) & 0x0F;
    H = (std::stoi(o2.value.substr(0,2)) - std::stoi(o2.value.substr(3,2))) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    
    instructions.emplace_back(risc8::OpCode::NOT);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::SEC);
    instructions.emplace_back(risc8::OpCode::IXR); 
    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
     
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);

    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 19;
    return;
  }
  case AVR::OpCode::push: {

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);

    instructions.emplace_back(risc8::OpCode::LIL_E);
    instructions.emplace_back(risc8::OpCode::LIH_F);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::LIL_F);
    instructions.emplace_back(risc8::OpCode::LIH_F);
    instructions.emplace_back(risc8::OpCode::CLC);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }


    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::OR);

    instructions.emplace_back(risc8::OpCode::LIL_E);
    instructions.emplace_back(risc8::OpCode::LIH_F);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 33;
    return;
  }
  case AVR::OpCode::pop: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);

    instructions.emplace_back(risc8::OpCode::LIL_E);
    instructions.emplace_back(risc8::OpCode::LIH_F);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MMA);

    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::OR);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);

    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);

    instructions.emplace_back(risc8::OpCode::LIL_E);
    instructions.emplace_back(risc8::OpCode::LIH_F);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::SEC);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 30;
    return;
  }
  case AVR::OpCode::com: {
    // We're doing this in the same way the AVR does it, to make sure the C flag is set properly, it expects C to be set
    return;
  }
  case AVR::OpCode::clr: {
    return;
  }
  case AVR::OpCode::cpi: {
    // note that this will leave the C flag in the 6502 borrow state, not normal carry state
    return;
  }
  case AVR::OpCode::brlo: {

    if (o1.value == ".+2") {
      // assumes 6502 'borrow' for Carry flag instead of carry, so bcc instead of bcs
      return;
    }
    return;
  }
  case AVR::OpCode::swap: {
    // from http://www.6502.org/source/general/SWN.html
    // ASL  A
    // ADC  #$80
    // ROL  A
    // ASL  A
    // ADC  #$80
    // ROL  A


    return;
  }
  case AVR::OpCode::add: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o2_reg_num) & 0x0F;
    H = (79 - o2_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::CLC);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    label_lix.front().second += 19;
    return;
  }
  case AVR::OpCode::adc: {
    return;
  }
  case AVR::OpCode::cp: {
    rd_rr_lastlabel[0] = (79 - o1_reg_num);
    rd_rr_lastlabel[1] = (79 - o2_reg_num);
    flags[0] = 1;
    return;
  }
  case AVR::OpCode::cpc: {
    // this instruction seems to need to be used in the case after a sbc operation, where the
    // carry flag is set to the appropriate borrow state, so I'm going to not invert
    // the carry flag here, and assume that it's set how the 6502 wants it to be
    // set from the previous operation

    return;
  }

  case AVR::OpCode::brsh: {
    if (flags[0] == 1){
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = rd_rr_lastlabel[0] & 0x0F;
    H = rd_rr_lastlabel[0] & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);

    L = rd_rr_lastlabel[1] & 0x0F;
    H = rd_rr_lastlabel[1] & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    
    

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::XOR);

    rd_rr_lastlabel[2]++;
    
    char buff[64];
    sprintf(buff, ".BR%d",rd_rr_lastlabel[2]);

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JMP);
    
    label_lix.front().second += 33;
    sprintf(buff, ".BR%d:",rd_rr_lastlabel[2]);
    label_lix.emplace_back(std::make_pair(fmt::format(".BR{}", rd_rr_lastlabel[2]), label_lix.front().second));
    instructions.emplace_back(risc8::Type::Label, buff);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".compare_diff"));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".compare_b"));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::CLC);
    instructions.emplace_back(risc8::OpCode::OR);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);

    label_lix.front().second += 19;
    sprintf(buff, ".BR%dloop:",rd_rr_lastlabel[2]);
    label_lix.emplace_back(std::make_pair(fmt::format(".BR{}loop", rd_rr_lastlabel[2]), label_lix.front().second));
    instructions.emplace_back(risc8::Type::Label, buff);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".compare_diff"));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::CLC);
    instructions.emplace_back(risc8::OpCode::SHL);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    instructions.emplace_back(risc8::OpCode::IXN);
    sprintf(buff, ".BR%dcheck",rd_rr_lastlabel[2]);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".compare_b"));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::SHL);
    instructions.emplace_back(risc8::OpCode::MAC);
    instructions.emplace_back(risc8::OpCode::STA);
    sprintf(buff, ".BR%dloop",rd_rr_lastlabel[2]);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, buff));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JMP);

    label_lix.front().second += 48;
    sprintf(buff, ".BR%dcheck:",rd_rr_lastlabel[2]);
    label_lix.emplace_back(std::make_pair(fmt::format(".BR{}check", rd_rr_lastlabel[2]), label_lix.front().second));
    instructions.emplace_back(risc8::Type::Label, buff);
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".compare_b"));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::SHL);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MBA);
    instructions.emplace_back(risc8::OpCode::ADL);
    instructions.emplace_back(risc8::OpCode::ADH);
    
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, o1);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);
    flags[0]=0;
    label_lix.front().second += 24;
    }


    return;
  }
  case AVR::OpCode::out: {
    if (o1.value == "__SP_L__") {
      return;
    }

    if (o1.value == "__SP_H__") {
      // officially nothing to do - we cannot change the high byte of the SP on 6502
      return;
    }

    throw std::runtime_error("Could not translate unknown 'out' instruction");
  }

  case AVR::OpCode::in: {
    if (o2.value == "__SP_L__") {
      return;
    }

    if (o2.value == "__SP_H__") {
      return;
    }

    throw std::runtime_error("Could not translate unknown 'in' instruction");
  }

  case AVR::OpCode::breq: {
    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIL_0);
    instructions.emplace_back(risc8::OpCode::LIH_0);
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    L = (79 - o2_reg_num) & 0x0F;
    H = (79 - o2_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::MBA);

    L = (79 - o1_reg_num) & 0x0F;
    H = (79 - o1_reg_num) & 0xF0;
    switch(L){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIL_0); break;
            case 0x01: instructions.emplace_back(risc8::OpCode::LIL_1); break;
            case 0x02: instructions.emplace_back(risc8::OpCode::LIL_2); break;
            case 0x03: instructions.emplace_back(risc8::OpCode::LIL_3); break;
            case 0x04: instructions.emplace_back(risc8::OpCode::LIL_4); break;
            case 0x05: instructions.emplace_back(risc8::OpCode::LIL_5); break;
            case 0x06: instructions.emplace_back(risc8::OpCode::LIL_6); break;
            case 0x07: instructions.emplace_back(risc8::OpCode::LIL_7); break;
            case 0x08: instructions.emplace_back(risc8::OpCode::LIL_8); break;
            case 0x09: instructions.emplace_back(risc8::OpCode::LIL_9); break;
            case 0x0A: instructions.emplace_back(risc8::OpCode::LIL_A); break;
            case 0x0B: instructions.emplace_back(risc8::OpCode::LIL_B); break;
            case 0x0C: instructions.emplace_back(risc8::OpCode::LIL_C); break;
            case 0x0D: instructions.emplace_back(risc8::OpCode::LIL_D); break;
            case 0x0E: instructions.emplace_back(risc8::OpCode::LIL_E); break;
            case 0x0F: instructions.emplace_back(risc8::OpCode::LIL_F); break;
    }
    switch(H){
            case 0x00: instructions.emplace_back(risc8::OpCode::LIH_0); break;
            case 0x10: instructions.emplace_back(risc8::OpCode::LIH_1); break;
            case 0x20: instructions.emplace_back(risc8::OpCode::LIH_2); break;
            case 0x30: instructions.emplace_back(risc8::OpCode::LIH_3); break;
            case 0x40: instructions.emplace_back(risc8::OpCode::LIH_4); break;
            case 0x50: instructions.emplace_back(risc8::OpCode::LIH_5); break;
            case 0x60: instructions.emplace_back(risc8::OpCode::LIH_6); break;
            case 0x70: instructions.emplace_back(risc8::OpCode::LIH_7); break;
            case 0x80: instructions.emplace_back(risc8::OpCode::LIH_8); break;
            case 0x90: instructions.emplace_back(risc8::OpCode::LIH_9); break;
            case 0xA0: instructions.emplace_back(risc8::OpCode::LIH_A); break;
            case 0xB0: instructions.emplace_back(risc8::OpCode::LIH_B); break;
            case 0xC0: instructions.emplace_back(risc8::OpCode::LIH_C); break;
            case 0xD0: instructions.emplace_back(risc8::OpCode::LIH_D); break;
            case 0xE0: instructions.emplace_back(risc8::OpCode::LIH_E); break;
            case 0xF0: instructions.emplace_back(risc8::OpCode::LIH_F); break;
    }
    
    

    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::LDA);
    instructions.emplace_back(risc8::OpCode::XOR);

    rd_rr_lastlabel[2]++;

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".BR" + rd_rr_lastlabel[2]));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, ".BR" + rd_rr_lastlabel[2]));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JNE);

    instructions.emplace_back(risc8::OpCode::IXN);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::IXR);
    instructions.emplace_back(risc8::OpCode::LIX, Operand(Operand::Type::literal, o1.value));
    instructions.emplace_back(risc8::OpCode::MMA);
    instructions.emplace_back(risc8::OpCode::JMP);
    
    char buff[64];
    label_lix.front().second += 33;
    sprintf(buff, ".BR%d:",rd_rr_lastlabel[2]);
    instructions.emplace_back(risc8::Type::Label, buff);
    label_lix.emplace_back(std::make_pair(fmt::format(".BR{}", rd_rr_lastlabel[2]), label_lix.front().second));
     return;
  }
  case AVR::OpCode::nop: {
    return;
  }
  case AVR::OpCode::unknown: {
    throw std::runtime_error("Could not translate 'unknown' instruction");
  }
  }

  throw std::runtime_error("Could not translate unhandled instruction");
}

void to_risc8(const Personality8 &personality, const AVR &from_instruction, std::vector<risc8> &instructions, bool needs[], int rd_rr_lastlabel[], int flags[], std::vector<std::pair<std::string,int>> &label_lix)
{
  try {
    switch (from_instruction.type) {
    case ASMLine::Type::Label:
      if (from_instruction.text == "0") {
        instructions.emplace_back(from_instruction.type, "-memcpy_0");
      } else if (from_instruction.text == "1") {
        instructions.emplace_back(from_instruction.type, "-mul2_1");
      } else {
        instructions.emplace_back(from_instruction.type, from_instruction.text);
      }
      return;
    case ASMLine::Type::Directive:
      if (from_instruction.text.starts_with(".string") || from_instruction.text.starts_with(".ascii")) {
        const auto &text = from_instruction.text;
        const auto start = [=]() -> std::size_t {
          if (text.starts_with(".string")) {
            return 9;
          } else {
            return 8;
          }
        }();

        const auto isdigit = [](char c) { return c <= '9' && c >= '0'; };

        for (std::size_t pos = start; text[pos] != '"'; ++pos) {
          if (text[pos] != '\\') {
            instructions.emplace_back(
              ASMLine::Type::Directive, fmt::format(".byt ${:02x}", static_cast<std::uint8_t>(text[pos])));
          } else {
            if (text[pos + 1] == 'f') {
              instructions.emplace_back(ASMLine::Type::Directive, fmt::format(".byt ${:02x}", 014));
              ++pos;
            } else if (text[pos + 1] == 'b') {
              instructions.emplace_back(ASMLine::Type::Directive, fmt::format(".byt ${:02x}", '\b'));
              ++pos;
            } else if (text[pos + 1] == 't') {
              instructions.emplace_back(ASMLine::Type::Directive, fmt::format(".byt ${:02x}", '\t'));
              ++pos;
            } else if (text[pos + 1] == 'r') {
              instructions.emplace_back(ASMLine::Type::Directive, fmt::format(".byt ${:02x}", '\r'));
              ++pos;
            } else if (text[pos + 1] == '"') {
              instructions.emplace_back(ASMLine::Type::Directive, fmt::format(".byt ${:02x}", '"'));
              ++pos;
            } else if (isdigit(text[pos + 1]) && isdigit(text[pos + 2]) && isdigit(text[pos + 3])) {
              std::string octal = "0";
              octal += text[pos + 1];
              octal += text[pos + 2];
              octal += text[pos + 3];
              instructions.emplace_back(
                ASMLine::Type::Directive, fmt::format(".byt ${:02x}", std::stoi(octal, nullptr, 8)));
              pos += 3;
            } else {
              spdlog::error("[{}]: Unhandled .string escape: '{}': {}",
                from_instruction.line_num,
                from_instruction.line_text,
                text[pos + 1]);
            }
          }
        }

        if (text.starts_with(".string")) {
          instructions.emplace_back(ASMLine::Type::Directive, ".byt 0");// terminating byte
        }
      } else if (from_instruction.text.starts_with(".word")) {

        const auto matcher = ctre::match<R"(\s*.word\s*(.*))">;

        if (const auto results = matcher(from_instruction.text); results) {
          const auto matched_gs = results.get<1>().to_string();
          instructions.emplace_back(ASMLine::Type::Directive, ".word " + std::string{ strip_gs(matched_gs) });
        } else {
          instructions.emplace_back(ASMLine::Type::Directive, ".word " + from_instruction.text.substr(6));
          // spdlog::warn("Unknown .word directive '{}'", from_instruction.text);
        }

      } else if (from_instruction.text.starts_with(".byte")) {
        instructions.emplace_back(ASMLine::Type::Directive, ".byt <" + from_instruction.text.substr(6));
      } else if (from_instruction.text.starts_with(".zero")) {
        const auto count = std::stoull(&*std::next(from_instruction.text.begin(), 6), nullptr, 10);

        std::string zeros;
        for (std::size_t i = 0; i < count; ++i) {
          if ((i % 40) == 0) {
            if (!zeros.empty()) {
              instructions.emplace_back(ASMLine::Type::Directive, zeros);
              zeros.clear();
            }
            zeros += ".byt 0";
          } else {
            zeros += ",0";
          }
        }

        if (!zeros.empty()) { instructions.emplace_back(ASMLine::Type::Directive, zeros); }
      } else if (from_instruction.text[0] == ';') {
        // it's a comment
        instructions.emplace_back(ASMLine::Type::Directive, from_instruction.text);
      } else {
        instructions.emplace_back(ASMLine::Type::Directive, "; Unknown directive: " + from_instruction.text);
      }
      return;
    case ASMLine::Type::Instruction:
      const auto head = instructions.size();
      try {
        translate_instruction(
          personality, instructions, from_instruction.opcode, from_instruction.operand1, from_instruction.operand2, needs, rd_rr_lastlabel, flags, label_lix);
      } catch (const std::exception &e) {
        instructions.emplace_back(
          ASMLine::Type::Directive, "; Unhandled opcode: '" + from_instruction.text + "' " + e.what());
        spdlog::error(
          "[{}]: Unhandled instruction: '{}': {}", from_instruction.line_num, from_instruction.line_text, e.what());
      }

      auto text = from_instruction.line_text;
      if (text[0] == '\t') { text.erase(0, 1); }
      for_each(std::next(instructions.begin(), static_cast<std::ptrdiff_t>(head)),
        instructions.end(),
        [text](auto &ins) { ins.comment = text; });
      return;
    }
  } catch (const std::exception &e) {
    spdlog::error(
      "[{}]: Unhandled instruction: '{}': {}", from_instruction.line_num, from_instruction.line_text, e.what());
  }
}


bool fix_long_branches(std::vector<mos6502> &instructions, int &branch_patch_count)
{
  std::map<std::string, size_t> labels;
  for (size_t op = 0; op < instructions.size(); ++op) {
    if (instructions[op].type == ASMLine::Type::Label) { labels[instructions[op].text] = op; }
  }

  for (size_t op = 0; op < instructions.size(); ++op) {
    if (instructions[op].is_branch
        && std::abs(static_cast<int>(labels[instructions[op].op.value]) - static_cast<int>(op)) * 4 > 255) {
      ++branch_patch_count;
      const auto going_to = instructions[op].op.value;
      const auto new_pos = "patch_" + std::to_string(branch_patch_count);
      // uh-oh too long of a branch, have to convert this to a jump...

      std::map<mos6502::OpCode, mos6502::OpCode> branch_mapping;

      branch_mapping[mos6502::OpCode::bne] = mos6502::OpCode::beq;
      branch_mapping[mos6502::OpCode::beq] = mos6502::OpCode::bne;
      branch_mapping[mos6502::OpCode::bcc] = mos6502::OpCode::bcs;
      branch_mapping[mos6502::OpCode::bcs] = mos6502::OpCode::bcc;

      const auto mapping = branch_mapping.find(instructions[op].opcode);

      if (mapping != branch_mapping.end()) {
        const auto comment = instructions[op].comment;
        instructions[op] = mos6502(mapping->second, Operand(Operand::Type::literal, new_pos));
        instructions.insert(std::next(std::begin(instructions), static_cast<std::ptrdiff_t>(op + 1)),
          mos6502(mos6502::OpCode::jmp, Operand(Operand::Type::literal, going_to)));
        instructions.insert(std::next(std::begin(instructions), static_cast<std::ptrdiff_t>(op + 2)),
          mos6502(ASMLine::Type::Label, new_pos));
        instructions[op].comment = instructions[op + 1].comment = instructions[op + 2].comment = comment;
        return true;
      }

      throw std::runtime_error("Don't know how to reorg this branch: " + instructions[op].to_string());
    }
  }
  return false;
}


std::vector<risc8> run(const Personality8 &personality, std::istream &input)
{
  std::regex Comment(R"(\s*(\#|;)(.*))");
  std::regex Label(R"(^\s*(\S+):.*)");
  std::regex Directive(R"(^\s*(\..+))");
  std::regex UnaryInstruction(R"(^\s+(\S+)\s+(\S+).*)");
  std::regex BinaryInstruction(R"(^\s+(\S+)\s+(\S+),\s*(\S+).*)");
  std::regex Instruction(R"(^\s+(\S+).*)");

  std::size_t lineno = 0;


  std::vector<AVR> instructions;

  const auto parse_line = [&](const auto &line) {
    try {
      std::smatch match;
      if (std::regex_match(line, match, Label)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Label, match[1].str() + ":");
      } else if (std::regex_match(line, match, Comment)) {
        // save comments!
        instructions.emplace_back(lineno, line, ASMLine::Type::Directive, "; " + match[2].str());
      } else if (std::regex_match(line, match, Directive)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Directive, match[1].str());
      } else if (std::regex_match(line, match, BinaryInstruction)) {
        instructions.emplace_back(
          lineno, line, ASMLine::Type::Instruction, match[1].str(), match[2].str(), match[3].str());
      } else if (std::regex_match(line, match, UnaryInstruction)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1].str(), match[2].str());
      } else if (std::regex_match(line, match, Instruction)) {
        instructions.emplace_back(lineno, line, ASMLine::Type::Instruction, match[1].str());
      } else if (line.empty()) {
        // skip empty lines
      }
    } catch (const std::exception &e) {
      spdlog::error("[{}]: parse exception with '{}': {}", lineno, line, e.what());
    }

    ++lineno;
  };

  const auto parse_stream = [&](auto &stream) {
    while (stream.good()) {
      std::string line;
      getline(stream, line);
      parse_line(line);
    }
  };
/*
  const auto parse_string = [&](const auto &string) {
    std::stringstream ss{ std::string(string) };
    parse_stream(ss);
  };
*/
  parse_stream(input);
/*
  const bool needs_mulhi3 = std::any_of(begin(instructions), end(instructions), [](const AVR &instruction) {
    return instruction.line_text.find("__mulhi3") != std::string::npos;
  });
  const bool needs_mulqi3 = std::any_of(begin(instructions), end(instructions), [](const AVR &instruction) {
    return instruction.line_text.find("__mulqi3") != std::string::npos;
  });

  if (needs_mulhi3) { parse_string(__mulhi3); }
  if (needs_mulqi3) { parse_string(__mulqi3); }
*/
  std::set<std::string> labels;

  for (const auto &i : instructions) {
    if (i.type == ASMLine::Type::Label) { labels.insert(i.text); }
  }

  std::set<std::string> used_labels{ "main:" };

  for (const auto &i : instructions) {
    const auto check_label = [&](const std::string &value) {
      if (labels.count(value) != 0) { used_labels.insert(value); }
    };

    if (i.type == ASMLine::Type::Instruction) {

      check_label(i.operand1.value + ":");
      check_label(i.operand2.value + ":");
      check_label(std::string{ strip_gs(strip_offset(strip_negate(strip_lo_hi(i.operand1.value + ":")))) });
      check_label(std::string{ strip_gs(strip_offset(strip_negate(strip_lo_hi(i.operand2.value + ":")))) });


    } else if (i.type == ASMLine::Type::Directive) {
      const auto matcher = ctre::match<R"(\s*.word\s*(.*))">;

      if (const auto results = matcher(i.text); results) {
        const auto matched_gs = results.get<1>().to_string();
        spdlog::trace("matched .word: '{}' from '{}'", matched_gs, i.text);
        check_label(std::string{ strip_gs(matched_gs) });
      }
    }
  }

  const auto new_labels = [&used_labels]() {
    std::map<std::string, std::string> result;
    for (const auto &l : used_labels) {

      const auto new_label = [](auto label) -> std::string {
              if (label[0] == '.') { label.erase(0, 1); }
	      uint8_t cc = 0;
              for (auto &c : label) {
                  if (c == '.') { c = '_'; }
		  if (c == '_') { label.erase(cc,1); }
		  cc++;
              }
	      
	      return label;
      };

      result.emplace(std::make_pair(l, new_label(l)));
    }
    return result;
  }();

  for (const auto &label : new_labels) { spdlog::trace("used label: '{}':'{}'", label.first, label.second); }

  for (auto &i : instructions) {
    if (i.type == ASMLine::Type::Label) {
      if (i.text == "0") {
        i.text = "-memcpy_0";
      } else if (i.text == "1") {
        i.text = "-mul2_1";
      } else {
        try {
          i.text = new_labels.at(i.text);
        } catch (...) {
          spdlog::warn("Unused label: '{}', consider making function static until we remove unused functions", i.text);
          i.text = "; Label is unused: " + i.text;
        }
      }
    }

    if (i.type == ASMLine::Type::Directive) {
      const auto matcher = ctre::match<R"(\s*.word\s*(.*))">;

      if (const auto results = matcher(i.text); results) {
        const auto matched_gs = results.get<1>().to_string();
        const auto possible_label = std::string{ strip_gs(matched_gs) };
        const auto matched_label = new_labels.find(possible_label);
        if (matched_label != new_labels.end()) { i.text = ".word " + matched_label->second; }
      }
    }

    if (i.operand2.value.starts_with("lo8(") || i.operand2.value.starts_with("hi8(")) {
      const auto lo_hi_operand = strip_lo_hi(i.operand2.value);
      const auto label_matcher = ctre::match<R"(-?\(?([A-Za-z0-9.]+).*)">;

      if (const auto results = label_matcher(lo_hi_operand); results) {
        std::string_view potential_label = results.get<1>();
        const auto start = std::distance(std::string_view{ i.operand2.value }.begin(), potential_label.begin());
        spdlog::trace("Label matched: '{}'", potential_label);
        const auto itr1 = new_labels.find(std::string{ potential_label });
        if (itr1 != new_labels.end()) {
          i.operand2.value.replace(static_cast<std::size_t>(start), potential_label.size(), itr1->second);
        }
        spdlog::trace("New statement: '{}'", i.operand2.value);
      }
    }

    if (const auto plus = i.operand1.value.find('+'); plus != std::string::npos) {
      const auto str = i.operand1.value.substr(0, plus);
      const auto itr1 = new_labels.find(str);
      if (itr1 != new_labels.end()) { i.operand1.value.replace(0, plus, itr1->second); }
    }

    if (const auto plus = i.operand2.value.find('+'); plus != std::string::npos) {
      const auto str = i.operand2.value.substr(0, plus);
      const auto itr1 = new_labels.find(str);
      if (itr1 != new_labels.end()) { i.operand2.value.replace(0, plus, itr1->second); }
    }

    const auto itr1 = new_labels.find(i.operand1.value + ":");
    if (itr1 != new_labels.end()) { i.operand1.value = itr1->second.substr(0, itr1->second.size()-1); }

    const auto itr2 = new_labels.find(i.operand2.value);
    if (itr2 != new_labels.end()) { i.operand2.value = itr2->second; }
  }


  std::vector<risc8> new_instructions;
  personality.insert_autostart_sequence(new_instructions);
  // set __zero_reg__ (reg 1 on AVR) to 0
  //new_instructions.emplace_back(mos6502::OpCode::lda, Operand(Operand::Type::literal, "#$00"));
  //new_instructions.emplace_back(mos6502::OpCode::sta, personality.get_register(1));
  //new_instructions.emplace_back(mos6502::OpCode::jmp, Operand(Operand::Type::literal, "main"));


  int instructions_to_skip = -1;
  bool need_mul8 = false;
  bool need_subf = false;
  bool need_addf = false;
  bool need_mulf = false;
  bool needs[4] = {need_mul8, need_subf, need_addf, need_mulf};
  int rd_rr_lastlabel[4] = {0,0,0};
  std::vector<std::pair<std::string, int>> label_lix;
  label_lix.emplace_back(std::make_pair("lix", 255));
  std::string help[] = {".asgn", ".aexp", ".aman", ".aman1", ".aman2", ".bsgn", ".bexp", ".bman", ".bman1", ".bman2", ".csgn", ".cexp", ".cman", ".cman1", ".cman2", ".mulaman_3", ".mulaman_2", ".mulaman_1", ".mulman", ".mulman1", ".mulman2", ".mulman3", ".mulman4", ".mulman5", ".compare_diff", ".compare_b", ".exp_diff", ".shift_carry", ".mul_exp_check", ".rcall_addr"};
  for (int l = 4; l <34 ;l++){
  	label_lix.emplace_back(std::make_pair(help[l-4], l));
  }
  std::cout << label_lix.back().first;
  std::cout << "\n";
  std::cout << label_lix.back().second;
  int flags [2] = {0, 0};
  std::string next_label_name;
  for (const auto &i : instructions) {
    to_risc8(personality, i, new_instructions, needs, rd_rr_lastlabel, flags, label_lix);

    // intentionally copy so we don't invalidate the reference
    const auto last_instruction = new_instructions.back();

    if (i.type == ASMLine::Type::Instruction) { --instructions_to_skip; }
    if (instructions_to_skip == 0) {
      new_instructions.emplace_back(ASMLine::Type::Label, next_label_name);
      // todo: I kind of hate this -1 as a marker
      instructions_to_skip = -1;
    }


    if (last_instruction.type == ASMLine::Type::Directive
        && last_instruction.text.starts_with("skip_next_instruction")) {
      instructions_to_skip = 1;
      next_label_name = last_instruction.text;
      new_instructions.pop_back();
    }
    if (last_instruction.type == ASMLine::Type::Directive
        && last_instruction.text.starts_with("skip_next_2_instructions")) {
      instructions_to_skip = 2;
      next_label_name = last_instruction.text;
      new_instructions.pop_back();
    }
  }


  const auto parse_risc_line = [&](const auto &line) {
    try {
      std::smatch match;
      if (std::regex_match(line, match, Label)) {
        new_instructions.emplace_back(risc8::Type::Label, line);
      } else if (std::regex_match(line, match, Comment)) {
        // save comments!
        new_instructions.emplace_back(risc8::Type::Directive, "; " + line);
      } else if (std::regex_match(line, match, Directive)) {
        new_instructions.emplace_back(risc8::Type::Directive, line);
      } else if (std::regex_match(line, match, BinaryInstruction)) {
        new_instructions.emplace_back(
          risc8::Type::Instruction, line);
      } else if (std::regex_match(line, match, UnaryInstruction)) {
        new_instructions.emplace_back(risc8::Type::Instruction, line);
      } else if (std::regex_match(line, match, Instruction)) {
        new_instructions.emplace_back(risc8::Type::Instruction, line);
      } else if (line.empty()) {
        // skip empty lines
      }
    } catch (const std::exception &e) {
      spdlog::error("[{}]: parse exception with '{}': {}", lineno, line, e.what());
    }
  };


  const auto parse_risc_stream = [&](auto &stream) {
    while (stream.good()) {
      std::string line;
      getline(stream, line);
      parse_risc_line(line);
    }
  };

  const auto parse_risc_string = [&](const auto &string) {
    std::stringstream ss{ std::string(string) };
    parse_risc_stream(ss);
  };

  std::cout << new_instructions.size();

  if (needs[0]){
    parse_risc_string(risc__mulqi3);
  }
  if (needs[1]){
    parse_risc_string(risc__subf3);
    needs[2] = true;
  }
  if (needs[2]){
    parse_risc_string(risc__addf3);
  }
  if (needs[3]){
    parse_risc_string(risc__mulf3);
  }
  std::cout << label_lix.back().first;
  std::cout << "\n";
  std::cout << label_lix.back().second;
 
  int nii = 0;
  for (const auto &ni : new_instructions){
  	if(ni.to_string().find("LIX")){
		std::cout << ni.to_string().substr(ni.to_string().find("LIX")+4, ni.to_string().find(" ", ni.to_string().find("LIX")+4) - ni.to_string().find("LIX")+4);
	}
	nii++;
  }

/*
  if (do_optimize) {
    spdlog::info("Running optimization passes");

    int count = 0;

    //while (optimize(new_instructions, personality)) {
      // do it however many times it takes
      //++count;
    //}

    spdlog::info("Optimization passes run: {}", count);
  } else {
    spdlog::info("Optimization passes disabled");
  }

  int branch_patch_count = 0;
*/
  return new_instructions;
}

enum struct Target { R8, C64, X16 };

int main(const int argc, const char **argv)
{
  spdlog::set_level(spdlog::level::warn);
  const std::map<std::string, Target> targets{{"R8", Target::R8}, { "C64", Target::C64 }, { "X16", Target::X16 }  };
  CLI::App app{ "C++ Compiler for 6502 processors" };

  std::filesystem::path filename{};
  std::filesystem::path output_filename{};

  Target target{ Target::R8 };
  bool optimize{ true };

  std::string goption;
  app.add_flag("-g", goption, "g option")->required(false);

  std::string soption;
  app.add_flag("-S", soption, "S option")->required(false);

  app.add_option("-o", output_filename, "output file name")->required(false);
  app.add_option("filename", filename, "C++ file to compile")->required(true);
  app.add_option("-t,--target", target, "6502 - based system to target")
    ->required(false)
    ->default_val("0")
    ->transform(CLI::CheckedTransformer(targets, CLI::ignore_case));

  std::string optimization_level;
  app.add_option("-O", optimization_level, "Optimization level to pass to GCC instance")
    ->required(false)
    ->check(CLI::IsMember({ "s", "0", "1", "2", "3" }))
    ->default_val("1");

  app.add_flag("--optimize", optimize, "Enable optimization of 6502 generated assembly")->default_val(true);

  std::vector<std::string> include_paths;
  app.add_option("-I", include_paths, "Extra include paths to pass to GCC instance")
    ->required(false)
    ->expected(1)
    ->take_all();

  CLI11_PARSE(app, argc, argv)


  include_paths.insert(include_paths.begin(), "~/avr-libstdcpp/include");
  const std::string_view warning_flags = "-Wall -Wextra -Wconversion";
  const std::string_view avr = "avr3";

  const auto make_output_file_name = [](auto input_filename, const auto &new_extension) {
    input_filename.replace_extension(new_extension);
    return std::filesystem::current_path() / input_filename.filename();
  };

  const auto avr_output_file = make_output_file_name(filename, "avr.asm");
  const auto mos6502_output_file = output_filename; //make_output_file_name(filename, "6502.asm");
  const auto program_output_file = make_output_file_name(filename, "prg");

  std::string disabled_optimizations;
  /*
    disabled_optimizations += " -fno-gcse-after-reload";
    disabled_optimizations += " -fno-ipa-cp-clone";
    disabled_optimizations += " -fno-loop-interchange";
    disabled_optimizations += " -fno-loop-unroll-and-jam";
    disabled_optimizations += " -fno-peel-loops";
    disabled_optimizations += " -fno-predictive-commoning";
    disabled_optimizations += " -fno-split-loops";
    disabled_optimizations += " -fno-split-paths";
    disabled_optimizations += " -fno-tree-loop-distribution";
    disabled_optimizations += " -fno-tree-loop-vectorize";
    disabled_optimizations += " -fno-tree-partial-pre";
    disabled_optimizations += " -fno-tree-slp-vectorize";
    disabled_optimizations += " -fno-unswitch-loops";
    disabled_optimizations += " -fvect-cost-model=cheap";
    disabled_optimizations += " -fno-version-loops-for-strides";
    */

  const std::string gcc_command = fmt::format(
    "avr-gcc -fverbose-asm -c -o {outfile} -S {warning_flags} -std=c++20 -mtiny-stack -fconstexpr-ops-limit=333554432 "
    "-mmcu={avr} -O{optimization} {disabled_optimizations} -I {user_include_dirs} {infile}",
    fmt::arg("outfile", avr_output_file.generic_string()),
    fmt::arg("warning_flags", warning_flags),
    fmt::arg("avr", avr),
    fmt::arg("optimization", optimization_level),
    fmt::arg("user_include_dirs", fmt::join(include_paths, " -I ")),
    fmt::arg("disabled_optimizations", disabled_optimizations),
    fmt::arg("infile", filename.generic_string()));

  spdlog::info("Executing gcc: `{}`", gcc_command);

  const auto result = std::system(gcc_command.c_str());

  if (result != EXIT_SUCCESS) {
    spdlog::critical("compile failed");
    return result;
  }


  std::ifstream input(avr_output_file);



  const auto new_instructions = [&]() {
    switch (target) {
      case Target::R8:
	return run(R8{}, input);
      default:
        spdlog::critical("Unhandled target type");
        return std::vector<risc8>{};
    }
  }();

  {
    // make sure file is closed before we try to re-open it with xa
    std::ofstream mos6502_output(mos6502_output_file, std::ofstream::trunc);
    for (const auto &i : new_instructions) { mos6502_output << i.to_string() << '\n'; }
  }

  /*const std::string xa_command = fmt::format("xa -O PETSCREEN -M -o {outfile} {infile}",
    fmt::arg("infile", mos6502_output_file.generic_string()),
    fmt::arg("outfile", program_output_file.generic_string()));

  spdlog::info("Executing xa: `{}`", xa_command);

  const auto xa_result = std::system(xa_command.c_str());

  if (xa_result != EXIT_SUCCESS) {
    spdlog::critical("assembly failed");
    return result;
  }*/
}
