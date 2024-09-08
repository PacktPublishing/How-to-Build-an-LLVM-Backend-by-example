add_llvm_component_group(H2BLB)

set(LLVM_TARGET_DEFINITIONS H2BLB.td)

tablegen(LLVM H2BLBGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM H2BLBGenRegisterInfo.inc -gen-register-info)

add_public_tablegen_target(H2BLBCommonTableGen)

add_llvm_target(H2BLBCodeGen
  H2BLBISelLowering.cpp
  H2BLBSimpleConstantPropagation.cpp
  H2BLBSubtarget.cpp
  H2BLBTargetMachine.cpp
  H2BLBTargetObjectFile.cpp
  H2BLBTargetTransformInfo.cpp

  LINK_COMPONENTS
  H2BLBDesc
  H2BLBInfo

  CodeGen
  Core
  Passes
  Support
  Target

  ADD_TO_COMPONENT
  H2BLB
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
