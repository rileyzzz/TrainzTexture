#pragma once
#include "TrainzTexture.h"

void SaveDDS(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
void SaveTGA(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
void SavePNG(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
void SaveHDR(const wchar_t* path, const TzTexture& tex, int texIndex = 0);