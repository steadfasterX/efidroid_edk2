#------------------------------------------------------------------------------
#
# Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http:#opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#------------------------------------------------------------------------------



#include <AsmMacroIoLib.h>

#
#UINT32
#EFIAPI
#__aeabi_lasr (
#  IN UINT32  Dividen
#  IN UINT32  Divisor
#  );
#
ASM_FUNC(__aeabi_lasr)
    SUBS     r3,r2,#0x20
    bpl      1f
    RSB      r3,r2,#0x20
    LSR      r0,r0,r2
    ORR      r0,r0,r1,LSL r3
    ASR      r1,r1,r2
    BX       lr

1:
    ASR      r0,r1,r3
    ASR      r1,r1,#31
    BX       lr

