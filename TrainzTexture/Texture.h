#pragma once
#include <iostream>
#include <vector>
class IOArchive;

enum class AlphaMode : uint8_t
{
	Opaque = 1,
	Masked = 2,
	Transparent = 3
};

enum class TextureType
{
	One,
	Two,
	Cubemap,
	Volume
};

enum class WrapValue
{
	Clamp,
	Clamp_To_Edge,
	Repeat
};
//RGBA8888 | BGRA8888 | RGB0888 | BGR0888 | RGB888 | BGR888 | BGRA4444 | BGR565 | BGR555 | DXT1 | DXT3 | DXT5
enum class TextureFormat
{
	RGBA8888 = 0,
	BGRA8888,
	RGB0888,
	BGR0888,
	RGB888,
	BGR888,
	BGRA4444,
	BGR565,
	BGR555,
	DXT1,
	DXT3,
	DXT5
};

uint8_t GetE2Type(const TextureType& type);
TextureType GetType(const uint8_t& type);

uint8_t GetE2Wrap(const WrapValue& wrap);
WrapValue GetWrap(const uint8_t& wrap);

const char* GetE2Format(TextureFormat& format);
TextureFormat GetFormat(const char* format);

struct MipData
{
public:
	uint32_t size;
	uint8_t* data;

	MipData(uint32_t in_size) : size(in_size)
	{
		data = new uint8_t[size];
	}
	~MipData()
	{
		delete[] data;
	}
};

class TzTexture
{
public:
	enum class FileType
	{
		JIRF,
		E2TF
	};
	FileType ResourceType;
	uint32_t version = 0;
	uint32_t Width;
	uint32_t Height;
	TextureType Type;
	uint32_t TextureCount; //not related to mips! 1 for normal textures, 6 for cubemaps
	WrapValue WrapS;
	WrapValue WrapT;
	TextureFormat Format;

	std::vector<MipData> textureMips;

	//TzTexture(const char* filepath);
	virtual bool Serialize(IOArchive& Ar) = 0;

	TzTexture(FileType InType) : ResourceType(InType), Width(0), Height(0) { }
};

class JIRFTexture : public TzTexture
{
public:
	bool Serialize(IOArchive& Ar) override;

	JIRFTexture(IOArchive& Ar);
};

class E2TFTexture : public TzTexture
{
public:
	uint8_t unknown1 = 0;
	AlphaMode AlphaBehavior;
	uint8_t unknown2 = 1;
	uint8_t base_color[4];

public:
	bool Serialize(IOArchive& Ar) override;

	E2TFTexture(IOArchive& Ar);
};
