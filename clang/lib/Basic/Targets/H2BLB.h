//===--- H2BLB.h - Declare H2BLB target feature support -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares H2BLB TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_H2BLB_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_H2BLB_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Compiler.h" // For LLVM_LIBRARY_VISIBILITY.
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

// H2BLB class
class LLVM_LIBRARY_VISIBILITY H2BLBTargetInfo : public TargetInfo {
public:
  H2BLBTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    resetDataLayout(
        // Little-endian.
        "e-"
        // Pointer size is 16-bit and aligned on 8-bit.
        "p:16:8:8-"
        // Supports natively 16-bit and 32-bit integer.
        "n16:32-"
        // i32 are aligned on 8, i16 on 8 and i1 on 8.
        "i32:8:8-i16:8:8-i1:8:8-"
        // f32 aligned on 8-bit.
        "f32:8:8-"
        // v32 aligned on 8-bit.
        "v32:8:8");
  }

  /// Appends the target-specific \#define values for this
  /// target set to the specified buffer.
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  /// Return information about target-specific builtins for
  /// the current primary target, and info about which builtins are non-portable
  /// across the current set of primary and secondary targets.
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;

  /// Returns the kind of __builtin_va_list type that should be used
  /// with this target.
  BuiltinVaListKind getBuiltinVaListKind() const override {
    return CharPtrBuiltinVaList;
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return false;
  }

  /// Returns a string of target-specific clobbers, in LLVM format.
  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    return std::nullopt;
  }
  ArrayRef<GCCRegAlias> getGCCRegAliases() const override {
    return std::nullopt;
  }
};
} // namespace targets
} // namespace clang
#endif // LLVM_CLANG_LIB_BASIC_TARGETS_H2BLB_H
