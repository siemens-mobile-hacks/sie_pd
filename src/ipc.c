#include <swilib.h>
#include "ipc.h"

void IPC_Send(IPC_REQ *ipc, int submess) {
    ipc->name_to = IPC_NAME;
    ipc->name_from = IPC_NAME;
    GBS_SendMessage(MMI_CEPID, MSG_IPC, submess, ipc);
}
