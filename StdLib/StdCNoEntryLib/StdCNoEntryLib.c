/** @file
  Establish the program environment and the "main" entry point.

  All of the global data in the gMD structure is initialized to 0, NULL, or
  SIG_DFL; as appropriate.

  Copyright (c) 2010 - 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials are licensed and made available under
  the terms and conditions of the BSD License that accompanies this distribution.
  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#include  <Uefi.h>
#include  <Library/UefiLib.h>
#include  <Library/DebugLib.h>

#include  <Library/MemoryAllocationLib.h>
#include  <Library/TimerLib.h>

#include  <LibConfig.h>

#include  <errno.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <time.h>
#include  <MainData.h>
#include  <unistd.h>

extern int __sse2_available;

extern struct  __MainData  *gMD;

void  exitCleanup(INTN ExitVal);

EFI_STATUS
EFIAPI
LibCConstructor (
  VOID
  )
{
  struct __filedes   *mfd;
  INTN   ExitVal;
  int                 i;

  ExitVal = (INTN)RETURN_SUCCESS;
  gMD = AllocateZeroPool(sizeof(struct __MainData));
  if( gMD == NULL ) {
    ExitVal = (INTN)RETURN_OUT_OF_RESOURCES;
  }
  else {
    /* Initialize data */
    __sse2_available      = 0;
    _fltused              = 1;
    errno                 = 0;
    EFIerrno              = 0;

    gMD->ClocksPerSecond  = 1;
    gMD->AppStartTime     = (clock_t)((UINT32)time(NULL));

    // Initialize file descriptors
    mfd = gMD->fdarray;
    for(i = 0; i < (FOPEN_MAX); ++i) {
      mfd[i].MyFD = (UINT16)i;
    }

    DEBUG((DEBUG_INIT, "StdLib: Open Standard IO.\n"));
    i = open("stdin:", (O_RDONLY | O_TTY_INIT), 0444);
    if(i == 0) {
      i = open("stdout:", (O_WRONLY | O_TTY_INIT), 0222);
      if(i == 1) {
        i = open("stderr:", O_WRONLY, 0222);
      }
    }
    if(i != 2) {
      Print(L"ERROR Initializing Standard IO: %a.\n    %r\n",
            strerror(errno), EFIerrno);
    }

    errno   = 0;    // Clean up any "scratch" values from startup.
  }

  return ExitVal;
}

EFI_STATUS
EFIAPI
LibCDestructor (
  VOID
  )
{
  int                 i;

  exitCleanup(0);

  /* Close any open files */
  for(i = OPEN_MAX - 1; i >= 0; --i) {
    (void)close(i);   // Close properly handles closing a closed file.
  }

  /* Free the global MainData structure */
  if(gMD != NULL) {
    if(gMD->NCmdLine != NULL) {
      FreePool( gMD->NCmdLine );
    }
    FreePool( gMD );
  }

  return EFI_SUCCESS;
}
