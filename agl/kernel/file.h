////////////////////////////////////////////////////////////////////////////////
// File

////////////////////////////////////////////////////////////////////////////////
//ファイルアクセスインタフェース
class file
{
public:
	static file * initialize( const char *filename, bool writable, bool binary, bool append = false );
	static void finalize( file * f );
	static bool exist( const char * filename );

	virtual ~file(){};
	virtual bool open( const char *_filename, bool writable, bool binary, bool append = false ) = 0;
	virtual void close() = 0;
	virtual bool eof() = 0;
	virtual unsigned int write( const void *buff, unsigned int size ) = 0;
	virtual unsigned int read( void *buff, unsigned int size ) = 0;
	virtual int seek( int pos, int origin ) = 0;
	virtual unsigned int tell() = 0;
	virtual void flush(){};

	char getChar()
	{
		char wk;
		if ( ! read( wk ) ) return ' ';
		return wk;
	}

	//サイズ固定の書き込み＆読み込み
	template<class T> bool write( const T & buff ) { return write( & buff, sizeof( T ) ) == sizeof( T ); }
	template<class T> bool read( T & buff ) { return read( & buff, sizeof( T ) ) == sizeof( T ); }

	//サイズ取得＆位置を先頭に
	virtual unsigned int getSize()
	{
		seek( 0, SEEK_END );
		const unsigned int size = tell();
		seek( 0, SEEK_SET );
		return size;
	}
	//ファイルへprintf構文を利用して書き込み
	void printf( char *fmt, ... )
	{
		char message[1024];
		va_list args;
		va_start( args, fmt );
		int length = vsprintf( message, fmt, args );
		va_end( args );
		if ( 0 < length ) write( message, length );
	}
	//改行コードまでの読み込み(ascii)
	int getLine( char *line, int maxLen )
	{
		if ( eof() ) return 0;
		if ( maxLen <= 0 ) return 0;
		int len = 0;
		maxLen--;
		while( len < maxLen ) {
			const char c = getChar();
			if ( eof() ) break;
			if( c == 0xa ) break;
			if( c != 0xd ) line[len++] = c;
		}
		line[len] = '\0';
		return len;
	}

};

////////////////////////////////////////////////////////////////////////////////
// 低水準入出力を利用した書き込み用
class lowFileSystem : public file
{
	int handle;
public:
	lowFileSystem() : handle( -1 ){}
	virtual ~lowFileSystem() { close(); };
	bool open( const char *filename, bool writable, bool binary, bool append )
	{
		if ( ! writable ) {
			if ( binary ) {
				handle = _open( filename, _O_RDONLY | _O_BINARY );
			} else {
				handle = _open( filename, _O_RDONLY | _O_TEXT );
			}
		} else {
			if ( binary ) {
				if ( append ) {
					handle = _open( filename, _O_RDWR | _O_APPEND | _O_BINARY, _S_IREAD | _S_IWRITE );
					if ( handle < 0 ) handle = _open( filename, _O_RDWR | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE );
				} else {
					handle = _open( filename, _O_RDWR | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE );
				}
			} else {
				if ( append ) {
					handle = _open( filename, _O_RDWR | _O_APPEND | _O_TEXT, _S_IREAD | _S_IWRITE );
					if ( handle < 0 ) handle = _open( filename, _O_RDWR | _O_CREAT | _O_TEXT, _S_IREAD | _S_IWRITE );
				} else {
					handle = _open( filename, _O_RDWR | _O_CREAT | _O_TRUNC | _O_TEXT, _S_IREAD | _S_IWRITE );
				}
			}
		}
		return ( handle < 0 ) ? false : true;
	}
	void close()
	{
		if ( 0 <= handle ) {
			_close( handle );
			handle = -1;
		}
	}
	bool eof(){ return _eof( handle ) ? true : false; }
	unsigned int write( const void *buff, unsigned int size ){ return _write( handle, buff, size ); }
	unsigned int read( void *buff, unsigned int size ){ return _read( handle, buff, size ); }
	int seek( int pos, int origin ){ return _lseek( handle, pos, origin ); }
	unsigned int tell(){ return _tell( handle ); }
	virtual void flush(){ _commit( handle ); };
};

