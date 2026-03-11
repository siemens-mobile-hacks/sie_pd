#pragma once

enum {
    IPC_SAVE_FILE,
};

#include <swilib.h>

#define IPC_NAME "SiePD"

void IPC_Send(IPC_REQ *ipc, int submess);
