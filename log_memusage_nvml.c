#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include <stdio.h>
#include <stdbool.h>
#include "log_memusage.h"
#include "log_memusage_impl.h"

#ifdef ENABLE_NVML
#include <nvml.h>


#define MAX_GPU_DEVICES 8

/*
 * Structure to "hide" implementation details,
 * as much as possible in C anyway...
 */
static struct log_memusage_nvml_data_str
{
  bool nvml_initialized;
  unsigned int device_count;
  nvmlDevice_t device[MAX_GPU_DEVICES];

  unsigned long total_mem[MAX_GPU_DEVICES], total_mem_all;
  unsigned long used_mem[MAX_GPU_DEVICES],  used_mem_all;
  unsigned long free_mem[MAX_GPU_DEVICES],  free_mem_all;

} log_memusage_impl_nvml_data;



__attribute__ ((constructor))
int log_memusage_initialize_nvml ()
{
  const int verbose      = (getenv("LOG_MEMUSAGE_VERBOSE")          != NULL) ? atoi(getenv("LOG_MEMUSAGE_VERBOSE")) : 0;

  int i =0;
  nvmlReturn_t result;
  log_memusage_impl_nvml_data.nvml_initialized = false;
  log_memusage_impl_nvml_data.device_count = 0;


  result = nvmlInit();
  if (NVML_SUCCESS != result)
    {
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
      printf("sizeof( log_memusage_impl_nvml_data) = %d\n",  sizeof log_memusage_impl_nvml_data);
    }

  return 0;
}



__attribute__ ((destructor))
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
