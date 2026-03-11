#include <swilib.h>
#include "csm.h"
#include "common.h"

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
