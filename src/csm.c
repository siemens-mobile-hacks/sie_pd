#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include "ui/ui.h"
#include "ipc.h"
#include "csm.h"

int MAIN_CSM_ID;

const int minus11 = -11;
unsigned short maincsm_name_body[140];

void UpdateCSMname(MAIN_CSM *csm);

static void OnCreate(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    csm->file_name = AllocWS(128);
    char file_name[128];
    const char *slash = strrchr(csm->file_path, '\\');
    const char *end = strrchr(csm->file_path, '\0');
    const size_t len = end - slash;
    strncpy(file_name, slash + 1, len);
    file_name[len] = '\0';
    str_2ws(csm->file_name, file_name, 128);
    UpdateCSMname(csm);
    csm->gui_id = CreateUI(csm);
}

static void OnClose(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    FreeWS(csm->file_name);
    SUBPROC(kill_elf);
}

static int OnMessage(CSM_RAM *data, GBS_MSG *msg) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    if ((msg->msg == MSG_GUI_DESTROYED) && ((int)msg->data0 == csm->gui_id)) {
        csm->csm.state = CSM_STATE_CLOSED;
    } else if (msg->msg == MSG_IPC) {
        IPC_REQ *ipc = msg->data0;
        if (strcmpi(ipc->name_to, IPC_NAME) == 0) {
            if (msg->submess == IPC_SAVE_FILE) {
                UI_DATA *ui_data = ipc->data;
                if (pd_write_file(csm->file_path, (const pd_node_t**)ui_data->csm->nodes) == 0) {
                    ShowMSG(1, (int)"File saved!");
                    ui_data->is_edited = 0;
                }
            }
        }
    }
    return 1;
}

static const struct {
    CSM_DESC maincsm;
    WSHDR maincsm_name;
} MAINCSM = {
    {
        OnMessage,
        OnCreate,
#ifdef NEWSGOLD
        0,
        0,
        0,
        0,
#endif
        OnClose,
        sizeof(MAIN_CSM),
        1,
        &minus11
    },
    {
        maincsm_name_body,
        NAMECSM_MAGIC1,
        NAMECSM_MAGIC2,
        0x0,
        139,
        0
        }
};

void UpdateCSMname(MAIN_CSM *csm) {
    WSHDR *ws = (WSHDR*)&(MAINCSM.maincsm_name);
    wsprintf(ws, "%s", "SiePD");
    if (csm->file_name) {
        wstrcatprintf(ws, " - %w", csm->file_name);
    }
}

int CSM_Create(const char *file_path) {
    MAIN_CSM main_csm = { 0 };
    strcpy(main_csm.file_path, file_path);
    LockSched();
    UpdateCSMname(&main_csm);
    MAIN_CSM_ID = CreateCSM(&MAINCSM.maincsm, &main_csm, 0);
    UnlockSched();
    return MAIN_CSM_ID;
}
