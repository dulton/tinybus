#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "trace.h"

#define MAX_BUF 512

int
main(int argc, char **argv)
{
    FILE *fp;
    char *buf;
    size_t len, buflen;
    int  index;
    
    if (argc < 2)
    {
        fprintf(stdout, "Usage: %s [Filename].\r\n", argv[0]);
        fprintf(stdout, "\tOpen a text file and print its context.\r\n");
        return 0;
    }
    
    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file:%s\r\n", argv[1]);
        return -1;
    }

    buf = (char *)malloc(MAX_BUF);
    if (buf == NULL)
    {
        fclose(fp);
        return -1;
    }
    buflen = MAX_BUF;

    TRACE_SUPPORT_INIT();
    TRACE_ADJUST_LEVEL(TRACE_DETAIL_LEVEL);


    for (index = 0; index < 2; index++)
    {
        while (!feof(fp))
        {
            len = getline(&buf, &buflen, fp);
            if (len > 0)
            {
                TRACE_TRACE("%s", buf);
            }
        }

        fseek(fp, 0, SEEK_SET);
    }

    TRACE_SUPPORT_UNINIT();

    free(buf);
    fclose(fp);

    return 0;
}
