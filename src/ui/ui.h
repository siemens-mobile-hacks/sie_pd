#pragma once

#include "../csm.h"

typedef struct {
    MAIN_CSM *csm;
    int is_edited;
} UI_DATA;

int CreateUI(MAIN_CSM *csm);
