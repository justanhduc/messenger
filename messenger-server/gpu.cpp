//
// Created by justanhduc on 11/10/20.
//

#include "gpu.h"
#include "logging.h"

#include <iomanip>
#include <nvml.h>


gpu_info queryGPU() {
    gpu_info gpuList;
    int nDevices;
    nvmlReturn_t result;

    result = nvmlInit_v2();
    if (NVML_SUCCESS != result)
        logger.log(ERROR, __FILE__, __LINE__, "Failed to initialize NVML: %s",
                   nvmlErrorString(result));

    result = nvmlDeviceGetCount_v2(reinterpret_cast<unsigned int *>(&nDevices));
    if (NVML_SUCCESS != result)
        logger.log(WARNING, __FILE__, __LINE__, "Failed to get device count: %s",
                   nvmlErrorString(result));
    for (size_t i = 0; i < nDevices; ++i) {
        nvmlMemory_t mem;
        nvmlDevice_t dev;
        char name[NVML_DEVICE_NAME_BUFFER_SIZE];

        result = nvmlDeviceGetHandleByIndex_v2(i, &dev);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__,
                       "Failed to get GPU handle for GPU %d: %s", i,
                       nvmlErrorString(result));
            goto Error;
        }

        result = nvmlDeviceGetMemoryInfo(dev, &mem);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__,
                       "Failed to get GPU memory for GPU %d: %s", i,
                       nvmlErrorString(result));
            goto Error;
        }

        result = nvmlDeviceGetName(dev, name, NVML_DEVICE_NAME_BUFFER_SIZE);
        if (result != 0) {
            logger.log(WARNING, __FILE__, __LINE__,
                       "Failed to get name of device %u: %s\n", i,
                       nvmlErrorString(result));
            goto Error;
        }

        gpuList.emplace_back(i, name, mem.free, mem.total);
    }
    return gpuList;

    Error : {
        result = nvmlShutdown();
        if (NVML_SUCCESS != result)
            logger.log(ERROR, __FILE__, __LINE__, "Failed to shutdown NVML: %s",
                       nvmlErrorString(result));

        return gpuList;
    }
}

ints getFreeGpuList() {
    auto gpuList = queryGPU();
    ints freeGpuList;
    for (auto it : gpuList) {
        auto memfree = std::get<2>(it);
        auto memtotal = std::get<3>(it);
        if (memfree >= .9 * memtotal)
            freeGpuList.push_back(std::get<0>(it));
    }
    return freeGpuList;
}

void showGpuInfo(bool show_free) {
    gpu_info gpuList = queryGPU();
    std::cout << "Device" << std::setw(7) << "Name" << std::setw(35)
            << "Total Memory (GB)" << std::setw(20) << "Free Memory (GB)"
            << std::endl;
    for (auto it : gpuList) {
        auto memfree = std::get<2>(it);
        auto memtotal = std::get<3>(it);
        if (show_free && (memfree < .9 * memtotal))
            continue;

        std::cout << std::get<0>(it) << std::setw(24)
                  << std::get<1>(it) << std::setw(13)
                  << memtotal / (1024. * 1024 * 1024) << std::setw(21)
                  << memfree / (1024. * 1024 * 1024) << std::endl;
    }
}
