// TrainzTextureTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <numeric>
#define NOMINMAX
#include <windows.h>
#include <SDL.h>
#include <GL/glew.h>
#include "TrainzTexture.h"

SDL_Window* window;
std::shared_ptr<TzTexture> activeTex;
GLuint activeTexID = 0;
void draw();

void deleteTexture(GLuint& texID)
{
	glDeleteTextures(1, &texID);
	texID = 0;
}

GLuint loadTexture(const char* path)
{
	activeTex = read_texture(path);

	GLuint texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, activeTex->textureMips.size() - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	GLenum format = GL_BGRA;
	GLenum internalformat = format;
	bool compressed = false;
	GLenum dtype = GL_UNSIGNED_BYTE;
	//GLenum format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	switch (activeTex->Format)
	{
	default:
	case TextureFormat::BGRA8888:
		format = GL_BGRA;
		internalformat = GL_RGBA;
		break;
	case TextureFormat::DXT1:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case TextureFormat::DXT3:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case TextureFormat::DXT5:
		compressed = true;
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case TextureFormat::HD4F:
		dtype = GL_FLOAT;
		format = GL_RGBA;
		internalformat = GL_RGB16F;
		break;
	}

	//for (int i = 0; i < activeTex->textureMips.size(); i++)
	//{
	//	int mipWidth = std::max(activeTex->Width / (i + 1), 1u);
	//	int mipHeight = std::max(activeTex->Height / (i + 1), 1u);
	//	//glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, activeTex->Width, activeTex->Height, 0, format, GL_UNSIGNED_BYTE, activeTex->textureMips[0].data);
	//	glCompressedTexImage2D(GL_TEXTURE_2D, i, format, mipWidth, mipHeight, 0, activeTex->textureMips[i].size, activeTex->textureMips[i].data);
	//}

	if(!compressed)
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, activeTex->Width, activeTex->Height, 0, format, dtype, activeTex->Textures[0].textureMips[0].data);
	else
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, activeTex->Width, activeTex->Height, 0, activeTex->Textures[0].textureMips[0].size, activeTex->Textures[0].textureMips[0].data);

	glGenerateMipmap(GL_TEXTURE_2D);

	

	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;

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

int scrw = 1400;
int scrh = 1024;

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

void draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	//resizeGLScene(DM.w, DM.h);
	float aspect = (float)activeTex->Width / (float)activeTex->Height;

	//float aspect = (float)DM.w / (float)DM.h;
	float width = (float)activeTex->Width / (float)scrw;
	float height = (float)activeTex->Height / (float)scrh;
	float max = std::max(width, height);
	width = width / max;
	height = height / max;

	//width = 1.0f;
	//height = 1.0f;

	glBindTexture(GL_TEXTURE_2D, activeTexID);
	//glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-width, -height);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(width, -height);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(width, height);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-width, height);
	glEnd();

	SDL_GL_SwapWindow(window);
}

int main(int argc, char** argv)
{
    

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
	{
		std::cout << "GLEW failed to init.\n";
	}


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
	activeTexID = loadTexture("G:/Games/N3V/trs19/build hhl1hrpw1/editing/kuid 268447 1646 Skarloey Railway Sheds/brick.texture");

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
				deleteTexture(activeTexID);
				activeTexID = loadTexture(event.drop.file);
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
