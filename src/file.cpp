#include "file.h"



File::File(const char* Filename, bool WriteAccess)
{
	if (WriteAccess)
		fopen_s(&m_File, Filename, "wb");
	else
		fopen_s(&m_File, Filename, "rb");
}



void File::Close()
{
	if (IsOpen())
		fclose(m_File);
	m_File = nullptr;
}



size_t File::GetSize() const
{
	if (!IsOpen())
		return 0;

	const size_t pos = ftell(m_File);

	fseek(m_File, 0, SEEK_END);
	const size_t fileSizeBytes = ftell(m_File);
	fseek(m_File, pos, SEEK_SET);

	return fileSizeBytes;
}



bool File::Write(const void* Data, size_t Size)
{
	if (!IsOpen())
		return false;

	return fwrite(Data, Size, 1, m_File) == 1;
}



bool File::Read(void* Data, size_t Size) const
{
	if (!IsOpen())
		return false;

	return fread(Data, Size, 1, m_File) == 1;
}



void File::operator >> (Blob& Data)
{
	if (!IsOpen())
		return;

	const auto cur_pos = ftell(m_File);

	fseek(m_File, 0, SEEK_END);
	const auto fileSizeBytes = ftell(m_File);

	fseek(m_File, cur_pos, SEEK_SET);

	unsigned char* pBuffer = new unsigned char[fileSizeBytes];
	size_t bytes_read = fread(pBuffer, 1, fileSizeBytes, m_File);

	Data.Append(pBuffer, bytes_read);

	delete[] pBuffer;
}



void File::operator << (const Blob& Data)
{
	if (!IsOpen())
		return;

	fwrite(Data, sizeof(uint8_t), Data.GetSize(), m_File);
}