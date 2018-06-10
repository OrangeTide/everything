#include <stdio.h>

#define JDM_STREAM_PARSING_IMPLEMENTATION
#include "jdm_stream.h"

void main() {
    char line[256];
    double fl;
    long num;
    char ident[8];
    struct stream_info p;
    while (fgets(line, sizeof(line), stdin)) {
        stream_initialize(&p, line, 0);
        if (stream_read_float(&p, &fl)) {
            printf("Started with float %g, ", fl);
            if (stream_read_integer(&p, &num))
                printf("followed by integer %ld, ", num);
            printf("ends with: \"%s\"\n", stream_remaining(&p, NULL));
        } else if (stream_read_identifier(&p, ident, sizeof(ident))) {
            printf("Started with identifier '%s', ", ident);
            printf("ends with: \"%s\"\n", stream_remaining(&p, NULL));
        } else {
            printf("an error occurred\n");
        }
    }
}
