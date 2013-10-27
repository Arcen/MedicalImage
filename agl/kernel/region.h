// 複雑な領域を表現するクラス
// 以下のデータ特性を有する
// データは必ず上下にソートされ、左右にソートされている。
// 同じtopを持つものは同じbottomを持つものしかない。
// 各矩形は重ならない。
class region
{
public:
	//点用
	class iterator
	{
		array<rect>::iterator itr, currentLineTop;
		point p;
	public:
		iterator( const iterator & it ) : itr( it.itr ), currentLineTop( it.currentLineTop ), p( it.p ) {}
		iterator( const region & base ) : itr( base.data ), currentLineTop( itr )
		{
			if ( itr ) {
				p.x = itr->left;
				p.y = itr->top;
			}
		}
		iterator & operator=( const iterator & it ){ itr = it.itr; currentLineTop = it.currentLineTop; p = it.p; return *this; }
		operator rect * () { return ( rect * ) itr; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		const point * operator->() const { assert( itr ); return & p; }
		point * operator->() { assert( itr ); return & p; }
		const point & operator*() const { assert( itr ); return p; }
		point & operator*() { assert( itr ); return p; }
		const point & operator()() const { assert( itr ); return p; }
		point & operator()() { assert( itr ); return p; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( ! itr ) return *this;
			if ( p.x + 1 < itr->right ) {
				++p.x;
			} else {
				++itr;
				if ( ( ! itr ) || ( itr->top != currentLineTop->top ) ) {
					if ( p.y + 1 < currentLineTop->bottom ) {
						itr = currentLineTop;
						p.x = itr->left;
						++p.y;
					} else {
						if ( itr ) {
							currentLineTop = itr;
							p.x = itr->left;
							p.y = itr->top;
						}
					}
				} else {
					p.x = itr->left;
				}
			}
			return *this;
		}
	private:
		////////////////////////////////////////////////////////////////////////////////
		//前へ 動作させない
		iterator operator--(int);
		iterator & operator--();
	};
	array<rect> data;
	region(){}
	region( const rect & r )
	{
		push_back( r );
	}
	region( const region & r )
	{
		*this = r;
	}
	void initialize()
	{
		data.allocate( 0 );
	}
	bool getBoundingBox( rect & r ) const
	{
		if ( ! data.size ) return false;
		r = data[0];
		array<rect>::iterator it( data );
		while ( it ) {
			r.left = minimum( r.left, it().left );
			r.right = maximum( r.right, it().right );
			r.top = minimum( r.top, it().top );
			r.bottom = maximum( r.bottom, it().bottom );
			++it;
		}
		return true;
	}
	// 後ろにつけるだけ
	rect & push_back( const rect & value ) { return data.push_back( value ); }
	// 後ろにつけるだけ
	rect & push_back( const int x, const int y ) { return data.push_back( rect::pixel( x, y ) ); }
	// 後ろにつけるだけ
	void push_back( const region & value )
	{
		for ( array<rect>::iterator it( value.data ); it; ++it ) data.push_back( it() );
	}
	// 中の開き領域を埋める
	void fill( region & result ) const
	{
		result.reserve( data.size );
		rect bb;
		if ( ! getBoundingBox( bb ) ) return;
		bb.expand( 1 );
		region outerBB( bb );
		region other = outerBB - *this;
		region outer; other.select( outer, bb.left, bb.top );
		result = outerBB - outer;
	}
	// 複数領域に分ける
	void divideMultiRegion( array<region> & result ) const
	{
		result.reserve( data.size );
		array<bool> flag;
		flag.allocate( data.size );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) itf() = true;
		itf = array<bool>::iterator( flag );
		for ( array<rect>::iterator it( data ); itf; ++it, ++itf ) {
			if ( ! itf() ) continue;
			region & rgn = result.push_back( region() );
			rgn.reserve( data.size );
			list<rect*> stack;
			stack.push_back( it ); *itf = false;
			while ( stack.size ) {
				rect * r = stack.loot_front();
				rgn.push_back( *r );
				array<rect>::iterator its( it );
				for ( array<bool>::iterator itsf( itf ); itsf; ++itsf, ++its ) {
					if ( itsf() &&
						( its().bottom == r->top || its().top == r->bottom ) &&
						its().left < r->right && r->left < its().right ) {
						itsf() = false;
						stack.push_back( & its() );
					}
				}
			}
			rgn.sort();
			rgn.connect();
		}
	}
	// 指定の領域だけを選択する
	void select( region & result, int x, int y ) const
	{
		array<bool> flag;
		flag.allocate( data.size );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) itf() = true;
		result.reserve( data.size );
		rect * r = inPoint( x, y );
		if ( ! r ) return;
		list<rect*> stack;
		stack.push_back( r );
		flag[ data.index( *r ) ] = false;
		while ( stack.size ) {
			rect * r = stack.loot_front();
			result.push_back( *r );
			array<rect>::iterator its( data );
			for ( array<bool>::iterator itsf( flag ); itsf; ++itsf, ++its ) {
				if ( itsf() &&
					( its().bottom == r->top || its().top == r->bottom ) &&
					its().left < r->right && r->left < its().right ) {
					itsf() = false;
					stack.push_back( & its() );
				}
			}
		}
		result.sort();
		result.connect();
	}
	//領域全体をオフセットする
	void offset( int x, int y )
	{
		for ( array<rect>::iterator it( data ); it; ++it ) it().offset( x, y );
	}
	// x, yを入れ替える
	void exchange( region & result ) const
	{
		result.reserve( data.size );
		rect r;
		array<bool> flag;
		getBoundingBox( r );
		int width = r.width(), height = r.height();
		flag.allocate( width * height );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) itf() = false;
		for ( iterator it( *this ); it; ++it ) flag[(it().y-r.top)+(it().x-r.left)*height]=true;
		result.set( flag, r.top, r.left, height, width );
	}
	// flag : width * heightの大きさの真偽値
	void set( const array<bool> & flag, int offsetX, int offsetY, int width, int height )
	{
		reserve( flag.size );
		int x, y;
		array<bool>::iterator it( flag );
		rect r;
		for ( y = 0; y < height; y++ ) {
			r.top = y + offsetY; r.bottom = r.top + 1;
			bool valid = false;
			for ( x = 0; x < width; x++ ) {
				if ( it() ) {
					if ( valid ) {
						r.right++;
					} else {
						valid = true;
						r.left = x + offsetX; r.right = r.left + 1;
					}
				} else {
					if ( valid ) {
						valid = false;
						push_back( r );
					}
				}
				++it;
			}
			if ( valid ) push_back( r );
		}
		connect();
	}
	// flag : width * heightの大きさの真偽値
	void get( array<bool> & flag, int offsetX, int offsetY, int width, int height )
	{
		flag.allocate( width * height );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		for ( array<rect>::iterator it( data ); it; ++it ) {
			rect r = it();
			if ( r.left < offsetX ) r.left = offsetX;
			if ( offsetX + width < r.right ) r.right = offsetX + width;
			if ( r.top < offsetY ) r.top = offsetY;
			if ( offsetY + height < r.bottom ) r.bottom = offsetY + height;
			for ( int y = r.top; y < r.bottom; y++ ) {
				for ( int x = r.left; x < r.right; x++ ) {
					flag[(x-offsetX)+(y-offsetY)*width] = true;
				}
			}
		}
	}
	void frame( region & result, const int band = 1 ) const
	{
		result.initialize();
		// set region
		rect r;
		if ( ! getBoundingBox( r ) ) return;
		int width = r.width();
		int height = r.height();
		// 初期化
		array<bool> flag;
		flag.allocate( width * height );
		{
			array<bool>::iterator it( flag );
			while ( it ) { *it = false; ++it; }
		}
		int x, y;
		{ // 設定
			for ( array<rect>::iterator it( data ); it; ++it ) {
				const int bottom = it->bottom - r.top;
				const int right = it->right - r.left;
				for ( y = it->top - r.top; y < bottom; y++ ) {
					for ( x = it->left - r.left; x < right; x++ ) {
						flag[x+y*width]=true;
					}
				}
			}
		}
		{ // 上下で貫く
			for ( x = 0; x < width; x++ ) {
				int size = 0;
				for ( y = 0; y < height; y++ ) {
					if ( flag[x+y*width] ) {
						if ( 2 * band <= size ) {
							flag[x+(y-band)*width]=false;
						}
						size++;
					} else {
						size = 0;
					}
				}
			}
		}
		{ // サイドを追加
			int i;
			for ( array<rect>::iterator it( data ); it; ++it ) {
				const int left = it->left-r.left;
				const int right = it->right-1-r.left;
				for ( int y = it->top; y < it->bottom; y++ ) {
					for ( i = 0; i < band && left + i <= right; i++ ) {
						flag[left+i+(y-r.top)*width] = true;
					}
					for ( i = 0; i < band && left <= right - i; i++ ) {
						flag[right-i+(y-r.top)*width] = true;
					}
				}
			}
		}
		result.set( flag, r.left, r.top, width, height );
	}
	// 外側に膨らませる
	void expand( region & result, int band = 1 ) const
	{
		rect r;
		getBoundingBox( r );
		r.expand( band );
		region boundingBox( r );
		r.expand( band );
		region outer( r );
		region inverse = outer - *this;
		region expandedFrame;
		inverse.frame( expandedFrame, band );
		result = *this | ( expandedFrame & boundingBox );
	}
	// 内側に縮める
	void shrink( region & result, int band = 1 ) const
	{
		region shrinkFrame;
		this->frame( shrinkFrame, band );
		result = *this - shrinkFrame;
	}
	//点が含まれている矩形を返す
	rect * inPoint( int x, int y ) const
	{
		for ( array<rect>::iterator it( data ); it; ++it ) {
			if ( it->top <= y && y < it->bottom &&
				it->left <= x && x < it->right ) {
				return it;
			}
		}
		return NULL;
	}
	// メモリ用意
	void reserve( int index )
	{
		data.reserve( index );
	}
	void optimize()
	{
		sort();
		connect();
	}
	// 面積計算
	int area()
	{
		int result = 0;
		for ( array<rect>::iterator it( data ); it; ++it ) {
			result += it->area();
		}
		return result;
	}
	// 線をセット
	void line( int x1, int y1, int x2, int y2 )
	{
		int dx = x2 - x1;
		int dy = y2 - y1;
		reserve( abs( dx ) * abs( dy ) );
		if ( dx == 0 && dy == 0 ) {
			push_back( rect::pixel( x1, y1 ) );
			return;
		}
		const int ax = ( dx > 0 ) ? 1 : ( dx < 0 ? ( dx *= -1, -1 ) : 0 );
		const int ay = ( dy > 0 ) ? 1 : ( dy < 0 ? ( dy *= -1, -1 ) : 0 );
		int c = 0, x = x1, y = y1;
		if ( dx >= dy ) {
			for ( int i = 0; i <= dx; i++ ) {
				push_back( rect::pixel( x, y ) );
				x += ax;

				c += dy;
				if ( c >= dx ) {
					y += ay;
					c -= dx;
				}
			}
		} else {
			for ( int i = 0; i <= dy; i++ ) {
				push_back( rect::pixel( x, y ) );
				y += ay;

				c += dx;
				if ( c >= dy ) {
					x += ax;
					c -= dy;
				}
			}
		}
		sort();
		connect();
	}
