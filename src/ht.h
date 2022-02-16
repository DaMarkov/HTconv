#pragma once
#include <fstream>
#include <unordered_map>
#include <string>


#define GL_TEXFMT_GZ       0x80000000//Zipped
#define GZ_TEXFMT_INDEX8   0x000080E5
#define GL_TEXFMT_RGBA4    0x00008056
#define GL_TEXFMT_RGB8     0x00008051
#define GL_TEXFMT_RGBA8    0x0000008058



class PNG;


class HT
{
public:
	struct TexInfo
	{
		int size = 0;
		uint8_t* data = nullptr;

		uint64_t StartLocation = 0;
		uint64_t EndLocation = 0;

		int width  = 0;
		int height = 0;
		unsigned int format = 0;
		unsigned short texture_format = 0;
		unsigned short pixel_type = 0;
		unsigned char is_hires_tex = 0;
	};

	bool get(uint64_t checksum, TexInfo& info, bool Unzip = false);
	bool IsValidLocation(int64_t position);
	bool findInCacheByLocation(int64_t location, TexInfo& info);

	virtual bool getByLocation(int64_t location, TexInfo& info, bool Load = true, bool Unzip = true) = 0;
	virtual bool writeAtLocation(int64_t location, TexInfo& info) = 0;

	bool Contains(uint64_t Key) const;
	bool Remove(uint64_t Key);//Only removes the entry in the header
	bool Append(uint64_t Key, const PNG& Image);
	bool Replace(uint64_t Key, const PNG& Image);

	virtual bool SaveTo(const std::string& Filename) = 0;

	virtual operator bool() const = 0;

	const std::unordered_map<uint64_t, TexInfo>& GetStorage() const {
		if (m_pSource)
			return m_pSource->GetStorage();
		return storage;
	}

	const std::unordered_map<uint64_t, TexInfo>& GetCache() const {
		if (m_pSource)
			return m_pSource->GetCache();
		return cache;
	}

protected:
	HT() = default;
	HT(HT& source) : m_pSource(&source) {}

	std::unordered_map<uint64_t, TexInfo>& SetStorage() {
		if (m_pSource)
			return m_pSource->SetStorage();
		return storage;
	}

	std::unordered_map<uint64_t, TexInfo>& SetCache() {
		if (m_pSource)
			return m_pSource->SetCache();
		return cache;
	}

	int64_t addImage(uint64_t Key, const PNG& Image, bool ZipCompress = false);

	template<class T>
	bool read(T& data)
	{
		return readRaw((void*)&data, sizeof(T));
	}
	template<class T>
	bool write(T& data)
	{
		return writeRaw((void*)&data, sizeof(T));
	}

private:
	std::unordered_map<uint64_t, TexInfo> storage;
	std::unordered_map<uint64_t, TexInfo> cache;

	virtual bool readRaw( void* data, size_t size) = 0;
	virtual bool writeRaw(void* data, size_t size) = 0;

	uint64_t GetEndOfLastLocation();

	HT* m_pSource = nullptr;
};

