#include "TextureSave.h"
#include "DirectXTex.h"

DirectX::Image dxImg(const TzTexture& tex)
{
	DirectX::Image img;
	img.width = tex.Width;
	img.height = tex.Height;
	int groupSize = 4;
	switch (tex.Format)
	{
	default:
	case TextureFormat::BGR0888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM; //unorm, typeless, or srgb
		groupSize = 4;
		break;
	case TextureFormat::BGRA8888:
		img.format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
		groupSize = 4;
		break;
	case TextureFormat::DXT1:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //just RGB?
		break;
	case TextureFormat::DXT3:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM;
		groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::DXT5_BridgeIt:
	case TextureFormat::DXT5:
		img.format = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
		groupSize = 4;
		//compressed = true;
		//format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormat::HD4F:
		img.format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
		groupSize = 16;
		//dtype = GL_FLOAT;
		//format = GL_RGBA;
		//internalformat = GL_RGB16F;
		break;
	case TextureFormat::MFTS_ETC2:
		std::cout << "ETC2 export not currently supported.\n";
		//compressed = true;
		//format = GL_COMPRESSED_RGB8_ETC2;
		break;
	}
	const auto& mip = tex.Textures[0].textureMips[0];
	//img.rowPitch = /*<number of bytes in a scanline of the source data>*/;
	//img.rowPitch = tex.Width; //unsure
	img.slicePitch = mip.size;
	img.pixels = mip.data;
	return img;
}

void SaveDDS(const char* path, const TzTexture& tex)
{
	DirectX::Image img = dxImg(tex);
	std::cout << "saving...\n";
	HRESULT hr = SaveToDDSFile(img, DirectX::DDS_FLAGS_NONE, L"G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 401543 2102 Loading Screen Logo/ts3-logo.dds");
	if (FAILED(hr))
	{
		std::cout << "Failed to save image.\n";
	}
	std::cout << "done.\n";
}
