// TrainzTextureTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <numeric>
#include <sstream>
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#include <atlcomcli.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include "TrainzTexture.h"
#include "TextureSave.h"
#include <filesystem>
#include "ETC2decompress.h"

int scrw = 1400;
int scrh = 1024;
SDL_Window* window;
TTF_Font* MainFont;
std::shared_ptr<TzTexture> activeTex;
//wchar_t currentFileDir[MAX_PATH];
std::filesystem::path currentFile;
std::stringstream TextureInfo;
std::vector<GLuint> loadedTextures;

//GLuint activeTexID = 0;
void draw();
std::string FormatString(const TextureFormat& format);

void deleteTexture(GLuint& texID)
{
	glDeleteTextures(1, &texID);
	texID = 0;
}

GLenum GetWrapGL(const WrapValue& value)
{
	switch (value)
	{
	case WrapValue::Clamp:
		return GL_CLAMP;
	case WrapValue::Clamp_To_Edge:
		return GL_CLAMP_TO_EDGE;
	case WrapValue::Repeat:
		return GL_REPEAT;
	}
}

GLuint loadTexture(int texIndex)
{
	GLuint texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, activeTex->textureMips.size() - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetWrapGL(activeTex->WrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetWrapGL(activeTex->WrapT));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	GLenum format = GL_BGRA;
	GLenum internalformat = format;
	bool compressed = false;
	GLenum dtype = GL_UNSIGNED_BYTE;
	//GLenum format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	switch (activeTex->Format)
	{
	default:
		MessageBoxA(0, ("Unhandled texture format: " + FormatString(activeTex->Format)).c_str(), "info", MB_OK);
		[[fallthrough]];
	case TextureFormat::BGR0888:
		format = GL_BGRA;
		internalformat = GL_RGB;
		break;
	case TextureFormat::BGRA8888:
		format = GL_BGRA;
		internalformat = GL_RGBA;
		break;
	case TextureFormat::DXT1:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; //just RGB?
		break;
	case TextureFormat::DXT3:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::DXT5_BridgeIt:
	case TextureFormat::DXT5:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormat::HD4F:
		dtype = GL_FLOAT;
		format = GL_RGBA;
		internalformat = GL_RGB16F;
		break;
	case TextureFormat::MFTS_ETC2:
		compressed = true;
		format = GL_COMPRESSED_RGB8_ETC2;
		//format = GL_RGBA;
		//compressed = false;
		//auto& etcmip = activeTex->Textures[texIndex].textureMips[0];
		//etcmip.data = decompressETC2(etcmip.size, etcmip.data, *activeTex);
		break;
	}

	//for (int i = 0; i < activeTex->textureMips.size(); i++)
	//{
	//	int mipWidth = std::max(activeTex->Width / (i + 1), 1u);
	//	int mipHeight = std::max(activeTex->Height / (i + 1), 1u);
	//	//glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, activeTex->Width, activeTex->Height, 0, format, GL_UNSIGNED_BYTE, activeTex->textureMips[0].data);
	//	glCompressedTexImage2D(GL_TEXTURE_2D, i, format, mipWidth, mipHeight, 0, activeTex->textureMips[i].size, activeTex->textureMips[i].data);
	//}
	const auto& mip = activeTex->Textures[texIndex].textureMips[0];
	if(!compressed)
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, activeTex->Width, activeTex->Height, 0, format, dtype, mip.data);
	else
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, activeTex->Width, activeTex->Height, 0, mip.size, mip.data);

	glGenerateMipmap(GL_TEXTURE_2D);

	

	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;

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
//HD4F //E2TF
std::string FormatString(const TextureFormat& format)
{
	switch (format)
	{
	case TextureFormat::RGBA8888:
		return "uncompressed RGBA8888";
	case TextureFormat::BGRA8888:
		return "uncompressed BGRA8888";
	case TextureFormat::RGB0888:
		return "uncompressed RGB0888";
	case TextureFormat::BGR0888:
		return "uncompressed BGR0888";
	case TextureFormat::RGB888:
		return "uncompressed RGB888";
	case TextureFormat::BGR888:
		return "uncompressed BGR888";
	case TextureFormat::BGRA4444:
		return "uncompressed BGRA4444";
	case TextureFormat::BGR565:
		return "uncompressed BGR565";
	case TextureFormat::BGR555:
		return "uncompressed BGR555";
	case TextureFormat::DXT1:
		return "compressed DXT1";
	case TextureFormat::DXT3:
		return "compressed DXT3";
	case TextureFormat::DXT5:
		return "compressed DXT5";
	case TextureFormat::DXT5_BridgeIt:
		return "compressed DXT5 (Bridge It)";
	case TextureFormat::HD4F:
		return "uncompressed HD4F (HDR)";
	case TextureFormat::MFTS_ETC2:
		return "compressed ETC2 (mobile)";
	default:
		return ("unknown - " + std::to_string((uint32_t)format));
	}
}
std::string TypeString(const TextureType& type)
{
	switch (type)
	{
	case TextureType::OneSided:
		return "one sided";
	case TextureType::TwoSided:
		return "two sided";
	case TextureType::Cubemap:
		return "cubemap";
	case TextureType::Volume:
		return "volume";
	default:
		return ("unknown - " + std::to_string((uint32_t)type));
	}
}
std::string AlphaString(const AlphaMode& mode)
{
	switch (mode)
	{
	case AlphaMode::Opaque:
		return "opaque";
	case AlphaMode::Masked:
		return "masked";
	case AlphaMode::Transparent:
		return "transparent";
	default:
		return ("unknown - " + std::to_string((uint32_t)mode));
	}
}

