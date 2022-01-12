static constexpr std::string_view risc__mulqi3 =
  R"(
.mulqi3:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 MBA
 IXR
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR
 LIX .mulman
 MMA
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x0
 LIH 0x0
 STA
 LIX .mulman1
 MMA
 MAC
 STA

.mulshift:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 MBA
 IXR
 LIX .mulman1
 MMA
 LDA
 SHR
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
      
 IXN
 LIX .muladd
 MMA
 IXR
 LIX .muladd
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .mulman1
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x0
 CLC
 ADL
 ADH

 IXN
 LIX .mulcontinue
 MMA
 IXR
 LIX .mulcontinue
 MMA
 JNE

 IXN
 LIX .mulend
 MMA
 IXR
 LIX .mulend
 MMA
 JMP

.mulcontinue:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .mulman
 MMA
 LDA
 SHL
 MAC
 STA

 IXN
 LIX .mulshift
 MMA
 IXR
 LIX .mulshift
 MMA
 JMP

.muladd:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .mulman
 MMA
 LDA
 MBA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 CLC
 ADL
 ADH
 MAC
 STA

 LIX .mulman
 MMA
 LDA
 SHL
 MAC
 STA

 IXN
 LIX .mulshift
 MMA
 IXR
 LIX .mulshift
 MMA
 JMP

.mulend:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 MAC
 STA
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 LDA
 IXN
 MMA
 IXR
 MAC
 MMA
 JMP
)";

static constexpr std::string_view risc__mulf3 =
 R"(
.mulf:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 OR
 MMA
 IXR

 LIX .mulman
 MMA
 MAC
 STA
 LIX .mulman1
 MMA
 MAC
 STA
 LIX .mulman2
 MMA
 MAC
 STA
 LIX .mulman3
 MMA
 MAC
 STA
 LIX .mulman4
 MMA
 MAC
 STA
 LIX .mulman5
 MMA
 MAC
 STA
 LIX .mulaman_1
 MMA
 MAC
 STA
 LIX .mulaman_2
 MMA
 MAC
 STA
 LIX .mulaman_3
 MMA
 MAC
 STA

 IXN
 LIX .mulf_extraction
 MMA
 IXR
 LIX .mulf_extraction
 MMA
 JMP

.mulf_extraction:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 AND

 IXN
 LIX .mulf_asgn1
 MMA
 IXR
 LIX .mulf_asgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LIL 0x0
 LIH 0x0
 STA

.mulf_rasgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 AND

 IXN
 LIX .mulf_bsgn1
 MMA
 IXR
 LIX .mulf_bsgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bsgn
 MMA
 LIL 0x0
 LIH 0x0
 STA

.mulf_rbsgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 SHL
 MAC
 CLC
 OR
 LIX .aexp
 MMA
 MAC
 STA

 LIL 0xB
 LIH 0x3
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 SHL
 MAC
 CLC
 OR
 LIX .bexp
 MMA
 MAC
 STA

 LIL 0x0
 LIH 0x8
 MBA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman
 MMA
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0x8
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman1
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman2
 MMA
 MAC
 STA

 LIL 0x0
 LIH 0x8
 MBA
 LIL 0xB
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman
 MMA
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0xC
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman1
 MMA
 MAC
 STA
 LIL 0xD
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman2
 MMA
 MAC
 STA

 IXN
 LIX .mulf_exceptions
 MMA
 IXR
 LIX .mulf_exceptions
 MMA
 JMP

.mulf_asgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LIL 0x1
 LIH 0x0
 STA

 IXN
 LIX .mulf_rasgn1
 MMA
 IXR
 LIX .mulf_rasgn1
 MMA
 JMP

.mulf_bsgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bsgn
 MMA
 LIL 0x1
 LIH 0x0
 STA

 IXN
 LIX .mulf_rbsgn1
 MMA
 IXR
 LIX .mulf_rbsgn1
 MMA
 JMP


.mulf_exceptions:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR

 LIX .aexp
 MMA
 LDA
 OR

 IXN
 LIX .mulf_isb0
 MMA
 IXR
 LIX .mulf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .aman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_isb0
 MMA
 IXR
 LIX .mulf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .aman1
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_isb0
 MMA
 IXR
 LIX .mulf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .aman2
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_isb0
 MMA
 IXR
 LIX .mulf_isb0
 MMA
 JNE

 IXN
 LIX .mulf_ais0
 MMA
 IXR
 LIX .mulf_ais0
 MMA
 JMP

.mulf_isb0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_no0
 MMA
 IXR
 LIX .mulf_no0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_no0
 MMA
 IXR
 LIX .mulf_no0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .bman1
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_no0
 MMA
 IXR
 LIX .mulf_no0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_no0
 MMA
 IXR
 LIX .mulf_no0
 MMA
 JNE

 IXN
 LIX .mulf_bis0
 MMA
 IXR
 LIX .mulf_bis0
 MMA
 JMP


.mulf_ais0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret0
 MMA
 IXR
 LIX .mulf_ret0
 MMA
 JNE

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JMP

.mulf_bis0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .aexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret0
 MMA
 IXR
 LIX .mulf_ret0
 MMA
 JNE

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JMP