////////////////////////////////////////////////////////////////////////////////
// 高水準入出力を利用した書き込み用
class fileSystem : public file
{
	FILE * handle;
public:
	fileSystem() : handle( NULL ){}
	virtual ~fileSystem() { close(); };
	bool open( const char *filename, bool writable, bool binary, bool append )
	{
		if ( ! writable ) {
			if ( binary ) {
				handle = fopen( filename, "rb" );
			} else {
				handle = fopen( filename, "rt" );
			}
		} else {
			if ( binary ) {
				if ( append ) {
					handle = fopen( filename, "ab" );
				} else {
					handle = fopen( filename, "wb" );
				}
			} else {
				if ( append ) {
					handle = fopen( filename, "at" );
				} else {
					handle = fopen( filename, "wt" );
				}
			}
		}
		return ( handle == NULL ) ? false : true;
	}
	void close()
	{
		if ( handle ) {
			fclose( handle );
			handle = NULL;
		}
	}
	bool eof(){ return feof( handle ) ? true : false; }
	unsigned int write( const void *buff, unsigned int size ){ return fwrite( buff, size, 1, handle ); }
	unsigned int read( void *buff, unsigned int size ){ return fread( buff, size, 1, handle ); }
	int seek( int pos, int origin ){ return fseek( handle, pos, origin ); }
	unsigned int tell(){ return ftell( handle ); }
	virtual void flush(){ fflush( handle ); };
};

////////////////////////////////////////////////////////////////////////////////
//メモリを利用した読み込み用
class fileMemory : public file
{
public:
	void * data;
	unsigned int size;
	unsigned int position;
	array<char> buffer;

	fileMemory( void * _data, int _size ) : data( _data ), size( _size ), position( 0 ){}
	virtual ~fileMemory() { close(); }
	bool open( const char * /*filename*/, bool /*writable*/, bool /*binary*/, bool append )
	{
		position = 0;
		if ( ! data && size ) {
			buffer.allocate( size );
			data = buffer.data;
		}
		if ( append ) {
			position = size;
		}
		return true;
	}
	void close() { size = position; }
	bool eof() { return size <= position; }
	unsigned int write( const void *buff, unsigned int _size )
	{
		int write_size = minimum( _size, size - position );
		if ( 0 < write_size ) memcpy( static_cast<char*>( data ) + position, buff, write_size );
		position += write_size;
		return write_size;
	}
	unsigned int read( void *buff, unsigned int _size )
	{
		int read_size = minimum( _size, size - position );
		if ( read_size <= 0 ) return 0;
		memcpy( buff, static_cast<char*>( data ) + position, read_size );
		position += read_size;
		return read_size;
	}
	int seek( int pos, int origin )
	{
		switch( origin ) {
			case SEEK_SET: position = pos; break;
			case SEEK_CUR: position += pos; break;
			case SEEK_END: position = size - pos; break;
		}
		return position = clamp<unsigned int>( 0, position, size );
	}
	unsigned int tell() { return position; }
};

//ファイルオープン。
inline file * file::initialize( const char *filename, bool writable, bool binary, bool append )
{
	fileSystem *fs = new fileSystem();
	if ( ! fs->open( filename, writable, binary, append ) ) {
		finalize( fs );
		return NULL;
	}
	if ( ! writable ) {
		//読み込みようならば､メモリにコピーする
		fileMemory *fm = new fileMemory( NULL, fs->getSize() );
		fm->open( filename, writable, binary, append );
		fs->read( fm->data, fm->size );
		finalize( fs );
		return fm;
	} else {
		return fs;
	}
}

//ファイルのクローズ。deleteでもよい
inline void file::finalize( file * f )
{
	f->close();
	delete f;
}

//ファイルの存在チェック
inline bool file::exist( const char * filename )
{
	fileSystem *fs = new fileSystem();
	bool result = fs->open( filename, false, true, false );
	finalize( fs );
	return result;
}
