// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <Zydis/Zydis.h>
#include "include/error.h"
#include "include/pe_parser.h"
#include "include/callback.h"
#include "include/interface.h"
#include "include/function_wrapper.h"
#include "include/object.h"
#include <winnt.h>

static interface_t* global_interface = NULL;

int save_game(FWCodeEvent* code_event)
{
    code_t* code = code_event->args._2;
    int last_status = MSL_SUCCESS;

    if (strstr(code->name, "gml_Object_o_player_KeyPress_116") != NULL)
    {
        rvalue_t scr_smoothSaveAuto;
        rvalue_t scr_actionsLogUpdate;
        rvalue_t arg_smoothSaveAuto = init_rvalue_str("scr_smoothSaveAuto");
        rvalue_t arg_actionsLogUpdate = init_rvalue_str("scr_actionsLogUpdate");
        rvalue_t arg_message = init_rvalue_str("You Save Game (Can I play, Daddy?)");

        rvalue_t args[2] = { arg_smoothSaveAuto };
        CHECK_CALL(global_interface->call_builtin, "asset_get_index", args, 1, &scr_smoothSaveAuto);

        args[0] = scr_smoothSaveAuto;
        CHECK_CALL(global_interface->call_builtin, "script_execute", args, 1, NULL);

        args[0] = arg_actionsLogUpdate;
        CHECK_CALL(global_interface->call_builtin, "asset_get_index", args, 1, &scr_actionsLogUpdate);

        args[0] = scr_actionsLogUpdate;
        args[1] = arg_message;
        CHECK_CALL(global_interface->call_builtin, "script_execute", args, 2, NULL);
    }

    CHECK_CALL(code_event->Call);
    return last_status;
}

int module_initialize(module_t* module, const char* module_path)
{
    UNREFERENCED_PARAMETER(module_path);
    int last_status = MSL_SUCCESS;
    CHECK_CALL_CUSTOM_ERROR(ob_get_interface, MSL_MODULE_DEPENDENCY_NOT_RESOLVED, "YYTK_Main", (interface_base_t**)(&global_interface));

    CHECK_CALL(global_interface->print_warning, "Hello Mod");
    CHECK_CALL(global_interface->create_callback, module, EVENT_OBJECT_CALL, save_game, 0);

    return last_status;
}

int main(int argc, char** argv)
{
    char* path = NULL;

    module_t module;
    int last_status = MSL_SUCCESS;

    if (argc == 1) path = "data/StoneShard.exe";
    else path = argv[1];
 
    CHECK_CALL(module_initialize, &module, path);

	printf("[>] Execution complete\n");
    return 0;
}
