#include "csm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmain"

int main(const char *exe_path, const char *file_path) {
    CSM_Create(file_path);
    return 0;
}

#pragma GCC diagnostic pop