//std::string UsageString(const TextureUsage& usage)
//{
//	switch (usage)
//	{
//	case TextureUsage::Mesh:
//		return "mesh";
//	case TextureUsage::UI:
//		return "UI";
//	case TextureUsage::Icon:
//		return "icon";
//	default:
//		return ("unknown - " + std::to_string((uint32_t)usage));
//	}
//}

void SetTextureInfo()
{
	TextureInfo.str(std::string()); //clear the stringstream
	TextureInfo << "Resource Type: " << (activeTex->ResourceType == TzTexture::FileType::E2TF ? "E2 Texture Format" : "JET Engine Texture Format") << "\n";
	std::stringstream version;
	if (activeTex->ResourceType == TzTexture::FileType::JIRF)
		version << "0x" << std::hex;
	version << activeTex->version;
	TextureInfo << "Version: " << version.str() << "\n";
	TextureInfo << "Image: " << activeTex->Width << "x" << activeTex->Height << " " << FormatString(activeTex->Format) << "\n";
	TextureInfo << "Type: " << TypeString(activeTex->Type) << "\n";
	TextureInfo << activeTex->Textures.size() << " texture" << (activeTex->Textures.size() == 1 ? "" : "s") << "\n";
	TextureInfo << activeTex->MipCount << " mip level" << (activeTex->MipCount == 1 ? "" : "s") << "\n";

	std::stringstream wrapstring;
	if (activeTex->WrapS != WrapValue::Repeat && activeTex->WrapT != WrapValue::Repeat)
		wrapstring << "none";
	else
		wrapstring << (activeTex->WrapS == WrapValue::Repeat ? "s" : "") << (activeTex->WrapT == WrapValue::Repeat ? "t" : "");
	TextureInfo << "Tile: " << wrapstring.str() << "\n";
	TextureInfo << "Alpha: " << AlphaString(activeTex->AlphaBehavior) << "\n";
	switch (activeTex->ResourceType)
	{
	case TzTexture::FileType::JIRF:
	{
		JIRFTexture* tex = dynamic_cast<JIRFTexture*>(activeTex.get());
		TextureInfo << "MipHint: " << (tex->Hint == MipHint::Static ? "static" : "dynamic") << "\n";
		TextureInfo << "Filter min: " << tex->MinFilter << " mag: " << tex->MagFilter << " mip: " << tex->MipFilter << "\n";
		if (tex->version == 0x108)
			TextureInfo << "Base Color: R: " << (uint32_t)tex->base_color[0] << " G: " << (uint32_t)tex->base_color[1] << " B: " << (uint32_t)tex->base_color[2] << " A: " << (uint32_t)tex->base_color[3] << "\n";
		//TextureInfo << "Usage Type: " << UsageString(tex->UsageType) << "\n";
		TextureInfo << "Unknown Values: \n";
		//TextureInfo << tex->unknown1 << " (0x103)\n";
		if (tex->version >= 0x104)
			TextureInfo << tex->unknown2 << " (0x104)\n";
		if (tex->version >= 0x107)
			TextureInfo << tex->unknown3[0] << " " << tex->unknown3[1] << " " << tex->unknown3[2] << " " << tex->unknown3[3] << " (0x107)\n";
		break;
	}
	case TzTexture::FileType::E2TF:
	{
		E2TFTexture* tex = dynamic_cast<E2TFTexture*>(activeTex.get());
		//TextureInfo << "Alpha: " << AlphaString(tex->AlphaBehavior) << "\n";
		TextureInfo << "Reference Color: R: " << (uint32_t)tex->base_color[0] << " G: " << (uint32_t)tex->base_color[1] << " B: " << (uint32_t)tex->base_color[2] << " A: " << (uint32_t)tex->base_color[3] << "\n";
		TextureInfo << "Unknown Values: " << (uint32_t)tex->unknown1 << " " << (uint32_t)tex->unknown2 << " (default 0 1)\n";
		break;
	}
	}

}

