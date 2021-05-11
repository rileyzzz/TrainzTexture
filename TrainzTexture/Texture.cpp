#include "pch.h"
#include <iostream>
#include <sstream>
#include <algorithm>
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
	if (!Serialize(Ar))
		std::cout << "Encountered file read error at offset " << Ar.tellg() << "\n";
}

E2TFTexture::E2TFTexture(IOArchive& Ar) : TzTexture(FileType::E2TF)
{
	if(!Serialize(Ar))
		std::cout << "Encountered file read error at offset " << Ar.tellg() << "\n";
}

bool JIRFTexture::Serialize(IOArchive& Ar)
{
	std::cout << "Type: JIRF\n";
	Ar << size;
	if (!Ar.ChunkHeader("MMTF")) return false;
	if (!Ar.ChunkHeader("INFO")) return false;
	uint32_t infosize = 60;
	Ar << infosize;
	version = 0x103;
	Ar << version;
	if (version != 0x103 && version != 0x104 && version != 0x107 && version != 0x108) //104 bridge it
	{
		std::cout << "Unsupported info version!\n";
		return false;
	}
	Ar << Type;
	//uint32_t CompressionLevel = 0;
	//switch (Format)
	//{
	//case TextureFormat::DXT1:
	//	CompressionLevel = 1;
	//	break;
	//case TextureFormat::DXT3:
	//	CompressionLevel = 2;
	//	break;
	//case TextureFormat::DXT5:
	//	CompressionLevel = 3;
	//	break;
	//}
	//Ar << CompressionLevel;

	//if (Textures.size()) MipCount = Textures[0].textureMips.size();
	Ar << MipCount;
	
	uint32_t TexCount = Textures.size();
	Ar << TexCount;
	if (Ar.IsLoading()) Textures.resize(TexCount);

	Ar << Format;

	Ar << Width;
	Ar << Height;

	Ar << Hint;
	Ar << WrapS;
	Ar << WrapT;

	std::cout << "mips: " << MipCount << " textures: " << TexCount << " format: " << (uint32_t)Format << " size: " << Width << "x" << Height << "\n";
		 
	uint32_t Reserved = 0;
	Ar << Reserved;

	Ar << MinFilter;
	Ar << MagFilter;
	Ar << MipFilter;

	//uint32_t AlphaHint = (uint8_t)AlphaBehavior - 1;
	Ar << AlphaBehavior;
	//AlphaBehavior = (AlphaMode)(AlphaHint + 1);

	//if (version == 0x108) Ar << unknown3;
	if(version >= 0x104) Ar << unknown2;
	//if (version == 0x108) Ar << unknown4;
	if (version >= 0x107)
		Ar << unknown3;

	if (version == 0x108) Ar << base_color;
	
	std::cout << "info end " << Ar.tellg() << "\n";
	for (auto& tex : Textures)
	{
		if(Ar.IsLoading()) tex.textureMips.resize(MipCount);
		for (auto it = tex.textureMips.rbegin(); it != tex.textureMips.rend(); it++)
		{
			auto& mip = *it;
			if (!Ar.ChunkHeader("MMAP")) return false;
			Ar << mip.size;
			if (Ar.IsLoading()) mip.data = new uint8_t[mip.size];
			Ar.Serialize(mip.data, mip.size);
			std::cout << "mip size " << mip.size << "\n";
			//std::cout << "location " << Ar.tellg() << "\n";
		}
	}
	return true;
}

