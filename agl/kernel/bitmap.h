
template<class P>
class bitmap
{
public:
	class bitmask
	{
		DWORD mask;
		int shift;
		double maximum;
	public:
		bitmask()
		{
			initialize( 0 );
		}
		void initialize( DWORD _mask )
		{
			mask = _mask;
			for ( shift = 0, maximum = 1; shift < 32 && mask; ++shift ) {
				if ( ( mask >> shift ) & 1 ) {
					maximum = static_cast<double>( mask >> shift );
					return;
				}
			}
		}
		double operator()( DWORD data )
		{
			if ( ! mask ) return 1;
			return ( ( data & mask ) >> shift ) / maximum;
		}
	};
	static pixelBGRA convert( bitmask mask[4], DWORD data )
	{
		pixelT<P::element> p;
		p.RGBA( mask[0]( data ), mask[1]( data ), mask[2]( data ), mask[3]( data ) );
		return pixelBGRA( p );
	}
	//２４・３２ビットで保存
	static bool write( file & f, const imageInterface<P> & img )
	{
		BITMAPFILEHEADER bmpfile;
		bmpfile.bfType = WORD( 'B' ) + WORD( 'M' ) * 0x100;
		bmpfile.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + img.width * img.height * ( P::channel <= 3 ? 3 : 4 );
		bmpfile.bfReserved1 = 0;
		bmpfile.bfReserved2 = 0;
		bmpfile.bfOffBits = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER );
		BITMAPINFOHEADER bmpinfo = { sizeof( BITMAPINFOHEADER ) };
		bmpinfo.biWidth = img.width;
		bmpinfo.biHeight = img.height;
		bmpinfo.biPlanes = 1;
		bmpinfo.biBitCount = ( P::channel <= 3 ? 24 : 32 );
		bmpinfo.biCompression = BI_RGB;
		int line = img.width * int( bmpinfo.biBitCount / 8 );
		const int padding = 4;
		int space = 0;
		if ( line % padding ) {
			space = line;
			line /= padding;
			line = ( line + 1 ) * padding;
			space = line - space;
		}
		bmpfile.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + img.height * line;
		f.write( bmpfile );
		f.write( bmpinfo );
		int x, y;
		double depth = bmpinfo.biBitCount / 8.0;
		for ( y = 0; y < img.height; y++ ) {
			for ( x = 0; x < img.width; x++ ) {
				pixelBGRA p = img.get( x, img.height - 1 - y );
				f.write( p.b );
				f.write( p.g );
				f.write( p.r );
				if ( P::channel == 4 ) f.write( p.a );
			}
			if ( space ) {
				unsigned char wk = 0;
				for ( int x = 0; x < space; ++x ) f.write( wk );
			}
		}
		return true;
	}
	//１・４・８・１６・２４・３２ビット画像読み込み
	static bool read( file & f, imageInterface<P> & img )
	{
		int fs = f.getSize();//ファイルサイズ
		BITMAPFILEHEADER bmpfile;//ビットマップファイルヘッダ
		f.read( bmpfile );
		if ( bmpfile.bfType != WORD( 'B' ) + WORD( 'M' ) * 0x100 ) return false;
		if ( int( bmpfile.bfSize ) != fs ) return false;//ファイルサイズ情報チェック
		if ( bmpfile.bfReserved1 ) return false;//予約０
		if ( bmpfile.bfReserved2 ) return false;//予約０
		if ( f.eof() ) return false;//ファイル長チェック
		BITMAPINFOHEADER bmpinfo;//ビットマップファイルヘッダ
		f.read( bmpinfo );
		//OS2形式の場合書き換える
		if ( bmpinfo.biSize == sizeof( BITMAPCOREHEADER ) ) {
			BITMAPCOREHEADER bmpcore;
			memcpy( & bmpcore, & bmpinfo, sizeof( bmpcore ) );
			bmpinfo.biSize = sizeof( BITMAPCOREHEADER );
			bmpinfo.biWidth = bmpcore.bcWidth;
			bmpinfo.biHeight = bmpcore.bcHeight;
			bmpinfo.biPlanes = bmpcore.bcPlanes;
			bmpinfo.biBitCount = bmpcore.bcBitCount;
			bmpinfo.biCompression = BI_RGB;
			bmpinfo.biSizeImage = 0;
			bmpinfo.biXPelsPerMeter = 0;
			bmpinfo.biYPelsPerMeter = 0;
			bmpinfo.biClrUsed = 0;
			bmpinfo.biClrImportant = 0;
			//パレット先頭まで移動
			f.seek( sizeof( bmpfile ) + sizeof( bmpcore ), SEEK_SET );
		}
		//形式チェック
		if ( bmpinfo.biSize != sizeof( BITMAPINFOHEADER ) ) return false;
		if ( bmpinfo.biPlanes != 1 ) return false;//予約１
		if ( bmpinfo.biCompression == BI_RLE4 ) if ( bmpinfo.biBitCount != 4 ) return false;
		if ( bmpinfo.biCompression == BI_RLE8 ) if ( bmpinfo.biBitCount != 8 ) return false;
		if ( bmpinfo.biCompression == BI_BITFIELDS ) if ( bmpinfo.biBitCount != 16 && bmpinfo.biBitCount != 32 ) return false;
		if ( ! bmpinfo.biWidth ) return false;//サイズ０は不許可
		if ( ! bmpinfo.biHeight ) return false;//サイズ０は不許可
		//色深度チェック
		if ( bmpinfo.biBitCount != 1 && bmpinfo.biBitCount != 4 && 
			bmpinfo.biBitCount != 8 && bmpinfo.biBitCount != 16 && 
			bmpinfo.biBitCount != 24 && bmpinfo.biBitCount != 32 ) return false;
		int offset_palette = f.tell();//パレット先頭アドレス保存
		f.seek( bmpfile.bfOffBits, SEEK_SET );//イメージ先頭まで移動
		if ( f.eof() ) return false;//ファイル長チェック
		//読み込み画像準備
		img.create( bmpinfo.biWidth, bmpinfo.biHeight );
		int x, y;
		array<unsigned char> data;
		int line = 0;
		//４バイトパディング
		const int padding = 4;
		bitmask colorMask[4];
		//画素データ読み込み
		if ( bmpinfo.biCompression == BI_RGB || bmpinfo.biCompression == BI_BITFIELDS ) {
			if ( bmpinfo.biCompression == BI_BITFIELDS ) {
				//カラーマスク読み込み
				f.seek( offset_palette, SEEK_SET );
				DWORD temp;
				for ( int i = 0; i < 3; ++i ) {
					f.read( temp );
					colorMask[i].initialize( temp );
				}
			} else {
				switch ( bmpinfo.biBitCount ) {
				case 16:
					colorMask[0].initialize( 0x00007C00 );
					colorMask[1].initialize( 0x000003E0 );
					colorMask[2].initialize( 0x0000001F );
					break;
				case 24:
					colorMask[0].initialize( 0x00FF0000 );
					colorMask[1].initialize( 0x0000FF00 );
					colorMask[2].initialize( 0x000000FF );
					break;
				case 32:
					colorMask[0].initialize( 0x00FF0000 );
					colorMask[1].initialize( 0x0000FF00 );
					colorMask[2].initialize( 0x000000FF );
					colorMask[3].initialize( 0xFF000000 );
					break;
				}
			}
			//横幅のサイズ計算
			line = bmpinfo.biWidth * int( bmpinfo.biBitCount / 8 );
			//バイト単位でパディングが必要な場合の計算
			switch ( bmpinfo.biBitCount ) {
			case 1: line = ( bmpinfo.biWidth + 7 ) / 8; break;
			case 4: line = ( bmpinfo.biWidth + 1 ) / 2; break;
			}
			line = int( ( line + padding - 1 ) / padding ) * padding;
			//読み込みバッファ
			data.allocate( bmpinfo.biHeight * line );
			f.read( data.data, bmpinfo.biHeight * line );
		} else if ( bmpinfo.biCompression == BI_RLE8 ) {//８ビットランレングス圧縮
			//画素データDECODE
			array<unsigned char> buffer( fs - bmpfile.bfOffBits );
			f.read( buffer, buffer.size );
			//横幅のサイズ計算
			line = bmpinfo.biWidth;
			line = int( ( line + padding - 1 ) / padding ) * padding;
			x = y = 0;
			data.allocate( line * img.height );
			memset( data.data, 0, data.size );
			for ( array<unsigned char>::iterator it( buffer ); it; ) {
				unsigned char first = it(); ++it; if ( ! it ) break;
				unsigned char second = it(); ++it; if ( ! it ) break;
				if ( first == 0 ) {
					if ( second == 0 ) {//特殊シーケンス行の終わり
						++y; x = 0;
						if ( y == img.height ) break;
						continue;
					} else if ( second == 1 ) {//特殊シーケンスビットマップの終わり
						break;
					} else if ( second == 2 ) {//特殊シーケンス位置の移動
						unsigned char xx = it(); ++it; if ( ! it ) break;
						unsigned char yy = it(); ++it; if ( ! it ) break;
						x = x + xx;
						y = y + yy;
					} else {//リテラルグループ（そのまま）
						for ( int i = 0; i < second; ++i ) {
							unsigned char d = it(); ++it; if ( ! it ) break;
							data[x+line*y] = d;
							++x;
							if ( x == img.width ) {
								++y; x = 0;
								if ( y == img.height ) break;
							}
						}
						if ( second & 1 ) ++it;//偶数パディングのためのから読み
					}
				} else {//反復グループ（繰り返し）
					for ( int i = 0; i < first; ++i ) {
						data[x+line*y] = second;
						++x;
						if ( x == img.width ) {
							++y; x = 0;
							if ( y == img.height ) break;
						}
					}
				}
			}
		} else if ( bmpinfo.biCompression == BI_RLE4 ) {//４ビットランレングス圧縮
			//画素データDECODE
			array<unsigned char> buffer( fs - bmpfile.bfOffBits );
			f.read( buffer, buffer.size );
			//横幅のサイズ計算
			line = ( bmpinfo.biWidth + 1 ) / 2;
			line = int( ( line + padding - 1 ) / padding ) * padding;
			x = y = 0;
			int l = 0;
			//都合上ニブル単位で一時保存用バッファ
			array<unsigned char> nibble( 2 * line * img.height );
			memset( nibble.data, 0, nibble.size );
			for ( array<unsigned char>::iterator it( buffer ); it; ) {
				unsigned char first = it(); ++it; if ( ! it ) break;
				unsigned char second = it(); ++it; if ( ! it ) break;
				if ( first == 0 ) {
					if ( second == 0 ) {//特殊シーケンス行の終わり
						++y; x = 0;
						if ( y == img.height ) break;
						continue;
					} else if ( second == 1 ) {//特殊シーケンスビットマップの終わり
						break;
					} else if ( second == 2 ) {//特殊シーケンス位置の移動
						unsigned char xx = it(); ++it; if ( ! it ) break;
						unsigned char yy = it(); ++it; if ( ! it ) break;
						x = x + xx;
						y = y + yy;
					} else {//リテラルグループ（そのまま）
						for ( int i = 0; i < second; i += 2 ) {
							unsigned char d = it(); ++it; if ( ! it ) break;
							nibble[y*line*2+x] = ( d >> 4 ) & 0xf;
							++x;
							if ( x == img.width ) {
								x = 0; ++y;
								if ( y == img.height ) break;
							}
							if ( i + 1 < second ) {
								nibble[y*line*2+x] = d & 0xf;
								++x;
								if ( x == img.width ) {
									x = 0; ++y;
									if ( y == img.height ) break;
								}
							}
						}
						if ( 0 < ( second % 4 ) && ( second % 4 ) <= 2 ) ++it;//偶数パディングのためのから読み
					}
				} else {//反復グループ（繰り返し）
					const unsigned char ph = ( second >> 4 ) & 0xf;
					const unsigned char pl = second & 0xf;
					for ( int i = 0; i < first; ++i ) {
						nibble[y*line*2+x] = ( i & 1 ) ? pl : ph;
						++x;
						if ( x == img.width ) {
							x = 0; ++y;
							if ( y == img.height ) break;
						}
					}
				}
			}
			//ニブルをバイトに変換
			data.allocate( line * img.height );
			array<unsigned char>::iterator itn( nibble );
			for ( array<unsigned char>::iterator itd( data ); itd; ++itd ) {
				const unsigned char ph = itn(); ++itn;
				const unsigned char pl = itn(); ++itn;
				itd() = ( ph & 0xf ) << 4 | ( pl & 0xf );
			}
		}
		if ( bmpinfo.biBitCount == 1 || bmpinfo.biBitCount == 4 || bmpinfo.biBitCount == 8 ) {
			//パレット読み込み
			f.seek( offset_palette, SEEK_SET );
			int num = 0;//パレットサイズ
			if ( bmpinfo.biClrUsed == 0 ) {
				//デフォルトサイズ
				switch ( bmpinfo.biBitCount ) {
				case 1: num = 2; break;
				case 4: num = 16; break;
				case 8: num = 256; break;
				}
			} else {
				num = bmpinfo.biClrUsed;
			}
			//パレット読み込み
			array<pixelBGRA> palette( 256 );
			RGBQUAD rgbquad;
			for ( int i = 0; i < num; i++ ) {
				f.read( rgbquad );
				palette[i] = pixelBGRA( rgbquad.rgbRed, rgbquad.rgbGreen, rgbquad.rgbBlue );
			}
			//画素読み込み
			switch ( bmpinfo.biBitCount ) {
			case 1:
				//上位ビットが左端
				{
					const int table[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };
					for ( y = 0; y < img.height; y++ ) {
						for ( x = 0; x < img.width; ) {
							int index8 = data[ ( bmpinfo.biHeight - 1 - y ) * line + int( x / 8 ) ];
							for ( int i = 0; i < 8 && x < img.width; i++, x++ ) {
								img.set( x, y, palette[( index8 & table[i] ) ? 1 : 0] );
							}
						}
					}
				}
				break;
			case 4:
				//上位ビットが左端
				for ( y = 0; y < img.height; y++ ) {
					for ( x = 0; x < img.width; x++ ) {
						int index2 = data[ ( bmpinfo.biHeight - 1 - y ) * line + int( x / 2 ) ];
						img.set( x, y, palette[( index2 >> 4 ) & 0xf] );
						if ( ++x < img.width ) break;
						img.set( x, y, palette[index2 & 0xf] );
					}
				}
				break;
			case 8:
				for ( y = 0; y < img.height; y++ ) {
					for ( x = 0; x < img.width; x++ ) {
						img.set( x, y, palette[ int( data[ ( bmpinfo.biHeight - 1 - y ) * line + x ] ) ] );
					}
				}
				break;
			}
		} else {//１６・２４・３２
			switch ( bmpinfo.biBitCount ) {
			case 16:
				for ( y = 0; y < img.height; y++ ) {
					for ( x = 0; x < img.width; x++ ) {
						int offset = ( bmpinfo.biHeight - 1 - y ) * line + x * 2;
						img.set( x, y, convert( colorMask, * reinterpret_cast<WORD*>( data.data + offset ) ) );
					}
				}
				break;
			case 24:
				for ( y = 0; y < img.height; y++ ) {
					for ( x = 0; x < img.width; x++ ) {
						int offset = ( bmpinfo.biHeight - 1 - y ) * line + x * 3;
						img.set( x, y, convert( colorMask, data[offset] | ( data[offset+1] << 8 ) | ( data[offset+2] << 16 ) ) );
					}
				}
				break;
			case 32:
				for ( y = 0; y < img.height; y++ ) {
					for ( x = 0; x < img.width; x++ ) {
						int offset = ( bmpinfo.biHeight - 1 - y ) * line + x * 4;
						img.set( x, y, convert( colorMask, * reinterpret_cast<DWORD*>( data.data + offset ) ) );
					}
				}
				break;
			}
		}
		return true;
	}
};

