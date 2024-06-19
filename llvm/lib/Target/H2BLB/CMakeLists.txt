add_llvm_component_group(H2BLB)

add_llvm_target(H2BLBCodeGen
  H2BLBTargetMachine.cpp

  LINK_COMPONENTS
  H2BLBDesc
  H2BLBInfo

  Support

  ADD_TO_COMPONENT
  H2BLB
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
