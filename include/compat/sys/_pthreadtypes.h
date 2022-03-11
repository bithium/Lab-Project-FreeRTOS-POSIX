/*
 * Copyright (C) 2022 Bithium S.A.  All Rights Reserved.
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
 * https://www.bithium.com
 * http://www.FreeRTOS.org
 */

/**
 * @file sys/_pthreadtypes.h
 * @brief Pthreads Data types.
 */

#ifndef _COMPAT_SYS_PTHREADTYPES_H_
#define _COMPAT_SYS_PTHREADTYPES_H_

/* C standard library includes. */
#include <stdint.h>

#define pthread_attr_t        pthread_attr_orig_t
#define pthread_barrier_t     pthread_barrier_orig_t
#define pthread_barrierattr_t pthread_barrierattr_orig_t
#define pthread_cond_t        pthread_cond_orig_t
#define pthread_condattr_t    pthread_condattr_orig_t
#define pthread_mutex_t       pthread_mutex_orig_t
#define pthread_mutexattr_t   pthread_mutexattr_orig_t
#define pthread_t             pthread_orig_t

#include_next <sys/_pthreadtypes.h>

#undef pthread_attr_t
#undef pthread_barrier_t
#undef pthread_barrierattr_t
#undef pthread_cond_t
#undef pthread_condattr_t
#undef pthread_mutex_t
#undef pthread_mutexattr_t
#undef pthread_t

/* FreeRTOS types include */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Used to identify a thread attribute object.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_ATTR_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_ATTR_T ) || ( posixconfigENABLE_PTHREAD_ATTR_T == 1 )
    typedef PthreadAttrType_t          pthread_attr_t;
#endif

/**
 * @brief Used to identify a barrier.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_BARRIER_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_BARRIER_T ) || ( posixconfigENABLE_PTHREAD_BARRIER_T == 1 )
    typedef PthreadBarrierType_t       pthread_barrier_t;
#endif

/**
 * @brief Used to define a barrier attributes object.
 */
typedef void *                         pthread_barrierattr_t;

/**
 * @brief Used for condition variables.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_COND_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_COND_T ) || ( posixconfigENABLE_PTHREAD_COND_T == 1 )
    typedef  PthreadCondType_t         pthread_cond_t;
#endif

/**
 * @brief Used to identify a condition attribute object.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_CONDATTR_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_CONDATTR_T ) || ( posixconfigENABLE_PTHREAD_CONDATTR_T == 1 )
    typedef void *                     pthread_condattr_t;
#endif

/**
 * @brief Used for mutexes.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_MUTEX_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_MUTEX_T ) || ( posixconfigENABLE_PTHREAD_MUTEX_T == 1 )
    typedef PthreadMutexType_t         pthread_mutex_t;
#endif

/**
 * @brief Used to identify a mutex attribute object.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_MUTEXATTR_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_MUTEXATTR_T ) || ( posixconfigENABLE_PTHREAD_MUTEXATTR_T == 1 )
    typedef PthreadMutexAttrType_t     pthread_mutexattr_t;
#endif

/**
 * @brief Used to identify a thread.
 *
 * Enabled/disabled by posixconfigENABLE_PTHREAD_T.
 */
#if !defined( posixconfigENABLE_PTHREAD_T ) || ( posixconfigENABLE_PTHREAD_T == 1 )
    typedef void *                     pthread_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _COMPAT_SYS_PTHREADTYPES_H_ */
