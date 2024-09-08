include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_component_library(LLVMH2BLBAsmParser
  H2BLBAsmParser.cpp

  LINK_COMPONENTS
  H2BLBDesc
  H2BLBInfo
  CodeGenTypes
  MC
  MCParser
  Support
  TargetParser

  ADD_TO_COMPONENT
  H2BLB
  )

