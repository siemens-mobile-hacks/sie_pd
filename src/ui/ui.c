#include <swilib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "edit.h"
#include "options.h"
#include "ui.h"

static HEADER_DESC HEADER_D = {{0, 0, 0, 0}, NULL, (int)"SiePD", LGP_NULL};

static const int SOFTKEYS[] = {
    SET_LEFT_SOFTKEY, SET_MIDDLE_SOFTKEY, SET_RIGHT_SOFTKEY
};

static const SOFTKEY_DESC SOFTKEY_D[] = {
    {0x003D, 0x0000, (int)"Options"},
    {0x0018, 0x0000, (int)LGP_EDIT_PIC},
    {0x0001, 0x0000, (int)"Exit"},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEY_D, 2,
};

static int OnKey(GUI *gui, GUI_MSG *msg) {
    UI_DATA *data = GUI_GetUserPointer(gui);
    if (msg->keys == 0x3D) {
        Options_CreateUI(gui);
    } else if (msg->keys == 0x18) {
        Edit_CreateUI(gui);
    } else if (msg->keys == 0x01) {
        Exit(data);
        return -1;
    }
    return 0;
}

static void GHook(GUI *gui, int cmd) {
    UI_DATA *data = GUI_GetUserPointer(gui);
    if (cmd == UI_CMD_REDRAW) {
        WSHDR *extra_header_ws = AllocWS(8);
        const int items_count = GetMenuItemCount(gui);
        if (items_count) {
            const int item_n = GetCurMenuItem(gui);
            wsprintf(extra_header_ws, "%d/%d", item_n + 1, items_count);
        } else {
            wsprintf(extra_header_ws, "");
        }
        void *header = GetHeaderPointer(gui);
        SetFileNameToHeader(header, data->csm);
        SetHeaderExtraText(header, extra_header_ws, malloc_adr(), mfree_adr());
    } else if (cmd == UI_CMD_DESTROY) {
        pd_free_nodes(data->csm->nodes);
        mfree(data);
    }
}

void ItemProc(void *gui, int item_n, void *user_pointer) {
    const UI_DATA *data = user_pointer;
    pd_node_t *node = data->csm->nodes[item_n];

    const size_t max_len = pd_get_max_group_size() + 1 + pd_get_max_key_size();
    void *item = AllocMenuItem(gui);
    WSHDR *ws = AllocMenuWS(gui, (int)max_len);

    if (strlen(node->group)) {
        wsprintf(ws, "%s.%s", node->group, node->key);
    } else {
        wsprintf(ws, "%s", node->key);
    }
    SetMenuItemText(gui, item, ws, item_n);
}

static const MENU_DESC MENU_D = {
    8,
    OnKey,
    GHook,
    NULL,
    SOFTKEYS,
    &SOFTKEYS_TAB,
    MENU_FLAGS_ENABLE_TEXT_SCROLLING,
    ItemProc,
    NULL,
    NULL,
    0,
};

int CreateUI(MAIN_CSM *csm) {
    pd_node_t **nodes = NULL;
    if (pd_read_file_ws(csm->file_path, &nodes) == 0) {
        memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));

        UI_DATA *data = malloc(sizeof(UI_DATA));
        zeromem(data, sizeof(UI_DATA));
        data->csm = csm;
        data->csm->nodes = nodes;
        return CreateMenu(0, 0, &MENU_D, &HEADER_D, 0, (int)pd_get_size(data->csm->nodes), data, NULL);
    }
    pd_free_nodes(nodes);
    return 0;
}