private:
	// 指示位置から同じ高さの領域のみを取る
	void getLine( region & result, array<rect>::iterator & it ) const
	{
		result.reserve( data.size );
		rect * startRect = it;
		while ( it ) {
			if ( it->top != startRect->top ) return;
			result.push_back( *it );
			++it;
		}
	}
	// 分割点候補の列挙
	void enumDividePointVertical( array<int> & result ) const
	{
		result.reserve( data.size * 2 );
		if ( ! data.size ) return;
		array<rect>::iterator it( data );
		int lastPoint = it->top - 1;
		while ( it ) {
			if ( lastPoint < it->top ) lastPoint = result.push_back( it->top );
			if ( lastPoint < it->bottom ) lastPoint = result.push_back( it->bottom );
			++it;
		}
	}
	// 分割点候補の列挙
	void enumDividePointHorizontal( array<int> & result, array<rect>::iterator & it ) const
	{
		result.reserve( data.size * 2 );
		if ( ! data.size ) return;
		if ( ! it ) return;
		rect * start = it;
		int lastPoint = start->left - 1;
		while ( it ) {
			if ( start->top != it->top ) return;

			if ( lastPoint < it->left ) lastPoint = result.push_back( it->left );
			if ( lastPoint < it->right ) lastPoint = result.push_back( it->right );
			++it;
		}
	}
	// 分割候補の合成
	static void orDividePoint( array<int> & result, array<int> & src1, array<int> & src2 )
	{
		result.reserve( src1.size + src2.size );
		if ( src1.size == 0 ) { result = src2; return; }
		if ( src2.size == 0 ) { result = src1; return; }
		array<int>::iterator it1( src1 );
		array<int>::iterator it2( src2 );
		while ( it1 && it2 ) {
			if ( it1() < it2() ) { result.push_back( it1() ); ++it1;
			} else if ( it2() < it1() ) { result.push_back( it2() ); ++it2;
			} else { result.push_back( it1() ); ++it1; ++it2; }
		}
		for ( ; it1; ++it1 ) result.push_back( *it1 );
		for ( ; it2; ++it2 ) result.push_back( *it2 );
	}
	// 分割候補による領域の分割
	void divideVertical( region & result, const array<int> & dividePoint ) const
	{
		result.initialize();
		result.reserve( data.size * dividePoint.size );

		array<rect>::iterator it( data );
		array<int>::iterator itDP( dividePoint );

		while ( it ) {
			rect & r = it();
			while ( itDP() < r.top ) ++itDP;
			// forward for bottom
			if ( itDP() == r.top ) ++itDP;
			region line;
			getLine( line, it );
			// divide
			int top = r.top;
			int bottom = itDP();
			while ( bottom <= r.bottom ) {
				for ( array<rect>::iterator itLine( line.data ); itLine; ++itLine ) {
					rect & push_backed = result.push_back( *itLine );
					push_backed.top = top;
					push_backed.bottom = bottom;
				}
				++itDP;
				if ( ! itDP ) break;
				top = bottom;
				bottom = itDP();
			}
		}
	}
	// 分割候補による領域の分割
	void divideHorizontal( region & result, int top, const array<int> & dividePoint ) const
	{
		result.initialize();
		result.reserve( data.size * dividePoint.size );

		for ( array<rect>::iterator it( data ); it; ++it ) {
			if ( it->top == top ) break;
		}
		if ( ! it ) return;

		array<int>::iterator itDP( dividePoint );
		for ( ; it; ++it ) {
			if ( it->top != top ) break;
			// forward
			while ( itDP() < it->left ) ++itDP;
			// forward for right
			if ( itDP() == it->left ) ++itDP;
			// divide
			rect wk = *it;
			for ( ; itDP && ( itDP() <= wk.right ); ++itDP ) {
				rect & push_backed = result.push_back( wk );
				wk.left = push_backed.right = itDP();
			}
		}
	}
	// 横の最適化
	void connectHorizontal( region & result ) const
	{
		result.reserve( data.size );
		rect prev;
		bool valid = false;
		for ( array<rect>::iterator it( data ); it; ++it ) {
			if ( valid ) {
				if ( prev.top == it->top ) {
					if ( prev.right == it->left ) {
						prev.right = it->right;
					} else {
						result.push_back( prev );
						prev.left = it->left;
						prev.right = it->right;
					}
				} else {
					result.push_back( prev );
					prev = it();
				}
			} else {
				prev = it();
				valid = true;
			}
		}
		if ( valid ) result.push_back( prev );
	}
	// 縦の最適化
	void connectVertical( region & result ) const
	{
		result.reserve( data.size );
		region prev;
		bool valid = false;
		for ( array<rect>::iterator it( data ); it; ) {
			region current;
			const int bottom = it->bottom;
			getLine( current, it );
			if ( valid ) {
				array<rect>::iterator it1( prev.data ), it2( current.data );
				bool same = false;
				if ( it1->bottom == it2->top ) {
					while ( it1 && it2 ) {
						if ( it1->left != it2->left || it1->right != it2->right ) break;
						++it1; ++it2;
					}
					if ( ! it1 && ! it2 ) same = true;
				}
				if ( same ) {
					for ( array<rect>::iterator itp( prev.data ); itp; ++itp ) {
						itp->bottom = bottom;
					}
				} else {
					result.push_back( prev );
					prev = current;
				}
			} else {
				prev = current;
				valid = true;
			}
		}
		if ( valid ) result.push_back( prev );
	}
