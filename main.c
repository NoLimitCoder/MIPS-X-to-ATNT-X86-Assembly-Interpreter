#include <stdio.h>
#include "xtra.h"


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No file specified\b");
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        return 1;
    }
    xtra(f);
    fclose(f);
    return 0;
}
