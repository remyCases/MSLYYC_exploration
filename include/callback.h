// Copyright (C) 2025 Rémy Cases
// See LICENSE file for extended copyright information.
// This file is part of MSLYYC_exploration project from https://github.com/remyCases/MSLYYC_exploration.

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <stdint.h>

#define DISPATCH_CALLBACKS(T)                                                                                               \
int dispatch_callbacks(interface_impl_t* interface_impl, EVENT_TRIGGERS trigger, FUNCTION_WRAPPER(T)* function) {   \
	for(size_t i = 0; i < interface_impl->registered_callbacks.size; i++) {                                             \
		if (interface_impl->registered_callbacks.arr[i].trigger == trigger)                                             \
			(void(*FUNCTION_WRAPPER(T))())(interface_impl->registered_callbacks.arr[i].routine)(function);              \
	}                                                                                                                       \
}

#endif  /* !CALLBACK_H_ */