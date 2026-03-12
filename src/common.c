#include <swilib.h>
#include "csm.h"
#include "common.h"

void SetFileNameToHeader(void *header, const MAIN_CSM *csm) {
    WSHDR *ws = AllocWS(128);
    wstrcpy(ws, csm->file_name);
    SetHeaderText(header, ws, malloc_adr(), mfree_adr());
}

void HandleExitConfirmation(int canceled) {
    if (!canceled) {
        CloseCSM(MAIN_CSM_ID);
    }
}

void Exit(const UI_DATA *data) {
    if (data->is_edited) {
        MsgBoxYesNo(1, (int)"Unsaved changes. Exit anyway?", HandleExitConfirmation);
    } else {
        HandleExitConfirmation(0);
    }
}
