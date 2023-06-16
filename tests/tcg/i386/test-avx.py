#! /usr/bin/env python3

# Generate test-avx.h from x86.csv

import csv
import sys
import logging
import argparse
from fnmatch import fnmatch

archs_avx2 = [
    "SSE", "SSE2", "SSE3", "SSSE3", "SSE4_1", "SSE4_2",
    "AES", "AVX", "AVX2", "AES+AVX", #"VAES+AVX",
    "F16C", "FMA",
]

archs_avx512 = [
    "AVX512F", "AVX512F+AVX512VL",
]

ignore = set(["FISTTP",
    "LDMXCSR", "VLDMXCSR", "STMXCSR", "VSTMXCSR"])

# Errors in the x86.csv file. Ref to Vol2A March 2023 for fixed version
fixup = {
    "VCVTDQ2PD xmm1, {k}{z}, xmm2/m128/m32bcst": "VCVTDQ2PD xmm1, {k}{z}, xmm2/m64/m32bcst",
    "VCVTDQ2PD ymm1, {k}{z}, xmm2/m256/m32bcst": "VCVTDQ2PD ymm1, {k}{z}, xmm2/m128/m32bcst",
    "VCVTDQ2PD zmm1, {k}{z}, ymm2/m512/m32bcst": "VCVTDQ2PD zmm1, {k}{z}, ymm2/m256/m32bcst",
    "VCVTPS2PD xmm1, {k}{z}, xmm2/m128/m32bcst": "VCVTPS2PD xmm1, {k}{z}, xmm2/m64/m32bcst",
    "VCVTPS2PD ymm1, {k}{z}, xmm2/m256/m32bcst": "VCVTPS2PD ymm1, {k}{z}, xmm2/m128/m32bcst",
    "VCVTPS2PD zmm1, {k}{z}, ymm2/m512/m32bcst": "VCVTPS2PD zmm1, {k}{z}, ymm2/m256/m32bcst{sae}",
    "VCVTUDQ2PD xmm1, {k}{z}, xmm2/m128/m32bcst": "VCVTUDQ2PD xmm1, {k}{z}, xmm2/m64/m32bcst",
    "VCVTUDQ2PD ymm1, {k}{z}, xmm2/m256/m32bcst": "VCVTUDQ2PD ymm1, {k}{z}, xmm2/m128/m32bcst",
    "VCVTUDQ2PD zmm1, {k}{z}, ymm2/m512/m32bcst": "VCVTUDQ2PD zmm1, {k}{z}, ymm2/m256/m32bcst",
}

# Default mask if not found here will be 0xff
imask = {
    'vBLENDPD': 0xff,
    'vBLENDPS': 0x0f,
    'CMP[PS][SD]': 0x07,
    'VCMP[PS][SD]': 0x1f,
    'vCVTPS2PH': 0x7,
    'vDPPD': 0x33,
    'vDPPS': 0xff,
    'vEXTRACTPS': 0x03,
    'vINSERTPS': 0xff,
    'MPSADBW': 0x7,
    'VMPSADBW': 0x3f,
    'vPALIGNR': 0x3f,
    'vPBLENDW': 0xff,
    'vPCMP[EI]STR*': 0x0f,
    'vPEXTRB': 0x0f,
    'vPEXTRW': 0x07,
    'vPEXTRD': 0x03,
    'vPEXTRQ': 0x01,
    'vPINSRB': 0x0f,
    'vPINSRW': 0x07,
    'vPINSRD': 0x03,
    'vPINSRQ': 0x01,
    'vPSHUF[DW]': 0xff,
    'vPSHUF[LH]W': 0xff,
    'vPS[LR][AL][WDQ]': 0x3f,
    'vPS[RL]LDQ': 0x1f,
    'vROUND[PS][SD]': 0x7,
    'vSHUFPD': 0x0f,
    'vSHUFPS': 0xff,
    'vAESKEYGENASSIST': 0xff,
    'VEXTRACT[FI]128': 0x01,
    'VINSERT[FI]128': 0x01,
    'VPBLENDD': 0xff,
    'VPERM2[FI]128': 0x33,
    'VPERMPD': 0xff,
    'VPERMQ': 0xff,
    'VPERMILPS': 0xff,
    'VPERMILPD': 0x0f,
    }

def strip_comments(x):
    for l in x:
        if l != '' and l[0] != '#':
            yield l

def reg_w(w):
    if w == 8:
        return 'al'
    elif w == 16:
        return 'ax'
    elif w == 32:
        return 'eax'
    elif w == 64:
        return 'rax'
    raise Exception("bad reg_w %d" % w)

