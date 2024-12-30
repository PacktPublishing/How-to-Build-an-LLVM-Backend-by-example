//===-- H2BLBScheduleDefs.td - Scheduling definitions -----*- tablegen -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Describes the scheduling events modeled in the H2BLB target.
//
//===----------------------------------------------------------------------===//

// Widening signed multiply events.
def WriteWSMUL : SchedWrite;
def ReadWSMULArg0 : SchedRead;
def ReadWSMULArg1 : SchedRead;

// Widening unsigned multiply events.
def WriteWUMUL : SchedWrite;
def ReadWUMULArg0 : SchedRead;
def ReadWUMULArg1 : SchedRead;