.mulf_no0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .aexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_isbnan
 MMA
 IXR
 LIX .mulf_isbnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .aman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_isbnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .aman1
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .aman2
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIX .mulf_ainf
 MMA
 IXR
 LIX .mulf_ainf
 MMA
 JMP

.mulf_ainf:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 IXR
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 IXR
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 IXR
 LIX .bman1
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LDA
 MBA
 LIX .bsgn
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JNE

 IXN
 LIX .mulf_reta
 MMA
 IXR
 LIX .mulf_reta
 MMA
 JMP


.mulf_isbnan:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 IXR
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_add_exp
 MMA
 IXR
 LIX .mulf_add_exp
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .bman1
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 OR

 IXN
 LIX .mulf_retnan
 MMA
 IXR
 LIX .mulf_retnan
 MMA
 JNE

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JMP

.mulf_retnan:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0x7
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_add_exp:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .aexp
 MMA
 LDA
 MBA
 LIX .bexp
 MMA
 LDA
 ADL
 ADH

 LIX .cexp
 MMA
 MAC
 STA
 MBA
 LIL 0x0
 LIH 0x8
 AND

 MAC
 MBA
 LIL 0x0
 LIH 0x0
 ADL
 ADH
 LIL 0x1
 LIH 0x8
 MBA
 MAC
 AND

 IXN
 LIX .mulf_exp_ok
 MMA
 IXR
 LIX .mulf_exp_ok
 MMA
 JNE

 IXN
 LIX .mulf_ret0
 MMA
 IXR
 LIX .mulf_ret0
 MMA
 JMP

.mulf_exp_ok:
 LIL 0x1
 LIH 0x0
 MBA
 MAC
 AND

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .mul_exp_check
 MMA
 MAC
 STA

 LIX .cexp
 MMA
 LDA
 MBA
 LIL 0xF
 LIH 0x7
 NOT
 MAC
 SEC
 ADL
 ADH
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 LIX .mul_exp_check
 MMA
 LDA
 ADL
 ADH
 MAC
 STA
 MBA

 LIL 0x2
 LIH 0x0
 XOR

 IXN
 LIX .mulf_mantissas
 MMA
 IXR
 LIX .mulf_mantissas
 MMA
 JNE

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JMP

.mulf_mantissas:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .bman
 MMA
 LDA
 SHR
 MAC
 STA

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .bman1
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_bman_shift_carry1
 MMA
 IXR
 LIX .mulf_bman_shift_carry1
 MMA
 JNE

.mulf_bman_shift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .bman2
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_bman_shift_carry2
 MMA
 IXR
 LIX .mulf_bman_shift_carry2
 MMA
 JNE

.mulf_bman_shift_carry2_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH

 IXN
 LIX .mulf_man_add
 MMA
 IXR
 LIX .mulf_man_add
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_man_continue
 MMA
 IXR
 LIX .mulf_man_continue
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman1
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_man_continue
 MMA
 IXR
 LIX .mulf_man_continue
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_man_continue
 MMA
 IXR
 LIX .mulf_man_continue
 MMA
 JNE

 IXN
 LIX .mulf_normalize
 MMA
 IXR
 LIX .mulf_normalize
 MMA
 JMP

.mulf_man_continue:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .aman2
 MMA
 LDA
 SHL
 MAC
 STA

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .aman1
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_aman_lshift_carry1
 MMA
 IXR
 LIX .mulf_aman_lshift_carry1
 MMA
 JNE

.mulf_aman_lshift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .aman
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_aman_lshift_carry0
 MMA
 IXR
 LIX .mulf_aman_lshift_carry0
 MMA
 JNE

.mulf_aman_lshift_carry0_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_1
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_aman_lshift_carry_1
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_1
 MMA
 JNE

.mulf_aman_lshift_carry_1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_2
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_aman_lshift_carry_2
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_2
 MMA
 JNE

.mulf_aman_lshift_carry_2_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_3
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR

 IXN
 LIX .mulf_aman_lshift_carry_3
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_3
 MMA
 JNE

.mulf_aman_lshift_carry_3_ret:
 IXN
 LIX .mulf_mantissas
 MMA
 IXR
 LIX .mulf_mantissas
 MMA
 JMP

.mulf_man_add:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .aman2
 MMA
 LDA
 MBA
 LIX .mulman5
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIX .aman1
 MMA
 LDA
 MBA
 LIX .mulman4
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIX .aman
 MMA
 LDA
 MBA
 LIX .mulman3
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_1
 MMA
 LDA
 MBA
 LIX .mulman2
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_2
 MMA
 LDA
 MBA
 LIX .mulman1
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIX .mulaman_3
 MMA
 LDA
 MBA
 LIX .mulman
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 IXN
 LIX .mulf_man_continue
 MMA
 IXR
 LIX .mulf_man_continue
 MMA
 JMP


.mulf_aman_lshift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .aman1
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA

 IXN
 LIX .mulf_aman_lshift_carry1_ret
 MMA
 IXR
 LIX .mulf_aman_lshift_carry1_ret
 MMA
 JMP

