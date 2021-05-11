#include <intrin.h>
#include <SDL.h>
#include <GL/glew.h>
#include "ETC2decompress.h"


uint8_t* decompressETC2(uint32_t size, const uint8_t* data, const TzTexture& tex)
{
	GLuint texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, tex.Width, tex.Height, 0, size, data);
	uint8_t* img = new uint8_t[4u * tex.Width * tex.Height];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	glDeleteTextures(1, &texID);
	return img;
}

uint8_t* decompressType(uint32_t type, uint32_t size, const uint8_t* data, const TzTexture& tex)
{
	GLuint texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, type, tex.Width, tex.Height, 0, size, data);
	uint8_t* img = new uint8_t[4u * tex.Width * tex.Height];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
	glDeleteTextures(1, &texID);
	return img;
}
