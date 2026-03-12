#pragma once

#include <swilib.h>
#include <pd.h>

typedef struct {
    CSM_RAM csm;
    WSHDR *file_path;
    WSHDR *file_name;
    int gui_id;
    int pw_gui_id;
    int ne_csm_id;
    pd_node_t **nodes;
} MAIN_CSM;

extern int MAIN_CSM_ID;

int CSM_Create(const char *file_path);

