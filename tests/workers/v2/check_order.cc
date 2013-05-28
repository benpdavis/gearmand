/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2013 Data Differential, http://datadifferential.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <gear_config.h>

#include <libtest/test.hpp>

using namespace libtest;

#include <libgearman-1.0/gearman.h>

#include "tests/workers/v2/check_order.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

gearman_return_t check_order_worker(gearman_job_st *job, void *context)
{
  uint32_t* last_number= (uint32_t*)context;

  errno= 0;
  uint32_t current_number= strtol((const char*)gearman_job_workload(job), NULL, 10);
  if (errno != 0)
  {
    char buffer[1024];
    if (strerror_r(errno, buffer, sizeof(buffer)) == 0)
    {
      gearman_return_t rc= gearman_job_send_exception(job, buffer, strlen(buffer));
      if (rc == GEARMAN_SUCCESS)
      {
        return GEARMAN_WORK_EXCEPTION;
      }
    }

    // Some other failure happened when sending exception
    return GEARMAN_ERROR;
  }

  if ((*last_number) +1 == current_number)
  {
    *last_number= current_number;
    return GEARMAN_SUCCESS;
  }

  char buffer[1024];
  int excep_length= snprintf(buffer, sizeof(buffer), "current number %u != %u +1 (last_number)", current_number, *last_number);
  gearman_return_t rc= gearman_job_send_exception(job, buffer, excep_length);
  if (rc == GEARMAN_SUCCESS)
  {
    return GEARMAN_WORK_EXCEPTION;
  }

  // Some other failure happened when sending exception
  return GEARMAN_ERROR;
}
