#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "log_memusage.h"
#include "log_memusage_impl.h"

#ifndef ENABLE_NVML

/*
 * Sane defaults when NVML is not available (assume no GPUs).
 */
int log_memusage_ngpus () { return 0; }

log_memusage_gpu_memory_t log_memusage_get_each_gpu ()
{
  log_memusage_gpu_memory_t gpu_memory;
  memset(&gpu_memory, 0, sizeof gpu_memory);
  return gpu_memory;
}

int log_memusage_get_all_gpus () { return 0; }
int log_memusage_get_max_gpu () { return 0; }


#else
#include <nvml.h>



/*
 * Structure to "hide" implementation details,
 * as much as possible in C anyway...
 */
static struct log_memusage_nvml_data_str
{
  bool nvml_initialized;
  unsigned int device_count;
  nvmlDevice_t device[LOG_MEMUSAGE_MAX_GPU_DEVICES];

} log_memusage_impl_nvml_data;



int log_memusage_ngpus ()
{
  if ( ! log_memusage_impl_nvml_data.nvml_initialized )
    return 0;

  return log_memusage_impl_nvml_data.device_count;
}




__attribute__ ((constructor (/* priority = */ 100)))
int log_memusage_initialize_nvml ()
{
  printf("..(constructor)... %s, line: %d\n", __FILE__, __LINE__);

  const int verbose = (getenv("LOG_MEMUSAGE_VERBOSE") != NULL) ? atoi(getenv("LOG_MEMUSAGE_VERBOSE")) : 0;

  int i =0;
  nvmlReturn_t result;
  log_memusage_impl_nvml_data.nvml_initialized = false;
  log_memusage_impl_nvml_data.device_count = 0;


  result = nvmlInit();
  if (NVML_SUCCESS != result)
    {
      /* skip this common known issue, when compiled with NVML but running elsewhere */
      if (NVML_ERROR_DRIVER_NOT_LOADED != result)
        fprintf(stderr, "Failed to initialize NVML: %s\n", nvmlErrorString(result));
      return 1;
    }

  result = nvmlDeviceGetCount(&log_memusage_impl_nvml_data.device_count);
  if (NVML_SUCCESS != result)
    {
      fprintf(stderr, "Failed to query device count: %s\n", nvmlErrorString(result));
      return 1;
    }

  log_memusage_impl_nvml_data.nvml_initialized = true;

  /* fluff below */
  if (verbose)
    {
      log_memusage_msg(stderr, "Listing %u GPU device%s found:\n",
              log_memusage_impl_nvml_data.device_count,
              log_memusage_impl_nvml_data.device_count != 1 ? "s" : "");

      for (i = 0; i < log_memusage_impl_nvml_data.device_count; i++)
        {
          char name[NVML_DEVICE_NAME_BUFFER_SIZE];
          nvmlPciInfo_t pci;
          nvmlMemory_t meminfo;

          result = nvmlDeviceGetHandleByIndex(i, &log_memusage_impl_nvml_data.device[i]);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "Failed to get handle for device %u: %s\n", i, nvmlErrorString(result));

          result = nvmlDeviceGetName(log_memusage_impl_nvml_data.device[i], name, NVML_DEVICE_NAME_BUFFER_SIZE);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "Failed to get name of device %u: %s\n", i, nvmlErrorString(result));

          /* pci.busId is very useful to know which device physically you're talking to */
          /* Using PCI identifier you can also match nvmlDevice handle to CUDA device. */
          result = nvmlDeviceGetPciInfo(log_memusage_impl_nvml_data.device[i], &pci);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "Failed to get pci info for device %u: %s\n", i, nvmlErrorString(result));

          result = nvmlDeviceGetMemoryInfo (log_memusage_impl_nvml_data.device[i], &meminfo);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "\nFailed to get memory info for device %u: %s\n", i, nvmlErrorString(result));

          log_memusage_msg(stderr, "%u. %s [%s] mem: {used:%ul free:%ul total:%ul} (MB)\n",
                           i, name, pci.busId,
                           meminfo.used  / 1024 / 1024,
                           meminfo.free  / 1024 / 1024,
                           meminfo.total / 1024 / 1024);
        }
      printf("sizeof( log_memusage_impl_nvml_data) = %lu\n",  sizeof log_memusage_impl_nvml_data);
    }

  const log_memusage_gpu_memory_t gpu_memory = log_memusage_get_each_gpu();

  return 0;
}



log_memusage_gpu_memory_t log_memusage_get_each_gpu ()
{
  int i=0;
  nvmlReturn_t result;
  log_memusage_gpu_memory_t gpu_memory;
  memset(&gpu_memory, 0, sizeof gpu_memory);

  if ( ! log_memusage_impl_nvml_data.nvml_initialized )
    return gpu_memory;

  gpu_memory.device_count = log_memusage_impl_nvml_data.device_count;

  for (i = 0; i < log_memusage_impl_nvml_data.device_count; i++)
    {
      nvmlMemory_t meminfo;

      result = nvmlDeviceGetMemoryInfo (log_memusage_impl_nvml_data.device[i], &meminfo);
      if (NVML_SUCCESS != result)
        {
          fprintf(stderr, "\nFailed to get memory info for device %u: %s\n", i, nvmlErrorString(result));
          return gpu_memory;
        }

      gpu_memory.used[i]  = meminfo.used  / 1024 / 1024;
      gpu_memory.free[i]  = meminfo.free  / 1024 / 1024;

      gpu_memory.total_used += gpu_memory.used[i];

      if (gpu_memory.used[i] > gpu_memory.max_used)
        gpu_memory.max_used = gpu_memory.used[i];
    }

  return gpu_memory;
}



int log_memusage_get_all_gpus ()
{
  const log_memusage_gpu_memory_t gpu_memory = log_memusage_get_each_gpu();
  return gpu_memory.total_used;
}




int log_memusage_get_max_gpu ()
{
  const log_memusage_gpu_memory_t gpu_memory = log_memusage_get_each_gpu();
  return gpu_memory.max_used;
}



__attribute__ ((destructor  (/* priority = */ 100)))
int log_memusage_finalize_nvml ()
{
  nvmlReturn_t result;
  result = nvmlShutdown();

  log_memusage_impl_nvml_data.nvml_initialized = false;

  if (NVML_SUCCESS != result)
    {
      fprintf(stderr, "Failed to shutdown NVML: %s\n", nvmlErrorString(result));
      return 1;
    }

  return 0;
}


#endif /* #ifdef ENABLE_NVML */
