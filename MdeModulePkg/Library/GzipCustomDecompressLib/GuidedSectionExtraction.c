#include "GzipDecompressLibInternal.h"

RETURN_STATUS
EFIAPI
GzipGuidedSectionGetInfo (
  IN  CONST VOID  *InputSection,
  OUT UINT32      *OutputBufferSize,
  OUT UINT32      *ScratchBufferSize,
  OUT UINT16      *SectionAttribute
  )
{
  ASSERT (InputSection != NULL);
  ASSERT (OutputBufferSize != NULL);
  ASSERT (ScratchBufferSize != NULL);
  ASSERT (SectionAttribute != NULL);

  if (IS_SECTION2 (InputSection)) {
    if (!CompareGuid (
        &gGzipCustomDecompressGuid,
        &(((EFI_GUID_DEFINED_SECTION2 *) InputSection)->SectionDefinitionGuid))) {
      return RETURN_INVALID_PARAMETER;
    }

    *SectionAttribute = ((EFI_GUID_DEFINED_SECTION2 *) InputSection)->Attributes;

    return GzipUefiDecompressGetInfo (
             (UINT8 *) InputSection + ((EFI_GUID_DEFINED_SECTION2 *) InputSection)->DataOffset,
             SECTION2_SIZE (InputSection) - ((EFI_GUID_DEFINED_SECTION2 *) InputSection)->DataOffset,
             OutputBufferSize,
             ScratchBufferSize
             );
  } else {
    if (!CompareGuid (
        &gGzipCustomDecompressGuid,
        &(((EFI_GUID_DEFINED_SECTION *) InputSection)->SectionDefinitionGuid))) {
      return RETURN_INVALID_PARAMETER;
    }

    *SectionAttribute = ((EFI_GUID_DEFINED_SECTION *) InputSection)->Attributes;

    return GzipUefiDecompressGetInfo (
             (UINT8 *) InputSection + ((EFI_GUID_DEFINED_SECTION *) InputSection)->DataOffset,
             SECTION_SIZE (InputSection) - ((EFI_GUID_DEFINED_SECTION *) InputSection)->DataOffset,
             OutputBufferSize,
             ScratchBufferSize
             );
  }
}

RETURN_STATUS
EFIAPI
GzipGuidedSectionExtraction (
  IN CONST  VOID    *InputSection,
  OUT       VOID    **OutputBuffer,
  OUT       VOID    *ScratchBuffer,        OPTIONAL
  OUT       UINT32  *AuthenticationStatus
  )
{
  ASSERT (OutputBuffer != NULL);
  ASSERT (InputSection != NULL);

  if (IS_SECTION2 (InputSection)) {
    if (!CompareGuid (
        &gGzipCustomDecompressGuid,
        &(((EFI_GUID_DEFINED_SECTION2 *) InputSection)->SectionDefinitionGuid))) {
      return RETURN_INVALID_PARAMETER;
    }

    //
    // Authentication is set to Zero, which may be ignored.
    //
    *AuthenticationStatus = 0;

    return GzipUefiDecompress (
             (UINT8 *) InputSection + ((EFI_GUID_DEFINED_SECTION2 *) InputSection)->DataOffset,
             SECTION2_SIZE (InputSection) - ((EFI_GUID_DEFINED_SECTION2 *) InputSection)->DataOffset,
             *OutputBuffer,
             ScratchBuffer
             );
  } else {
    if (!CompareGuid (
        &gGzipCustomDecompressGuid,
        &(((EFI_GUID_DEFINED_SECTION *) InputSection)->SectionDefinitionGuid))) {
      return RETURN_INVALID_PARAMETER;
    }

    //
    // Authentication is set to Zero, which may be ignored.
    //
    *AuthenticationStatus = 0;

    return GzipUefiDecompress (
             (UINT8 *) InputSection + ((EFI_GUID_DEFINED_SECTION *) InputSection)->DataOffset,
             SECTION_SIZE (InputSection) - ((EFI_GUID_DEFINED_SECTION *) InputSection)->DataOffset,
             *OutputBuffer,
             ScratchBuffer
    );
  }
}


/**
  Register GzipDecompress and GzipDecompressGetInfo handlers with GzipCustomerDecompressGuid.

  @retval  RETURN_SUCCESS            Register successfully.
  @retval  RETURN_OUT_OF_RESOURCES   No enough memory to store this handler.
**/
EFI_STATUS
EFIAPI
GzipDecompressLibConstructor (
  )
{
  return ExtractGuidedSectionRegisterHandlers (
          &gGzipCustomDecompressGuid,
          GzipGuidedSectionGetInfo,
          GzipGuidedSectionExtraction
          );      
}

