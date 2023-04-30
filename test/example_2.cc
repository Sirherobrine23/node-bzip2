extern "C" {
  #include <bzip2/bzlib.h>
  #include <bzip2/bzlib_private.h>
}

void main() {
    bz_stream bzStream;
    int bzError, cumulativeBytesWritten, bzBytesWritten;
    char* bzBuffer = {0};
    long bzBytesWritten = 0UL;
    long long cumulativeBytesWritten = 0ULL;
    char* myBuffer = {0};
    size_t myBufferLength = 0;
    /* read in the final batch of bytes into myBuffer (with a total byte size of `myBufferLength`... */

    /* compress remaining myBufferLength bytes in myBuffer */
    bzStream.next_in = myBuffer;
    bzStream.avail_in = myBufferLength;
    bzStream.next_out = bzBuffer;
    bzStream.avail_out = 5000;
    do
    {
        bzStream.next_out = bzBuffer;
        bzStream.avail_out = 5000;
        bzError = BZ2_bzCompress(&bzStream, (bzStream.avail_in) ? BZ_RUN : BZ_FINISH);

        /* bzError error checking... */

        /* increment cumulativeBytesWritten by `bz_stream` struct `total_out_*` members */
        bzBytesWritten = ((unsigned long) bzStream.total_out_hi32 << 32) + bzStream.total_out_lo32;
        cumulativeBytesWritten += bzBytesWritten;

        /* write compressed data in bzBuffer to standard output */
        fwrite(bzBuffer, 1, bzBytesWritten, stdout);
        fflush(stdout);
    }
    while (bzError != BZ_STREAM_END);

    /* close stream */
    bzError = BZ2_bzCompressEnd(&bzStream);

    /* bzError checking... */
}
