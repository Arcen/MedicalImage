//CT�摜�w�b�_�t�@�C���ǂݍ��݃N���X
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
	//line����tag��T���Ă��̌��𐔎���
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
		//depth���O�̎�������̂ŁC��͂��Ȃ���ǉ��œǂݍ���
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