.mulf_aman_lshift_carry0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .aman
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA

 IXN
 LIX .mulf_aman_lshift_carry0_ret
 MMA
 IXR
 LIX .mulf_aman_lshift_carry0_ret
 MMA
 JMP

.mulf_aman_lshift_carry_1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .mulaman_1
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA

 IXN
 LIX .mulf_aman_lshift_carry_1_ret
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_1_ret
 MMA
 JMP

.mulf_aman_lshift_carry_2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .mulaman_2
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA

 IXN
 LIX .mulf_aman_lshift_carry_2_ret
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_2_ret
 MMA
 JMP

.mulf_aman_lshift_carry_3:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .mulaman_3
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA

 IXN
 LIX .mulf_aman_lshift_carry_3_ret
 MMA
 IXR
 LIX .mulf_aman_lshift_carry_3_ret
 MMA
 JMP


.mulf_bman_shift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .bman1
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA

 IXN
 LIX .mulf_bman_shift_carry1_ret
 MMA
 IXR
 LIX .mulf_bman_shift_carry1_ret
 MMA
 JMP

.mulf_bman_shift_carry2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA

 IXN
 LIX .mulf_bman_shift_carry2_ret
 MMA
 IXR
 LIX .mulf_bman_shift_carry2_ret
 MMA
 JMP

.mulf_normalize:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x8
 MBA
 LIX .mulman
 MMA
 LDA
 AND

 IXN
 LIX .mulf_normalize_exp
 MMA
 IXR
 LIX .mulf_normalize_exp
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .mulman3
 MMA
 LDA
 SHL
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA

 LIX .mulman2
 MMA
 LDA
 SHL
 MAC
 OR
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA

 LIX .mulman1
 MMA
 LDA
 SHL
 MAC
 OR
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA

 LIX .mulman
 MMA
 LDA
 SHL
 MAC
 OR
 MAC
 STA

.mulf_normalize_exp_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .mulman3
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 AND
 IXN
 LIX .mulf_normalize_round
 MMA
 IXR 
 LIX .mulf_normalize_round
 MMA
 JNE
 
.mulf_normalize_round_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .bsgn
 MMA
 LDA
 MBA
 LIX .asgn
 MMA
 LDA
 XOR
 LIX .csgn
 MMA
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 LIX .mulman
 MMA
 LDA
 OR
 LIX .cman
 MMA
 MAC
 STA

 LIX .mulman1
 MMA
 LDA
 OR
 LIX .cman1
 MMA
 MAC
 STA

 LIX .mulman2
 MMA
 LDA
 OR
 LIX .cman2
 MMA
 MAC
 STA

 IXN
 LIX .mulf_ret_c
 MMA
 IXR
 LIX .mulf_ret_c
 MMA
 JMP

.mulf_normalize_exp:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 CLC
 LIL 0x1
 LIH 0x0
 MBA
 LIX .cexp
 MMA
 LDA
 ADL
 ADH

 MAC
 STA
 NOT

 IXN
 LIX .mulf_normalize_exp_ret
 MMA
 IXR
 LIX .mulf_normalize_exp_ret
 MMA
 JNE

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JMP

.mulf_normalize_round:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR

 LIX .mulman2
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA

 LIX .mulman1
 MMA
 LDA
 ADL
 ADH
 MAC
 STA
 
 LIX .mulman
 MMA
 LDA
 ADL
 ADH
 MAC
 STA

 LIL 0x0
 LIH 0x0
 ADL
 ADH
 
 IXN
 LIX .mulf_normalize_round_up
 MMA
 IXR
 LIX .mulf_normalize_round_up
 MMA
 JNE

 IXN
 LIX .mulf_normalize_round_ret
 MMA
 IXR
 LIX .mulf_normalize_round_ret
 MMA
 JMP

.mulf_normalize_round_up:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 
 LIX .mulman
 MMA
 LIL 0x0
 LIH 0x8
 STA

 CLC
 LIL 0x1
 LIH 0x0
 MBA
 LIX .cexp
 MMA
 LDA
 ADL
 ADH

 MAC
 STA
 NOT

 IXN
 LIX .mulf_normalize_round_ret
 MMA
 IXR
 LIX .mulf_normalize_round_ret
 MMA
 JNE

 IXN
 LIX .mulf_ret_inf
 MMA
 IXR
 LIX .mulf_ret_inf
 MMA
 JMP

.mulf_ret0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LDA
 MBA
 LIX .bsgn
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret_nan_sgn1
 MMA
 IXR
 LIX .mulf_ret_nan_sgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA

.mulf_ret_nan_sgn1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x0
 OR
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_ret_nan_sgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA

 IXN
 LIX .mulf_ret_nan_sgn1_ret
 MMA
 IXR
 LIX .mulf_ret_nan_sgn1_ret
 MMA
 JMP

.mulf_reta:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_retb:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xB
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xC
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xD
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_ret_inf:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .asgn
 MMA
 LDA
 MBA
 LIX .bsgn
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_ret_inf_sgn1
 MMA
 IXR
 LIX .mulf_ret_inf_sgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA

