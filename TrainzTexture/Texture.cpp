#include "pch.h"
#include <iostream>
#include <sstream>
#include "Texture.h"
#include "IOArchive.h"

//bool TzTexture::SerializeJIRF(IOArchive& Ar)
//{
//	std::cout << "Type: JIRF\n";
//	return true;
//}
//
//bool TzTexture::SerializeE2TF(IOArchive& Ar)
//{
//	std::cout << "Type: E2TF\n";
//	Ar << version;
//	std::cout << "version " << version << "\n";
//
//	return true;
//}

JIRFTexture::JIRFTexture(IOArchive& Ar) : TzTexture(FileType::JIRF)
{
	Serialize(Ar);
}

E2TFTexture::E2TFTexture(IOArchive& Ar) : TzTexture(FileType::E2TF), AlphaBehavior(AlphaMode::Opaque)
{
	Serialize(Ar);
}

bool JIRFTexture::Serialize(IOArchive& Ar)
{
	std::cout << "Type: JIRF\n";
	return true;
}

bool E2TFTexture::Serialize(IOArchive& Ar)
{
	std::cout << "Type: E2TF\n";
	Ar << version;
	Ar << Width;
	Ar << Height;
	std::cout << "version " << version << " size: " << Width << "x" << Height << "\n";
	Ar << unknown1;

	uint8_t MipCount = textureMips.size();
	Ar << MipCount;

	Ar << AlphaBehavior;

	uint8_t TexType = GetE2Type(Type);
	Ar << TexType;
	if (Ar.IsLoading()) Type = GetType(TexType);

	uint8_t TexCount = static_cast<uint8_t>(TextureCount);
	Ar << TexCount;
	if (Ar.IsLoading()) TextureCount = static_cast<uint32_t>(TexCount);

	std::cout << "mip count: " << static_cast<int>(MipCount) << " alpha behavior: " << static_cast<int>(AlphaBehavior) << " Type: " << static_cast<int>(TexType) << " with " << TextureCount << " textures\n";

	uint8_t TexWrapS = GetE2Wrap(WrapS);
	uint8_t TexWrapT = GetE2Wrap(WrapT);
	Ar << TexWrapS;
	Ar << TexWrapT;
	if (Ar.IsLoading()) WrapS = GetWrap(TexWrapS);
	if (Ar.IsLoading()) WrapT = GetWrap(TexWrapT);
	Ar << unknown2;

	char ReadFormat[5];
	strcpy_s(ReadFormat, GetE2Format(Format));
	Ar.Serialize(&ReadFormat[0], 4);
	if (Ar.IsLoading()) Format = GetFormat(&ReadFormat[0]);
	std::stringstream wrapstring;
	if (WrapS != WrapValue::Repeat && WrapT != WrapValue::Repeat)
		wrapstring << "none";
	else
		wrapstring << (WrapS == WrapValue::Repeat ? "s" : "") << (WrapT == WrapValue::Repeat ? "t" : "");

	std::cout << "wrap: " << wrapstring.str() << " format: " << &ReadFormat[0] << "\n";

	Ar.Serialize(&base_color[0], 4);

	if (Ar.IsLoading())
	{
		for (int i = 0; i < MipCount; i++)
		{
			uint32_t mipSize;
			Ar << mipSize;
			textureMips.emplace_back(mipSize);
		}
	}
	
	for (auto& mip : textureMips)
		Ar.Serialize(mip.data, mip.size);

	std::cout << "loaded " << textureMips.size() << " mips\n";

	std::cout << "file end " << Ar.tellg() << "\n";

	return true;
}

uint8_t GetE2Type(const TextureType& type)
{
	switch (type)
	{
	default:
	case TextureType::One:
		return 0;
	case TextureType::Cubemap:
		return 2;
	}
}

TextureType GetType(const uint8_t& type)
{
	switch (type)
	{
	default:
		std::cout << "Unknown type " << static_cast<int>(type) << "\n";
	case 0:
		return TextureType::One;
	case 2:
		return TextureType::Cubemap;
	}
}

uint8_t GetE2Wrap(const WrapValue& wrap)
{
	switch (wrap)
	{
	default:
	case WrapValue::Clamp:
		return 1;
	case WrapValue::Repeat:
		return 3;
	}
}

WrapValue GetWrap(const uint8_t& wrap)
{
	switch (wrap)
	{
	default:
		std::cout << "Unknown wrap value " << static_cast<int>(wrap) << "\n";
	case 1:
		return WrapValue::Clamp;
	case 3:
		return WrapValue::Repeat;
	}
}

const char* GetE2Format(TextureFormat& format)
{
	switch (format)
	{
	default:
	case TextureFormat::RGBA8888:
		return "abgr";
	case TextureFormat::DXT1:
		return "1txd";
	case TextureFormat::DXT3:
		return "3txd";
	case TextureFormat::DXT5:
		return "5txd";
	}
}

TextureFormat GetFormat(const char* format)
{
	if (strcmp(format, "abgr") == 0)
		return TextureFormat::RGBA8888;
	else if (strcmp(format, "1txd") == 0)
		return TextureFormat::DXT1;
	else if (strcmp(format, "3txd") == 0)
		return TextureFormat::DXT3;
	else if (strcmp(format, "5txd") == 0)
		return TextureFormat::DXT5;

	std::cout << "Unknown format " << format << "\n";
	return TextureFormat::RGBA8888;
}
