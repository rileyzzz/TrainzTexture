// TrainzASTC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <TrainzTexture.h>
#include "IOArchive.h"


#pragma pack(push, 1)
struct ASTCHeader
{
    char FileMagic[4];
    uint8_t blockdim_x;
    uint8_t blockdim_y;
    uint8_t blockdim_z;
    uint8_t size_x[3];
    uint8_t size_y[3];
    uint8_t size_z[3];
};
#pragma pack(pop)

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        printf("Usage: tastc file.texture.txt\n");
        //return 0;
    }

    std::string path(argv[0]);
    std::string astcPath = (std::filesystem::path(path).parent_path() / "astcenc-sse2.exe").string();

    //for (int i = 1; i < argc; i++)
    {
        //std::string file(argv[i]);

        std::string file = "G:/Games/N3V/trs19/build hhl1hrpw1/editing/edit 8t7sgaw65542/osterlok_body/osterlok.texture.txt";

        //if (file.substr(file.find_last_of(".")) != ".txt")
        //    continue;

        std::filesystem::path fspath(file);

        std::map<std::string, std::vector<std::string>> Data;

        std::ifstream str(file, std::ios::in);

        std::string line;
        while (std::getline(str, line))
        {
            auto split = line.find("=");
            if (split != std::string::npos)
            {
                //remove newline?
                //line.pop_back();

                std::string key = line.substr(0, split);
                std::string val = line.substr(split + 1);

                if (Data.find(key) == Data.end())
                    Data[key] = { val };
                else
                    Data[key].push_back(val);
            }
        }
        str.close();

        if (Data.find("Primary") == Data.end())
        {
            printf("Unable to find .texture primary image in file %s!\n", file.c_str());
            //continue;
        }

        std::string primaryPath = (fspath.parent_path() / Data["Primary"][0]).string();
        std::string outputPath = (std::filesystem::path(path).parent_path() / "temp.astc").string();
        std::string command = astcPath + " -cl \"" + primaryPath + "\" \"" + outputPath + "\" 8x8 -medium";

        //printf("command %s\n", command.c_str());
        system(command.c_str());

        size_t imageSize;
        uint8_t* imageData = nullptr;
        ASTCHeader header;
        {
            IOArchive Ar(outputPath, IODirection::Import);
            Ar << header;
            
            imageSize = Ar.GetFilesize() - Ar.tellg();
            imageData = new uint8_t[imageSize];
            Ar.Serialize(imageData, imageSize);
        }

        uint32_t width = (header.size_x[0] << 16) | (header.size_x[1] << 8) | header.size_x[0];
        uint32_t height = (header.size_y[0] << 16) | (header.size_y[1] << 8) | header.size_y[0];

        E2TFTexture tex;
        tex.Width = width;
        tex.Height = height;

        tex.MipCount = 1;
        tex.AlphaBehavior = AlphaMode::Transparent;
        tex.Type = TextureType::OneSided;

        tex.WrapS = WrapValue::Clamp_To_Edge;
        tex.WrapT = WrapValue::Clamp_To_Edge;

        tex.Format = TextureFormat::ASTC;
        tex.blockSizeX = 8;
        tex.blockSizeY = 8;

        tex.Textures.resize(1);
        tex.Textures[0].textureMips.emplace_back(imageSize);
        tex.Textures[0].textureMips[0].data = new uint8_t[imageSize];
        memcpy(tex.Textures[0].textureMips[0].data, imageData, imageSize);
        delete[] imageData;

        {
            IOArchive out(file.substr(0, file.find_last_of(".")), IODirection::Export);
            out.Serialize("FT2E", 4);
            tex.Serialize(out);
        }

    }
}

