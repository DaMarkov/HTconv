#pragma once
#include "blob.h"



enum class ColorType : uint32_t
{
	R8G8B8,
	B8G8R8,
	R8G8B8A8
};


inline uint32_t ColorTypeSize(ColorType ColorType)
{
	switch (ColorType)
	{
	case ColorType::R8G8B8:
	case ColorType::B8G8R8:
		return 3;
	case ColorType::R8G8B8A8:
		return 4;
	}

	return 0;
}


struct Color
{
	Color() = default;
	Color(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_ = 255)
	{
		r = r_;
		g = g_;
		b = b_;
		a = a_;
	}

	unsigned char r, g, b, a;
};



	class Image
	{
	public:
		Image() = default;
		Image(Image&& rhs) = default;
		Image(uint32_t Width, uint32_t Height, ColorType ColorType);
		Image(Blob&& Data, uint32_t Width, uint32_t Height, ColorType ColorType) :
			m_Width(Width), m_Height(Height), m_ColorType(ColorType),
			m_ImageData(std::move(Data)) {}

		virtual ~Image() {}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		ColorType GetColorType() const { return m_ColorType; }

		bool IsValid() const { return !m_ImageData.IsEmpty(); }
		operator bool () const { return IsValid(); }

		void Fill(Color Color);
		void CreateCircle();

		bool InsertAt(const Image& Source, uint32_t DestinationX, uint32_t DestinationY);

		Color GetPixel(uint32_t X, uint32_t Y);
		void  SetPixel(uint32_t X, uint32_t Y, Color NewColor);

		void ConvertTo(ColorType Format);
		void Flip();

		explicit operator Blob& () { return m_ImageData; }
		explicit operator const Blob& () const { return m_ImageData; }

		operator void* () { return m_ImageData; }
		operator const void* () const { return m_ImageData; }

		uint8_t& operator [] (uint32_t Index) { return m_ImageData[Index]; }
		uint8_t  operator [] (uint32_t Index) const { return m_ImageData[Index]; }

protected:
	uint32_t m_Width  = 0;
	uint32_t m_Height = 0;

	ColorType m_ColorType = ColorType::R8G8B8;

	Blob m_ImageData;
};