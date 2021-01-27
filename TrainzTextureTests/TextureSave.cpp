#include "TextureSave.h"
#include "DirectXTex.h"

DirectX::Image dxImg(const TzTexture& tex, int texIndex)
{
	DirectX::Image img;
	img.width = tex.Width;
	img.height = tex.Height;
	//int groupSize = 4;
	int pitch = 4;
	switch (tex.Format)
	{
	default:
	case TextureFormat::BGR0888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM; //unorm, typeless, or srgb
		//groupSize = 4;
		pitch = 4;
		break;
	case TextureFormat::BGRA8888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
		//groupSize = 4;
		pitch = 4;
		break;
	case TextureFormat::DXT1:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		//groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //just RGB?
		break;
	case TextureFormat::DXT3:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM;
		//groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::DXT5_BridgeIt:
	case TextureFormat::DXT5:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
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
		pitch = 16;
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
	img.rowPitch = tex.Width * pitch; //unsure
	img.slicePitch = mip.size;
	img.pixels = mip.data;
	return img;
}

void SaveDDS(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToDDSFile(img, DirectX::DDS_FLAGS_NONE, path);
	if (FAILED(hr))
		std::cout << "Failed to save image.\n";
	else
		std::cout << "Done.\n";
}

void SaveTGA(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToTGAFile(img, path);
	if (FAILED(hr))
		std::cout << "Failed to save image.\n";
	else
		std::cout << "Done.\n";
}

void SavePNG(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToWICFile(img, DirectX::WIC_FLAGS_NONE, GetWICCodec(DirectX::WIC_CODEC_PNG), path);
	if (FAILED(hr))
		std::cout << "Failed to save image.\n";
	else
		std::cout << "Done.\n";
}

void SaveHDR(const wchar_t* path, const TzTexture& tex, int texIndex)
{
	if (tex.Format != TextureFormat::HD4F)
	{
		std::cout << "This texture is not HDR! Unable to export.\n";
		return;
	}
	std::cout << "Saving...\n";
	DirectX::Image img = dxImg(tex, texIndex);

	HRESULT hr = DirectX::SaveToHDRFile(img, path);
	if (FAILED(hr))
		std::cout << "Failed to save image.\n";
	else
		std::cout << "Done.\n";
}
