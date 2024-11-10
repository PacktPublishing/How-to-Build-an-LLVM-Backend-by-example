add_llvm_component_group(H2BLB)

set(LLVM_TARGET_DEFINITIONS H2BLB.td)

tablegen(LLVM H2BLBGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM H2BLBGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM H2BLBGenInstrInfo.inc -gen-instr-info)
# MCInst to .s
tablegen(LLVM H2BLBGenAsmWriter.inc -gen-asm-writer)
# .s to MCInst
tablegen(LLVM H2BLBGenAsmMatcher.inc -gen-asm-matcher)
# Encoding.
tablegen(LLVM H2BLBGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM H2BLBGenCallingConv.inc -gen-callingconv)
tablegen(LLVM H2BLBGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM H2BLBGenFastISel.inc -gen-fast-isel)

add_public_tablegen_target(H2BLBCommonTableGen)

add_llvm_target(H2BLBCodeGen
  # Technically the AsmPrinter could be its own library, but this is how
  # most targets expose their AsmPrinter.
  H2BLBAsmPrinter.cpp
  H2BLBCallingConvention.cpp
  H2BLBISelDAGToDAG.cpp
  H2BLBISelLowering.cpp
  H2BLBSimpleConstantPropagation.cpp
  H2BLBSubtarget.cpp
  H2BLBFastISel.cpp
  H2BLBFrameLowering.cpp
  H2BLBInstrInfo.cpp
  H2BLBRegisterInfo.cpp
  H2BLBTargetMachine.cpp
  H2BLBTargetObjectFile.cpp
  H2BLBTargetTransformInfo.cpp

  # GlobalISel related files.
  GISel/H2BLBCallLowering.cpp
  GISel/H2BLBLegalizerInfo.cpp

  LINK_COMPONENTS
  H2BLBDesc
  H2BLBInfo

  AsmPrinter
  CodeGen
  Core
  GlobalISel
  Passes
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  H2BLB
)

add_subdirectory(AsmParser)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
