#include "ht.h"
#include "png.h"



bool HT::get(uint64_t checksum, TexInfo& info, bool Unzip)
{
	if (!checksum || (GetStorage().empty() && GetCache().empty()) )
		return false;

	//find a match in storage
	auto entry = GetStorage().find(checksum);
	if (entry == GetStorage().end())
	{
		//find a match in cache
		auto entry = GetCache().find(checksum);
		if (entry == GetCache().end())
			return false;

		info = entry->second;
		return true;
	}

	info = entry->second;

	if (m_pSource)
		return m_pSource->getByLocation(info.StartLocation, info, true, Unzip);
	return getByLocation(info.StartLocation, info, true, Unzip);
}



bool HT::IsValidLocation(int64_t position)
{
	for (auto& [key, value] : GetStorage())
	{
		if (value.StartLocation == position)
			return true;
	}
	return false;
}



bool HT::Contains(uint64_t Key) const
{
	return GetStorage().find(Key) != GetStorage().end();
}



bool HT::Remove(uint64_t Key)
{
	if (!Contains(Key))
		return false;

	storage.erase(Key);
	return true;
}



bool HT::Append(uint64_t Key, const PNG& Image)
{
	if (Contains(Key))
		return false;

	auto location = addImage(Key, Image);

	return true;
}



bool HT::Replace(uint64_t Key, const PNG& Image)
{
	if (!Remove(Key))
		return false;

	return Append(Key, Image);
}



bool HT::findInCacheByLocation(int64_t location, TexInfo& Info)
{
	for (auto& [key, value] : cache)
	{
		if (value.StartLocation == location)
		{
			Info = value;
			return true;
		}
	}

	return false;
}



uint64_t HT::GetEndOfLastLocation()
{
	int64_t  lastKey  = 0;
	uint64_t location = 0;

	if (cache.empty())
	{
		for (auto& [key, value] : storage)
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
		for (auto& [key, info] : cache)
		{
			if (info.EndLocation > location)
				location = info.EndLocation;
		}
	}

	return location;
}



int64_t HT::addImage(uint64_t Key, const PNG& Image, bool ZipCompress)
{
	TexInfo newTexInfo;
	newTexInfo.width  = Image.GetWidth();
	newTexInfo.height = Image.GetHeight();

	newTexInfo.format = 0x00008058;
	newTexInfo.texture_format = 0x1908;
	newTexInfo.pixel_type = 0x1401;
	newTexInfo.is_hires_tex = 1;

	newTexInfo.size = newTexInfo.width * newTexInfo.height * 4;
	newTexInfo.data = new uint8_t[newTexInfo.size];
	memcpy(newTexInfo.data, Image, newTexInfo.size);

	auto location = GetEndOfLastLocation();

	if (location <= 0)
		return -1;

	newTexInfo.StartLocation = location;
	newTexInfo.EndLocation   = newTexInfo.StartLocation + newTexInfo.size;

	cache.insert({ Key, newTexInfo});
	return location;
}