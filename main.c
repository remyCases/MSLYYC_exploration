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

static msl_interface_t* global_msl_interface = NULL;

void save_game(FWCodeEvent* code_event)
{
    code_t* code = code_event->args._2;
    int last_status = MSL_UNKNWON_ERROR;

    if (strstr(code->name, "gml_Object_o_player_KeyPress_116") != NULL)
    {
        rvalue_t scr_smoothSaveAuto;
        rvalue_t scr_actionsLogUpdate;
        rvalue_t arg_smoothSaveAuto = init_rvalue_str("scr_smoothSaveAuto");
        rvalue_t arg_actionsLogUpdate = init_rvalue_str("scr_actionsLogUpdate");
        rvalue_t arg_message = init_rvalue_str("You Save Game (Can I play, Daddy?)");

        rvalue_t args[2] = { arg_smoothSaveAuto };
        last_status = global_msl_interface->call_builtin("asset_get_index", args, 1, &scr_smoothSaveAuto);

        args[0] = scr_smoothSaveAuto;
        last_status = global_msl_interface->call_builtin("script_execute", args, 1, NULL);

        args[0] = arg_actionsLogUpdate;
        last_status = global_msl_interface->call_builtin("asset_get_index", args, 1, &scr_actionsLogUpdate);

        args[0] = scr_actionsLogUpdate;
        args[1] = arg_message;
        last_status = global_msl_interface->call_builtin("script_execute", args, 2, NULL);
    }

    code_event->Call();
}

int module_initialize(module_t* module, const char* module_path)
{
    UNREFERENCED_PARAMETER(module_path);

    int last_status = LOG_ON_ERR(ob_get_interface, "YYTK_Main", (msl_interface_base_t**)(&global_msl_interface));
    if (last_status) return MSL_MODULE_DEPENDENCY_NOT_RESOLVED;

    global_msl_interface->print_warning("Hello Mod");
    global_msl_interface->create_callback(module, EVENT_OBJECT_CALL, save_game, 0);

    return MSL_SUCCESS;
}

int main(int argc, char** argv)
{
    char* path = NULL;

    module_t module;
    int last_status = MSL_UNKNWON_ERROR;

    if (argc == 1) path = "data/StoneShard.exe";
    else path = argv[1];
 
    last_status = LOG_ON_ERR(module_initialize, &module, path);

	printf("[>] Execution complete\n");
    return 0;
}
