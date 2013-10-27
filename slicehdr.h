//CT画像ヘッダファイル読み込みクラス
class ctHeader
{
public:
	double fieldOfViewMM;
	int width, height, depth;
	array<double> position;
	string line;
	ctHeader() : fieldOfViewMM( 0 ), width( 0 ), height( 0 ), depth( 0 )
	{
		line.allocate( 1024 );
	}
private:
	//lineからtagを探してその後ろを数字に
	bool search( int & value, const char * tag )
	{
		char * result = strstr( line.chars(), tag );
		if ( result ) {
			value = atoi( result + strlen( tag ) );
			return true;
		}
		return false;
	}
	bool search( double & value, const char * tag )
	{
		char * result = strstr( line.chars(), tag );
		if ( result ) {
			value = atof( result + strlen( tag ) );
			return true;
		}
		return false;
	}
public:
	bool initialize( const char * filename )
	{
		retainer<file> f = file::initialize( filename, false, true );
		if ( ! f ) return false;
		while ( f->getLine( line.chars(), 1024 ) ) {
			if ( line == "END;" ) break;
			search( depth, "   VALID_SLICES : " );
			search( fieldOfViewMM, "   FIELD_OF_VIEW_MM : " );
			search( width, "   IMAGE_X_PIXNUM : " );
			search( height, "   IMAGE_Y_PIXNUM : " );
		}
		//depthが０の事もあるので，解析しながら追加で読み込む
		position.reserve( depth );
		while ( ! f->eof() ) {
			bool found = false;
			while ( f->getLine( line.chars(), 1024 ) ) {
				if ( line == "END;" ) break;
				if ( found ) continue;
				double wk;
				if ( search( wk, "   IMAGE_LOCATION : " ) ) {
					position.push_back( wk );
					found = true;
				}
			}
		}
		depth = position.size;
		return depth != 0;
	}
};
