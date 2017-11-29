#ifndef __GZIP_DECOMPRESS_GUID_H__
#define __GZIP_DECOMPRESS_GUID_H__

///
/// The Global ID used to identify a section of an FFS file of type 
/// EFI_SECTION_GUID_DEFINED, whose contents have been compressed using LZMA.
///
#define LZMA_CUSTOM_DECOMPRESS_GUID  \
  { 0x38bf8eb3, 0xbd52, 0x497a, { 0xbd, 0x2f, 0x1a, 0x9d, 0xef, 0x11, 0xd8, 0x4f } }

extern GUID gGzipCustomDecompressGuid;

#endif
