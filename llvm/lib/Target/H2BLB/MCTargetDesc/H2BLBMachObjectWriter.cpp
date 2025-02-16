//===-- H2BLBMachObjectWriter.cpp - H2BLB Mach Object Writer --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/H2BLBMCFixups.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAsmInfoDarwin.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCFragment.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MathExtras.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

namespace {

class H2BLBMachObjectWriter : public MCMachObjectTargetWriter {
public:
  H2BLBMachObjectWriter(uint32_t CPUType, uint32_t CPUSubtype)
      : MCMachObjectTargetWriter(/*is64Bit=*/false, CPUType, CPUSubtype) {}

  void recordRelocation(MachObjectWriter *Writer, MCAssembler &Asm,
                        const MCFragment *Fragment, const MCFixup &Fixup,
                        MCValue Target, uint64_t &FixedValue) override;
};

} // end anonymous namespace

void H2BLBMachObjectWriter::recordRelocation(
    MachObjectWriter *Writer, MCAssembler &Asm, const MCFragment *Fragment,
    const MCFixup &Fixup, MCValue Target, uint64_t &FixedValue) {
  unsigned IsPCRel = Writer->isFixupKindPCRel(Asm, Fixup.getKind());

  // See <reloc.h>.
  uint32_t FixupOffset = Asm.getFragmentOffset(*Fragment);
  // We only support 11 bit address so the reloc fits in 2 bytes and
  // log2(2) == 1.
  unsigned Log2Size = 1;
  // Addend for the symbol.
  int64_t Value = 0;
  // Where to find the symbol (index in the symbol table or section number).
  // This is a section number when RelSymbol == nullptr.
  unsigned Index = 0;
  // Type of the relocation.
  unsigned Type = 0;
  const MCSymbol *RelSymbol = nullptr;

  FixupOffset += Fixup.getOffset();

  // Fixup describes a A - B + Cst expression.
  // The only thing we support right now if A, so no B and Cst == 0.
  Value = Target.getConstant();
  // Constant should have been resolved during fixup.
  assert(!Value && "constant not supported");
  assert(!Target.isAbsolute() && "absolute constant not supported");
  assert(!Target.getSymB() && "symbol arithmetic not supported");

  const MCSymbol *Symbol = &Target.getSymA()->getSymbol();

  const MCSymbol *Base = Writer->getAtom(*Symbol);

  if (Base) {
    RelSymbol = Base;

    // Add the local offset, if needed.
    if (Base != Symbol)
      Value += Asm.getSymbolOffset(*Symbol) - Asm.getSymbolOffset(*Base);
  } else {
    assert(Symbol->isInSection());
    // Adjust the relocation to be section-relative.
    // The index is the section ordinal (1-based).
    const MCSection &Sec = Symbol->getSection();
    Index = Sec.getOrdinal() + 1;
    Value += Writer->getSymbolAddress(*Symbol, Asm);

    if (IsPCRel)
      Value -= Writer->getFragmentAddress(Asm, Fragment) + Fixup.getOffset() +
               (1ULL << Log2Size);
  }
  // Use a dummy reloc type instead of creating one.
  // None of the binary tool exists at this point.
  Type = unsigned(MachO::ARM64_RELOC_BRANCH26);

  // If there's any addend left to handle, encode it in the instruction.
  FixedValue = Value;
  // struct relocation_info (8 bytes)
  MachO::any_relocation_info MRE;
  MRE.r_word0 = FixupOffset;
  MRE.r_word1 =
      (Index << 0) | (IsPCRel << 24) | (Log2Size << 25) | (Type << 28);
  Writer->addRelocation(RelSymbol, Fragment->getParent(), MRE);
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createH2BLBMachObjectWriter(uint32_t CPUType, uint32_t CPUSubtype) {
  return std::make_unique<H2BLBMachObjectWriter>(CPUType, CPUSubtype);
}
