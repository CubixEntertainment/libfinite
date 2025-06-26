#include "../include/draw/window.h"

FiniteBtn *finite_button_create(FiniteShell *shell, void (*on_select_callback)(FiniteBtn *self, int id, void *data), void (*on_focus_callback)(FiniteBtn *self, int id, void *data), void (*on_unfocus_callback)(FiniteBtn *self, int id, void *data), void *data) {
    FiniteBtn *btn = calloc(1, sizeof(FiniteBtn));
    if (!btn) {
        printf("[Finite] - Unable to create a button");
        return NULL;
    }

    btn->isActive = false;

    // increment and realloc
    if (shell->btns) {
        int x = (shell->_btns + 1);
        FiniteBtn **tmp = realloc(shell->btns, x * sizeof(FiniteBtn *));
        if (!tmp) {
            printf("[Finite] - Unable to realloc the buttons array");
            free(btn);
            return NULL;
        }
        shell->btns = tmp;
    } else {
        shell->btns = calloc(1, sizeof(FiniteBtn **));
        if (!shell->btns) {
            printf("[Finite] - Unable to alloc the buttons array");
            free(btn);
            return NULL;
        }
        // this is button 0 so it needs to be active
        btn->isActive = true;
        shell->activeButton = 0;
    }

    shell->btns[shell->_btns] = btn;
    btn->self = btn;
    btn->id = shell->_btns;
    shell->_btns++;

    printf("[Finite] - Created new button with id %d\n", btn->id);

    btn->on_focus_callback = on_focus_callback;
    btn->on_unfocus_callback = on_unfocus_callback;
    btn->on_select_callback = on_select_callback;
    btn->data = data;
    btn->link = shell;

    // if this is button 0 call the focus callback
    // if (shell->activeButton == 0 && shell->activeButton == btn->id) {
    //     btn->on_focus_callback(btn, btn->id, btn->data);
    // }

    return btn;
}

void finite_button_create_relation(FiniteShell *shell, FiniteBtn *btn, FiniteDirectionType dir, int relation) {
    if (!shell) {
        printf("[Finite] - Unable to create a button relationship with a NULL shell");
        return;
    }
    if (!btn) {
        printf("[Finite] - Unable to create a button relationship with a NULL btn");
        return;
    }
    if (dir < FINITE_DIRECTION_UP || dir > FINITE_DIRECTION_RIGHT_DOWN) {
        printf("[Finite] - Unable to create a button relationship in a NULL direction or direction %d", dir);
        return;
    }
    
    if (relation > shell->_btns) {
        printf("[Finite] - Provided relation id is out of range");
        return;
    }

    if (!btn->relations) {
        btn->relations = calloc(1, sizeof(FiniteBtnRelationShips));
        if (!btn->relations) {
            printf("[Finite] - Unable to create a button relationship");
            return;
        }
    }

    switch (dir) {
        case FINITE_DIRECTION_DOWN:
            btn->relations->down = relation;
            break;
        case FINITE_DIRECTION_UP:
            btn->relations->up = relation;
            break;
        case FINITE_DIRECTION_LEFT:
            btn->relations->left = relation;
            break;
        case FINITE_DIRECTION_RIGHT:
            btn->relations->right = relation;
            break;
        case FINITE_DIRECTION_LEFT_UP:
            btn->relations->leftUp = relation;
            break;
        case FINITE_DIRECTION_LEFT_DOWN:
            btn->relations->leftDown = relation;
            break;
        case FINITE_DIRECTION_RIGHT_UP:
            btn->relations->rightUp = relation;
            break;
        case FINITE_DIRECTION_RIGHT_DOWN:
            btn->relations->rightDown = relation;
            break;
        default:
            printf("[Finite] - Unknown relation provided.");
            break;
    }
}

// remove item id from the array
void finite_button_delete(FiniteShell *shell, int id) {
    if (!shell) {
        printf("[Finite] - Unable to delete button from a non-existent shell");
        return;
    }

    if (id > shell->_btns) {
        printf("[Finite] - Provided id is out of range");
        return;
    }

    // free relation data first
    if (shell->btns[id]->relations) {
        free(shell->btns[id]->relations);
    }

    free(shell->btns[id]);

    // Shift all buttons after the deleted one left by 1
    for (int i = id; i < shell->_btns - 1; i++) {
        shell->btns[i] = shell->btns[i + 1];
        shell->btns[i]->id = i;  // Keep the IDs in sync with array positions
    }

    shell->_btns -= 1;
    if (shell->_btns > 0) {
        FiniteBtn **tmp = realloc(shell->btns, shell->_btns  * sizeof(FiniteBtn *));
        if (!tmp) {
            printf("[Finite] - Unable to realloc the buttons array");
            return;
        }

        shell->btns = tmp;
    } else {
        free(shell->btns);
        shell->btns = NULL;
    }
}

void finite_button_delete_all(FiniteShell *shell) {
    if (!shell) {
        printf("[Finite] - Unable to delete button from a non-existent shell");
        return;
    }

    if (!shell->btns) {
        printf("[Finite] - Shell has no valid buttons");
        return;
    }

    for (int i = 0; i < shell->_btns; i++) {
        if (shell->btns[i]) {
            if (shell->btns[i]->relations) {
                free(shell->btns[i]->relations);
            }
            free(shell->btns[i]);
        }
    }

    free(shell->btns);
    shell->btns = NULL;
    shell->_btns = 0;
}