#pragma once
#include "TrainzTexture.h"
#define NOMINMAX
#include <windows.h>

uint8_t* decompressETC2(uint32_t size, const uint8_t* data, const TzTexture& tex);