void resizeGLScene(GLsizei width, GLsizei height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int filterEvent(void* userdata, SDL_Event* event) {
	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		scrw = event->window.data1;
		scrh = event->window.data2;
		resizeGLScene(event->window.data1, event->window.data2);
		draw();
		return 0;
	}
	return 1;
}

void RenderText(const char* text, int X, int Y, SDL_Color color = { 255, 255, 255 })
{
	GLuint TextureID = 0;
	//SDL_Color White = { 255, 255, 255 };
	SDL_Surface* surfaceMessage = TTF_RenderText_Blended(MainFont, text, color);

	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	int Mode = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, Mode, surfaceMessage->w, surfaceMessage->h, 0, Mode, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// For Ortho mode, of course
	int TextWidth = surfaceMessage->w;
	int TextHeight = surfaceMessage->h;

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(X, Y, 0);
	glTexCoord2f(1, 0); glVertex3f(X + TextWidth, Y, 0);
	glTexCoord2f(1, -1); glVertex3f(X + TextWidth, Y + TextHeight, 0);
	glTexCoord2f(0, -1); glVertex3f(X, Y + TextHeight, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &TextureID);
	SDL_FreeSurface(surfaceMessage);
}

#define glf(a) ((a - 0.5f) * 2.0f)

void drawTile(float x, float y, float width, float height)
{
	float tileWidth = (width / 4.0f) * 2.0f;
	float tileHeight = (height / 3.0f) * 2.0f;

	float xoffset = -width + tileWidth * (float)x;
	float yoffset = -height + tileHeight * (float)y;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(xoffset, yoffset);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(xoffset + tileWidth, yoffset);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(xoffset + tileWidth, yoffset + tileHeight);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(xoffset, yoffset + tileHeight);
	glEnd();
}

