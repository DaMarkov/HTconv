#pragma once
#include <string>
#include <zlib.h>
#include "blob.h"
#include "ht.h"


class PNG;



class HTC : public HT
{
public:
	HTC(const std::string& Filename);

	bool readHeader();
	bool getByLocation(int64_t location, HT::TexInfo& info, bool Load = true, bool Unzip = true);
	bool writeAtLocation(int64_t location, HT::TexInfo& info);

	bool SaveTo(const std::string& Filename);

	operator bool() const { return m_File; }


private:
	void WriteHeader();

	bool readRaw(void* data, size_t size)
	{
		return gzread(m_File, data, (uint32_t)size) != -1;
	}

	bool writeRaw(void* data, size_t size)
	{
		//outfile.write((char*)data, size);
		//return HTSfile.good();
		return false;
	}


	gzFile m_File;

	int m_Config = 0;

	uint8_t* gzdest = nullptr;
};