.mulf_ret_inf_sgn1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0x7
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_ret_inf_sgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA

 IXN
 LIX .mulf_ret_inf_sgn1_ret
 MMA
 IXR
 LIX .mulf_ret_inf_sgn1_ret
 MMA
 JMP

.mulf_ret_c:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIX .csgn
 MMA
 LDA
 OR

 IXN
 LIX .mulf_move_sgn1
 MMA
 IXR
 LIX .mulf_move_sgn1
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA

.mulf_move_sgn1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .cexp
 MMA
 LDA
 XOR

 IXN
 LIX .mulf_move_not_inf
 MMA
 IXR
 LIX .mulf_move_not_inf
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0x7
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP

.mulf_move_not_inf:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIX .cexp
 MMA
 LDA
 SHR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 MBA
 LDA
 OR
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH

 IXN
 LIX .mulf_move_exp1
 MMA
 IXR
 LIX .mulf_move_exp1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA


.mulf_move_exp1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0xF
 LIH 0x7
 MBA
 LIX .cman
 MMA
 LDA
 AND
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 MBA
 LDA
 OR
 MAC
 STA

 LIL 0x0
 LIH 0x0
 MBA
 LIX .cman1
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA

 LIX .cman2
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA

 IXN
 LIX .mulf_end
 MMA
 IXR
 LIX .mulf_end
 MMA
 JMP


.mulf_move_sgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA

 IXN
 LIX .mulf_move_sgn1_ret
 MMA
 IXR
 LIX .mulf_move_sgn1_ret
 MMA
 JMP

.mulf_move_exp1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA

 IXN
 LIX .mulf_move_exp1_ret
 MMA
 IXR
 LIX .mulf_move_exp1_ret
 MMA
 JMP

.mulf_end:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 MAC
 STA
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 LDA
 IXN
 MMA
 IXR
 MAC
 MMA
 JMP
)";

static constexpr std::string_view risc__addf3 =
 R"(
.addf:
 IXN
 LIX .addf_isa0
 MMA
 IXR
 LIX .addf_isa0
 MMA
 JMP

.addf_isa0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_isb0
 MMA
 IXR
 LIX .addf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_isb0
 MMA
 IXR
 LIX .addf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x8
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_isb0
 MMA
 IXR
 LIX .addf_isb0
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_isb0
 MMA
 IXR
 LIX .addf_isb0
 MMA
 JNE

 IXN
 LIX .addf_retb
 MMA
 IXR
 LIX .addf_retb
 MMA
 JMP

.addf_isb0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_extraction 
 MMA
 IXR
 LIX .addf_extraction
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIL 0xB
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_extraction
 MMA
 IXR
 LIX .addf_extraction
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIL 0xC
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_extraction 
 MMA
 IXR
 LIX .addf_extraction
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIL 0xD
 LIH 0x3
 MMA
 LDA
 OR

 IXN
 LIX .addf_extraction
 MMA
 IXR
 LIX .addf_extraction
 MMA
 JNE

 IXN
 LIX .addf_reta 
 MMA
 IXR
 LIX .addf_reta
 MMA
 JMP

.addf_extraction:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0x0
 LIH 0x8
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 AND

 IXN
 LIX .addf_asgn1 
 MMA
 IXR
 LIX .addf_asgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LIL 0x0
 LIH 0x0
 STA

.addf_rasgn1:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0xA
 LIH 0x3
 MMA
 LDA
 AND 

 IXN
 LIX .addf_bsgn1
 MMA
 IXR
 LIX .addf_bsgn1
 MMA
 JNE 

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bsgn
 MMA
 LIL 0x0
 LIH 0x0
 STA 

.addf_rbsgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR 

 LIL 0x7
 LIH 0x3
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 SHL
 MAC
 CLC
 OR
 LIX .aexp
 MMA
 MAC
 STA 

 LIL 0xB
 LIH 0x3
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 MBA
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 SHL
 MAC
 CLC
 OR
 LIX .bexp
 MMA
 MAC
 STA 

 LIL 0x0
 LIH 0x8
 MBA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman
 MMA
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0x8
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman1
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR
 LIX .aman2
 MMA
 MAC
 STA 

 LIL 0x0
 LIH 0x8
 MBA
 LIL 0xB
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman
 MMA
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0xC
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman1
 MMA
 MAC
 STA
 LIL 0xD
 LIH 0x3
 MMA
 LDA
 OR
 LIX .bman2
 MMA
 MAC
 STA 

 IXN
 LIX .addf_exceptions
 MMA
 IXR
 LIX .addf_exceptions
 MMA
 JMP 

.addf_asgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR 

 LIX .asgn
 MMA
 LIL 0x1
 LIH 0x0
 STA 

 IXN
 LIX .addf_rasgn1 
 MMA
 IXR
 LIX .addf_rasgn1
 MMA
 JMP

.addf_bsgn1:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIX .bsgn
 MMA
 LIL 0x1
 LIH 0x0
 STA

 IXN
 LIX .addf_rbsgn1
 MMA
 IXR
 LIX .addf_rbsgn1
 MMA
 JMP

