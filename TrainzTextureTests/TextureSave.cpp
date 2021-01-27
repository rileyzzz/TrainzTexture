#include <filesystem>
#include <fstream>
#include "TextureSave.h"
#include "DirectXTex.h"

DirectX::Image dxImg(const TzTexture& tex, int texIndex)
{
	DirectX::Image img;
	img.width = tex.Width;
	img.height = tex.Height;
	//int groupSize = 4;
	int pitch = 1;
	switch (tex.Format)
	{
	default:
	case TextureFormat::BGR0888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM; //unorm, typeless, or srgb
		//groupSize = 4;
		pitch = tex.Width * 4;
		break;
	case TextureFormat::BGRA8888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
		//groupSize = 4;
		pitch = tex.Width * 4;
		break;
	case TextureFormat::DXT1:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		pitch = ((tex.Width + 3) / 4) * 8;
		//groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //just RGB?
		break;
	case TextureFormat::DXT3:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM;
		pitch = ((tex.Width + 3) / 4) * 16;
		//groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::DXT5_BridgeIt:
	case TextureFormat::DXT5:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
		pitch = ((tex.Width + 3) / 4) * 16;
		//groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormat::HD4F:
		img.format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT; //DXGI_FORMAT_R32G32B32A32_FLOAT
		//groupSize = 16;
		//dtype = GL_FLOAT;
		//format = GL_RGBA;
		//internalformat = GL_RGB16F;
		pitch = tex.Width * 16;
		break;
	case TextureFormat::MFTS_ETC2:
		std::cout << "ETC2 export not currently supported.\n";
		//compressed = true;
		//format = GL_COMPRESSED_RGB8_ETC2;
		break;
	}
	const auto& mip = tex.Textures[texIndex].textureMips[0];
	//img.rowPitch = /*<number of bytes in a scanline of the source data>*/;
	//img.rowPitch = tex.Width; //unsure
	//img.rowPitch = tex.Width * pitch; //unsure
	img.rowPitch = pitch; //unsure
	img.slicePitch = mip.size;
	img.pixels = mip.data;
	return img;
}

bool SaveDDS(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToDDSFile(img, DirectX::DDS_FLAGS_NONE, path);
	if (FAILED(hr))
	{
		std::cout << "Failed to save image.\n";
		return false;
	}
	std::cout << "Done.\n";
	return true;
}

bool SaveTGA(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToTGAFile(img, path);
	if (FAILED(hr))
	{
		std::cout << "Failed to save image.\n";
		return false;
	}
	std::cout << "Done.\n";
	return true;
}

bool SavePNG(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToWICFile(img, DirectX::WIC_FLAGS_NONE, GetWICCodec(DirectX::WIC_CODEC_PNG), path);
	if (FAILED(hr))
	{
		std::cout << "Failed to save image.\n";
		return false;
	}
	std::cout << "Done.\n";
	return true;
}

bool SaveHDR(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	if (tex.Format != TextureFormat::HD4F)
	{
		std::cout << "This texture is not HDR! Unable to export.\n";
		return false;
	}
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToHDRFile(img, path);
	if (FAILED(hr))
	{
		std::cout << "Failed to save image.\n";
		return false;
	}
	std::cout << "Done.\n";
	return true;
}

//RGBA8888 = 0,
//BGRA8888,
//RGB0888,
//BGR0888,
//RGB888,
//BGR888,
//BGRA4444,
//BGR565,
//BGR555,
//DXT1 = 10,
//DXT3 = 11,
//DXT5 = 12,
//DXT5_BridgeIt = 13,
//MFTS_ETC2 = 28,
//HD4F //E2TF
std::string AlphaHint(const TzTexture& tex)
{
	if (tex.ResourceType == TzTexture::FileType::E2TF)
	{
		const E2TFTexture* e2tex = dynamic_cast<const E2TFTexture*>(&tex);
		switch (e2tex->AlphaBehavior)
		{
		default:
		case AlphaMode::Opaque:
			return "opaque";
		case AlphaMode::Masked:
			return "masked";
		case AlphaMode::Transparent:
			return "transparent";
		}
	}
	switch (tex.Format)
	{
	case TextureFormat::RGB0888:
	case TextureFormat::BGR0888:
	case TextureFormat::RGB888:
	case TextureFormat::BGR888:
	case TextureFormat::BGR565:
	case TextureFormat::BGR555:
	case TextureFormat::DXT1:
	case TextureFormat::MFTS_ETC2:
		return "opaque";
	default:
		return "transparent";
	}
}

#define ISPOWEROF2(n) ((n & (n - 1)) == 0)

void SaveTextureTXT(const wchar_t* path, const TzTexture& tex)
{
	std::filesystem::path texPath(path);
	std::filesystem::path txtPath = texPath.parent_path() / (texPath.stem().string() + ".texture.txt");
	std::ofstream file(txtPath, std::ios::out);
	switch (tex.Type)
	{
	default: //unsure if volume/twosided is supported?
	case TextureType::OneSided:
		file << "Primary=" << texPath.filename().string() << "\n";
		file << "Alpha=" << texPath.filename().string() << "\n";
		file << "AlphaHint=" << AlphaHint(tex) << "\n";
		break;
	case TextureType::Cubemap:
		file << "Cubemap=1\n";
		file << "Left="		<< texPath.stem().string() << "_left"	<< texPath.extension().string() << "\n";
		file << "Right="	<< texPath.stem().string() << "_right"	<< texPath.extension().string() << "\n";
		file << "Top="		<< texPath.stem().string() << "_top"	<< texPath.extension().string() << "\n";
		file << "Bottom="	<< texPath.stem().string() << "_bottom" << texPath.extension().string() << "\n";
		file << "Front="	<< texPath.stem().string() << "_front"	<< texPath.extension().string() << "\n";
		file << "Back="		<< texPath.stem().string() << "_back"	<< texPath.extension().string() << "\n";
		break;
	}
	file << "Tile=" << (tex.WrapS == WrapValue::Repeat ? "s" : "") << (tex.WrapT == WrapValue::Repeat ? "t" : "") << "\n";

	if (!ISPOWEROF2(tex.Width) || !ISPOWEROF2(tex.Height))
		file << "nonpoweroftwo=1\n";

	//file << "Primary=" << texPath
	file.close();
}
