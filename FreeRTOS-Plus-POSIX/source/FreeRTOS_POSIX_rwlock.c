/*
 * Copyright (C) 2021 Bithium S.A.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/pthread.h"

#include "portmacro.h"

int pthread_rwlock_init (pthread_rwlock_t *restrict rwlock,
                         const pthread_rwlockattr_t *restrict attr)
{
   (void) attr;

   if (rwlock == NULL || rwlock->xIsInitialized == pdTRUE)
      return EINVAL;

   SemaphoreHandle_t handle =
       xSemaphoreCreateRecursiveMutexStatic (&(rwlock->xReadLock));
   configASSERT (handle);

   handle = xSemaphoreCreateRecursiveMutexStatic (&(rwlock->xWriteLock));
   configASSERT (handle);

   rwlock->uReaderCount = 0;
   rwlock->pOwner       = NULL;

   rwlock->xIsInitialized = pdTRUE;

   return 0;
}

/*-----------------------------------------------------------*/

int pthread_rwlock_rdlock (pthread_rwlock_t *rwlock)
{
   if (rwlock == NULL || rwlock->xIsInitialized == pdFALSE)
      return EINVAL;

   TaskHandle_t self = xTaskGetCurrentTaskHandle ();

   if (self == rwlock->pOwner)
   {
      return EDEADLK;
   }

   BaseType_t res = pdFALSE;
   res            = xSemaphoreTake ((SemaphoreHandle_t) & (rwlock->xReadLock),
                                    (TickType_t) portMAX_DELAY);
   configASSERT (res);

   int result = 0;

   if (rwlock->uReaderCount + 1 == UINT32_MAX)
   {
      result = EBUSY;
      goto _end;
   }

   rwlock->uReaderCount++;
   if (rwlock->uReaderCount == 1)
   {
      res = xSemaphoreTake ((SemaphoreHandle_t) & (rwlock->xWriteLock),
                            portMAX_DELAY);
      configASSERT (res);
   }

_end:
   res = xSemaphoreGive ((SemaphoreHandle_t) & (rwlock->xReadLock));
   configASSERT (res);

   return result;
}

/*-----------------------------------------------------------*/

int pthread_rwlock_wrlock (pthread_rwlock_t *rwlock)
{
   if (rwlock == NULL || rwlock->xIsInitialized == pdFALSE)
      return EINVAL;

   TaskHandle_t self = xTaskGetCurrentTaskHandle ();

   if (self == rwlock->pOwner)
   {
      return EDEADLK;
   }

   BaseType_t res = xSemaphoreTake ((SemaphoreHandle_t) & (rwlock->xWriteLock),
                                    portMAX_DELAY);
   configASSERT (res);

   rwlock->pOwner = self;

   return 0;
}

/*-----------------------------------------------------------*/

int pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
   if (rwlock == NULL || rwlock->xIsInitialized == pdFALSE)
      return EINVAL;

   BaseType_t res = xSemaphoreTake ((SemaphoreHandle_t) & (rwlock->xReadLock),
                                    portMAX_DELAY);
   configASSERT (res);

   configASSERT (rwlock->uReaderCount >= 1);

   rwlock->uReaderCount--;
   if (rwlock->uReaderCount == 0)
   {
      rwlock->pOwner = NULL;
      res = xSemaphoreGive ((SemaphoreHandle_t) & (rwlock->xWriteLock));
      configASSERT (res);
   }

   res = xSemaphoreGive ((SemaphoreHandle_t) & (rwlock->xReadLock));
   configASSERT (res);

   return 0;
}

/*-----------------------------------------------------------*/

int pthread_rwlock_destroy (pthread_rwlock_t *rwlock)
{
   if (rwlock == NULL || rwlock->xIsInitialized == pdFALSE)
      return EINVAL;

   if (rwlock->uReaderCount > 0)
   {
      return EBUSY;
   }

   if (xSemaphoreGetMutexHolder ((SemaphoreHandle_t) & (rwlock->xReadLock)) !=
       NULL)
   {
      return EBUSY;
   }

   if (xSemaphoreGetMutexHolder ((SemaphoreHandle_t) & (rwlock->xWriteLock)) !=
       NULL)
   {
      return EBUSY;
   }

   vSemaphoreDelete ((SemaphoreHandle_t) & (rwlock->xReadLock));
   vSemaphoreDelete ((SemaphoreHandle_t) & (rwlock->xWriteLock));

   return 0;
}
