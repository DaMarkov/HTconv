#include <zlib.h>
#include "hts.h"
#include "png.h"



HTS::HTS(const std::string& Filename)
{
	HTSfile.open(Filename, std::ifstream::binary);
	if (HTSfile)
		readHeader();
}



bool HTS::readHeader()
{
	read(m_Config);

	read(m_storagePos);

	if (m_storagePos < 0)
		return false;

	HTSfile.seekg(m_storagePos, std::ifstream::beg);

	int storageSize = 0;
	read(storageSize);

	if (storageSize <= 0)
		return false;

	for (int i = 0; i < storageSize; ++i)
	{
		uint64_t key;
		int64_t value;

		read(key);
		read(value);
		TexInfo info;
		info.StartLocation = value;
		SetStorage().insert({ key, info });
	}

	return true;
}



bool HTS::getByLocation(int64_t location, TexInfo& info, bool Load, bool Unzip)
{
	info.StartLocation = location;
	HTSfile.seekg(location, std::ifstream::beg);

	read(info.width);
	read(info.height);
	read(info.format);
	read(info.texture_format);
	read(info.pixel_type);
	read(info.is_hires_tex);

	if (info.width < 0 || info.height < 0)
		return false;

	uint32_t rawSize = 0U;
	read(rawSize);
	if (rawSize == 0)
		return false;

	info.size = rawSize;
	info.EndLocation = info.StartLocation + sizeof(int) * 3 + sizeof(unsigned short) * 2 + sizeof(char) + rawSize;

	if (Load)
	{
		uint8_t* raw = new uint8_t[rawSize];
		readRaw(raw, rawSize);

		if (Unzip && info.format & GL_TEXFMT_GZ)//Texture is zipped
		{
			uLongf destLen = 67108864;
			if (!gzdest)
				gzdest = new uint8_t[destLen];

			if (uncompress(gzdest, &destLen, raw, rawSize) != Z_OK)
			{
				printf("Error: zlib decompression failed!\n");
				delete[] gzdest;
				return false;
			}

			info.data = gzdest;
			info.format &= ~GL_TEXFMT_GZ;

			gzdest = nullptr;
			delete[] raw;
		}
	
		else
			info.data = raw;
	}

	return true;
}



bool HTS::writeAtLocation(int64_t location, TexInfo& info)
{
	if (!info.data)
		return false;

	outfile.seekp(location, std::ifstream::beg);

	write(info.width);
	write(info.height);
	write(info.format);
	write(info.texture_format);
	write(info.pixel_type);
	write(info.is_hires_tex);

	write(info.size);
	writeRaw(info.data, info.size);
	return true;
}



bool HTS::IsValidLocation(int64_t position)
{
	for (auto& [key, value] : GetStorage())
	{
		if (value.StartLocation == position)
			return true;
	}
	return false;
}



void HTS::WriteHeader(uint64_t pos)
{
	outfile.seekp(0, std::ifstream::beg);
	write(m_Config);

	if (pos == 0)
		pos = GetEndOfLastLocation() + 4;
	write(pos);
}



void HTS::WriteLocationTable()
{
	size_t storageSize = GetStorage().size() + GetStorage().size();
	write(storageSize);


	for (auto& [key, info] : GetStorage())
	{
		write(key);
		write(info.StartLocation);
	}

	for (auto& [key, info] : GetCache())
	{
		write(key);
		write(info.StartLocation);
	}
}



uint64_t HTS::GetEndOfLastLocation()
{
	int64_t  lastKey  = 0;
	uint64_t location = 0;

	if (GetCache().empty())
	{
		for (auto& [key, value] : GetStorage())
		{
			if (value.StartLocation > location)
			{
				lastKey = key;
				location = value.StartLocation;
			}
		}

		TexInfo info;
		getByLocation(location, info, false);
		location = info.EndLocation;
	}
	else
	{
		for (auto& [key, info] : GetCache())
		{
			if (info.EndLocation > location)
				location = info.EndLocation;
		}
	}

	return location;
}



bool HTS::SaveTo(const std::string& Filename)
{
	outfile.open(Filename, std::ofstream::binary);

	if (!outfile)
		return false;

	WriteHeader();	

	auto& storage = SetStorage();

	uint64_t count = 0;
	for (auto& [key, org_info] : storage)
	{
		float percent = ((count++) * 100.0f) / (float)storage.size();
		if (count % 200 == 0)
			printf("[%.01f%%] Texture %d / %d\n", percent, (int)count, (int)storage.size());

		auto location = outfile.tellp();

		TexInfo info;
		get(key, info);

		org_info.StartLocation = location;

		if (!writeAtLocation(location, info))
			return false;

		delete[] info.data;
	}

	for (auto& [key, info] : SetCache())
	{
		auto location = outfile.tellp();

		info.StartLocation = location;

		if (!writeAtLocation(location, info))
			return false;
	}

	uint64_t locationtable = outfile.tellp();

	WriteLocationTable();
	WriteHeader(locationtable);

	return true;
}