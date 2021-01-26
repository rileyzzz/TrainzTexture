// TrainzTextureTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <numeric>
#include <sstream>
#define NOMINMAX
#include <windows.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <GL/glew.h>
#include "TrainzTexture.h"
#include "TextureSave.h"

int scrw = 1400;
int scrh = 1024;
SDL_Window* window;
TTF_Font* MainFont;
std::shared_ptr<TzTexture> activeTex;
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
		//format = GL_COMPRESSED_RGBA8_ETC2_EAC;
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
	case TextureType::TwoSided:
		return "two sided";
	case TextureType::OneSided:
		return "one sided";
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

	switch (activeTex->ResourceType)
	{
	case TzTexture::FileType::JIRF:
	{
		JIRFTexture* tex = dynamic_cast<JIRFTexture*>(activeTex.get());
		TextureInfo << "MipHint: " << (tex->Hint == MipHint::Static ? "static" : "dynamic") << "\n";
		TextureInfo << "Filter min: " << tex->MinFilter << " mag: " << tex->MagFilter << " mip: " << tex->MipFilter << "\n";
		TextureInfo << "Unknown Values: \n";
		TextureInfo << tex->unknown1 << " (0x103)\n";
		if (tex->version >= 0x104)
			TextureInfo << tex->unknown2 << " (0x104)\n";
		if (tex->version == 0x107)
			TextureInfo << tex->unknown3[0] << " " << tex->unknown3[1] << " " << tex->unknown3[2] << " " << tex->unknown3[3] << " (0x107)\n";
		break;
	}
	case TzTexture::FileType::E2TF:
	{
		E2TFTexture* tex = dynamic_cast<E2TFTexture*>(activeTex.get());
		TextureInfo << "Alpha: " << AlphaString(tex->AlphaBehavior) << "\n";
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
	glClear(GL_COLOR_BUFFER_BIT);

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
	

	SDL_GL_SwapWindow(window);
}

void saveTex()
{
	MessageBoxA(0, "save", "info", MB_OK);
	SaveDDS("", *activeTex);
}

int main(int argc, char** argv)
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		std::cout << "Error initializing DirectX\n";
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

	//activeTexID = loadTexture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 401543 2102 Loading Screen Logo/ts3-logo.texture");
	//activeTexID = loadTexture("G:/Games/N3V/trs19/build hhl1hrpw1/editing//kuid -25 1332 QR PB15/mesh/pb15_738_sp_boiler_albedo.texture");
	activeTex = read_texture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 401543 2102 Loading Screen Logo/ts3-logo.texture");
	//activeTex = read_texture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 30501 311279 Generic Environmental/cubemap.texture");
	//activeTex = read_texture("G:/P9L/MFTSRips/alltextures/eric.texture");
	//activeTex = read_texture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 268447 1646 Skarloey Railway Sheds/brick.texture");
	SetTextureInfo();
	for(int i = 0; i < activeTex->Textures.size(); i++)
		loadedTextures.push_back(loadTexture(i));

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
				for (auto& tex : loadedTextures)
					deleteTexture(tex);
				loadedTextures.clear();
				activeTex = read_texture(event.drop.file);
				SetTextureInfo();
				for (int i = 0; i < activeTex->Textures.size(); i++)
					loadedTextures.push_back(loadTexture(i));
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

		draw();
	}

    return 0;
}
