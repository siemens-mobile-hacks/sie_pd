#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include "ui/ui.h"
#include "ipc.h"
#include "csm.h"

int MAIN_CSM_ID;

const int minus11 = -11;
unsigned short maincsm_name_body[140];

void SetFileName(MAIN_CSM *csm) {
    uint16_t p = wstrrchr(csm->file_path, csm->file_path->wsbody[0], '\\');
    if (p != 0xFFFF) {
        p++;
        wstrcpybypos(csm->file_name, csm->file_path, p, 128);
    }
}

void UpdateCSMname(MAIN_CSM *csm);

int NE_ShowItemHandler(NATIVE_EXPLORER_ITEM *item) {
    if (!(item->attr & FA_DIRECTORY)) {
        uint16_t p = wstrrchr(item->file_name, item->file_name->wsbody[0], '.');
        if (p != 0xFFFF) {
            p++;
            WSHDR pd, ext;
            uint16_t wsbody_pd[2 + 1], wsbody_ext[2 + 1];
            CreateLocalWS(&ext, wsbody_ext, 2 + 1);
            CreateLocalWS(&pd, wsbody_pd, 2 + 1);
            wstrcpybypos(&ext, item->file_name, p, 2);
            wsprintf(&pd, "%s", "pd");
            if (wstricmp(&ext, &pd) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int NE_UserHandler(NATIVE_EXPLORER_ITEM *item) {
    return 1;
}

static void OnCreate(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    csm->file_name = AllocWS(128);
    if (wstrlen(csm->file_path)) {
        SetFileName(csm);
        UpdateCSMname(csm);
        csm->gui_id = CreateUI(csm);
    } else {
        UpdateCSMname(csm);
        static NativeExplorerData ne_data = { 0 };
        ne_data.mode = NATIVE_EXPLORER_MODE_SELECT;
        ne_data.full_filename = csm->file_path;
        ne_data.show_item_handler = NE_ShowItemHandler;
        ne_data.user_handler = NE_UserHandler;
        csm->pw_gui_id = ShowPleaseWaitBox(1);
        csm->ne_csm_id = StartNativeExplorer(&ne_data);
    }
}

static void OnClose(CSM_RAM *data) {
    MAIN_CSM *csm = (MAIN_CSM*)data;
    FreeWS(csm->file_path);
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
                if (pd_write_file_ws(csm->file_path, (const pd_node_t**)ui_data->csm->nodes) == 0) {
                    ShowMSG(1, (int)"File saved!");
                    ui_data->is_edited = 0;
                }
            }
        }
    } else if (msg->msg == MSG_CSM_DESTROYED) {
        if (csm->ne_csm_id && !FindCSMbyID(csm->ne_csm_id)) {
            if (csm->file_path->wsbody[0]) {
                csm->ne_csm_id = 0;
                SetFileName(csm);
                UpdateCSMname(csm);
                GeneralFunc_flag1(csm->pw_gui_id, 1);
                csm->pw_gui_id = 0;
                csm->gui_id = CreateUI(csm);
            } else {
                CloseCSM(csm->csm.id);
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
    if (csm->file_name->wsbody[0]) {
        wstrcatprintf(ws, " - %w", csm->file_name);
    }
}

int CSM_Create(const char *file_path) {
    MAIN_CSM main_csm = { 0 };
    main_csm.file_path = AllocWS(256);
    if (file_path) {
        str_2ws(main_csm.file_path, file_path, 256);
    }
    LockSched();
    MAIN_CSM_ID = CreateCSM(&MAINCSM.maincsm, &main_csm, 0);
    UnlockSched();
    return MAIN_CSM_ID;
}
