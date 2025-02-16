//===-- H2BLBAsmBackend.cpp - H2BLB Assembler Backend ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/H2BLBMCFixups.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h"
#include "llvm/BinaryFormat/MachO.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

namespace {

class H2BLBAsmBackend : public MCAsmBackend {
  bool IsMachO;
  Triple TheTriple;

public:
  H2BLBAsmBackend(bool IsMachO, const Triple &TT)
      : MCAsmBackend(endianness::little), IsMachO(IsMachO), TheTriple(TT) {}

  // Need this one to teach the generic code about our target specific fixup.
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    if (IsMachO) {
      uint32_t CPUType = cantFail(MachO::getCPUType(TheTriple));
      uint32_t CPUSubType = cantFail(MachO::getCPUSubType(TheTriple));
      return createH2BLBMachObjectWriter(CPUType, CPUSubType);
    }
    llvm_unreachable("Not yet implemented");
  }

  unsigned getNumFixupKinds() const override {
    return H2BLB::NumTargetFixupKinds;
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override {
    return false;
  }
};

} // end anonymous namespace
static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  int64_t SignedValue = static_cast<int64_t>(Value);

  switch (Fixup.getTargetKind()) {
  case H2BLB::FK_H2BLB_PCRel_11: {
    if (!isInt<11>(SignedValue))
      Ctx.reportError(Fixup.getLoc(), "branch size exceeds simm11");

    return Value;
  }
  default:
    llvm_unreachable("unhandled fixup kind");
  }
}

void H2BLBAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                 const MCValue &Target,
                                 MutableArrayRef<char> Data, uint64_t Value,
                                 bool IsResolved,
                                 const MCSubtargetInfo *STI) const {
  if (!Value)
    return; // Doesn't change encoding.
  unsigned Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return;
  Value = adjustFixupValue(Fixup, Value, Asm.getContext());
  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned NumBytes = (Info.TargetSize + 7) / 8;
  uint32_t Offset = Fixup.getOffset();
  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the bits from
  // the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i)
    Data[Offset + i] |= static_cast<uint8_t>((Value >> (i * 8)) & 0xff);
}

const MCFixupKindInfo &
H2BLBAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[H2BLB::NumTargetFixupKinds] = {
      // name                    , offset, bits, flags
      {"fixup_h2blb_pcrel_call11",      0,   11, MCFixupKindInfo::FKF_IsPCRel},
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

MCAsmBackend *llvm::createH2BLBAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &,
                                          const MCTargetOptions &) {
  const Triple &TheTriple = STI.getTargetTriple();
  return new H2BLBAsmBackend(TheTriple.isOSBinFormatMachO(), TheTriple);
}
