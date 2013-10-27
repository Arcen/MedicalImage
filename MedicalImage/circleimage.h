//円形以外の個所の画素情報を削除する事によって使用メモリ量を削減する予定で書いているソース

template<class P,class R>
class regionImageInterface : public imageBase<P>
{
	region<R> _rgn;
public:
	region<R> * rgn;
	class element
	{
	public:
		const region<R>::rectangle & r;
		const int offset;
		element( region<R>::rectangle & _r, int _offset ) : r( _r ), offset( _offset )
		{
		}
		element( const element & e ) : r( e.r ), offset( e.offset )
		{
		}
	};
private:
	array< list< retainer<element> > > _table;
public:
	array< list< retainer<element> > > * table;
	regionImageInterface() 
		: imageBase<P>( 0 ), rgn( & _rgn ), table( & table ){}
	regionImageInterface( const regionImageInterface & value ) 
		: imageBase<P>( 0 ), rgn( & _rgn ), table( & table )
	{ *this = value; }
	virtual ~regionImageInterface(){}
	virtual void finalize()
	{
		_rgn.release();
		_table.release();
		release();
	}

	bool create( const region<R> & defaultReagion, const array< list< retainer<element> > > & defaultTable )
	{
		if ( rgn == & defaultReagion && table == & defaultTable ) return true;
		rgn = & defaultReagion;
		table = & defaultTable;

		int area = rgn->area();
		if ( ! imageBase<P>::create( area ) ) return false;
		if ( ! enable() ) return true;
		fill( P() );
		return true;
	}
	bool create( const region<R> & defaultReagion )
	{
		if ( rgn == & defaultReagion ) return true;
		rgn = & defaultReagion;
		table = & _table;
		const region<R>::rectangle & bbox = rgn->get();
		table->allocate( bbox.height() );
		int index = 0;
		for ( array<region<R>::rectangle>::iterator it( rgn->rectangleIterator() ); it; ++it ) {
			region<R>::rectangle & r = it();
			for ( int y = 0; y < r.height(); ++y ) {
				list< retainer<element> > & l = (*table)[y+r.top-bbox.top];
				l.push_back( retainer<element>( new element( r, y * r.width() + index ) );
			}
			index += r.area();
		}
	}
	const regionImageInterface & operator=( const regionImageInterface & src )
	{
		create( *src.rgn, *src.table );
		if ( enable() ) {
			memcpy( data.data, src.data.data, sizeof( P ) * length );
		}
		return *this;
	}
	const int address( int x, int y ) const
	{
		const region<R>::rectangle & bbox = rgn->get();
		if ( ! bbox.in( x, y ) ) return -1;
		list< retainer<element> > & l = (*table)[y-bbox.top];
		for ( list< retainer<element> >::iterator it( (*table)[y-bbox.top] ); it; ++it ) {
			element & e = it()();
			if ( x < e.r.left ) continue;
			if ( x < e.r.right ) return e.offset + x - e.r.left;
			return -1;
		}
		return -1;
	}
	const bool in( int x, int y ) const
	{
		return address( x, y ) != -1;
	}
	const P & getInternal( int x, int y ) const { return imageBase::getInternal( address( x, y ) ); }
	void setInternal( int x, int y, const P & p ) { imageBase::setInternal( address( x, y ), p ); }
	const P & get( int x, int y ) const { const int addr = address( x, y ); if ( addr == -1 ) { static P wk; return wk; } return get( addr ); }
	void set( int x, int y, const P & p ) { const int addr = address( x, y ); if ( addr == -1 ) return; set( addr, p ); }

	const P & getInternal( int index ) const { return imageBase::getInternal( index ); }
	void setInternal( int index, const P & p ) { imageBase::setInternal( index, p ); }
	const P & get( int index ) const { return imageBase::get( index ); }
	void set( int index, const P & p ) { imageBase::set( index, p ); }
};

