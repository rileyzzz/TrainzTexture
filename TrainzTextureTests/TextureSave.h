#pragma once
#include "TrainzTexture.h"

bool SaveDDS(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
bool SaveTGA(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
bool SavePNG(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
bool SaveHDR(const wchar_t* path, const TzTexture& tex, int texIndex = 0);
void SaveTextureTXT(const wchar_t* path, const TzTexture& tex);