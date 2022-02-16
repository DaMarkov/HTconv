#pragma once
#include "blob.h"
#include <stdio.h>
#include <string>



class File
{
public:
	File() = delete;
	File(const char* Filename, bool WriteAccess = false);
	File(const std::string& Filename, bool WriteAccess = false) : File(Filename.c_str(), WriteAccess) {}

	~File() { Close(); }
		
	bool IsOpen() const { return m_File; }

	void Close();
	size_t GetSize() const;

	bool Write(const void* Data, size_t Size);
	bool Read(void* Data, size_t Size) const;


	void operator >> (Blob& Data);
	void operator << (const Blob& Data);

	template<typename T>
	void operator << (const T& Data)
	{
		Write(&Data[0], sizeof(T));
	}

	template<typename T>
	void operator >> (T& Data) const
	{
		Read(&Data[0], sizeof(T));
	}

	operator bool () const { return IsOpen(); }
	operator FILE* () { return m_File; }

	FILE* GetFileHandle() { return m_File; }

private:
	FILE* m_File = nullptr;
};