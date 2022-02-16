#pragma once
#include <string>


class File;


class Blob
{
public:
		Blob() = default;
		Blob(Blob& Original) : m_Data(Original.m_Data), m_Size(Original.m_Size)
		{
			m_IsShared = true;
		}
		Blob(const Blob& Original) = delete;
		/*Blob(const Blob& Original) : m_Data(Original.m_Data), m_Size(Original.m_Size)
		{
			m_IsShared = true;
		}*/
		Blob(Blob&& Blob) noexcept : m_Data(Blob.m_Data), m_Size(Blob.m_Size), m_ReadCursor(Blob.m_WriteCursor), m_WriteCursor(Blob.m_WriteCursor), m_IsShared(Blob.m_IsShared)
		{
			Blob.m_Data = nullptr;
			Blob.m_Size = 0;
		}

		Blob(size_t Size);

		Blob(const void* Source, size_t Size) : m_Data((uint8_t*)Source), m_Size(Size)//Copy operation
		{
			m_Data = new uint8_t[Size];
			memcpy(m_Data, Source, Size);
		}

		Blob(void*&& Source, size_t Size, bool TakeOwnership = true) : m_Data((uint8_t*)Source), m_Size(Size)//Move operation
		{
			m_IsShared = !TakeOwnership;
		}

		Blob(const std::string& Source) : m_Size(Source.length())//Copy operation
		{
			m_Data = new uint8_t[m_Size];
			memcpy(m_Data, Source.c_str(), m_Size);
		}

		Blob(File& File);

		void operator =(Blob& Blob)
		{
			m_Data = Blob.m_Data;
			m_Size = Blob.m_Size;

			m_IsShared = Blob.m_IsShared;

			m_ReadCursor  = Blob.m_ReadCursor;
			m_WriteCursor = Blob.m_WriteCursor;
		}

		void operator =(Blob&& Blob) noexcept
		{
			*this = Blob;
			Blob.m_Data = nullptr;
			Blob.m_Size = 0;
		}

		~Blob();


		bool IsEmpty() const { return m_Data == nullptr; }
		size_t GetSize() const { return m_Size; }

		void MakeShared() { m_IsShared = true; }

		size_t OutputTo(void* Data, size_t BytesToRead);//Reads BytesToRead many bytes and puts them in Data. Returns the number of bytes actually read
		void Append(const void* Data, size_t BytesToWrite);
		bool InsertAt(size_t Index, const void* Data, size_t BytesToWrite);
		template <typename T>
		bool InsertAt(size_t Index, const T& Data) {
			return InsertAt(Index, &Data, sizeof(T));
		}

		void operator << (Blob& Blob);
		void operator >> (uint8_t* Data);

		template <typename T>
		Blob& operator >> (T& Data) {
			OutputTo(&Data, sizeof(T));
			return *this;
		}

		template <typename T>
		Blob& operator << (const T& Data) {
			Append(&Data, sizeof(T));
			return *this;
		}

		operator void*  () { return m_Data; }
		operator uint8_t* () { return m_Data; }

		operator const void* () const { return m_Data; }
		operator const uint8_t* () const { return m_Data; }
		explicit operator const char* () const { return (char*)m_Data; }

	private:
		size_t GetReadCursor() const { return m_ReadCursor; }

		uint8_t* m_Data = nullptr;
		size_t m_Size = 0;

		bool m_IsShared = false;//True if and only if the data is shared

		size_t m_ReadCursor  = 0;
		size_t m_WriteCursor = 0;
};