def mem_w(w):
    offset = 32
    if w == 8:
        t = "BYTE"
    elif w == 16:
        t = "WORD"
    elif w == 32:
        t = "DWORD"
    elif w == 64:
        t = "QWORD"
    elif w == 128:
        t = "XMMWORD"
    elif w == 256:
        t = "YMMWORD"
    elif w == 512:
        t = "ZMMWORD"
        offset = 64
    else:
        raise Exception()

    return "%s PTR %s[rdx]" % (t, offset)

class XMMArg():
    isxmm = True
    def __init__(self, reg, mw, mb=None, reg_mod=None):
        if mw not in [0, 8, 16, 32, 64, 128, 256, 512]:
            raise Exception("Bad /m width: %s" % mw)
        if mb is not None and mb not in [32, 64]:
            raise Exception("Bad m*bcst width: %s" % mb)
        self.reg = reg
        self.mw = mw
        self.mb = mb
        self.reg_mod = reg_mod
        self.ismem = mw != 0
    def regstr(self, n):
        if n < 0:
            # TODO: self.mb ignored for now (additional {1toX} after addressing mode)
            return mem_w(self.mw)
        else:
            # TODO: reg_mod not used for now (additional {reg_mod} after register)
            return "%smm%d" % (self.reg, n)

class MMArg():
    isxmm = True
    def __init__(self, mw):
        if mw not in [0, 32, 64]:
            raise Exception("Bad mem width: %s" % mw)
        self.mw = mw
        self.ismem = mw != 0
    def regstr(self, n):
        return "mm%d" % (n & 7)

def match(op, pattern):
    if pattern[0] == 'v':
        return fnmatch(op, pattern[1:]) or fnmatch(op, 'V'+pattern[1:])
    return fnmatch(op, pattern)

class ArgVSIB():
    isxmm = True
    ismem = False
    def __init__(self, reg, w):
        if w not in [32, 64]:
            raise Exception("Bad vsib width: %s" % w)
        self.w = w
        self.reg = reg
    def regstr(self, n):
        reg = "%smm%d" % (self.reg, n >> 2)
        return "[rsi + %s * %d]" % (reg, 1 << (n & 3))

class ArgImm8u():
    isxmm = False
    ismem = False
    def __init__(self, op):
        for k, v in imask.items():
            if match(op, k):
                self.mask = imask[k];
                return
        self.mask = 0xff
        #raise Exception("Unknown immediate")
    def vals(self):
        mask = self.mask
        yield 0
        n = 0
        while n != mask:
            n += 1
            while (n & ~mask) != 0:
                n += (n & ~mask)
            yield n

class ArgRM():
    isxmm = False
    def __init__(self, rw, mw, reg_mod=None):
        if rw not in [8, 16, 32, 64]:
            raise Exception("Bad r/w width: %s" % w)
        if mw not in [0, 8, 16, 32, 64]:
            raise Exception("Bad r/w width: %s" % w)
        self.rw = rw
        self.mw = mw
        self.ismem = mw != 0
        self.reg_mod = reg_mod
    def regstr(self, n):
        if n < 0:
            return mem_w(self.mw)
        else:
            # TODO: reg_mod not used for now (additional {reg_mod} after register)
            return reg_w(self.rw)

class ArgMem():
    isxmm = False
    ismem = True
    def __init__(self, w):
        if w not in [8, 16, 32, 64, 128, 256, 512]:
            raise Exception("Bad mem width: %s" % w)
        self.w = w
    def regstr(self, n):
        return mem_w(self.w)

class ArgK():
    isxmm = True # Actually K register file, but we want to generate these
    ismem = False
    def __init__(self, reg_mod=None):
        self.reg_mod = reg_mod

    def regstr(self, n):
        # TODO: reg_mod not used for now (additional {reg_mod} after register)
        return "k%s" % (n % 8,)

class ArgKMask():
    isxmm = False
    ismem = False
    def __init__(self, optional=True, has_z=False):
        self.has_z = has_z
        self.optional = optional

    def regstr(self, n, zero=0):
        if n == 0:
            assert self.optional
            return "" # k0 for no write mask
        z = ""
        if zero:
            assert self.has_z
            z = "{z}"
        return "{k%s}%s" % (n % 8, z)

class SkipInstruction(Exception):
    pass

