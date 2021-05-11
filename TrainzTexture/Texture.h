#pragma once
#include <iostream>
#include <vector>
class IOArchive;

//enum class AlphaMode : uint8_t
//{
//	Opaque = 1,
//	Masked = 2,
//	Transparent = 3
//};

enum class AlphaMode : uint32_t
{
	Opaque = 0,
	Transparent,
	Masked
};

enum class TextureType : uint32_t
{
	OneSided = 0,
	TwoSided,
	Cubemap,
	Volume
};


enum class WrapValue : uint32_t
{
	Clamp,
	Clamp_To_Edge,
	Repeat
};
//RGBA8888 | BGRA8888 | RGB0888 | BGR0888 | RGB888 | BGR888 | BGRA4444 | BGR565 | BGR555 | DXT1 | DXT3 | DXT5
enum class TextureFormat : uint32_t
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
	DXT1 = 10,
	DXT3 = 11,
	DXT5 = 12,
	DXT5_BridgeIt = 13,
	MFTS_ETC2 = 28,
	HD4F, //E2TF
	ASTC //E2TF
};

uint8_t GetE2Alpha(const AlphaMode& mode);
AlphaMode GetAlpha(const uint8_t& mode);

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

	//MipData(uint32_t in_size) : size(in_size)
	//{
	//	data = new uint8_t[size];
	//}
	MipData(uint32_t in_size) : data(nullptr), size(in_size) {}
	~MipData()
	{
		delete[] data;
	}
	MipData() : data(nullptr), size(0) {}
};

struct TextureData
{
	std::vector<MipData> textureMips;
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
	//uint32_t TextureCount; //not related to mips! 1 for normal textures, 6 for cubemaps
	WrapValue WrapS;
	WrapValue WrapT;
	TextureFormat Format;

	AlphaMode AlphaBehavior;

	std::vector<TextureData> Textures;
	uint32_t MipCount = 0;

	//TzTexture(const char* filepath);
	virtual bool Serialize(IOArchive& Ar) = 0;

	TzTexture(FileType InType) : ResourceType(InType), Width(0), Height(0) { }
};

enum class MipHint : uint32_t
{
	Static = 0,
	Dynamic = 1
};
class JIRFTexture : public TzTexture
{
public:
	uint32_t size;
	MipHint Hint;
	uint32_t MinFilter;
	uint32_t MagFilter;
	uint32_t MipFilter;
	//TextureUsage UsageType;
	float unknown2 = 1.0f; //world size? 16 for baseboard - also seems to correlate with anisotropy
	uint32_t unknown3[4] = { 0x00 };
	float base_color[4];
public:
	bool Serialize(IOArchive& Ar) override;

	JIRFTexture(IOArchive& Ar);
};

class E2TFTexture : public TzTexture
{
public:
	uint8_t mipSkip = 0;
	WrapValue WrapR;

	uint8_t blockSizeX = 0;
	uint8_t blockSizeY = 0;

	uint8_t colorHint[4];

public:
	bool Serialize(IOArchive& Ar) override;

	E2TFTexture(IOArchive& Ar);
};
