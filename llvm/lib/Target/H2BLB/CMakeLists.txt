add_llvm_component_group(H2BLB)

add_llvm_target(H2BLBCodeGen
  H2BLBISelLowering.cpp
  H2BLBTargetMachine.cpp
  H2BLBTargetTransformInfo.cpp
  H2BLBSubtarget.cpp

  LINK_COMPONENTS
  H2BLBDesc
  H2BLBInfo

  CodeGen
  Core
  Support
  Target

  ADD_TO_COMPONENT
  H2BLB
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