def ArgGenerator(arg, op):
    reg_mod = None
    for mod in ["{er}", "{sae}"]: # must be exclusive
        if mod in arg:
            reg_mod = mod[1:-1]
            arg = arg.replace(mod, "")

    if arg[:3] == 'xmm' or arg[:3] == "ymm" or arg[:3] == "zmm":
        if "/" in arg:
            options = arg.split('/')
            r, m = options[0], options[1]
            if (m[0] != 'm'):
                raise Exception("Expected /m: '%s'" % (arg,))
            if len(options) == 2:
                return XMMArg(arg[0], int(m[1:]))
            elif len(options) == 3:
                mb = options[2]
                if mb != "m32bcst" and mb != "m64bcst":
                    raise Exception("Expected [xyz]mm*/m*/m[32|64]bcst: '%s'" % (arg,))
                return XMMArg(arg[0], int(m[1:]), int(mb[1:3]))
        else:
            return XMMArg(arg[0], 0, reg_mod=reg_mod);
    elif arg[:2] == 'mm':
        if "/" in arg:
            r, m = arg.split('/')
            if (m[0] != 'm'):
                raise Exception("Expected /m: %s", arg)
            return MMArg(int(m[1:]))
        else:
            return MMArg(0)
    elif arg[:4] == 'imm8':
        return ArgImm8u(op)
    elif arg == '<XMM0>':
        return None
    elif arg[0] == 'r':
        if arg[:3] == 'rmr':
            return ArgRM(int(arg[3:]), 0)
        elif '/m' in arg:
            r, m = arg.split('/')
            if (m[0] != 'm'):
                raise Exception("Expected /m: %s", arg)
            mw = int(m[1:])
            if r == 'r':
                rw = mw
            else:
                rw = int(r[1:])
            return ArgRM(rw, mw)
            
        return ArgRM(int(arg[1:]), 0, reg_mod=reg_mod);
    elif arg[0] == 'm':
        return ArgMem(int(arg[1:]))
    elif arg[:2] == 'vm':
        return ArgVSIB(arg[-1], int(arg[2:-1]))
    elif arg[0] == 'k':
        return ArgK(reg_mod=reg_mod)
    elif arg[:2] == '{k':
        if arg == '{k1-k7}':
            return ArgKMask(optional=False, has_z=False)
        elif arg[-3:] == '{z}':
            return ArgKMask(has_z=True)
        else:
            return ArgKMask(has_z=False)
    else:
        raise Exception("Unrecognised arg: %s", arg)

