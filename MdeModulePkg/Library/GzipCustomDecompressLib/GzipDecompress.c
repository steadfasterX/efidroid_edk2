#include "GzipDecompressLibInternal.h"
#include <zlib.h>

#define SCRATCH_BUFFER_REQUEST_SIZE SIZE_64KB

typedef struct
{
  VOID     *Buffer;
  UINTN    BufferSize;
} ISzAllocWithData;

/**
  Allocation routine used by LZMA decompression.

  @param P                Pointer to the ISzAlloc instance
  @param Size             The size in bytes to be allocated

  @return The allocated pointer address, or NULL on failure
**/
VOID *
SzAlloc (
  VOID *P,
  UINTN Items,
  UINTN ItemSize
  )
{
  VOID *Addr;
  ISzAllocWithData *Private;
  UINTN Size = Items * ItemSize;

  Private = (ISzAllocWithData*) P;

  if (Private->BufferSize >= Size) {
    Addr = Private->Buffer;
    Private->Buffer = (VOID*) ((UINT8*)Addr + Size);
    Private->BufferSize -= Size;
    return Addr;
  } else {
    ASSERT (FALSE);
    return NULL;
  }
}

/**
  Free routine used by LZMA decompression.

  @param P                Pointer to the ISzAlloc instance
  @param Address          The address to be freed
**/
VOID
SzFree (
  VOID *P,
  VOID *Address
  )
{
  //
  // We use the 'scratch buffer' for allocations, so there is no free
  // operation required.  The scratch buffer will be freed by the caller
  // of the decompression code.
  //
}

/**
  Get the size of the uncompressed buffer by parsing EncodeData header.

  @param EncodedData  Pointer to the compressed data.

  @return The size of the uncompressed buffer.
**/
UINT64
GetDecodedSizeOfBuf(
  IN  CONST VOID  *Source,
  IN  UINT32      SourceSize
  )
{
  UINT32 *Footer = (UINT32*)(Source + SourceSize - sizeof(UINT32));
  return *Footer;
}

RETURN_STATUS
EFIAPI
GzipUefiDecompressGetInfo (
  IN  CONST VOID  *Source,
  IN  UINT32      SourceSize,
  OUT UINT32      *DestinationSize,
  OUT UINT32      *ScratchSize
  )
{
  UINT64  DecodedSize;

  ASSERT(SourceSize >= sizeof(UINT32));

  DecodedSize = GetDecodedSizeOfBuf(Source, SourceSize);

  *DestinationSize = (UINT32)DecodedSize;
  *ScratchSize = SCRATCH_BUFFER_REQUEST_SIZE;
  return RETURN_SUCCESS;
}

RETURN_STATUS
EFIAPI
GzipUefiDecompress (
  IN CONST VOID  *Source,
  IN UINTN       SourceSize,
  IN OUT VOID    *Destination,
  IN OUT VOID    *Scratch
  )
{
  CONST UINT8 *zbuf = Source;
  struct z_stream_s Stream;
  int rc;
  ISzAllocWithData  AllocFuncs;
  RETURN_STATUS Status;

  AllocFuncs.Buffer           = Scratch;
  AllocFuncs.BufferSize       = SCRATCH_BUFFER_REQUEST_SIZE;

  // verify the gzip header
  if (SourceSize < 10 || zbuf[0] != 0x1f || zbuf[1] != 0x8b || zbuf[2] != 0x08) {
    return RETURN_INVALID_PARAMETER;
  }

  SetMem(&Stream, sizeof(Stream), 0);

  // skip over gzip header (1f,8b,08... 10 bytes total + possible asciz filename)
  Stream.next_in = zbuf + 10;
  Stream.avail_in = SourceSize - 10;

  // skip over asciz filename
  if (zbuf[3] & 0x8) {
    do {
      /*
      * If the filename doesn't fit into the buffer,
      * the file is very probably corrupt. Don't try
      * to read more data.
      */
      if (Stream.avail_in == 0) {
        return RETURN_INVALID_PARAMETER;
      }
      --Stream.avail_in;
    } while (*Stream.next_in++);
  }

  Stream.zalloc = SzAlloc;
  Stream.zfree = SzFree;
  Stream.opaque = &AllocFuncs;
  Stream.next_out = Destination;
  Stream.avail_out = GetDecodedSizeOfBuf(Source, SourceSize);

  rc = inflateInit2(&Stream, -MAX_WBITS);
  if (rc != Z_OK) {
    return RETURN_INVALID_PARAMETER;
  }

  rc = inflate(&Stream, 0);

  // after Z_FINISH, only Z_STREAM_END is "we unpacked it all"
  if (rc == Z_STREAM_END) {
    Status = RETURN_SUCCESS;
  } else if (rc != Z_OK) {
    Status = RETURN_INVALID_PARAMETER;
  }

  inflateEnd(&Stream);

  return Status;
}

