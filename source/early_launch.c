// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#include "../include/error.h"
#include "../include/early_launch.h"

int el_is_process_suspended(bool* suspended)
{
    int last_status;
    system_thread_information_t entrypoint_thread_information = { 0 };

    // Make sure we got the entrypoint thread
    bool flag;
    CHECK_CALL(el_get_entrypoint_thread, entrypoint_thread_information, &flag);
    if (!flag) return MSL_EXTERNAL_ERROR;

    // We're checking if the entrypoint thread is waiting because it's suspended
    if (entrypoint_thread_information.ThreadState != WAITING) return MSL_EXTERNAL_ERROR;

    *suspended = (entrypoint_thread_information.WaitReason == SUSPENDED);
    return last_status;
}