class InsnGenerator:
    def __init__(self, op, args, accesses, opts):
        self.opts = opts
        self.op = op
        if op[-2:] in ["PH", "PS", "PD", "SS", "SD"]:
            if op[-1] == 'H':
                self.optype = 'F16'
            elif op[-1] == 'S':
                self.optype = 'F32'
            else:
                self.optype = 'F64'
        else:
            self.optype = 'I'

        self.accesses = accesses
        try:
            self.args= list(ArgGenerator(a, op) for a in args)
            if not any((x.isxmm for x in self.args)):
                raise SkipInstruction
            if len(self.args) > 0 and self.args[-1] is None:
                self.args = self.args[:-1]
                self.accesses = self.accesses[:-1]
        except SkipInstruction:
            raise
        except Exception as e:
            raise Exception("Bad arg %s: %s" % (op, e))

    def gen(self):
        if self.opts.avx512:
            regs = (18, 11, 12) # Test with a high register
        else:
            regs = (10, 11, 12)
        dest = 9
        kregs = (1,) # non 0 kregs to test

        kmask = -1, None
        for n, arg in enumerate(self.args):
            if isinstance(arg, ArgKMask):
                kmask = n, arg
        if kmask[0] != -1:
            del self.args[kmask[0]]

        nreg = len(self.args)
        if nreg == 0:
            yield self.op
            return
        if isinstance(self.args[-1], ArgImm8u):
            nreg -= 1
            immarg = self.args[-1]
        else:
            immarg = None

        memarg = -1
        for n, arg in enumerate(self.args):
            if arg.ismem:
                memarg = n

        if (self.op.startswith("VGATHER") or self.op.startswith("VPGATHER")):
            if "GATHERD" in self.op:
                ireg = 13 << 2
            else:
                ireg = 14 << 2
            regset = [
                (dest, ireg | 0, regs[0]),
                (dest, ireg | 1, regs[0]),
                (dest, ireg | 2, regs[0]),
                (dest, ireg | 3, regs[0]),
                ]
            if memarg >= 0:
                raise Exception("vsib with memory: %s" % self.op)
        elif nreg == 1:
            regset = [(dest,)]
            regset = [(regs[0],)]
            if memarg == 0:
                regset += [(-1,)]
        elif nreg == 2:
            regset = [
                (dest, regs[1]),
                (dest, regs[0]),
                (regs[0], regs[1]),
                (regs[0], regs[0]),
                ]
            if memarg == 0:
                regset += [(-1, regs[0])]
            elif memarg == 1:
                regset += [(dest, -1)]
        elif nreg == 3:
            regset = [
                (dest, regs[0], regs[1]),
                (dest, regs[0], regs[0]),
                (regs[0], regs[0], regs[1]),
                (regs[0], regs[1], regs[0]),
                (regs[0], regs[0], regs[0]),
                ]
            if memarg == 2:
                regset += [
                    (dest, regs[0], -1),
                    (regs[0], regs[0], -1),
                    ]
            elif memarg > 0:
                raise Exception("Memarg %d" % memarg)
        elif nreg == 4:
            regset = [
                (dest, regs[0], regs[1], regs[2]),
                (dest, regs[0], regs[0], regs[1]),
                (dest, regs[0], regs[1], regs[0]),
                (dest, regs[1], regs[0], regs[0]),
                (dest, regs[0], regs[0], regs[0]),
                (regs[0], regs[0], regs[1], regs[2]),
                (regs[0], regs[1], regs[0], regs[2]),
                (regs[0], regs[1], regs[2], regs[0]),
                (regs[0], regs[0], regs[0], regs[1]),
                (regs[0], regs[0], regs[1], regs[0]),
                (regs[0], regs[1], regs[0], regs[0]),
                (regs[0], regs[0], regs[0], regs[0]),
                ]
            if memarg == 2:
                regset += [
                    (dest, regs[0], -1, regs[1]),
                    (dest, regs[0], -1, regs[0]),
                    (regs[0], regs[0], -1, regs[1]),
                    (regs[0], regs[1], -1, regs[0]),
                    (regs[0], regs[0], -1, regs[0]),
                    ]
            elif memarg > 0:
                raise Exception("Memarg4 %d" % memarg)
        else:
            raise Exception("Too many regs: %s(%d)" % (self.op, nreg))

        args_expanded = []
        for regv in regset:
            kargs = []
            if kmask[0] == -1 or kmask[1].optional:
                kargs.append(0)
            if kmask[0] != -1 and not self.opts.no_mask:
                kargs.extend(kregs)
            for kreg in kargs:
                zeroes = [False] if kreg == 0 or kmask[1].has_z == False or kmask[0] - 1 == memarg and regv[memarg] == -1 else [False, True]
                for zero in zeroes:
                    argstr = []
                    for i in range(nreg):
                        arg = self.args[i]
                        if i != kmask[0] - 1:
                            argstr.append(arg.regstr(regv[i]))
                        else:
                            argstr.append(arg.regstr(regv[i]) + kmask[1].regstr(kreg, zero=zero))
                    args_expanded.append(argstr)
        for argstr in args_expanded:
            if immarg is None:
                yield self.op + ' ' + ','.join(argstr)
            else:
                for immval in immarg.vals():
                    yield self.op + ' ' + ','.join(argstr) + ',' + str(immval)
                    if self.opts.no_full_imm:
                        break

def split0(s):
    if s == '':
        return []
    return s.split(',')

def main():
    logging.basicConfig()
    logger = logging.getLogger(__name__)
    logger.setLevel(logging.INFO)

    parser = argparse.ArgumentParser(description="AVX/AVX512 table generator")
    parser.add_argument("--avx512", action='store_true', help="Generate AVX512 subset instread of avx")
    parser.add_argument("--no-mask", action='store_true', help="Do not generate mask predicate operands")
    parser.add_argument("--no-full-imm", action='store_true', help="Do not generate all immediates combinations")
    parser.add_argument("--debug", action='store_true', help="Debug mode")
    parser.add_argument("CSV", help="input csv file")
    parser.add_argument("OUT", help="output header file")
    args = parser.parse_args()

    if args.debug:
        logger.setLevel(logging.DEBUG)

    subsets = archs_avx512 if args.avx512 else archs_avx2
    n = 0
    csvfile = open(args.CSV, 'r', newline='')
    with open(args.OUT, "w") as outf:
        outf.write("// Generated by test-avx.py. Do not edit.\n")
        for row in csv.reader(strip_comments(csvfile)):
            if row[0] in fixup:
                row[0] = fixup[row[0]]
            insn = row[0].replace(',', '').split()
            if insn[0] in ignore:
                continue
            accesses = row[8].split(',')
            cpuid = row[6]
            if cpuid in subsets:
                try:
                    logger.debug("Insn: %s" % (insn,))
                    g = InsnGenerator(insn[0], insn[1:], accesses, args)
                    for insn in g.gen():
                        outf.write('TEST(%d, "%s", %s)\n' % (n, insn, g.optype))
                        n += 1
                except SkipInstruction:
                    logger.debug("SKIP: %s" % (insn,))
                    pass
        outf.write("#undef TEST\n")
        csvfile.close()

if __name__ == "__main__":
    main()
