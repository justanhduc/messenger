//
// Created by justanhduc on 11/10/20.
//

#ifndef MESSENGER_SERVER_GPU_H
#define MESSENGER_SERVER_GPU_H

#include <tuple>
#include <string>
#include <vector>

#include "utils.h"

// GPU ID, GPU name, free memory, total memory, and usage percentage
#define gpu_info std::vector<std::tuple<int, std::string, double , double, double>>

gpu_info queryGPU();

ints getFreeGpuList();

void showGpuInfo(bool show_free=true);

#endif // MESSENGER_SERVER_GPU_H