void draw()
{

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	AlphaMode blendmode = AlphaMode::Transparent;
	if (activeTex->ResourceType == TzTexture::FileType::E2TF)
		blendmode = dynamic_cast<E2TFTexture*>(activeTex.get())->AlphaBehavior;

	switch (blendmode)
	{
	default:
	case AlphaMode::Opaque:
		glDisable(GL_BLEND);
		break;
	case AlphaMode::Masked:
	case AlphaMode::Transparent:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//resizeGLScene(DM.w, DM.h);
	glEnable(GL_TEXTURE_2D);

	int numTextures = loadedTextures.size();
	if (activeTex->Type != TextureType::Cubemap)
	{
		float width = (float)activeTex->Width / (float)scrw * numTextures;
		float height = (float)activeTex->Height / (float)scrh;
		float max = std::max(width, height);
		width = width / max;
		height = height / max;
		float tileWidth = width / (float)numTextures;

		for (int i = 0; i < numTextures; i++)
		{
			glBindTexture(GL_TEXTURE_2D, loadedTextures[i]);
			glBegin(GL_QUADS);
			float xoffset = glf((float)i / (float)numTextures) * width;
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(xoffset, -height);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(xoffset + tileWidth * 2, -height);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(xoffset + tileWidth * 2, height);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(xoffset, height);
			glEnd();
		}
	}
	else
	{

		float width = (float)activeTex->Width / (float)scrw * 4.0f;
		float height = (float)activeTex->Height / (float)scrh * 3.0f;
		float max = std::max(width, height);
		width = width / max;
		height = height / max;
		//in the range of -width, -height to width, height


		//right
		glBindTexture(GL_TEXTURE_2D, loadedTextures[0]);
		drawTile(2, 1, width, height);

		//left
		glBindTexture(GL_TEXTURE_2D, loadedTextures[1]);
		drawTile(0, 1, width, height);

		//top
		glBindTexture(GL_TEXTURE_2D, loadedTextures[2]);
		drawTile(1, 2, width, height);

		//bottom
		glBindTexture(GL_TEXTURE_2D, loadedTextures[3]);
		drawTile(1, 0, width, height);

		//front
		glBindTexture(GL_TEXTURE_2D, loadedTextures[4]);
		drawTile(1, 1, width, height);

		//back
		glBindTexture(GL_TEXTURE_2D, loadedTextures[5]);
		drawTile(3, 1, width, height);
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, scrw, 0, scrh);
	/*glScalef(1, -1, 1);
	glTranslatef(0, -height, 0);*/
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int infoheight = 22;
	std::stringstream ss(TextureInfo.str());
	std::string to;
	while (std::getline(ss, to, '\n'))
	{
		RenderText(to.c_str(), 12, scrh - infoheight);
		infoheight += 14;
	}
	

	//glTexCoord2f(0.0f, 1.0f);
	//glVertex2f(-width, -height);
	//glTexCoord2f(1.0f, 1.0f);
	//glVertex2f(width, -height);
	//glTexCoord2f(1.0f, 0.0f);
	//glVertex2f(width, height);
	//glTexCoord2f(0.0f, 0.0f);
	//glVertex2f(-width, height);
}

bool saveTexFile(const wchar_t* path, int texIndex = 0)
{
	std::filesystem::path outPath(path);
	std::cout << "save to " << outPath.string() << "\n";
	std::string ext = outPath.extension().string();
	if (ext == ".dds")
		return SaveDDS(path, *activeTex, texIndex);
	else if (ext == ".tga")
		return SaveTGA(path, *activeTex, texIndex);
	else if (ext == ".hdr")
		return SaveHDR(path, *activeTex, texIndex);
	else if (ext == ".png")
		return SavePNG(path, *activeTex, texIndex);
	else
		std::cout << "Unrecognized format '" << ext << "'\n";
	return false;
}

std::string cubemapDirString(int index)
{
	switch (index)
	{
	default:
	case 0:
		return "right";
	case 1:
		return "left";
	case 2:
		return "top";
	case 3:
		return "bottom";
	case 4:
		return "front";
	case 5:
		return "back";
	}
}

void saveTex()
{
	//MessageBoxA(0, "save", "info", MB_OK);
	//SaveDDS("", *activeTex);
	std::wstring initialTexName = currentFile.stem().wstring() + L".dds";
	std::filesystem::path texPath = currentFile.parent_path() / (currentFile.stem().string() + ".dds");
	std::cout << "texpath " << texPath.string() << "\n";

	//wchar_t filepath[MAX_PATH];
	//ZeroMemory(&filepath, sizeof(filepath));

	//wchar_t initialDir[MAX_PATH];
	//wcscpy_s(initialDir, currentFile.parent_path().wstring().c_str());

	COMDLG_FILTERSPEC rgSpec[] =
	{
		{ L"DirectDraw Surface (DDS)", L"*.dds" },
		{ L"TGA", L"*.tga" },
		{ L"HDR", L"*.hdr" },
		{ L"PNG", L"*.png" },
		//{ L"All Files", L"*.*" },
	};

	CComPtr<IFileSaveDialog> pFileOpen;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileOpen));
	if (SUCCEEDED(hr))
	{
		CComPtr<IShellItem> psiFolder;
		hr = SHCreateItemFromParsingName(currentFile.parent_path().wstring().c_str(), NULL, IID_PPV_ARGS(&psiFolder));
		if(SUCCEEDED(hr))
			pFileOpen->SetFolder(psiFolder);

		pFileOpen->SetFileName(initialTexName.c_str());
		pFileOpen->SetDefaultExtension(L"dds");
		
		pFileOpen->SetFileTypes(_countof(rgSpec), rgSpec);
		hr = pFileOpen->Show(NULL);
		if (SUCCEEDED(hr))
		{
			CComPtr<IShellItem> pItem;
			hr = pFileOpen->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pszFilePath;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					bool exportSuccessful = true;
					std::filesystem::path outPath(pszFilePath);
					switch (activeTex->Type)
					{
					default:
					case TextureType::OneSided:
						exportSuccessful = exportSuccessful && saveTexFile(pszFilePath);
						break;
					case TextureType::TwoSided:
					case TextureType::Volume:
					{
						for (int i = 0; i < activeTex->Textures.size(); i++)
						{
							std::filesystem::path finalPath = outPath.parent_path() / (outPath.stem().string() + "_" + std::to_string(i) + outPath.extension().string());
							exportSuccessful = exportSuccessful && saveTexFile(finalPath.wstring().c_str(), i);
						}
						break;
					}
					case TextureType::Cubemap:
					{
						for (int i = 0; i < activeTex->Textures.size(); i++)
						{
							std::filesystem::path finalPath = outPath.parent_path() / (outPath.stem().string() + "_" + cubemapDirString(i) + outPath.extension().string());
							exportSuccessful = exportSuccessful && saveTexFile(finalPath.wstring().c_str(), i);
						}
						break;
					}
					}
					
					if (exportSuccessful)
						SaveTextureTXT(pszFilePath, *activeTex);
					else
						std::cout << "Texture export failed.\n";
					CoTaskMemFree(pszFilePath);
				}
			}
		}
	}

	

	//OPENFILENAME ofn;
	//ZeroMemory(&ofn, sizeof(ofn));
	//ofn.lStructSize = sizeof(ofn);
	//ofn.hwndOwner = NULL;
	//ofn.lpstrFilter = L"DDS Image\0 *.dds\0All Files\0 *.*\0";
	//ofn.lpstrFile = filepath;
	//ofn.nMaxFile = MAX_PATH;
	//ofn.lpstrTitle = L"Save Texture";
	//ofn.lpstrInitialDir = initialDir;
	////ofn.lpstrFileTitle =
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

	//if (GetSaveFileName(&ofn))
	//	SaveDDS(filepath, *activeTex);
	//else
	//	std::cout << "Failed to save file.\n";
	//ofn.
	//GetOpenFileNameA();
}