public:
	// 分離している領域をつなげる最適化
	void connect()
	{
		region wk;
		connectHorizontal( wk );
		wk.connectVertical( *this );
	}
	// 並びを整える最適化
	void sort()
	{
		region result;
		bool valid = true;
		while ( valid ) {
			rect * r;
			valid = false;
			for ( array<rect>::iterator it( data ); it; ++it ) {
				if ( it->top < it->bottom ) {
					if ( ( ! valid ) ||
						it->top < r->top || 
						( it->top == r->top && it->left < r->left ) ) {
						r = it;
						valid = true;
					}
				}
			}
			if ( valid ) {
				result.push_back( *r );
				r->bottom = r->top;
			} else {
				*this = result;
				return;
			}
		}
	}
	bool isSorted() const 
	{
		array<rect>::iterator it( data );
		if ( ! it ) return true;
		if ( it->bottom <= it->top ) return false;
		if ( it->right <= it->left ) return false;
		rect prev = *it; ++it;
		for ( ; it; ++it ) {
			if ( it->bottom <= it->top ) return false;
			if ( it->right <= it->left ) return false;
			if ( it->top < prev.top ) return false;
			if ( prev.top == it->top ) {
				if ( prev.bottom != it->bottom ) return false;
				if ( it->left < prev.right ) return false;
			} else {
				if ( it->top < prev.bottom ) return false;
			}
		}
		return true;
	}
	bool isConnected() const 
	{
		array<rect>::iterator it( data );
		if ( ! it ) return true;
		if ( it->bottom <= it->top ) return false;
		if ( it->right <= it->left ) return false;
		rect prev = *it; ++it;
		for ( ; it; ++it ) {
			if ( it->bottom <= it->top ) return false;
			if ( it->right <= it->left ) return false;
			if ( it->top < prev.top ) return false;
			if ( prev.top == it->top ) {
				if ( prev.bottom != it->bottom ) return false;
				if ( it->left <= prev.right ) return false;
			} else {
				if ( it->top < prev.bottom ) return false;
			}
		}
		return true;
	}
