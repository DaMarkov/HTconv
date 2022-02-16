#include <zlib.h>
#include "htc.h"
#include "png.h"



HTC::HTC(const std::string& Filename)
{
	m_File = gzopen(Filename.c_str(), "rb");
	if (m_File)
		readHeader();
}



bool HTC::readHeader()
{
	read(m_Config);

	while (!gzeof(m_File))
	{
		uint64_t checksum;
		read(checksum);

		TexInfo info;
		read(info.width);
		read(info.height);
		read(info.format);
		read(info.texture_format);
		read(info.pixel_type);
		read(info.is_hires_tex);
		read(info.size);

		info.StartLocation = gztell(m_File);

		info.data = new uint8_t[info.size];
		readRaw(info.data, info.size);
		//gzseek(m_File, info.size, SEEK_CUR);//Skip

		SetStorage().insert({ checksum, info });
	}

	return true;
}



bool HTC::getByLocation(int64_t location, TexInfo& info, bool Load, bool Unzip)
{
	if (info.data)//Already loaded?
		return true;

	gzseek(m_File, (uint32_t)location, SEEK_SET);

	if (info.format != (GL_TEXFMT_RGBA8 | GL_TEXFMT_GZ))
	{
		return false;//TODO
	}

	if (Load)
	{
		if (info.data)
		{
			delete[] info.data;
			info.data = nullptr;
		}

		if (Unzip && info.format & GL_TEXFMT_GZ)//Zipped?
		{
			uint8_t* temp = new uint8_t[info.size];
			readRaw(temp, info.size);

			uLongf destLen = 67108864;
			if (!gzdest)
				gzdest = new uint8_t[destLen];

			if (uncompress(gzdest, &destLen, temp, info.size) != Z_OK)
			{
				printf("Error: zlib decompression failed!\n");
				delete[] gzdest;
				return false;
			}

			info.data = gzdest;
			info.format &= ~GL_TEXFMT_GZ;

			gzdest = nullptr;
			delete[] temp;
			return true;
		}
		else
		{
			info.data = new uint8_t[info.size];
			return readRaw(info.data, info.size);
		}
	}

	return false;
}



bool HTC::writeAtLocation(int64_t location, TexInfo& info)
{
	/*if (!info.data)
		return false;

	outfile.seekp(location, std::ifstream::beg);

	write(info.width);
	write(info.height);
	write(info.format);
	write(info.texture_format);
	write(info.pixel_type);
	write(info.is_hires_tex);

	write(info.size);
	write(info.data, info.size);*/
	return true;
}



void HTC::WriteHeader()
{
	/*outfile.seekp(0, std::ifstream::beg);
	write(m_Config);

	if (pos == 0)
		pos = GetEndOfLastLocation() + 4;//Bug no idea why + 4
	write(pos);*/
}



bool HTC::SaveTo(const std::string& Filename)
{
	//outfile.open(Filename, std::ofstream::binary);

	//if (!outfile)
		//return false;

	//WriteHeader();	

	return true;
}