void openTexture(const char* path)
{
	currentFile = std::filesystem::path(path);
	for (auto& tex : loadedTextures)
		deleteTexture(tex);
	loadedTextures.clear();
	activeTex = read_texture(path);
	if (activeTex.get())
	{
		SetTextureInfo();
		for (int i = 0; i < activeTex->Textures.size(); i++)
			loadedTextures.push_back(loadTexture(i));
	}
}
//switch between textures in the active directory
void scrollTex(int wheel)
{
	std::vector<std::filesystem::path> dirFiles;
	for (const auto& entry : std::filesystem::directory_iterator(currentFile.parent_path()))
	{
		if (entry.path().extension().string() == ".texture")
			dirFiles.push_back(entry.path());
	}
	auto curIndex = dirFiles.begin();
	curIndex = std::find(dirFiles.begin(), dirFiles.end(), currentFile);
	if (wheel > 0) // scroll up
	{
		if (curIndex != dirFiles.begin())
			openTexture((--curIndex)->string().c_str());
	}
	else if (wheel < 0) // scroll down
	{
		if (++curIndex != dirFiles.end())
			openTexture((curIndex)->string().c_str());
	}
}

int main(int argc, char** argv)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		std::cout << "Error initializing COM\n";
		return 0;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	//SDL_INIT_EVERYTHING
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
	}


	
	window = SDL_CreateWindow("Texture Viewer",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		scrw, scrh, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	if (glewInit() != GLEW_OK)
		std::cout << "GLEW failed to init.\n";

	if (TTF_Init() == -1)
		printf("error initializing SDL_ttf: %s\n", TTF_GetError());

	MainFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 12);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	//glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//const char* debugPath = "G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 401543 2102 Loading Screen Logo/ts3-logo.texture";
	////activeTexID = loadTexture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 401543 2102 Loading Screen Logo/ts3-logo.texture");
	////activeTexID = loadTexture("G:/Games/N3V/trs19/build hhl1hrpw1/editing//kuid -25 1332 QR PB15/mesh/pb15_738_sp_boiler_albedo.texture");
	//activeTex = read_texture(debugPath);
	////activeTex = read_texture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 30501 311279 Generic Environmental/cubemap.texture");
	////activeTex = read_texture("G:/P9L/MFTSRips/alltextures/eric.texture");
	////activeTex = read_texture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 268447 1646 Skarloey Railway Sheds/brick.texture");
	//currentFile = std::filesystem::path(debugPath);
	//SetTextureInfo();
	//for(int i = 0; i < activeTex->Textures.size(); i++)
	//	loadedTextures.push_back(loadTexture(i));


	resizeGLScene(scrw, scrh);

	SDL_SetEventFilter(filterEvent, nullptr);
	int close = 0;
	while (!close)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				close = 1;
				break;
			}
			else if (event.type == SDL_DROPFILE)
			{
				//MultiByteToWideChar(CP_UTF8, 0, event.drop.file, -1, nameW, MAX_PATH);
				openTexture(event.drop.file);
			}

			switch (event.type)
			{
			case SDL_KEYDOWN:
			{
				auto* keystate = SDL_GetKeyboardState(NULL);
				if (keystate[SDL_SCANCODE_LCTRL] && event.key.keysym.sym == SDLK_s)
					saveTex();
				break;
			}
			case SDL_MOUSEWHEEL:
				scrollTex(event.wheel.y);
				break;
			}
			//if (event.type == SDL_WINDOWEVENT)
			//{
			//	if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			//	{
			//		scrw = event.window.data1;
			//		scrh = event.window.data2;
			//		resizeGLScene(event.window.data1, event.window.data2);
			//	}
			//}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		if(activeTex.get())
			draw();

		SDL_GL_SwapWindow(window);
	}

	CoUninitialize();
    return 0;
}