.addf_exceptions:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .aexp
 MMA
 LDA
 XOR

 IXN
 LIX .addf_isbnan 
 MMA
 IXR
 LIX .addf_isbnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0x0
 LIH 0x8
 MBA
 LIX .aman
 MMA
 LDA
 XOR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .aman1
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIX .aman2
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIX .addf_ainf
 MMA
 IXR
 LIX .addf_ainf
 MMA
 JNE

.addf_ainf:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 IXR
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .addf_reta
 MMA
 IXR
 LIX .addf_reta
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .bman1
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan 
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .asgn
 MMA
 LDA
 MBA
 LIX .bsgn
 MMA
 LDA
 XOR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIX .addf_reta
 MMA
 IXR
 LIX .addf_reta
 MMA
 JMP

.addf_isbnan:
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR

 LIL 0xF
 LIH 0xF
 MBA
 LIX .bexp
 MMA
 LDA
 XOR

 IXN
 LIX .addf_compare_exp
 MMA
 IXR
 LIX .addf_compare_exp
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x8
 MBA
 LIX .bman
 MMA
 LDA
 XOR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 MBA
 LIX .bman1
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 OR

 IXN
 LIX .addf_retnan
 MMA
 IXR
 LIX .addf_retnan
 MMA
 JNE

 IXN
 LIX .addf_retb
 MMA
 IXR
 LIX .addf_retb
 MMA
 JMP

.addf_retnan:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0x7
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0xF
 LIH 0xF
 STA

 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP

.addf_reta:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x8
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA

 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP

.addf_retb:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xB
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xC
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0xD
 LIH 0x3
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_end 
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP
 
.addf_compare_exp:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bexp
 MMA
 LDA
 MBA
 LIX .aexp
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_exp_not_equal
 MMA
 IXR
 LIX .addf_exp_not_equal
 MMA
 JNE
 
 IXN
 LIX .addf_exp_equal
 MMA
 IXR
 LIX .addf_exp_equal
 MMA
 JMP
 
.addf_exp_not_equal:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 MAC
 STA
 LIX .compare_b
 MMA
 LIL 0x0
 LIH 0x0
 OR
 MAC
 STA
 
.addf_exp_not_equal_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 LDA
 CLC
 SHL
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_exp_not_equal_check
 MMA
 IXR
 LIX .addf_exp_not_equal_check
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0 
 MMA
 IXR
 LIX .compare_b
 MMA
 LDA
 SHL
 MAC
 STA
 
 IXN
 LIX .addf_exp_not_equal_loop
 MMA
 IXR
 LIX .addf_exp_not_equal_loop
 MMA
 JMP
 
.addf_exp_not_equal_check:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_b
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_higher_b_exp
 MMA
 IXR
 LIX .addf_higher_b_exp
 MMA
 JNE
 
 IXN
 LIX .addf_higher_a_exp
 MMA
 IXR
 LIX .addf_higher_a_exp
 MMA
 JMP
 
.addf_higher_a_exp:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .aexp
 MMA
 LDA
 OR
 LIX .cexp
 MMA
 MAC
 STA
 
 LIX .bexp
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .aexp
 MMA
 LDA
 SEC
 ADL
 ADH
 LIX .exp_diff
 MMA
 MAC
 STA
 
.addf_higher_a_exp_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bman
 MMA
 LDA
 SHR
 MAC
 STA
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .bman1
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_bman_shift_carry1
 MMA
 IXR
 LIX .addf_bman_shift_carry1
 MMA
 JNE
 
.addf_bman_shift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .bman2
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_bman_shift_carry2
 MMA
 IXR
 LIX .addf_bman_shift_carry2
 MMA
 JNE
 
.addf_bman_shift_carry2_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bexp
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 CLC
 ADL
 ADH
 MAC
 STA
 LIX .exp_diff
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 NOT
 MAC
 SEC
 ADL
 ADH
 MAC
 STA
 
 IXN
 LIX .addf_higher_a_exp_loop
 MMA
 IXR
 LIX .addf_higher_a_exp_loop
 MMA
 JNE
 
 IXN
 LIX .addf_exp_equal
 MMA
 IXR
 LIX .addf_exp_equal
 MMA
 JMP
 
.addf_bman_shift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bman1
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_bman_shift_carry1_ret
 MMA
 IXR
 LIX .addf_bman_shift_carry1_ret
 MMA
 JMP
 
.addf_bman_shift_carry2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bman2
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_bman_shift_carry2_ret
 MMA
 IXR
 LIX .addf_bman_shift_carry2_ret
 MMA
 JMP
 
.addf_higher_b_exp:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bexp
 MMA
 LDA
 OR
 LIX .cexp
 MMA
 MAC
 STA
 
 LIX .aexp
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .bexp
 MMA
 LDA
 SEC
 ADL
 ADH
 LIX .exp_diff
 MMA
 MAC
 STA
 
.addf_higher_b_exp_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .aman
 MMA
 LDA
 SHR
 MAC
 STA
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .aman1
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_aman_shift_carry1
 MMA
 IXR
 LIX .addf_aman_shift_carry1
 MMA
 JNE
 
