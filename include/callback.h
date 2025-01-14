// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>
#include "interface.h"

#define DISPATCH_CALLBACKS(T)                                                                                               \
int dispatch_callbacks(msl_interface_impl_t* msl_interface_impl, EVENT_TRIGGERS trigger, FUNCTION_WRAPPER(T)* function) {   \
	for(size_t i = 0; i < msl_interface_impl->registered_callbacks.size; i++) {                                             \
		if (msl_interface_impl->registered_callbacks.arr[i].trigger == trigger)                                             \
			(void(*FUNCTION_WRAPPER(T))())(msl_interface_impl->registered_callbacks.arr[i].routine)(function);              \
	}                                                                                                                       \
}

#endif  /* !CALLBACK_H_ */