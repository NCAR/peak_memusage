#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

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
void log_memusage_initialize_nvml () { return; }
void log_memusage_finalize_nvml () { return; }

#else
#include <nvml.h>



/**
 * \private
 * Structure to "hide" implementation details,
 * as much as possible in C anyway...
 */
static struct log_memusage_nvml_data_str
{
  bool nvml_initialized;
  unsigned int device_count;
  nvmlDevice_t device[LOG_MEMUSAGE_MAX_GPU_DEVICES];

} log_memusage_impl_nvml_data;


/**
 * \private
 * In order to support runtime existance (or not) of libnvidia-ml.so,
 * we do not link with it. Instead we will attempt to load with dlopen(),
 * and access the desired functions through dlsym() lookup.
 */
void *nvml_dl_handle __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlInit) () __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlDeviceGetHandleByIndex) ( unsigned int  index, nvmlDevice_t* device ) __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlDeviceGetMemoryInfo)    ( nvmlDevice_t device, nvmlMemory_t* memory ) __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlShutdown) ( void ) __attribute__ ((visibility ("hidden")));
const DECLDIR char* (*dl_nvmlErrorString) ( nvmlReturn_t result ) __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlDeviceGetName) ( nvmlDevice_t device, char* name, unsigned int  length ) __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlDeviceGetCount) ( unsigned int* deviceCount ) __attribute__ ((visibility ("hidden")));
nvmlReturn_t (*dl_nvmlDeviceGetPciInfo) ( nvmlDevice_t device, nvmlPciInfo_t* pci ) __attribute__ ((visibility ("hidden")));



int log_memusage_ngpus ()
{
  if (!log_memusage_impl_nvml_data.nvml_initialized)
    return 0;

  return log_memusage_impl_nvml_data.device_count;
}