.addf_aman_shift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .aman2
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_aman_shift_carry2
 MMA
 IXR
 LIX .addf_aman_shift_carry2
 MMA
 JNE
 
.addf_aman_shift_carry2_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .aexp
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 CLC
 ADL
 ADH
 MAC
 STA
 LIX .exp_diff
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 NOT
 MAC
 SEC
 ADL
 ADH
 MAC
 STA
 
 IXN
 LIX .addf_higher_b_exp_loop
 MMA
 IXR
 LIX .addf_higher_b_exp_loop
 MMA
 JNE
 
 IXN
 LIX .addf_exp_equal
 MMA
 IXR
 LIX .addf_exp_equal
 MMA
 JMP
 
.addf_aman_shift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .aman1
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_aman_shift_carry1_ret
 MMA
 IXR
 LIX .addf_aman_shift_carry1_ret
 MMA
 JMP
 
.addf_aman_shift_carry2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .aman2
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_aman_shift_carry2_ret 
 MMA
 IXR
 LIX .addf_aman_shift_carry2_ret
 MMA
 JMP
 
.addf_exp_equal:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIX .aexp
 MMA
 LDA
 OR
 LIX .cexp
 MMA
 MAC
 STA
 LIX .bsgn
 MMA
 LDA
 MBA
 LIX .asgn
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_compare_man
 MMA
 IXR
 LIX .addf_compare_man
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x0
 LIH 0x0
 OR
 LIX .csgn
 MMA
 MAC
 STA
 
.addf_add_man:
 CLC
 LIX .bman2
 MMA
 LDA
 MBA
 LIX .aman2
 MMA
 LDA
 ADL
 ADH
 LIX .cman2
 MMA
 MAC
 STA
 
 LIX .bman1
 MMA
 LDA
 MBA
 LIX .aman1
 MMA
 LDA
 ADL
 ADH
 LIX .cman1
 MMA
 MAC
 STA
 
 LIX .bman
 MMA
 LDA
 MBA
 LIX .aman
 MMA
 LDA
 ADL
 ADH
 LIX .cman
 MMA
 MAC
 STA
 
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_normalize_too_big
 MMA
 IXR
 LIX .addf_normalize_too_big
 MMA
 JNE
 
 IXN
 LIX .addf_normalize
 MMA
 IXR
 LIX .addf_normalize
 MMA
 JMP
 
.addf_compare_man:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .bman
 MMA
 LDA
 MBA
 
 LIX .aman
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_compare_man_not_equal_0
 MMA
 IXR
 LIX .addf_compare_man_not_equal_0
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman1
 MMA
 LDA
 MBA
 
 LIX .aman1
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_compare_man_not_equal_1
 MMA
 IXR
 LIX .addf_compare_man_not_equal_1
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .bman2
 MMA
 LDA
 MBA
 
 LIX .aman2
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_compare_man_not_equal_2
 MMA
 IXR
 LIX .addf_compare_man_not_equal_2
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .csgn
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 LIX .cexp
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 LIX .cman
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 LIX .cman1
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 LIX .cman2
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 IXN
 LIX .addf_move_c
 MMA
 IXR
 LIX .addf_move_c
 MMA
 JMP
 
.addf_lowerbman:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIX .asgn
 MMA
 LDA
 OR
 LIX .csgn
 MMA
 MAC
 STA
 
 LIX .bman2
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .aman2
 MMA
 LDA
 SEC
 ADL
 ADH
 LIX .cman2
 MMA
 MAC
 STA
 
 LIX .bman1
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .aman1
 MMA
 LDA
 ADL
 ADH
 LIX .cman1
 MMA
 MAC
 STA
 
 LIX .bman
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .aman
 MMA
 LDA
 ADL
 ADH
 LIX .cman
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_normalize
 MMA
 IXR
 LIX .addf_normalize
 MMA
 JMP
 
 
.addf_higherbman:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIX .bsgn
 MMA
 LDA
 OR
 LIX .csgn
 MMA
 MAC
 STA
 
 LIX .aman2
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .bman2
 MMA
 LDA
 SEC
 ADL
 ADH
 LIX .cman2
 MMA
 MAC
 STA
 
 LIX .aman1
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .bman1
 MMA
 LDA
 ADL
 ADH
 LIX .cman1
 MMA
 MAC
 STA
 
 LIX .aman
 MMA
 LDA
 NOT
 MAC
 MBA
 LIX .bman
 MMA
 LDA
 ADL
 ADH
 LIX .cman
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_normalize
 MMA
 IXR
 LIX .addf_normalize
 MMA
 JMP
 
.addf_compare_man_not_equal_0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 MAC
 STA
 LIX .compare_b
 MMA
 LIL 0x0
 LIH 0x0
 OR
 MAC
 STA
 
.addf_compare_man_not_equal_0_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 LDA
 CLC
 SHL
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_compare_man_not_equal_0_check
 MMA
 IXR
 LIX .addf_compare_man_not_equal_0_check
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .compare_b
 MMA
 LDA
 SHL
 MAC
 STA
 
 IXN
 LIX .addf_compare_man_not_equal_0_loop
 MMA
 IXR
 LIX .addf_compare_man_not_equal_0_loop
 MMA
 JMP
 