private:
	static void divideVertical( region & result1, region & result2, const region & src1, const region & src2 )
	{
		array<int> src1dp, src2dp, dp;
		src1.enumDividePointVertical( src1dp );
		src2.enumDividePointVertical( src2dp );
		orDividePoint( dp, src1dp, src2dp );
		src1.divideVertical( result1, dp );
		src2.divideVertical( result2, dp );
	}
	static void divideHorizontal( region & result1, region & result2, array<rect>::iterator & it1, array<rect>::iterator & it2, const region & src1, const region & src2 )
	{
		const int top = ( ( rect * ) it1 )->top;
		array<int> src1dp, src2dp, dp;
		src1.enumDividePointHorizontal( src1dp, it1 );
		src2.enumDividePointHorizontal( src2dp, it2 );
		orDividePoint( dp, src1dp, src2dp );
		src1.divideHorizontal( result1, top, dp );
		src2.divideHorizontal( result2, top, dp );
	}
	typedef bool ( * operatorFunc )( bool left, bool right );
	static const region regionOperator( operatorFunc op, const region & src1, const region & src2 )
	{
		region result;
		region v1, v2, h1, h2;
		divideVertical( v1, v2, src1, src2 );
		array<rect>::iterator itv1( v1.data ), itv2( v2.data );
		rect r;
		while ( itv1 || itv2 ) {
			bool progress1 = false, progress2 = false;
			if ( itv1 && itv2 ) {
				if ( itv1->top == itv2->top ) {
					r.top = itv1->top; r.bottom = itv1->bottom;
					divideHorizontal( h1, h2, itv1, itv2, v1, v2 );
					array<rect>::iterator ith1( h1.data ), ith2( h2.data );
					while ( ith1 && ith2 ) {
						if ( ith1->left < ith2->left ) {
							if ( op( true, false ) ) result.push_back( ith1() );
							++ith1;
						} else if ( ith2->left < ith1->left ) {
							if ( op( false, true ) ) result.push_back( ith2() );
							++ith2;
						} else {
							if ( op( true, true ) ) result.push_back( ith1() );
							++ith1; ++ith2;
						}
					}
					for ( ; ith1; ++ith1 ) if ( op( true, false ) ) result.push_back( ith1() );
					for ( ; ith2; ++ith2 ) if ( op( false, true ) ) result.push_back( ith2() );
				} else if ( itv1->top < itv2->top ) { progress1 = true;
				} else if ( itv2->top < itv1->top ) { progress2 = true;
				}
			} else if ( itv1 ) { progress1 = true;
			} else if ( itv2 ) { progress2 = true;
			}
			if ( progress1 ) {
				const int top = itv1->top;
				for ( ; itv1; ++itv1 ) {
					if ( itv1->top != top ) break;
					if ( op( true, false ) ) result.push_back( itv1() );
				}
			}
			if ( progress2 ) {
				const int top = itv2->top;
				for ( ; itv2; ++itv2 ) {
					if ( itv2->top != top ) break;
					if ( op( false, true ) ) result.push_back( itv2() );
				}
			}
		}
		result.connect();
		return result;
	}