__attribute__ ((visibility ("hidden")))
void log_memusage_initialize_nvml ()
{
  const int verbose = (getenv("LOG_MEMUSAGE_VERBOSE") != NULL) ? atoi(getenv("LOG_MEMUSAGE_VERBOSE")) : 0;

  int i =0;
  nvmlReturn_t result;

  /* run-time detection of libnvidia-ml.so */
  {
    nvml_dl_handle = dlopen ("libnvidia-ml.so", RTLD_LAZY);

    if (NULL == nvml_dl_handle)
      {
        /* fputs (dlerror(), stderr); */

        log_memusage_msg(stderr, "Failed to load libnvidia-ml.so, skipping NVML API\n");

        dl_nvmlInit                   = NULL;
        dl_nvmlDeviceGetHandleByIndex = NULL;
        dl_nvmlDeviceGetMemoryInfo    = NULL;
        dl_nvmlShutdown               = NULL;
        dl_nvmlErrorString            = NULL;
        dl_nvmlDeviceGetName          = NULL;
        dl_nvmlDeviceGetCount         = NULL;
        dl_nvmlDeviceGetPciInfo       = NULL;

        log_memusage_impl_nvml_data.nvml_initialized = true;
        log_memusage_impl_nvml_data.device_count = 0;

        return;
      }

    dl_nvmlInit                   = dlsym(nvml_dl_handle, "nvmlInit");
    dl_nvmlDeviceGetHandleByIndex = dlsym(nvml_dl_handle, "nvmlDeviceGetHandleByIndex");
    dl_nvmlDeviceGetMemoryInfo    = dlsym(nvml_dl_handle, "nvmlDeviceGetMemoryInfo");
    dl_nvmlShutdown               = dlsym(nvml_dl_handle, "nvmlShutdown");
    dl_nvmlErrorString            = dlsym(nvml_dl_handle, "nvmlErrorString");
    dl_nvmlDeviceGetName          = dlsym(nvml_dl_handle, "nvmlDeviceGetName");
    dl_nvmlDeviceGetCount         = dlsym(nvml_dl_handle, "nvmlDeviceGetCount");
    dl_nvmlDeviceGetPciInfo       = dlsym(nvml_dl_handle, "nvmlDeviceGetPciInfo");
  }

  log_memusage_impl_nvml_data.nvml_initialized = false;
  log_memusage_impl_nvml_data.device_count = 0;

  /* OK, now the NVML api is ready... */
  result = dl_nvmlInit();
  if (NVML_SUCCESS != result)
    {
      /* skip this common known issue, when compiled with NVML but running elsewhere */
      if (NVML_ERROR_DRIVER_NOT_LOADED != result)
        fprintf(stderr, "Failed to initialize NVML: %s\n", dl_nvmlErrorString(result));
      return;
    }

  result = dl_nvmlDeviceGetCount (&log_memusage_impl_nvml_data.device_count);
  if (NVML_SUCCESS != result)
    {
      fprintf(stderr, "Failed to query device count: %s\n", dl_nvmlErrorString(result));
      return;
    }

  for (i = 0; i < log_memusage_impl_nvml_data.device_count; i++)
    {
      result = dl_nvmlDeviceGetHandleByIndex (i, &log_memusage_impl_nvml_data.device[i]);
      if (NVML_SUCCESS != result)
        fprintf(stderr, "Failed to get handle for device %u: %s\n", i, dl_nvmlErrorString(result));
    }

  log_memusage_impl_nvml_data.nvml_initialized = true;

  /* fluff below */
  if (verbose)
    {
      log_memusage_msg (stderr, "%u GPU device%s found:\n",
                        log_memusage_impl_nvml_data.device_count,
                        log_memusage_impl_nvml_data.device_count != 1 ? "s" : "");

      for (i = 0; i < log_memusage_impl_nvml_data.device_count; i++)
        {
          char name[NVML_DEVICE_NAME_BUFFER_SIZE];
          nvmlPciInfo_t pci;
          nvmlMemory_t meminfo;

          result = dl_nvmlDeviceGetName (log_memusage_impl_nvml_data.device[i], name, NVML_DEVICE_NAME_BUFFER_SIZE);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "Failed to get name of device %u: %s\n", i, dl_nvmlErrorString(result));

          /* pci.busId is very useful to know which device physically you're talking to */
          /* Using PCI identifier you can also match nvmlDevice handle to CUDA device. */
          result = dl_nvmlDeviceGetPciInfo (log_memusage_impl_nvml_data.device[i], &pci);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "Failed to get pci info for device %u: %s\n", i, dl_nvmlErrorString(result));

          result = dl_nvmlDeviceGetMemoryInfo (log_memusage_impl_nvml_data.device[i], &meminfo);
          if (NVML_SUCCESS != result)
            fprintf(stderr, "\nFailed to get memory info for device %u: %s\n", i, dl_nvmlErrorString(result));

          log_memusage_msg (stderr, "\t%u. %s [%s] mem: {used:%d free:%d total:%d} (MB)\n",
                            i, name, pci.busId,
                            (int) (meminfo.used  / 1024 / 1024),
                            (int) (meminfo.free  / 1024 / 1024),
                            (int) (meminfo.total / 1024 / 1024));
        }
    }

  return;
}



log_memusage_gpu_memory_t log_memusage_get_each_gpu ()
{
  int i=0;
  nvmlReturn_t result;
  log_memusage_gpu_memory_t gpu_memory;
  memset(&gpu_memory, 0, sizeof gpu_memory);

  if ( (! log_memusage_impl_nvml_data.nvml_initialized) ||
       (NULL == dl_nvmlDeviceGetMemoryInfo))
    return gpu_memory;

  gpu_memory.device_count = log_memusage_impl_nvml_data.device_count;

  for (i = 0; i < log_memusage_impl_nvml_data.device_count; i++)
    {
      nvmlMemory_t meminfo;

      result = dl_nvmlDeviceGetMemoryInfo (log_memusage_impl_nvml_data.device[i], &meminfo);
      if (NVML_SUCCESS != result)
        {
          fprintf(stderr, "\nFailed to get memory info for device %u: %s\n", i, dl_nvmlErrorString(result));
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



__attribute__ ((visibility ("hidden")))
void log_memusage_finalize_nvml ()
{
  nvmlReturn_t result;

  log_memusage_impl_nvml_data.nvml_initialized = false;

  if (NULL != dl_nvmlShutdown)
    {
      result = dl_nvmlShutdown();

      if (NVML_SUCCESS != result)
        {
          fprintf(stderr, "Failed to shutdown NVML: %s\n", dl_nvmlErrorString(result));
          return;
        }
    }

  if (NULL != nvml_dl_handle)
    {
      dlclose(nvml_dl_handle);
      nvml_dl_handle = NULL;
    }

  return;
}


#endif /* #ifdef ENABLE_NVML */
