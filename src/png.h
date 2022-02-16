#pragma once
#include <string>
#include "ht.h"
#include "image.h"

typedef struct png_struct_def png_struct;
typedef struct png_info_def png_info;



class PNG : public Image
{
public:
	PNG(const char* Filename);//Open PNG file from disk
	PNG(const std::string& Filename) : PNG(Filename.c_str()) {}
	PNG(const Blob& Blob);

	//PNG(Image&& Img) : PNG(std::move((Blob)Img), Img.GetWidth(), Img.GetHeight(), Img.GetColorType()) {}
	PNG(Image&& Img) : Image(std::move(Img)) {}
	PNG(Blob& Blob);//Open PNG data from memory
	PNG(Blob&& Blob) : PNG(Blob) {}//Open PNG data from memory

	PNG(Blob&  Blob, uint32_t Width, uint32_t Height, ColorType ColorType);//Opens raw image. Data is copied from the data blob
	PNG(Blob&& Blob, uint32_t Width, uint32_t Height, ColorType ColorType);//Opens raw image. Data is moved from the data blob

	bool Save(const char* Filename);
	static bool SavePNG(const char* Filename, HT::TexInfo& texture);

	static Blob Compress(Image& Image);

private:
	void Decompress(png_struct* png_ptr, png_info* info_ptr);
};