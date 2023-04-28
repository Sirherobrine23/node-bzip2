#include <bzip2/bzlib.h>
#define BZIPBUFSIZ     8192

static char *_qdbm_bzencode_impl(const char *ptr, int size, int *sp) {
    bz_stream zs;
    char *buf, *swap, obuf[BZIPBUFSIZ];
    int rv, asiz, bsiz, osiz;
    if(size < 0) size = strlen(ptr);
    zs.bzalloc = NULL;
    zs.bzfree = NULL;
    zs.opaque = NULL;
    if(BZ2_bzCompressInit(&zs, 9, 0, 30) != BZ_OK) return NULL;
    asiz = size + 16;
    if(asiz < BZIPBUFSIZ) asiz = BZIPBUFSIZ;
    if(!(buf = malloc(asiz))) {
        BZ2_bzCompressEnd(&zs);
        return NULL;
    }
    bsiz = 0;
    zs.next_in = (char *)ptr;
    zs.avail_in = size;
    zs.next_out = obuf;
    zs.avail_out = BZIPBUFSIZ;
    while((rv = BZ2_bzCompress(&zs, BZ_FINISH)) == BZ_FINISH_OK) {
        osiz = BZIPBUFSIZ - zs.avail_out;
        if(bsiz + osiz > asiz) {
            asiz = asiz * 2 + osiz;
            if(!(swap = realloc(buf, asiz))) {
                free(buf);
                BZ2_bzCompressEnd(&zs);
                return NULL;
            }
            buf = swap;
        }
        memcpy(buf + bsiz, obuf, osiz);
        bsiz += osiz;
        zs.next_out = obuf;
        zs.avail_out = BZIPBUFSIZ;
    }
    if(rv != BZ_STREAM_END) {
        free(buf);
        BZ2_bzCompressEnd(&zs);
        return NULL;
    }
    osiz = BZIPBUFSIZ - zs.avail_out;
    if(bsiz + osiz + 1 > asiz) {
        asiz = asiz * 2 + osiz;
        if(!(swap = realloc(buf, asiz))) {
            free(buf);
            BZ2_bzCompressEnd(&zs);
            return NULL;
        }
        buf = swap;
    }
    memcpy(buf + bsiz, obuf, osiz);
    bsiz += osiz;
    buf[bsiz] = '\0';
    *sp = bsiz;
    BZ2_bzCompressEnd(&zs);
    return buf;
}