.addf_compare_man_not_equal_0_check:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_b
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_higherbman
 MMA
 IXR
 LIX .addf_higherbman
 MMA
 JNE
 
 IXN
 LIX .addf_lowerbman
 MMA
 IXR
 LIX .addf_lowerbman
 MMA
 JMP
 
.addf_compare_man_not_equal_1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 MAC
 STA
 LIX .compare_b
 MMA
 LIL 0x0
 LIH 0x0
 OR
 MAC
 STA
 
.addf_compare_man_not_equal_1_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 LDA
 CLC
 SHL
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_compare_man_not_equal_1_check
 MMA
 IXR
 LIX .addf_compare_man_not_equal_1_check
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .compare_b
 MMA
 LDA
 SHL
 MAC
 STA
 
 IXN
 LIX .addf_compare_man_not_equal_1_loop
 MMA
 IXR
 LIX .addf_compare_man_not_equal_1_loop
 MMA
 JMP
 
.addf_compare_man_not_equal_1_check:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_b
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_higherbman
 MMA
 IXR
 LIX .addf_higherbman
 MMA
 JNE
 
 IXN
 LIX .addf_lowerbman
 MMA
 IXR
 LIX .addf_lowerbman
 MMA
 JMP
 
.addf_compare_man_not_equal_2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 MAC
 STA
 LIX .compare_b
 MMA
 LIL 0x0
 LIH 0x0
 OR
 MAC
 STA
 
.addf_compare_man_not_equal_2_loop:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_diff
 MMA
 LDA
 CLC
 SHL
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_compare_man_not_equal_2_check
 MMA
 IXR
 LIX .addf_compare_man_not_equal_2_check
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .compare_b
 MMA
 LDA
 SHL
 MAC
 STA
 
 IXN
 LIX .addf_compare_man_not_equal_2_loop
 MMA
 IXR
 LIX .addf_compare_man_not_equal_2_loop
 MMA
 JMP
 
.addf_compare_man_not_equal_2_check:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .compare_b
 MMA
 LDA
 SHL
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_higherbman
 MMA
 IXR
 LIX .addf_higherbman
 MMA
 JNE
 
 IXN
 LIX .addf_lowerbman
 MMA
 IXR
 LIX .addf_lowerbman
 MMA
 JMP
 
.addf_normalize_too_big:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x8
 MBA
 LIX .cman
 MMA
 LDA
 SHR
 MAC
 OR
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .cman1
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_cman_shift_carry1
 MMA
 IXR
 LIX .addf_cman_shift_carry1
 MMA
 JNE
 
.addf_cman_shift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .cman2
 MMA
 LDA
 SHR
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_cman_shift_carry2
 MMA
 IXR
 LIX .addf_cman_shift_carry2
 MMA
 JNE
 
.addf_cman_shift_carry2_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x1
 LIH 0x0
 MBA
 CLC
 
 LIX .cexp
 MMA
 LDA
 ADL
 ADH
 MAC
 STA
 
 IXN
 LIX .addf_move_c
 MMA
 IXR
 LIX .addf_move_c
 MMA
 JMP
 
.addf_cman_shift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cman1
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_cman_shift_carry1_ret
 MMA
 IXR
 LIX .addf_cman_shift_carry1_ret
 MMA
 JMP
 
.addf_cman_shift_carry2:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cman2
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA
 
 IXN
 LIX .addf_cman_shift_carry2_ret
 MMA
 IXR
 LIX .addf_cman_shift_carry2_ret
 MMA
 JMP
 
.addf_normalize:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x8
 MBA
 
 LIX .cman
 MMA
 LDA
 AND
 
 IXN
 LIX .addf_move_c
 MMA
 IXR
 LIX .addf_move_c
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIX .cexp
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 NOT
 MAC
 SEC
 ADL
 ADH
 MAC
 STA
 MBA
 LIL 0x0
 LIH 0x0
 XOR
 
 IXN
 LIX .addf_normalize_not_0
 MMA
 IXR
 LIX .addf_normalize_not_0
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0x1
 LIH 0x0
 MBA
 LIX .csgn
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_normalize_plus_0
 MMA
 IXR
 LIX .addf_normalize_plus_0
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x8
 MBA
 OR
 
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x0
 LIH 0x0
 MBA
 OR
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP
 
.addf_normalize_plus_0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x0
 MBA
 OR
 
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP
 
.addf_normalize_not_0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cman2
 MMA
 LDA
 SHL
 MAC
 STA
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .cman1
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_cman_lshift_carry1
 MMA
 IXR
 LIX .addf_cman_lshift_carry1
 MMA
 JNE
 
.addf_cman_lshift_carry1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .shift_carry
 MMA
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 MAC
 STA
 
 LIX .cman
 MMA
 LDA
 SHL
 MAC
 STA
 LIX .shift_carry
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_cman_lshift_carry0
 MMA
 IXR
 LIX .addf_cman_lshift_carry0
 MMA
 JNE
 
