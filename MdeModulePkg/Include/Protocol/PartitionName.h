#ifndef __PARTITION_NAME_H__
#define __PARTITION_NAME_H__

#include <Uefi/UefiSpec.h>

#define EFI_PARTITION_NAME_PROTOCOL_GUID \
  { \
    0xf3287350, 0x384a, 0x44ec, {0x95, 0xcf, 0xdb, 0x4b, 0x86, 0x7c, 0x96, 0x8c } \
  }

typedef struct _EFI_PARTITION_NAME_PROTOCOL  EFI_PARTITION_NAME_PROTOCOL;

///
/// Protocol GUID name
/// 
#define PARTITION_NAME_PROTOCOL       EFI_PARTITION_NAME_PROTOCOL_GUID

///
/// Protocol
/// 
typedef EFI_PARTITION_NAME_PROTOCOL   EFI_PARTITION_NAME;

#define EFI_PARTITION_NAME_PROTOCOL_REVISION  0x00010000
#define EFI_PARTITION_NAME_PROTOCOL_REVISION2 0x00020001
#define EFI_PARTITION_NAME_PROTOCOL_REVISION3 0x00020031

///
/// Revision
/// 
#define EFI_PARTITION_NAME_INTERFACE_REVISION   EFI_PARTITION_NAME_PROTOCOL_REVISION

///
///  This protocol provides the name of a partition
///
struct _EFI_PARTITION_NAME_PROTOCOL {
  CHAR16              Name[36];
};

extern EFI_GUID gEfiPartitionNameProtocolGuid;

#endif

