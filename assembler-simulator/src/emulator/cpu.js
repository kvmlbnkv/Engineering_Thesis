app.service('cpu', ['opcodes', 'memory', function(opcodes, memory) {
    var cpu = {
        step: function() {
            var self = this;

            if (self.fault === true) {
                throw "FAULT. Reset to continue.";
            }

            try {
                var checkGPR = function(reg) {
                    if (reg < 0 || reg >= self.gpr.length) {
                        throw "Invalid register: " + reg;
                    } else {
                        return reg;
                    }
                };

                var checkGPR_SP = function(reg) {
                    if (reg < 0 || reg >= 1 + self.gpr.length) {
                        throw "Invalid register: " + reg;
                    } else {
                        return reg;
                    }
                };

                var setGPR_SP = function(reg,value)
                {
                    if(reg >= 0 && reg < self.gpr.length) {
                        self.gpr[reg] = value;
                    } else if(reg == self.gpr.length) {
                        self.sp = value;

                        // Not likely to happen, since we always get here after checkOpertion().
                        if (self.sp < self.minSP) {
                            throw "Stack overflow";
                        } else if (self.sp > self.maxSP) {
                            throw "Stack underflow";
                        }
                    } else {
                        throw "Invalid register: " + reg;
                    }
                };

                var getGPR_SP = function(reg)
                {
                    if(reg >= 0 && reg < self.gpr.length) {
                        return self.gpr[reg];
                    } else if(reg == self.gpr.length) {
                        return self.sp;
                    } else {
                        throw "Invalid register: " + reg;
                    }
                };

                var indirectRegisterAddress = function(value) {
                    var reg = value % 8;
                    
                    var base;
                    if (reg < self.gpr.length) {
                        base = self.gpr[reg];
                    } else {
                        base = self.sp;
                    }
                    
                    var offset = Math.floor(value / 8);
                    if ( offset > 15 ) {
                        offset = offset - 32;
                    }
                    
                    return base+offset;
                };

                var checkOperation = function(value) {
                    self.zero = false;
                    self.carry = false;

                    if (value >= 256) {
                        self.carry = true;
                        value = value % 256;
                    } else if (value === 0) {
                        self.zero = true;
                    } else if (value < 0) {
                        self.carry = true;
                        value = 256 - (-value) % 256;
                    }

                    return value;
                };

                var jump = function(newIP) {
                    if (newIP-1 < 0 || newIP >= memory.data.length) {
                        throw "IP outside memory";
                    } else {
                        self.pc = newIP-1;
                    }
                };

                var push = function(value) {
                    memory.store(self.sp--, value);
                    if (self.sp < self.minSP) {
                        throw "Stack overflow";
                    }
                };

                var pop = function() {
                    var value = memory.load(++self.sp);
                    if (self.sp > self.maxSP) {
                        throw "Stack underflow";
                    }

                    return value;
                };

                var division = function(divisor) {
                    if (divisor === 0) {
                        throw "Division by 0";
                    }

                    return Math.floor(self.gpr[0] / divisor);
                };

                if (self.pc < 0 || self.pc >= memory.data.length) {
                    throw "Instruction pointer is outside of memory";
                }
                
                var instr = memory.load(self.pc);
                switch(instr) {
                    case opcodes.HLT:
                        return false;
                    case opcodes.NOP:
                        self.pc++;
                        break;
                    case opcodes.ADL:
                        setGPR_SP(checkGPR_SP(2),((getGPR_SP(2) & 0xF0)+(getGPR_SP(0) & 0x0F)+(getGPR_SP(1) & 0x0F)+self.carry) & 0xFF);
                        if ((getGPR_SP(0) & 0x0F)+(getGPR_SP(1) & 0x0F)+self.carry > 15){
                            self.carry = true;
                        }
                        else{
                            self.carry = false;
                        }
                        self.pc++;
                        break;
                    case opcodes.ADH:
                        setGPR_SP(checkGPR_SP(2),(getGPR_SP(2) & 0x0F)+((getGPR_SP(0) & 0xF0)+((getGPR_SP(1) & 0xF0)+(self.carry << 4))) & 0xFF);
                        if (((getGPR_SP(0) & 0xF0) >> 4)+((getGPR_SP(1) & 0xF0) >> 4)+self.carry > 15){
                            self.carry = true;
                        }
                        else{
                            self.carry = false;
                        }
                        self.pc++;
                        break;
                    case opcodes.AND:
                        setGPR_SP(checkGPR_SP(2), getGPR_SP(0) & getGPR_SP(1));
                        self.pc++;
                        break;
                    case opcodes.OR:
                        setGPR_SP(checkGPR_SP(2), getGPR_SP(0) | getGPR_SP(1));
                        self.pc++;
                        break;
                    case opcodes.XOR:
                        setGPR_SP(checkGPR_SP(2), getGPR_SP(0) ^ getGPR_SP(1));
                        self.pc++;
                        break;
                    case opcodes.NOT:
                        setGPR_SP(checkGPR_SP(2), ((~getGPR_SP(0))>>>0) & 0xFF);
                        self.pc++;
                        break;
                    case opcodes.CLC:
                        self.carry = false;
                        self.pc++;
                        break;
                    case opcodes.SEC:
                        self.carry = true;
                        self.pc++;
                        break;
                    case opcodes.SHL:
                        setGPR_SP(checkGPR_SP(2), (getGPR_SP(0) << 1) & 0xFF);
                        self.carry = getGPR_SP(0) >>> 7 == 1 ? true : false;
                        self.pc++;
                        break;
                    case opcodes.SHR:
                        setGPR_SP(checkGPR_SP(2), getGPR_SP(0) >>> 1);
                        self.carry = getGPR_SP(0) & 0x01 == 1 ? true : false;
                        self.pc++;
                        break;
                    case opcodes.JMP:
                        jump(getGPR_SP(3));
                        self.pc++;
                        break;
                    case opcodes.JNE:
                        if (getGPR_SP(2) !== 0){
                            jump(getGPR_SP(3));
                        }
                        self.pc++;
                        break;
                    case opcodes.LDA:
                        setGPR_SP(checkGPR_SP(0), memory.load(getGPR_SP(3)));
                        self.pc++;
                        break;
                    case opcodes.STA:
                        memory.store(getGPR_SP(3), getGPR_SP(0));
                        self.pc++;
                        break;
                    case opcodes.MAC:
                        setGPR_SP(checkGPR_SP(0), getGPR_SP(2));
                        self.pc++;
                        break;
                    case opcodes.MBA:
                        setGPR_SP(checkGPR_SP(1), getGPR_SP(0));
                        self.pc++;
                        break;
                    case opcodes.MAM:
                        setGPR_SP(checkGPR_SP(0), getGPR_SP(3));
                        self.pc++;
                        break;
                    case opcodes.MMA:
                        setGPR_SP(checkGPR_SP(3), getGPR_SP(0));
                        self.pc++;
                        break;
                    case opcodes.MAP:
                        setGPR_SP(checkGPR_SP(0), self.pc);
                        self.pc++;
                        break;
                    case opcodes.MGM:
                        self.pc++;
                        break;
                    case opcodes.MLG:
                        self.pc++;
                        break;
                    case opcodes.MGL:
                        self.pc++;
                        break;
                    case opcodes.IXR:
                        self.pc++;
                        break;
                    case opcodes.IXN:
                        self.pc++;
                        break;
                    case opcodes.LIL_0:
                        setGPR_SP(checkGPR_SP(0), (getGPR_SP(0) & 0xF0)+0);
                        self.pc++;
                        break;
                    case opcodes.LIL_1:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+1));
                        self.pc++;
                        break;
                    case opcodes.LIL_2:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+2));
                        self.pc++;
                        break;
                    case opcodes.LIL_3:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+3));
                        self.pc++;
                        break;
                    case opcodes.LIL_4:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+4));
                        self.pc++;
                        break;
                    case opcodes.LIL_5:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+5));
                        self.pc++;
                        break;
                    case opcodes.LIL_6:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+6));
                        self.pc++;
                        break;
                    case opcodes.LIL_7:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+7));
                        self.pc++;
                        break;
                    case opcodes.LIL_8:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+8));
                        self.pc++;
                        break;
                    case opcodes.LIL_9:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+9));
                        self.pc++;
                        break;
                    case opcodes.LIL_A:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+10));
                        self.pc++;
                        break;
                    case opcodes.LIL_B:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+11));
                        self.pc++;
                        break;
                    case opcodes.LIL_C:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+12));
                        self.pc++;
                        break;
                    case opcodes.LIL_D:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+13));
                        self.pc++;
                        break;
                    case opcodes.LIL_E:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+14));
                        self.pc++;
                        break;
                    case opcodes.LIL_F:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0xF0)+15));
                        self.pc++;
                        break;
                    case opcodes.LIH_0:
                        setGPR_SP(checkGPR_SP(0), (getGPR_SP(0) & 0x0F)+(0<<4));
                        self.pc++;
                        break;
                    case opcodes.LIH_1:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(1<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_2:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(2<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_3:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(3<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_4:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(4<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_5:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(5<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_6:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(6<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_7:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(7<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_8:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(8<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_9:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(9<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_A:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(10<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_B:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(11<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_C:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(12<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_D:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(13<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_E:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(14<<4)));
                        self.pc++;
                        break;
                    case opcodes.LIH_F:
                        setGPR_SP(checkGPR_SP(0), checkOperation((getGPR_SP(0) & 0x0F)+(15<<4)));
                        self.pc++;
                        break;
                    default:
                        throw "Invalid op code: " + instr;
                }

                return true;
            } catch(e) {
                self.fault = true;
                throw e;
            }
        },
        reset: function() {
            var self = this;
            self.maxSP = 231;
            self.minSP = 0;

            self.gpr = [0, 0, 0, 0];
            self.sp = self.maxSP;
            self.pc = 0;
            self.zero = false;
            self.carry = false;
            self.fault = false;
        }
    };

    cpu.reset();
    return cpu;
}]);
