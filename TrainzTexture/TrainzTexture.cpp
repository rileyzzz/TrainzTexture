#include "pch.h"
#include <iostream>
#include "IOArchive.h"
#include "TrainzTexture.h"

TRAINZTEXTURE_API std::shared_ptr<TzTexture> read_texture(const char* filepath)
{
	std::cout << "Loading texture " << filepath << "\n";

	IOArchive Ar(filepath, IODirection::Import);


	//loading only!!
	char ReadHeader[5] = { 0x00 };
	Ar.Serialize(&ReadHeader[0], 4);
	if (strcmp(ReadHeader, "JIRF") == 0)
		return std::shared_ptr<TzTexture>(new JIRFTexture(Ar));
	else if (strcmp(ReadHeader, "FT2E") == 0)
		return std::shared_ptr<TzTexture>(new E2TFTexture(Ar));
	else
		std::cout << "Unrecognized texture format. Payware?\n";

	return nullptr;
}