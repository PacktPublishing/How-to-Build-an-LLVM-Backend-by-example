//===--- H2BLB.cpp - Implement H2BLB target feature support -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements H2BLB TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Targets/H2BLB.h"
#include "clang/Basic/MacroBuilder.h"
#include "clang/Basic/TargetBuiltins.h"

using namespace clang;
using namespace clang::targets;

void H2BLBTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  Builder.defineMacro("__H2BLB__", "1");
}

static constexpr Builtin::Info BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, nullptr, HEADER, ALL_LANGUAGES},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, FEATURE, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#include "clang/Basic/BuiltinsH2BLB.inc"
};

ArrayRef<Builtin::Info> H2BLBTargetInfo::getTargetBuiltins() const {
  return llvm::ArrayRef(BuiltinInfo,
                        clang::H2BLB::LastTSBuiltin - Builtin::FirstTSBuiltin);
}
