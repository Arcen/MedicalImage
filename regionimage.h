//領域以外の個所の画素情報を削除する事によって使用メモリ量を削減する予定で書いているソース

template<class P,class R>
class regionImageInterface : public imageBase<P>
{
public:
	region<R> shape;
	int width, height;
	// x, y -> index : search rectangle, need offset each rectangle
	class element
	{
	public:
		typename region<R>::rectangle r;
		int offset;
		element() : offset( 0 ){}
		element( const typename region<R>::rectangle & _r, int _offset ) : r( _r ), offset( _offset )
		{
		}
		element( const element & e ) : r( e.r ), offset( e.offset )
		{
		}
		bool operator==( const element & v ) const
		{
			return r == v.r && offset == v.offset;
		}
		bool operator!=( const element & v ) const { return ! ( *this == v ); }
	};
public:
	list< array<element> > linelist;
	array< array<element> * > ytable;
public:
	class iterator
	{
		typename list< array<element> >::iterator lineit;
		typename array<element>::iterator elementit;
		int index;
		point2<R> position;
	public:
		////////////////////////////////////////////////////////////////////////////////
		//生成・消滅
		iterator( const regionImageInterface & src ) 
			: lineit( src.linelist ),
			elementit( lineit ? array<element>::iterator( lineit() ) : array<element>::iterator() ), 
			index( 0 ), position( 0, 0 )
		{
		}
		iterator( const iterator & it ) 
			: lineit( it.lineit ), elementit( it.elementit ), index( it.index ), 
			position( it.position ){}
		iterator & operator=( const iterator & src )
		{
			lineit = src.lineit;
			elementit = src.elementit;
			index = src.index;
			position = src.position;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator bool () const 
		{
			if ( ! lineit ) return false;
			if ( ! elementit ) return false;
			return between<int>( 0, index, elementit().r.area() - 1 );
		}
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		int operator()() const { return index + ( elementit().offset ); }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( ! lineit ) return *this;
			if ( ! elementit ) return *this;
			++index;
			++position.x;
			if ( position.x == elementit().r.width() ) {
				++position.y;
				position.x = 0;
			}
			if ( position.y == elementit().r.height() ) {
				index = 0;
				position.x = position.y = 0;
				++elementit;
				if ( ! elementit ) {
					++lineit;
					if ( lineit ) {
						elementit = array<element>::iterator( lineit() );
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		int x() const { return position.x + elementit().r.left; }
		int y() const { return position.y + elementit().r.top; }
	};
	regionImageInterface() : imageBase<P>( 0 ), width( 0 ), height( 0 ) {}
	regionImageInterface( const regionImageInterface & value ) : imageBase<P>( 0 ), width( 0 ), height( 0 )
	{ *this = value; }
	virtual ~regionImageInterface(){}
	virtual void finalize()
	{
		width = 0; height = 0;
		shape.release();
		linelist.release();
		ytable.release();
		release();
	}
	void create( R width, R height )
	{
		create( region<R>( region<R>::rectangle::size( width, height ) ) );
	}
	void create( const region<R> & input )
	{
		finalize();
		shape = input;
		const region<R>::rectangle & bbox = shape.get();
		width = bbox.width();
		height = bbox.height();
		ytable.allocate( height );
		for ( array< array<element> * >::iterator it( ytable ); it; ++it ) {
			it() = NULL;
		}
		int offset = 0;
		int lasty = bbox.top - 1;
		array<element> * lastlinenode = NULL;
        for ( region<R>::rectangle_const_enumrator it( shape.range() ); it; ++it ) {
			const region<R>::rectangle & r = *it;
			if ( r.top != lasty ) {
				lastlinenode = & linelist.push_back( array<element>() );
				lasty = r.top;
				for ( int y = r.top; y < r.bottom; ++y ) {
					ytable[y-bbox.top] = lastlinenode;
				}
			}
			lastlinenode->push_back( element( r, offset ) );
			offset += r.area();
		}
		imageBase<P>::create( offset );
	}
	regionImageInterface & operator=( const regionImageInterface & src )
	{
		create( src.shape );
		width = src.width;
		height = src.height;
		if ( enable() ) {
			for ( int i = 0; i < size; ++i ) {
				data[i] = src.data[i];
			}
		}
		return *this;
	}
	regionImageInterface & operator=( const imageInterface<P> & src )
	{
		create( src.width, src.height );
		if ( enable() ) {
			for ( int i = 0; i < size; ++i ) {
				data[i] = src.data[i];
			}
		}
		return *this;
	}
	bool copy( const regionImageInterface & src )
	{
		bool notenough = false;
		for ( iterator it( *this ); it; ++it ) {
			int x = it.x();
			int y = it.y();
			int addr = src.address( x, y );
			if ( addr < 0 ) {
				notenough = true;
				continue;
			}
			setInternal( it(), src.getInternal( addr ) );
		}
		return ! notenough;//コピー先に空きがあり十分でない．
	}
	bool copy( const imageInterface<P> & src )
	{
		bool notenough = false;
		for ( iterator it( *this ); it; ++it ) {
			int x = it.x();
			int y = it.y();
			if ( ! src.inrect( x, y ) ) {
				notenough = true;
				continue;
			}
			setInternal( it(), src.getInternal( x, y ) );
		}
		return ! notenough;//コピー先に空きがあり十分でない．
	}
	const int address( int x, int y ) const
	{
		const region<R>::rectangle & bbox = shape.get();
		if ( ! bbox.in( x, y ) ) return -1;
		array<element> * line = ytable[y-bbox.top];
		if ( ! line ) return -1;
		for ( array<element>::iterator it( *line ); it; ++it ) {
			element & e = it();
			if ( e.r.right <= x ) continue;
			if ( e.r.left <= x ) return e.offset + x - e.r.left + ( y - e.r.top ) * e.r.width();
			return -1;
		}
		return -1;
	}
	const bool in( int x, int y ) const
	{
		return address( x, y ) != -1;
	}
	const P & getInternal( int x, int y ) const { return imageBase<P>::getInternal( address( x, y ) ); }
	void setInternal( int x, int y, const P & p ) { imageBase<P>::setInternal( address( x, y ), p ); }
	const P & get( int x, int y ) const { const int addr = address( x, y ); if ( addr == -1 ) { static P wk; return wk; } return get( addr ); }
	void set( int x, int y, const P & p ) { const int addr = address( x, y ); if ( addr == -1 ) return; set( addr, p ); }

	const P & getInternal( int index ) const { return imageBase<P>::getInternal( index ); }
	void setInternal( int index, const P & p ) { imageBase<P>::setInternal( index, p ); }
	const P & get( int index ) const { return imageBase<P>::get( index ); }
	void set( int index, const P & p ) { imageBase<P>::set( index, p ); }
};

