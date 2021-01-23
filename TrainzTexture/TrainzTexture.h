#pragma once
#include "Texture.h"
#ifdef TRAINZTEXTURE_EXPORTS
#define TRAINZTEXTURE_API __declspec(dllexport)
#else
#define TRAINZTEXTURE_API __declspec(dllimport)
#endif

extern "C++" TRAINZTEXTURE_API std::shared_ptr<TzTexture> read_texture(const char* filepath);