public:
	static bool orOperator( bool left, bool right ){ return left || right; }
	static bool andOperator( bool left, bool right ){ return left && right; }
	static bool xorOperator( bool left, bool right ){ return ( left && ( ! right ) ) || ( right && ( ! left ) ) ; }
	static bool minusOperator( bool left, bool right ){ return left && ( ! right ); }

	region & operator=( const region & value )
	{
		data = value.data;
		return *this;
	}
	const region operator|( const region & value ) const { return regionOperator( orOperator, *this, value ); }
	const region operator&( const region & value ) const { return regionOperator( andOperator, *this, value ); }
	const region operator^( const region & value ) const { return regionOperator( xorOperator, *this, value ); }
	const region operator-( const region & value ) const { return regionOperator( minusOperator, *this, value ); }
	region & operator|=( const region & value ) { return *this = *this | value; }
	region & operator&=( const region & value ) { return *this = *this & value; }
	region & operator^=( const region & value ) { return *this = *this ^ value; }
	region & operator-=( const region & value ) { return *this = *this - value; }

	bool operator==( const region & value ) const
	{
		array<rect>::iterator it1( data );
		array<rect>::iterator it2( value.data );
		while ( it1 && it2 ) {
			if ( *it1 != *it2 ) return false;
			++it1; ++it2;
		}
		if ( it1 || it2 ) return false;
		return true;
	}
	bool operator!=( const region & value ) const
	{
		return ! ( *this == value );
	}
};
