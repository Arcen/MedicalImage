
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
	//�Q�S�E�R�Q�r�b�g�ŕۑ�
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
	//�P�E�S�E�W�E�P�U�E�Q�S�E�R�Q�r�b�g�摜�ǂݍ���
	static bool read( file & f, imageInterface<P> & img )
	{
		int fs = f.getSize();//�t�@�C���T�C�Y
		BITMAPFILEHEADER bmpfile;//�r�b�g�}�b�v�t�@�C���w�b�_
		f.read( bmpfile );
		if ( bmpfile.bfType != WORD( 'B' ) + WORD( 'M' ) * 0x100 ) return false;
		if ( int( bmpfile.bfSize ) != fs ) return false;//�t�@�C���T�C�Y���`�F�b�N
		if ( bmpfile.bfReserved1 ) return false;//�\��O
		if ( bmpfile.bfReserved2 ) return false;//�\��O
		if ( f.eof() ) return false;//�t�@�C�����`�F�b�N
		BITMAPINFOHEADER bmpinfo;//�r�b�g�}�b�v�t�@�C���w�b�_
		f.read( bmpinfo );
		//OS2�`���̏ꍇ����������
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
			//�p���b�g�擪�܂ňړ�
			f.seek( sizeof( bmpfile ) + sizeof( bmpcore ), SEEK_SET );
		}
		//�`���`�F�b�N
		if ( bmpinfo.biSize != sizeof( BITMAPINFOHEADER ) ) return false;
		if ( bmpinfo.biPlanes != 1 ) return false;//�\��P
		if ( bmpinfo.biCompression == BI_RLE4 ) if ( bmpinfo.biBitCount != 4 ) return false;
		if ( bmpinfo.biCompression == BI_RLE8 ) if ( bmpinfo.biBitCount != 8 ) return false;
		if ( bmpinfo.biCompression == BI_BITFIELDS ) if ( bmpinfo.biBitCount != 16 && bmpinfo.biBitCount != 32 ) return false;
		if ( ! bmpinfo.biWidth ) return false;//�T�C�Y�O�͕s����
		if ( ! bmpinfo.biHeight ) return false;//�T�C�Y�O�͕s����
		//�F�[�x�`�F�b�N
		if ( bmpinfo.biBitCount != 1 && bmpinfo.biBitCount != 4 && 
			bmpinfo.biBitCount != 8 && bmpinfo.biBitCount != 16 && 
			bmpinfo.biBitCount != 24 && bmpinfo.biBitCount != 32 ) return false;
		int offset_palette = f.tell();//�p���b�g�擪�A�h���X�ۑ�
		f.seek( bmpfile.bfOffBits, SEEK_SET );//�C���[�W�擪�܂ňړ�
		if ( f.eof() ) return false;//�t�@�C�����`�F�b�N
		//�ǂݍ��݉摜����
		img.create( bmpinfo.biWidth, bmpinfo.biHeight );
		int x, y;
		array<unsigned char> data;
		int line = 0;
		//�S�o�C�g�p�f�B���O
		const int padding = 4;
		bitmask colorMask[4];
		//��f�f�[�^�ǂݍ���
		if ( bmpinfo.biCompression == BI_RGB || bmpinfo.biCompression == BI_BITFIELDS ) {
			if ( bmpinfo.biCompression == BI_BITFIELDS ) {
				//�J���[�}�X�N�ǂݍ���
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
			//�����̃T�C�Y�v�Z
			line = bmpinfo.biWidth * int( bmpinfo.biBitCount / 8 );
			//�o�C�g�P�ʂŃp�f�B���O���K�v�ȏꍇ�̌v�Z
			switch ( bmpinfo.biBitCount ) {
			case 1: line = ( bmpinfo.biWidth + 7 ) / 8; break;
			case 4: line = ( bmpinfo.biWidth + 1 ) / 2; break;
			}
			line = int( ( line + padding - 1 ) / padding ) * padding;
			//�ǂݍ��݃o�b�t�@
			data.allocate( bmpinfo.biHeight * line );
			f.read( data.data, bmpinfo.biHeight * line );
		} else if ( bmpinfo.biCompression == BI_RLE8 ) {//�W�r�b�g���������O�X���k
			//��f�f�[�^DECODE
			array<unsigned char> buffer( fs - bmpfile.bfOffBits );
			f.read( buffer, buffer.size );
			//�����̃T�C�Y�v�Z
			line = bmpinfo.biWidth;
			line = int( ( line + padding - 1 ) / padding ) * padding;
			x = y = 0;
			data.allocate( line * img.height );
			memset( data.data, 0, data.size );
			for ( array<unsigned char>::iterator it( buffer ); it; ) {
				unsigned char first = it(); ++it; if ( ! it ) break;
				unsigned char second = it(); ++it; if ( ! it ) break;
				if ( first == 0 ) {
					if ( second == 0 ) {//����V�[�P���X�s�̏I���
						++y; x = 0;
						if ( y == img.height ) break;
						continue;
					} else if ( second == 1 ) {//����V�[�P���X�r�b�g�}�b�v�̏I���
						break;
					} else if ( second == 2 ) {//����V�[�P���X�ʒu�̈ړ�
						unsigned char xx = it(); ++it; if ( ! it ) break;
						unsigned char yy = it(); ++it; if ( ! it ) break;
						x = x + xx;
						y = y + yy;
					} else {//���e�����O���[�v�i���̂܂܁j
						for ( int i = 0; i < second; ++i ) {
							unsigned char d = it(); ++it; if ( ! it ) break;
							data[x+line*y] = d;
							++x;
							if ( x == img.width ) {
								++y; x = 0;
								if ( y == img.height ) break;
							}
						}
						if ( second & 1 ) ++it;//�����p�f�B���O�̂��߂̂���ǂ�
					}
				} else {//�����O���[�v�i�J��Ԃ��j
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
		} else if ( bmpinfo.biCompression == BI_RLE4 ) {//�S�r�b�g���������O�X���k
			//��f�f�[�^DECODE
			array<unsigned char> buffer( fs - bmpfile.bfOffBits );
			f.read( buffer, buffer.size );
			//�����̃T�C�Y�v�Z
			line = ( bmpinfo.biWidth + 1 ) / 2;
			line = int( ( line + padding - 1 ) / padding ) * padding;
			x = y = 0;
			int l = 0;
			//�s����j�u���P�ʂňꎞ�ۑ��p�o�b�t�@
			array<unsigned char> nibble( 2 * line * img.height );
			memset( nibble.data, 0, nibble.size );
			for ( array<unsigned char>::iterator it( buffer ); it; ) {
				unsigned char first = it(); ++it; if ( ! it ) break;
				unsigned char second = it(); ++it; if ( ! it ) break;
				if ( first == 0 ) {
					if ( second == 0 ) {//����V�[�P���X�s�̏I���
						++y; x = 0;
						if ( y == img.height ) break;
						continue;
					} else if ( second == 1 ) {//����V�[�P���X�r�b�g�}�b�v�̏I���
						break;
					} else if ( second == 2 ) {//����V�[�P���X�ʒu�̈ړ�
						unsigned char xx = it(); ++it; if ( ! it ) break;
						unsigned char yy = it(); ++it; if ( ! it ) break;
						x = x + xx;
						y = y + yy;
					} else {//���e�����O���[�v�i���̂܂܁j
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
						if ( 0 < ( second % 4 ) && ( second % 4 ) <= 2 ) ++it;//�����p�f�B���O�̂��߂̂���ǂ�
					}
				} else {//�����O���[�v�i�J��Ԃ��j
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
			//�j�u�����o�C�g�ɕϊ�
			data.allocate( line * img.height );
			array<unsigned char>::iterator itn( nibble );
			for ( array<unsigned char>::iterator itd( data ); itd; ++itd ) {
				const unsigned char ph = itn(); ++itn;
				const unsigned char pl = itn(); ++itn;
				itd() = ( ph & 0xf ) << 4 | ( pl & 0xf );
			}
		}
		if ( bmpinfo.biBitCount == 1 || bmpinfo.biBitCount == 4 || bmpinfo.biBitCount == 8 ) {
			//�p���b�g�ǂݍ���
			f.seek( offset_palette, SEEK_SET );
			int num = 0;//�p���b�g�T�C�Y
			if ( bmpinfo.biClrUsed == 0 ) {
				//�f�t�H���g�T�C�Y
				switch ( bmpinfo.biBitCount ) {
				case 1: num = 2; break;
				case 4: num = 16; break;
				case 8: num = 256; break;
				}
			} else {
				num = bmpinfo.biClrUsed;
			}
			//�p���b�g�ǂݍ���
			array<pixelBGRA> palette( 256 );
			RGBQUAD rgbquad;
			for ( int i = 0; i < num; i++ ) {
				f.read( rgbquad );
				palette[i] = pixelBGRA( rgbquad.rgbRed, rgbquad.rgbGreen, rgbquad.rgbBlue );
			}
			//��f�ǂݍ���
			switch ( bmpinfo.biBitCount ) {
			case 1:
				//��ʃr�b�g�����[
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
				//��ʃr�b�g�����[
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
		} else {//�P�U�E�Q�S�E�R�Q
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

