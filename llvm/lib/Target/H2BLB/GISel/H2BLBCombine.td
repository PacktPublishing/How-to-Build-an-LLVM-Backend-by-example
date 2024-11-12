//===- H2BLBCombine.td - Combine rule definitions ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Declare GlobalISel combine rules and provide mechanisms to opt-out.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/GlobalISel/Combine.td"

def registers_matchinfo: GIDefMatchData<"SmallVector<Register>">;

def insertvectorelt_to_build_vector : GICombineRule<
  (defs root:$root, registers_matchinfo:$matchinfo),
  (match (wip_match_opcode G_INSERT_VECTOR_ELT):$root,
         [{ return matchInsertVectorElt(*${root}, ${matchinfo}); }]),
  (apply [{ applyInsertVectorElt(*${root}, ${matchinfo}); }])>;

def H2BLBMandatoryPreLegalizerCombiner: GICombiner<
  "H2BLBMandatoryPreLegalizerCombinerImpl", [optnone_combines, insertvectorelt_to_build_vector]> {
}

// Turn G_UNMERGE_VALUES -> G_EXTRACT_VECTOR_ELT's
// This implementation comes from AArch64, this should just put it in the legalizer helper.
def vector_unmerge_lowering : GICombineRule <
  (defs root:$root),
  (match (wip_match_opcode G_UNMERGE_VALUES):$root,
          [{ return matchScalarizeVectorUnmerge(*${root}, MRI); }]),
  (apply [{ applyScalarizeVectorUnmerge(*${root}, MRI, B); }])
>;

def H2BLBMandatoryPostLegalizerCombiner : GICombiner<
  "H2BLBMandatoryPostLegalizerCombinerImpl", [vector_unmerge_lowering]> {
}
