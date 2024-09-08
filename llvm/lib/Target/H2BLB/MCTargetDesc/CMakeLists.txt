add_llvm_component_library(LLVMH2BLBDesc
  H2BLBMCAsmInfo.cpp
  H2BLBMCTargetDesc.cpp
  H2BLBInstPrinter.cpp

  LINK_COMPONENTS
  Support
  H2BLBInfo
  MC

  ADD_TO_COMPONENT
  H2BLB
  )
