#include <swilib.h>
#include <string.h>
#include "ui.h"
#include "icons.h"
#include "../ipc.h"
#include "../common.h"

#define ITEMS_N 4

static HEADER_DESC HEADER_D = {{0, 0, 0, 0},NULL, (int)"Options", LGP_NULL};

static const int SOFTKEYS[] = {SET_LEFT_SOFTKEY, SET_MIDDLE_SOFTKEY, SET_RIGHT_SOFTKEY};

static const SOFTKEY_DESC SOFTKEYS_D[]= {
    {0x0018,0x0000,(int)"Select"},
    {0x003D,0x0000,(int)LGP_DOIT_PIC},
    {0x0001,0x0000,(int)"Back"},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEYS_D, 3
};

enum {
    MENU_ITEM_ADD_NODE,
    MENU_ITEM_DELETE_NODE,
    MENU_ITEM_SAVE,
    MENU_ITEM_EXIT,
};

static void AddNode_Proc(GUI *gui) {
    GUI *main_gui = GUI_GetUserPointer(gui);
    UI_DATA *data = GUI_GetUserPointer(main_gui);

    pd_node_t *node = pd_alloc_string_node("", "new_node", "");
    pd_add_node(&(data->csm->nodes), node);
    const int size = (int)pd_get_size(data->csm->nodes);
    Menu_SetItemCountDyn(main_gui, size);
    SetCursorToMenuItem(main_gui, size - 1);
    GeneralFuncF1(1);

    data->is_edited = 1;
}

static void DeleteNode_Proc(GUI *gui) {
    GUI *main_gui = GUI_GetUserPointer(gui);
    UI_DATA *data = GUI_GetUserPointer(main_gui);

    const size_t item_n = GetCurMenuItem(main_gui);
    pd_delete_node(&(data->csm->nodes), item_n);
    const int size = (int)pd_get_size(data->csm->nodes);
    Menu_SetItemCountDyn(main_gui, size);
    GeneralFuncF1(1);

    data->is_edited = 1;
}

static void Save_Proc(GUI *gui) {
    GUI *main_gui = GUI_GetUserPointer(gui);
    UI_DATA *data = GUI_GetUserPointer(main_gui);

    static IPC_REQ ipc;
    ipc.data = data;
    IPC_Send(&ipc, IPC_SAVE_FILE);
    GeneralFuncF1(1);
}

static void Exit_Proc(GUI *gui) {
    GUI *main_gui = GUI_GetUserPointer(gui);
    const UI_DATA *data = GUI_GetUserPointer(main_gui);
    Exit(data);
    GeneralFuncF1(1);
}

static int ICON[] = {ICON_EMPTY};

static MENUITEM_DESC ITEMS[ITEMS_N] = {
    {ICON, (int)"Add node", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Delete node", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Save", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
    {ICON, (int)"Exit", LGP_NULL, 0, NULL, MENU_FLAG3, MENU_FLAG2},
};

static const MENUPROCS_DESC PROCS[ITEMS_N] = {
    AddNode_Proc,
    DeleteNode_Proc,
    Save_Proc,
    Exit_Proc,
};

static const MENU_DESC MENU_D = {
    8,
    NULL,
    NULL,
    NULL,
    SOFTKEYS,
    &SOFTKEYS_TAB,
    MENU_FLAGS_ENABLE_TEXT_SCROLLING | MENU_FLAGS_ENABLE_ICONS,
    NULL,
    ITEMS,
    PROCS,
    ITEMS_N
};

int Options_CreateUI(GUI *gui) {
    UI_DATA *data = GUI_GetUserPointer(gui);

    memcpy(&(HEADER_D.rc), GetOptionsHeaderRECT(), sizeof(RECT));

    int items_count = 0;
    int to_remove[ITEMS_N + 1];

    const int items = GetMenuItemCount(gui);
    if (!items) {
        to_remove[++items_count] = MENU_ITEM_DELETE_NODE;
    }
    if (!data->is_edited) {
        to_remove[++items_count] = MENU_ITEM_SAVE;
    }
    to_remove[0] = items_count;
    return CreateMenu(1, 0, &MENU_D, &HEADER_D, 0, items_count, gui, to_remove);
}