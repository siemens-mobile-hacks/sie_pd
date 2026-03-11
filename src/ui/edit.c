#include <swilib.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"
#include "edit.h"

enum {
    EC_GROUP = 2,
    EC_KEY = 4,
    EC_TYPE = 6,
    EC_VALUE = 8,
};

typedef struct {
    UI_DATA *ui_data;
    pd_node_t *node;
    int cbox_type_id;
} EDIT_DATA;

static HEADER_DESC HEADER_D = {{0, 0, 0, 0}, NULL, (int)"SiePD", LGP_NULL};

static SOFTKEY_DESC SOFTKEY_D[] = {
    {0x0018, 0x0000, (int)"Save"},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEY_D, 0,
};

void GetType_ws(WSHDR *ws, pd_node_type_t type) {
    if (type == PD_NODE_INT) {
        wsprintf(ws, "%s", "Integer");
    } else if (type == PD_NODE_STR) {
        wsprintf(ws, "%s", "String");
    } else {
        wsprintf(ws, "%s", "Unknown");
    }
}

static int OnKey(GUI *gui, GUI_MSG *msg) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);

    EDITCONTROL ec;
    if (msg->keys == 0x18) {
        EDITCONTROL ec_key;
        EDITCONTROL ec_value;
        ExtractEditControl(gui, EC_KEY, &ec_key);
        ExtractEditControl(gui, EC_VALUE, &ec_value);
        if (!wstrlen(ec_key.pWS)) {
            MsgBoxError(1, (int)"\"Key\" is empty!");
            return 0;
        }
        ExtractEditControl(gui, EC_GROUP, &ec);
        ws_2str(ec.pWS, data->node->group, pd_get_max_group_size());
        ws_2str(ec_key.pWS, data->node->key, pd_get_max_key_size());
        ExtractEditControl(gui, EC_TYPE, &ec);
        data->node->type = ec.start_item_cbox - 1;

        char *value = malloc(pd_get_max_value_size() + 1);
        ws_2str(ec_value.pWS, value, pd_get_max_value_size());
        if (*value) {
            if (data->node->type == PD_NODE_INT) {
                data->node->value.integer = strtol(value, NULL, 10);
            } else {
                strcpy(data->node->value.string, value);
            }
        } else {
            data->node->type = PD_NODE_STR;
            data->node->value.string[0] = '\0';
        }
        mfree(value);
        data->ui_data->is_edited = 1;
        return 1;
    }
    return 0;
}

static void GHook(GUI *gui, int cmd) {
    EDIT_DATA *data = EDIT_GetUserPointer(gui);

    WSHDR ws;
    EDITCONTROL ec;
    uint16_t wsbody[128];
    CreateLocalWS(&ws, wsbody, 128);


    if (cmd == TI_CMD_COMBOBOX_FOCUS) {
        const int item_n = EDIT_GetItemNumInFocusedComboBox(gui);
        if (!item_n) { // for header
            ExtractEditControl(gui, EDIT_GetFocus(gui) - 1, &ec);
            wstrcpy(&ws,ec.pWS);
        } else {
            GetType_ws(&ws, item_n - 1);
        }
        EDIT_SetTextToFocused(gui, &ws);
    } else if (cmd == TI_CMD_REDRAW) {
        SetSoftKey(gui, &SOFTKEY_D[0], SET_LEFT_SOFTKEY);

        ExtractEditControl(gui, EC_TYPE, &ec);
        if (ec.start_item_cbox != data->cbox_type_id) {
            data->cbox_type_id = ec.start_item_cbox;
            EDITCONTROL ec_value;
            ExtractEditControl(gui, EC_VALUE, &ec_value);
            if (data->cbox_type_id == 1) { // Integer
                ec_value.type = ECT_NORMAL_NUM;
            } else { // Other
                ec_value.type = ECT_NORMAL_TEXT;
            }
            StoreEditControl(gui, 8, &ec_value);
            EDIT_SetTextToEditControl(gui, 8, ec_value.pWS);
        }
    } else if (cmd == TI_CMD_DESTROY) {
        mfree(data);
    }
}

static INPUTDIA_DESC INPUTDIA_D = {
    1,
    OnKey,
    GHook,
    NULL,
    0,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_SMALL,
    0x64,
    0x65,
    0,
    0,
    INPUTDIA_FLAGS_SWAP_SOFTKEYS,
};

int Edit_CreateUI(GUI *gui) {
    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(INPUTDIA_D.rc), GetMainAreaRECT(), sizeof(RECT));

    UI_DATA *ui_data = GUI_GetUserPointer(gui);
    const int item_n = GetCurMenuItem(gui);

    EDIT_DATA *data = malloc(sizeof(EDIT_DATA));
    zeromem(data, sizeof(EDIT_DATA));
    data->ui_data = ui_data;
    data->node = ui_data->csm->nodes[item_n];

    const void *ma = malloc_adr();
    void *eq = AllocEQueue(ma, mfree_adr());

    WSHDR ws;
    size_t len = 0;
    uint16_t wsbody[128];
    EDITCONTROL ec;
    PrepareEditControl(&ec);
    CreateLocalWS(&ws, wsbody, 128);

    wsprintf(&ws, "%s", "Group:")
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    len = pd_get_max_group_size();
    str_2ws(&ws, data->node->group, len);
    ConstructEditControl(&ec, ECT_NORMAL_TEXT, ECF_APPEND_EOL, &ws, (int)len);
    AddEditControlToEditQend(eq, &ec, ma);

    wsprintf(&ws, "%s", "Key:")
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    len = pd_get_max_key_size();
    str_2ws(&ws, data->node->key, len);
    ConstructEditControl(&ec, ECT_NORMAL_TEXT, ECF_APPEND_EOL, &ws, (int)len);
    AddEditControlToEditQend(eq, &ec, ma);

    wsprintf(&ws, "%s", "Type:")
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    data->cbox_type_id = (int)data->node->type + 1;
    GetType_ws(&ws, data->node->type);
    ConstructComboBox(&ec, ECT_COMBO_BOX, ECF_APPEND_EOL, &ws, 32, 32,
            2, data->cbox_type_id);
    AddEditControlToEditQend(eq, &ec, ma);

    wsprintf(&ws, "%s", "Value:")
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    int ect = ECT_NORMAL_TEXT;
    len = pd_get_max_value_size();
    if (data->node->type == PD_NODE_INT) {
        wsprintf(&ws, "%d", data->node->value.integer);
        ect = ECT_NORMAL_NUM;
    } else {
        str_2ws(&ws, data->node->value.string, len);
    }
    ConstructEditControl(&ec, ect, ECF_APPEND_EOL, &ws, (int)len);
    AddEditControlToEditQend(eq, &ec, ma);

    return CreateInputTextDialog(&INPUTDIA_D, &HEADER_D, eq, 1, data);
}
