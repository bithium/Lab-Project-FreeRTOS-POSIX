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
#include "portable.h"
#include <stdint.h>

#ifndef PTHREAD_KEYS_INDEX
#define PTHREAD_KEYS_INDEX 1
#endif

#ifndef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX 10
#endif

typedef struct _pthread_key_entry_t
{
   pthread_key_t key;
   void *value;
   void (*destructor) (void *);
   struct _pthread_key_entry_t *next;
} pthread_key_entry_t;

typedef struct _pthread_key_value_t
{
   pthread_key_t key;
   void *value;
   struct _pthread_key_value_t *next;
} pthread_key_value_t;

static uint32_t s_next_key = 0;
static pthread_key_entry_t *s_keys = NULL;

/*-----------------------------------------------------------*/

static void *find_key(pthread_key_t key)
{
   for(pthread_key_entry_t *it = s_keys; it; it = it->next)
   {
      if(it->key == key)
      {
         return it;
      }
   }

   return NULL;
}

static pthread_key_value_t *find_value(pthread_key_t key)
{
   TaskHandle_t handle = xTaskGetCurrentTaskHandle ();
   pthread_key_value_t *entry =
       (pthread_key_value_t *) pvTaskGetThreadLocalStoragePointer (
           handle, PTHREAD_KEYS_INDEX);

   for (; entry; entry = entry->next)
   {
      if (entry->key == key)
      {
         return entry;
      }
   }

   return NULL;
}

/*-----------------------------------------------------------*/

int pthread_key_create (pthread_key_t *key, void (*destructor) (void *))
{
   pthread_key_entry_t *new_entry =
       (pthread_key_entry_t *) pvPortMalloc (sizeof (pthread_key_entry_t));

   if (new_entry == NULL)
   {
      return ENOMEM;
   }

   vTaskSuspendAll();

   new_entry->key        = ++s_next_key;
   new_entry->destructor = destructor;
   new_entry->value      = NULL;
   new_entry->next       = NULL;

   new_entry->next = s_keys;

   s_keys = new_entry;

   *key = new_entry->key;

   xTaskResumeAll();

   return 0;
}

/*-----------------------------------------------------------*/

int pthread_key_delete (pthread_key_t key)
{
   vTaskSuspendAll();

   for(pthread_key_entry_t **it = &s_keys; *it; it = &((*it)->next)) {
      if((*it)->key == key) {
         pthread_key_entry_t *entry = *it;
         *it = (*it)->next;
         vPortFree(entry);
         xTaskResumeAll();
         return 0;
      }
   }

   xTaskResumeAll();

   return EINVAL;
}

/*-----------------------------------------------------------*/

void *pthread_getspecific (pthread_key_t key)
{
   pthread_key_entry_t * key_entry = find_key(key);
   if(key_entry == NULL)
   {
      return NULL;
   }

   pthread_key_value_t *entry = find_value(key);
   if(entry)
      return entry->value;

   return NULL;
}

/*-----------------------------------------------------------*/

int pthread_setspecific (pthread_key_t key, const void *value)
{
   pthread_key_entry_t * key_entry = find_key(key);
   if(key_entry == NULL)
   {
      return EINVAL;
   }

   pthread_key_value_t *entry = find_value(key);
   if(entry) {
      entry->value = (void *) value;
      return 0;
   }

   pthread_key_value_t *new_entry =
       (pthread_key_value_t *) pvPortMalloc (sizeof (pthread_key_value_t));

   if (new_entry == NULL)
   {
      return ENOMEM;
   }

   new_entry->key = key;
   new_entry->value = (void *) value;
   new_entry->next = NULL;

   TaskHandle_t handle = xTaskGetCurrentTaskHandle ();
   entry = (pthread_key_value_t *) pvTaskGetThreadLocalStoragePointer (
         handle, PTHREAD_KEYS_INDEX);

   if (entry)
   {
      new_entry->next = entry->next;
      entry->next     = new_entry;
   }
   else
   {
      vTaskSetThreadLocalStoragePointer (handle, PTHREAD_KEYS_INDEX,
                                         (void *) new_entry);
   }

   return 0;
}

/*-----------------------------------------------------------*/

void pthread_key_cleanup(TaskHandle_t handle)
{
   pthread_key_value_t *entry = (pthread_key_value_t *)
      pvTaskGetThreadLocalStoragePointer (handle, PTHREAD_KEYS_INDEX);

   if(entry == NULL)
      return;

   uint32_t count = 0;
   do {
      count = 0;
      for(pthread_key_value_t *it = entry; it; it = it->next) {
         if(it->value == NULL)
            continue;

         pthread_key_entry_t *key = find_key(it->key);
         if(key == NULL) {
            it->value = NULL;
            continue;
         }
         if(key->destructor) {
            key->destructor(it->value);
            count++;
         }
      }
   } while(count > 0);

   while(entry) {
      pthread_key_value_t *current = entry;
      entry = current->next;
      vPortFree(current);
   }
}
