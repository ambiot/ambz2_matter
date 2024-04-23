/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
    See http://www.freertos.org/a00110.html for an explanation of the
    definitions contained in this file.
******************************************************************************/

#ifndef FREERTOS_CONFIG_MATTER_H
#define FREERTOS_CONFIG_MATTER_H

#if defined(CONFIG_PLATFORM_8710C)
#undef configUSE_TRACE_FACILITY
#undef configRECORD_STACK_HIGH_ADDRESS
#undef INCLUDE_uxTaskGetStackSize
#undef INCLUDE_uxTaskGetFreeStackSize

#define configUSE_TRACE_FACILITY                1
#define configRECORD_STACK_HIGH_ADDRESS         1
#define INCLUDE_uxTaskGetStackSize              1
#define INCLUDE_uxTaskGetFreeStackSize          1

#elif defined(CONFIG_PLATFORM_8721D)

#undef configTOTAL_HEAP_SIZE
#undef CONFIG_DYNAMIC_HEAP_SIZE
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 200 * 1024 ) ) //default
#define CONFIG_DYNAMIC_HEAP_SIZE                0

#define INCLUDE_uxTaskGetStackSize              1
#define INCLUDE_uxTaskGetFreeStackSize          1
#endif

#endif /* FREERTOS_CONFIG_MATTER_H */