bool E2TFTexture::Serialize(IOArchive& Ar)
{
	std::cout << "Type: E2TF\n";
	Ar << version;
	Ar << Width;
	Ar << Height;
	std::cout << "version " << version << " size: " << Width << "x" << Height << "\n";
	Ar << mipSkip;

	//if(Ar.IsSaving() && Textures.size()) MipCount = Textures[0].textureMips.size();
	uint8_t E2MipCount = MipCount;
	Ar << E2MipCount;
	if (Ar.IsLoading()) MipCount = E2MipCount;

	uint8_t AlphaHint = GetE2Alpha(AlphaBehavior);
	Ar << AlphaHint;
	if (Ar.IsLoading()) AlphaBehavior = GetAlpha(AlphaHint);

	uint8_t TexType = GetE2Type(Type);
	Ar << TexType;
	if (Ar.IsLoading()) Type = GetType(TexType);

	uint8_t TexCount = Textures.size();
	Ar << TexCount;
	if (Ar.IsLoading()) Textures.resize(TexCount);

	std::cout << "mip count: " << static_cast<int>(MipCount) << " alpha behavior: " << static_cast<int>(AlphaBehavior) << " Type: " << static_cast<int>(TexType) << " with " << Textures.size() << " textures\n";

	uint8_t TexWrapS = GetE2Wrap(WrapS);
	uint8_t TexWrapT = GetE2Wrap(WrapT);
	uint8_t TexWrapR = GetE2Wrap(WrapR);
	Ar << TexWrapS;
	Ar << TexWrapT;
	Ar << TexWrapR;
	if (Ar.IsLoading()) WrapS = GetWrap(TexWrapS);
	if (Ar.IsLoading()) WrapT = GetWrap(TexWrapT);
	if (Ar.IsLoading()) WrapR = GetWrap(TexWrapR);

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

	if (Format == TextureFormat::ASTC)
	{
		Ar << blockSizeX;
		Ar << blockSizeY;
		printf("ASTC block size %u %u\n", blockSizeX, blockSizeY);
	}

	Ar.Serialize(&colorHint[0], 4);

	

	for (int i = 0; i < MipCount; i++)
	{
		for (auto& tex : Textures)
		{
			uint32_t mipSize = 0;
			if (Ar.IsSaving())
				mipSize = tex.textureMips[i].size;
			Ar << mipSize;
			std::cout << "mip size: " << mipSize << "\n";
			//textureMips.emplace(textureMips.begin(), mipSize);
			if (Ar.IsLoading()) tex.textureMips.emplace_back(mipSize);
			//textureMips[i] = MipData(mipSize);
		}
	}
	

	
	//for (int i = 0; i < textureMips.size(); i++)
	//{
	//	auto& mip = textureMips[i];
	//	mip.data = new uint8_t[mip.size];
	//	Ar.Serialize(mip.data, mip.size);
	//}

	//might need to go the other way
	for (int i = MipCount - 1; i >= 0; i--)
	{
		for (auto& tex : Textures)
		{
			auto& mip = tex.textureMips[i];
			if(Ar.IsLoading()) mip.data = new uint8_t[mip.size];
			Ar.Serialize(mip.data, mip.size);
		}
	}
	//for (auto& tex : Textures)
	//{
	//	for (auto it = tex.textureMips.rbegin(); it != tex.textureMips.rend(); it++)
	//	{
	//		auto& mip = *it;
	//		mip.data = new uint8_t[mip.size];
	//		Ar.Serialize(mip.data, mip.size);
	//	}
	//	std::cout << "loaded " << tex.textureMips.size() << " mips\n";
	//}
	

	

	//std::cout << "file end " << Ar.tellg() << "\n";

	return true;
}

uint8_t GetE2Alpha(const AlphaMode& mode)
{
	switch (mode)
	{
	default:
	case AlphaMode::Opaque:
		return 1;
	case AlphaMode::Masked:
		return 2;
	case AlphaMode::Transparent:
		return 3;
	}
}

AlphaMode GetAlpha(const uint8_t& mode)
{
	switch (mode)
	{
	default:
	case 1:
		return AlphaMode::Opaque;
	case 2:
		return AlphaMode::Masked;
	case 3:
		return AlphaMode::Transparent;
	}
}

uint8_t GetE2Type(const TextureType& type)
{
	switch (type)
	{
	default:
	case TextureType::OneSided:
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
	case 0: //intentional pass through
		return TextureType::OneSided;
	case 2:
		return TextureType::Cubemap;
	}
}

uint8_t GetE2Wrap(const WrapValue& wrap)
{
	//verified correct https://forums.auran.com/trainz/showthread.php?128330-Updated-file-format-parsers
	switch (wrap)
	{
	default:
	case WrapValue::Clamp_To_Edge:
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
	case 1: //intentional pass through
		return WrapValue::Clamp_To_Edge;
	case 3:
		return WrapValue::Repeat;
	}
}

const char* GetE2Format(TextureFormat& format)
{
	switch (format)
	{
	default:
	case TextureFormat::BGRA8888:
		return "abgr";
	case TextureFormat::DXT1:
		return "1txd";
	case TextureFormat::DXT3:
		return "3txd";
	case TextureFormat::DXT5:
		return "5txd";
	case TextureFormat::HD4F:
		return "f4dh";
	case TextureFormat::ASTC:
		return "ctsa";
	}
}

TextureFormat GetFormat(const char* format)
{
	if (strcmp(format, "abgr") == 0)
		return TextureFormat::BGRA8888;
	if (strcmp(format, "1txd") == 0)
		return TextureFormat::DXT1;
	if (strcmp(format, "3txd") == 0)
		return TextureFormat::DXT3;
	if (strcmp(format, "5txd") == 0)
		return TextureFormat::DXT5;
	if (strcmp(format, "f4dh") == 0)
		return TextureFormat::HD4F;
	if (strcmp(format, "ctsa") == 0)
		return TextureFormat::ASTC;

	std::cout << "Unknown format " << format << "\n";
	assert(false);
	return TextureFormat::RGBA8888;
}
