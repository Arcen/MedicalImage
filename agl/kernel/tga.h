#pragma pack( push )
//構造体アライメントを１バイトにする
#pragma pack(1)

template<class P>
class targa
{
public:
	struct TargaHeader
	{
		unsigned char IDFieldLength;
		unsigned char colorMapStyle;
		unsigned char imageStyleOrColorMap;
		unsigned short firstColorMapEntry;
		unsigned short colorMapLength;
		unsigned char colorMapEntrySize;
		unsigned short x, y, width, height;
		unsigned char bitPerPixel;
		unsigned char imageIteratorBit;
	};
	static bool write( file & f, const imageInterface<P> & img )
	{
		TargaHeader header = { 0 };
		header.imageStyleOrColorMap = 2; // Ture Color and non compress
		header.width = img.width;
		header.height = img.height;
		header.bitPerPixel = 32;
		// high 4bit maen left to right, bottom to up
		// low 4bit mean bit per channel
		header.imageIteratorBit = 0x08;
		f.write( header );
		pixelBGRA bgra;
		for ( int y = 0; y < img.height; y++ ) {
			for ( int x = 0; x < img.width; x++ ) {
				bgra = img.get( x, img.height - 1 - y );
				f.write( bgra.b );
				f.write( bgra.g );
				f.write( bgra.r );
				f.write( bgra.a );
			}
		}
		return true;
	}
	static bool read( file & f, imageInterface<P> & img )
	{
		TargaHeader header;
		f.read( header );
		if ( header.imageStyleOrColorMap != 2 ) return false; // Ture Color and non compress
		if ( header.bitPerPixel != 32 ) return false;
		// high 4bit maen left to right, bottom to up
		// low 4bit mean bit per channel
		if ( header.imageIteratorBit != 0x08 ) return false;
		if ( header.width <= 0 ) return false;
		if ( header.height <= 0 ) return false;
		img.create( header.width, header.height );
		pixelBGRA bgra;
		for ( int y = 0; y < img.height; y++ ) {
			for ( int x = 0; x < img.width; x++ ) {
				f.read( bgra.b );
				f.read( bgra.g );
				f.read( bgra.r );
				f.read( bgra.a );
				img.set( x, img.height - 1 - y, bgra );
			}
		}
		return true;
	}
};
#pragma pack( pop )
