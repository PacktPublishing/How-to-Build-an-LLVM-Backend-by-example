add_llvm_component_library(LLVMH2BLBDesc
  H2BLBMCTargetDesc.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  H2BLB
  )
