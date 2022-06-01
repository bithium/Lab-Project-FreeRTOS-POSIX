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
   int res = 0;

   if (rwlock == NULL || rwlock->xIsInitialized == pdTRUE)
      return EINVAL;

   rwlock->xReadLock = xSemaphoreCreateRecursiveMutex();
   if(rwlock->xReadLock == NULL) {
      res = ENOMEM;
      goto _end;
   }

   rwlock->xWriteLock = xSemaphoreCreateRecursiveMutex ();
   if(rwlock->xWriteLock == NULL) {
      res = ENOMEM;
      goto _end;
   }

   rwlock->uReaderCount = 0;
   rwlock->pOwner       = NULL;

   rwlock->xIsInitialized = pdTRUE;

_end:
   if(res) {
      if(rwlock->xReadLock) {
         vSemaphoreDelete(rwlock->xReadLock);
      }
      if(rwlock->xWriteLock) {
         vSemaphoreDelete(rwlock->xWriteLock);
      }
   }

   return res;
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
   res            = xSemaphoreTake (rwlock->xReadLock, (TickType_t) portMAX_DELAY);
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
      res = xSemaphoreTake (rwlock->xWriteLock, portMAX_DELAY);
      configASSERT (res);
   }

_end:
   res = xSemaphoreGive (rwlock->xReadLock);
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

   BaseType_t res = xSemaphoreTake (rwlock->xWriteLock, portMAX_DELAY);
   configASSERT (res);

   rwlock->pOwner = self;

   return 0;
}

/*-----------------------------------------------------------*/

int pthread_rwlock_unlock (pthread_rwlock_t *rwlock)
{
   if (rwlock == NULL || rwlock->xIsInitialized == pdFALSE)
      return EINVAL;

   BaseType_t res = xSemaphoreTake (rwlock->xReadLock, portMAX_DELAY);
   configASSERT (res);

   if(rwlock->uReaderCount >= 1)
      rwlock->uReaderCount--;

   if (rwlock->uReaderCount == 0)
   {
      rwlock->pOwner = NULL;
      res = xSemaphoreGive (rwlock->xWriteLock);
      configASSERT (res);
   }

   res = xSemaphoreGive (rwlock->xReadLock);
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

   if (xSemaphoreGetMutexHolder (rwlock->xReadLock) != NULL)
   {
      return EBUSY;
   }

   if (xSemaphoreGetMutexHolder (rwlock->xWriteLock) != NULL)
   {
      return EBUSY;
   }

   vSemaphoreDelete (rwlock->xReadLock);
   vSemaphoreDelete (rwlock->xWriteLock);

   return 0;
}
