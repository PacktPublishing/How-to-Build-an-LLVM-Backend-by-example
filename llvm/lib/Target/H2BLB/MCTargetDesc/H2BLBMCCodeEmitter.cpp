//===-- H2BLBMCCodeEmitter.cpp - Convert H2BLB code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the H2BLBMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/H2BLBMCFixups.h"
#include "MCTargetDesc/H2BLBMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {

class H2BLBMCCodeEmitter : public MCCodeEmitter {
  MCContext &MCCtxt;

public:
  H2BLBMCCodeEmitter(MCContext &MCCtxt) : MCCodeEmitter(), MCCtxt(MCCtxt) {}
  ~H2BLBMCCodeEmitter() override = default;

  // TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // Return binary encoding of operand. If the machine
  // operand requires relocation, record the relocation and return zero.
  // This method is used in the TableGen'erated code.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

} // end anonymous namespace

MCCodeEmitter *llvm::createH2BLBMCCodeEmitter(const MCInstrInfo &MCII,
                                              MCContext &MCCtxt) {
  return new H2BLBMCCodeEmitter(MCCtxt);
}

unsigned
H2BLBMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return MCCtxt.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  // At this point we expect a symbol reference for the branches.
  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  assert(Expr->getKind() == MCExpr::SymbolRef);

  if (MI.getOpcode() == H2BLB::CALL) {
    // FIXME: At this point we have to issue a fixup, but we need the
    // MCAsmBackend to do that, which we don't have.
    // Just do nothing for now.
    // Fixups.push_back(MCFixup::create(0, Expr,
    // (MCFixupKind)H2BLB::FK_H2BLB_PCRel_11));
  } else
    llvm_unreachable("We don't have any operation with symbols");

  return 0;
}

void H2BLBMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                           SmallVectorImpl<char> &CB,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  // Get instruction encoding and emit it
  uint64_t Encoding = getBinaryCodeForInstr(MI, Fixups, STI);
  assert(((Encoding & 0xffffffffffff0000) == 0) &&
         "Only the first 16-bit should be set");
  support::endian::write<uint16_t>(CB, Encoding, llvm::endianness::little);
}

#include "H2BLBGenMCCodeEmitter.inc"