.addf_cman_lshift_carry0_ret:
 IXN
 LIX .addf_normalize
 MMA
 IXR
 LIX .addf_normalize
 MMA
 JMP
 
 
.addf_cman_lshift_carry0:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cman
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA
 
 IXN
 LIX .addf_cman_lshift_carry0_ret
 MMA
 IXR
 LIX .addf_cman_lshift_carry0_ret
 MMA
 JMP
 
.addf_cman_lshift_carry1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cman1
 MMA
 LDA
 MBA
 LIL 0x1
 LIH 0x0
 OR
 MAC
 STA
 
 IXN
 LIX .addf_cman_lshift_carry1_ret
 MMA
 IXR
 LIX .addf_cman_lshift_carry1_ret
 MMA
 JMP
 
.addf_move_c:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x0
 LIH 0x0
 MBA
 CLC
 LIX .csgn
 MMA
 LDA
 OR
 
 IXN
 LIX .addf_move_sgn1
 MMA
 IXR
 LIX .addf_move_sgn1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA  

.addf_move_sgn1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0xF
 LIH 0xF
 MBA
 LIX .cexp
 MMA
 LDA
 XOR
 
 IXN
 LIX .addf_move_not_inf
 MMA
 IXR
 LIX .addf_move_not_inf
 MMA
 JNE
 
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0xF
 LIH 0x7
 MBA
 LIL 0x6
 LIH 0x3
 MMA
 LDA
 OR
 MAC
 STA
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA
 
 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP
 
.addf_move_not_inf:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIX .cexp
 MMA
 LDA
 SHR
 LIL 0x6
 LIH 0x3
 MMA
 MAC
 MBA
 LDA
 OR
 MAC
 STA
 
 LIL 0x0
 LIH 0x0
 MBA
 ADL
 ADH
 
 IXN
 LIX .addf_move_exp1
 MMA
 IXR
 LIX .addf_move_exp1
 MMA
 JNE

 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
.addf_move_exp1_ret:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR

 LIL 0x8
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 LIL 0x9
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x0
 STA
 
 
 LIL 0xF
 LIH 0x7
 MBA
 LIX .cman
 MMA
 LDA
 AND
 LIL 0x7
 LIH 0x3
 MMA
 MAC
 MBA
 LDA
 OR
 MAC
 STA
 
 LIL 0x0
 LIH 0x0
 MBA
 LIX .cman1
 MMA
 LDA
 OR
 LIL 0x8
 LIH 0x3
 MMA
 MAC
 STA
 
 LIX .cman2
 MMA
 LDA
 OR
 LIL 0x9
 LIH 0x3
 MMA
 MAC
 STA
 
 IXN
 LIX .addf_end
 MMA
 IXR
 LIX .addf_end
 MMA
 JMP
 
.addf_move_sgn1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x6
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA
 
 IXN
 LIX .addf_move_sgn1_ret
 MMA
 IXR
 LIX .addf_move_sgn1_ret
 MMA
 JMP
 
.addf_move_exp1:
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 
 LIL 0x7
 LIH 0x3
 MMA
 LIL 0x0
 LIH 0x8
 STA
 
 IXN
 LIX .addf_move_exp1_ret
 MMA
 IXR
 LIX .addf_move_exp1_ret
 MMA
 JMP
 
.addf_end:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 MAC
 STA
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 LDA
 IXN
 MMA
 IXR
 MAC
 MMA
 JMP
)";


static constexpr std::string_view risc__subf3 =
 R"(
.subf:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 MBA
 LIL 0xF
 LIH 0xF
 CLC
 ADL
 ADH
 MAC
 STA
 MMA
     
 LIX .subf_end
 STA
      
 LIL 0x0
 LIH 0x0
 MBA
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 MBA
 LIL 0xF
 LIH 0xF
 CLC
 ADL
 ADH
 MAC
 STA
 MMA 

 IXN
 LIX .subf_end
 STA
      
 IXR
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 AND
 
 IXN
 LIX .subf_sgn
 MMA
 IXR
 LIX .subf_sgn
 MMA
 JNE

 IXR
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 MBA
 LIL 0x0
 LIH 0x8
 OR
 MAC
 STA

.subf_sgn_ret:
 IXN
 LIX .addf
 MMA
 IXR
 LIX .addf
 MMA
 JMP

.subf_sgn:
 IXR
 IXN
 LIL 0x0
 LIH 0x0
 MMA
 IXR
 LIL 0xA
 LIH 0x3
 MMA
 LDA
 MBA
 LIL 0xF
 LIH 0x7
 AND
 MAC
 STA

 IXN
 LIX .subf_sgn_ret
 MMA
 IXR
 LIX .subf_sgn_ret
 MMA
 JMP

.subf_end:
 IXN
 LIL 0x0
 LIH 0x0
 MBA
 MMA
 IXR
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA

 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 MAC
 STA
 LIL 0xE
 LIH 0xF
 MMA
 LDA
 SEC
 ADL
 ADH
 MAC
 STA
 MMA
 LDA
 OR
 LIX .rcall_addr
 MMA
 LDA
 IXN
 MMA
 IXR
 MAC
 MMA
 JMP

)";
