#pragma once
#include <fstream>
#include <unordered_map>
#include <string>
#include "blob.h"
#include "ht.h"


class PNG;



class HTS : public HT
{
public:
	HTS(const std::string& Filename);
	HTS(HT& source) : HT(source) {}

	bool IsValidLocation(int64_t position);
	bool getByLocation(int64_t location, TexInfo& info, bool Load = true, bool Unzip = true);
	bool writeAtLocation(int64_t location, TexInfo& info);

	bool SaveTo(const std::string& Filename);

	operator bool() const { return HTSfile.good(); }


private:
	bool readHeader();
	uint64_t GetEndOfLastLocation();
	void WriteHeader(uint64_t pos = 0);
	void WriteLocationTable();

	bool readRaw(void* data, size_t size)
	{
		HTSfile.read((char*)data, size);
		return HTSfile.good();
	}

	bool writeRaw(void* data, size_t size)
	{
		outfile.write((char*)data, size);
		return HTSfile.good();
	}

	std::ifstream HTSfile;
	std::ofstream outfile;

	int m_Config = 0;
	int64_t m_storagePos = 0;

	uint8_t* gzdest = nullptr;
};

