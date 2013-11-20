#include "configuration.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#define MIN_CHUNK 64

typedef struct _FcitxGetLine
{
    boolean isFile;
    union {
        const char* buf;
        FILE* fp;
    };
} FcitxGetLine;

void getline_buf(char** lineptr, size_t* n, FcitxGetLine gl)
{
    if (gl.isFile) {
        return getline(lineptr, n, gl.fp);
    } else {
        int nchars_avail;     /* Allocated but unused chars in *LINEPTR.  */
        char *read_pos;       /* Where we're reading into *LINEPTR. */
        int ret;
        const char* p = gl.buf;

        for (;;) {
            register char c = *p;
            if (*n > MIN_CHUNK) {
                *n *= 2;
            } else {
                *n += MIN_CHUNK;
            }

            *lineptr = realloc(*lineptr, *n);
        }

        nchars_avail = *n - offset;
        read_pos = *lineptr + offset;

        for (;;)
        {
            int save_errno;
            register int c = getc (stream);

            save_errno = errno;

            /* We always want at least one char left in the buffer, since we
            always (unless we get an error while reading the first char)
            NUL-terminate the line buffer.  */

            assert((*lineptr + *n) == (read_pos + nchars_avail));
            if (nchars_avail < 2)
            {
                if (*n > MIN_CHUNK)
                    *n *= 2;
                else
                    *n += MIN_CHUNK;

                nchars_avail = *n + *lineptr - read_pos;
                *lineptr = realloc (*lineptr, *n);
                if (!*lineptr)
                {
                    errno = ENOMEM;
                    return -1;
                }
                read_pos = *n - nchars_avail + *lineptr;
                assert((*lineptr + *n) == (read_pos + nchars_avail));
            }

            if (ferror (stream))
            {
                /* Might like to return partial line, but there is no
                   place for us to store errno.  And we don't want to just
                   lose errno.  */
                errno = save_errno;
                return -1;
            }

            if (c == EOF)
            {
                /* Return partial line, if any.  */
                if (read_pos == *lineptr)
                    return -1;
                else
                    break;
            }

            *read_pos++ = c;
            nchars_avail--;

            if (c == terminator)
                /* Return the line.  */
                break;
        }

        /* Done - NUL terminate and return the number of chars read.  */
        *read_pos = '\0';

        ret = read_pos - (*lineptr + offset);
        return ret;
    }
}

FcitxConfiguration* _fcitx_ini_parse(Getline)
{
}
