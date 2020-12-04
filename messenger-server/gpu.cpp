//
// Created by justanhduc on 11/10/20.
//

#include "gpu.h"
#include "logging.h"

#include <iomanip>
#include <nvml.h>

ints getFreeGpuList() {
    ints gpuList;
    int nDevices;
    nvmlReturn_t result;

    result = nvmlInit_v2();
    if (NVML_SUCCESS != result)
        logger.log(ERROR, __FILE__, __LINE__, "Failed to initialize NVML: %s", nvmlErrorString(result));

    result = nvmlDeviceGetCount_v2(reinterpret_cast<unsigned int *>(&nDevices));
    if (NVML_SUCCESS != result)
        logger.log(WARNING, __FILE__, __LINE__, "Failed to get device count: %s", nvmlErrorString(result));
    for (size_t i = 0; i < nDevices; ++i) {
        nvmlMemory_t mem;
        nvmlDevice_t dev;
        result = nvmlDeviceGetHandleByIndex_v2(i, &dev);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__, "Failed to get GPU handle for GPU %d: %s", i, nvmlErrorString(result));
            goto Error;
        }

        result = nvmlDeviceGetMemoryInfo(dev, &mem);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__, "Failed to get GPU memory for GPU %d: %s", i, nvmlErrorString(result));
            goto Error;
        }
        if (mem.free > .9 * mem.total)
            gpuList.push_back(i);
    }
    return gpuList;

    Error:
    {
        result = nvmlShutdown();
        if (NVML_SUCCESS != result)
            logger.log(ERROR, __FILE__, __LINE__, "Failed to shutdown NVML: %s", nvmlErrorString(result));

        return ints();
    }
}

ints selectFreeGpus(int n) {
    ints freeGpus;
    ints gpuList = getFreeGpuList();
    if (gpuList.size() >= n)
        for (int i = 0; i < n; i++)
            freeGpus.push_back(gpuList[i]);
    else
        logger.log(ERROR, __FILE__, __LINE__, "There are %d free GPUs but requested %d", gpuList.size(), n);
    return freeGpus;
}

void showFreeGpuInfo() {
    int nDevices;
    nvmlReturn_t result;

    result = nvmlInit_v2();
    if (NVML_SUCCESS != result)
        logger.log(ERROR, __FILE__, __LINE__, "Failed to initialize NVML: %s", nvmlErrorString(result));

    result = nvmlDeviceGetCount_v2(reinterpret_cast<unsigned int *>(&nDevices));
    if (NVML_SUCCESS != result)
        logger.log(WARNING, __FILE__, __LINE__, "Failed to get device count: %s", nvmlErrorString(result));

    ints freeGpuList = getFreeGpuList();
    std::cout << "Device" << std::setw(7) << "Name" <<
              std::setw(35) << "Total Memory (GB)" << std::setw(20)
              << "Free Memory (GB)" << std::endl;
    for (auto it : freeGpuList) {
        nvmlMemory_t mem;
        nvmlDevice_t dev;
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];
        result = nvmlDeviceGetHandleByIndex_v2(it, &dev);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__, "Failed to get GPU handle for GPU %d: %s", it, nvmlErrorString(result));
            goto Error;
        }

        result = nvmlDeviceGetMemoryInfo(dev, &mem);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__, "Failed to get GPU memory for GPU %d: %s", it, nvmlErrorString(result));
            goto Error;
        }
        result = nvmlDeviceGetName(dev, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__, "Failed to get name of device %u: %s\n", it, nvmlErrorString(result));
            goto Error;
        }
        std::cout << it << std::setw(24) << name <<
                  std::setw(13) << mem.total / (1024. * 1024 * 1024) <<
                  std::setw(21) << mem.free / (1024. * 1024 * 1024) << std::endl;
    }

    Error:
    {
        result = nvmlShutdown();
        if (NVML_SUCCESS != result)
            logger.log(ERROR, __FILE__, __LINE__, "Failed to shutdown NVML: %s", nvmlErrorString(result));
    }
}

std::string getCudaVisibleFlag(int n) {
    auto list = selectFreeGpus(n);
    std::string flag = "CUDA_VISIBLE_DEVICES=";
    for (auto i = 0; i < n; ++i) {
        flag += std::to_string(list[i]);
        if (i < n - 1)
            flag += ",";
    }
    return flag;
}
