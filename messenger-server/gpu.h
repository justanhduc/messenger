//
// Created by justanhduc on 11/10/20.
//

#ifndef MESSENGER_SERVER_GPU_H
#define MESSENGER_SERVER_GPU_H

#include "utils.h"

ints getFreeGpuList();
ints selectFreeGpus(int n);
void showFreeGpuInfo();
std::string getCudaVisibleFlag(int n);

#endif // MESSENGER_SERVER_GPU_H
