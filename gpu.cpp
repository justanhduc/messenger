//
// Created by justanhduc on 11/10/20.
//

#include "gpu.h"

#include <iomanip>
#include <cuda_runtime_api.h>

ints getFreeGpuList() {
    ints gpuList;
    int nDevices;
    cudaGetDeviceCount(&nDevices);
    for (int i = 0; i < nDevices; ++i) {
        cudaSetDevice(i);
        size_t freeMem;
        size_t totalMem;
        cudaMemGetInfo(&freeMem, &totalMem);
        if (freeMem > .9 * totalMem)
            gpuList.push_back(i);
    }
    return gpuList;
}

ints selectFreeGpus(int n) {
    ints freeGpus;
    ints gpuList = getFreeGpuList();
    if (gpuList.size() >= n)
        for (int i = 0; i < n; i++)
            freeGpus.push_back(gpuList[i]);
    else {
        logging.log("There are %d free GPUs but requested %d", gpuList.size(), n);
        exit(-1);
    }
    return freeGpus;
}

void showFreeGpuInfo() {
    int nDevices;
    size_t freeMem;
    size_t totalMem;
    cudaGetDeviceCount(&nDevices);
    ints freeGpuList = getFreeGpuList();
    std::cout << "Device" << std::setw(7) << "Name" <<
              std::setw(35) << "Total Memory (GB)" << std::setw(20)
              << "Free Memory (GB)" << std::endl;
    for (auto it : freeGpuList) {
        cudaDeviceProp prop{};
        cudaSetDevice(it);
        cudaMemGetInfo(&freeMem, &totalMem);
        cudaGetDeviceProperties(&prop, it);
        std::cout << it << std::setw(24) << prop.name <<
                  std::setw(13) << totalMem / (1024. * 1024) <<
                  std::setw(21) << freeMem / (1024. * 1024) << std::endl;
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
