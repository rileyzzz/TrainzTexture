#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

enum class IODirection
{
	Import,
	Export
};

class IOArchive
{
private:
	IODirection Dir;
	std::ifstream input;
	std::ofstream output;
	std::streamsize filesize;
public:
	inline bool IsLoading()
	{
		return Dir == IODirection::Import;
	}
	inline bool IsSaving()
	{
		return Dir == IODirection::Export;
	}
	template<typename T>
	void operator<<(T& other)
	{
		if (IsLoading())
			input.read(reinterpret_cast<char*>(&other), sizeof(other));
		else
			output.write(reinterpret_cast<char*>(&other), sizeof(other));
	}
	void operator<<(std::string& other)
	{
		uint32_t length = other.size() + 1;
		*this << length;
		if (IsLoading())
		{
			char* str = new char[length];
			Serialize(str, length);
			other = str;
			delete[] str;
		}
		else
		{
			//uses const char data, write only serialize
			Serialize(other.data(), length);
		}
	}
	void Serialize(void* data, std::streamsize count)
	{
		if (IsLoading())
			input.read(reinterpret_cast<char*>(data), count);
		else
			output.write(reinterpret_cast<char*>(data), count);
	}
	void Serialize(const void* data, std::streamsize count)
	{
		assert(IsSaving());
		output.write(reinterpret_cast<const char*>(data), count);
	}
	IOArchive(std::string file, IODirection InDir) : Dir(InDir)
	{
		if (IsLoading())
			input = std::ifstream(file, std::ios::in | std::ios::binary);
		else
			output = std::ofstream(file, std::ios::out | std::ios::binary);

		if (IsLoading())
		{
			input.seekg(0x0, std::ios::end);
			filesize = input.tellg();
			input.seekg(0x0, std::ios::beg);
		}
	}
	~IOArchive()
	{
		if (IsLoading())
			input.close();
		else
			output.close();
	}
	bool ChunkHeader(const char* header)
	{
		if (IsSaving())
		{
			Serialize(header, 4);
		}
		else
		{
			char ReadHeader[5] = { 0x00 };
			Serialize(&ReadHeader[0], 4);
			if (strcmp(ReadHeader, header) != 0)
			{
				std::cout << "Failed to read chunk " << header << "\n";
				return false;
			}
		}
		return true;
	}

	std::streampos tellg()
	{
		assert(IsLoading());
		return input.tellg();
	}

	std::streamsize GetFilesize()
	{
		return filesize;
	}

	//remove eventually
	std::ifstream& GetInput()
	{
		return input;
	}

	void ignore(std::streamsize count)
	{
		assert(IsLoading());
		input.ignore(count);
	}
	//void seekg(std::streamoff off, std::ios_base::seekdir way)
	//{
	//	assert(IsLoading());
	//	input.seekg(off, way);
	//}
};

