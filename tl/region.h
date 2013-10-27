// ���G�ȗ̈��\������N���X
// �ȉ��̃f�[�^������L����悤�Ɉ���
// �f�[�^�͕K���㉺�Ƀ\�[�g����A���E�Ƀ\�[�g����Ă���B
// ����top�������͓̂���bottom�������̂����Ȃ��B
// �e��`�͏d�Ȃ�Ȃ��B
// �e��`�͍��E�ɗׂ荇������connecte�֐��ɂ��Ȃ���D
template<class T>
class region
{
public:
	typedef rectangle2<T> rectangle;
	typedef point2<T> point;
	//�_�p
	class iterator
	{
		typename array<rectangle>::iterator itr, currentLineTop;
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
		operator rectangle * () { return ( rectangle * ) itr; }
		operator const rectangle * () const { return ( const rectangle * ) itr; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		point & operator()() { assert( itr ); return p; }
		const point & operator()() const { assert( itr ); return p; }
		////////////////////////////////////////////////////////////////////////////////
		//���� ���P���C�����Ƃɉ��ɐi��ł����A�̈���̈�_���w��
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
		////////////////////////////////////////////////////////////////////////////////
		//�O�� ���e�X�g
		iterator & operator--()
		{
			if ( ! itr ) return *this;
			if ( itr->left < p.x ) {
				--p.x;
			} else {
				--itr;
				if ( ( ! itr ) || ( itr->top != currentLineTop->top ) ) {
					if ( currentLineTop->top < p.y ) {
						itr = currentLineTop;
						array<rectangle>::iterator next( itr );
						++next;
						while ( next && next->top == currentLineTop->top ) {
							itr = next;
							++next;
						}
						p.x = itr->right - 1;
						--p.y;
					} else {
						if ( itr ) {
							currentLineTop = itr;
							array<rectangle>::iterator prev( currentLineTop );
							--prev;
							while ( prev && prev->top == itr->top ) {
								currentLineTop = prev;
								--prev;
							}
							p.x = itr->right - 1;
							p.y = itr->bottom - 1;
						}
					}
				} else {
					p.x = itr->right - 1;
				}
			}
			return *this;
		}
	public:
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
		point & operator*() { return ( * this )(); } const point & operator*() const { return ( * this )(); }
		point * operator->() { return & ( * this )(); } const point * operator->() const { return & ( * this )(); }
	};
private:
	friend iterator;
	array<rectangle> data;
	rectangle bbox;//�o�E���f�B���O�{�b�N�X
	double size;//�ʐ�
public:
	region() : size( 0 ){}
	region( const rectangle & r ) : size( 0 )
	{
		push_back( r );
	}
	region( const point & r ) : size( 0 )
	{
		push_back( rectangle::pixel( r.x, r.y ) );
	}
	region( const region & r )
	{
		*this = r;
	}
	void initialize()
	{
		data.allocate( 0 );
		size = 0;
		bbox.initialize();
	}
	void release()
	{
		initialize();
		data.release();
	}
	//�o�E���f�B���O�{�b�N�X
	const rectangle & get() const { return bbox; }
	//�ʐ�
	double area() const { return size; }
	//��`�̃C�e���[�^�[
	typename array<rectangle>::iterator rectangleIterator() const 
	{
		return array<rectangle>::iterator( data );
	}
	// �������p��
	void reserve( int index )
	{
		initialize();
		data.reserve( index );
	}
    bool full() const
    {
        return data.capacity() == data.size;
    }
	bool empty() const
	{
		return size ? false : true;
	}
	// ���ɂ��āC�ʐςƃo�E���f�B���O�{�b�N�X���v�Z
	const rectangle & push_back( const rectangle & value )
	{
		rectangle & ret = data.push_back( value );
		if ( bbox.area() == 0 ) {
			bbox = value;
		} else {
			bbox.left = minimum( bbox.left, value.left );
			bbox.right = maximum( bbox.right, value.right );
			bbox.top = minimum( bbox.top, value.top );
			bbox.bottom = maximum( bbox.bottom, value.bottom );
		}
		size += value.area();
		return ret;
	}
	const rectangle & push_back( const T x, const T y )
	{
		return push_back( rectangle::pixel( x, y ) );
	}
	void push_back( const region & value )
	{
		for ( array<rectangle>::iterator it( value.data ); it; ++it ) {
			push_back( it() );
		}
	}
	//�ŏ��ƍŌ�̋�`��Ԃ�
	const rectangle & first() const
	{
		return data.first();
	}
	const rectangle & last() const
	{
		return data.last();
	}
	//�_���܂܂�Ă����`��Ԃ�
	rectangle * in( T x, T y ) const
	{
		if ( ! bbox.in( x, y ) ) return NULL;//�o�E���f�B���O�{�b�N�X�O�Ȃ炠�肦�Ȃ�
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			if ( it->in( x, y ) ) return it;
		}
		return NULL;
	}
	// �d�S�v�Z
	point center() const
	{
		point2<double> result( 0, 0 );
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			const double a = it->area();
			result += point2<double>( ( it->left + it->right - 1 ) * a / 2.0,
									  ( it->top + it->bottom - 1 ) * a / 2.0 );
		}
		return point( T( result.x / size ), T( result.y / size ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// �̈�ύX
	////////////////////////////////////////////////////////////////////////////////
	//�̈�S�̂��I�t�Z�b�g����
	void offset( T x, T y )
	{
		for ( array<rectangle>::iterator it( data ); it; ++it ) 
			it().offset( x, y );
	}
	void offset( region & result, T x, T y ) const 
	{
		result = *this;
		result.offset( x, y );
	}
	// x, y�����ւ���
	void exchange( region & result ) const
	{
		result.reserve( data.size );
		array2<bool> flags;
		flags.allocate( bbox.height(), bbox.width() );
		getExchange( flags, bbox.left, bbox.top );
		result.set( flags, r.top, r.left );
	}
	// flag : width * height�̑傫���̐^�U�l
	void set( const array2<bool> & flag, T offsetX, T offsetY )
	{
		const T width = flag.w;
		const T height = flag.h;
		reserve( flag.size );
		array2<bool>::iterator it( flag );
		rectangle r;
		for ( T y = 0; y < height; y++ ) {
			r.top = y + offsetY; r.bottom = r.top + 1;
			bool valid = false;
			for ( T x = 0; x < width; x++ ) {
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
	// flag : width * height�̑傫���̐^�U�l
	void get( array2<bool> & flag, T offsetX, T offsetY ) const
	{
		const T width = flag.w;
		const T height = flag.h;
		const T flag_left = offsetX;
		const T flag_top = offsetY;
		const T flag_right = offsetX + width;
		const T flag_bottom = offsetY + height;
		for ( array2<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			rectangle r = it();
			if ( r.left < flag_left ) r.left = flag_left;
			if ( flag_right < r.right ) r.right = flag_right;
			if ( r.top < flag_top ) r.top = flag_top;
			if ( flag_bottom < r.bottom ) r.bottom = flag_bottom;
            r.offset( - offsetX, - offsetY );
			for ( T y = r.top; y < r.bottom; y++ ) {
				for ( T x = r.left; x < r.right; x++ ) {
					flag( x, y ) = true;
				}
			}
		}
	}
private:
	//X,Y�������ւ��Ĕz��ɓ����
	void getExchange( array2<bool> & flag, T offsetX, T offsetY ) const
	{
		const int & width = flag.h;
		const int & height = flag.w;
		for ( array2<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			rectangle r = it();
			if ( r.left < offsetX ) r.left = offsetX;
			if ( offsetX + width < r.right ) r.right = offsetX + width;
			if ( r.top < offsetY ) r.top = offsetY;
			if ( offsetY + height < r.bottom ) r.bottom = offsetY + height;
			for ( T y = r.top; y < r.bottom; y++ ) {
				for ( T x = r.left; x < r.right; x++ ) {
					flag( y - offsetY, x - offsetX ) = true;
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	// �̈�ڑ�
	////////////////////////////////////////////////////////////////////////////////
private:
	//�Q�s���Ȃ���
	static bool connect( region & line, region & prevLine )
	{
		if ( prevLine.data.size != line.data.size ) return false;
		if ( prevLine.data.first().bottom != line.data.first().top ) return false;
		array<rectangle>::iterator itpl( prevLine.data );
		array<rectangle>::iterator itl( line.data );
		for ( ; itl && itpl; ++itl, ++itpl ) {
			if ( itl->left != itpl->left ||
				itl->right != itpl->right ) return false;
		}
		T newBottom = line.data.first().bottom;
		for ( array<rectangle>::iterator it( prevLine.data ); it; ++it ) {
			it->bottom = newBottom;
		}
		return true;
	}
	//�Q�s���q���邩�e�X�g���āC�q����Ȃ��Ȃ�Ό��ʂɈȑO�̍s��ǉ�
	static void connect( region * & now, region * & prev, region & result )
	{
		if ( prev->empty() ) {//�O�̍s�������ꍇ�ɂ́C��񂵂ɂ��Ď���Ă���
			swap( prev, now );//���O�̍s��ݒ�
		} else if ( ! connect( *now, *prev ) ) {//�q���邩���ׂāC�q����悤�Ȃ�΁C�ȑO�̃��C�������������Ă���
			//�Ȃ���Ȃ�����
			result.push_back( *prev );//���ʂɈȑO�̋�`��ǉ���
			swap( prev, now );//���O�̍s��ݒ�
		}
		now->initialize();
	}
public:
	// �ڑ� ���ʂɌq�����Ă��Ȃ����������ď����𖞂����Ă����Ԃ𐮓ڂ���
	void connect()
	{
		region result;
		result.reserve( data.size );

		region lines[2];
		region * prev = & lines[0], * now = & lines[1];
		prev->reserve( data.size );
		now->reserve( data.size );
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			if ( now->empty() ) {//�ŏ��̋�`�͖������ǉ�
				now->push_back( it() );
			} else {
				rectangle & prevrect = now->data.last();//�Ō�̋�`�����o��
				if ( prevrect.top == it->top ) {//���݂Əオ��v���Ă��邩
					if ( prevrect.right == it->left ) {//���łȂ���
						prevrect.right = it->right;
					} else {//����Ă��āC���������Ȃ̂Œǉ�
						now->push_back( it() );
					}
				} else {//���̍s�̋�`�����݂̃^�[�Q�b�g�ɂȂ��Ă���
					connect( now, prev, result );
					now->push_back( it() );
				}
			}
		}
		if ( ! now->empty() ) {
			connect( now, prev, result );
			result.push_back( *prev );
		}
		*this = result;//���ʂȗe�ʂ���x�m�ۂ��Ă���̂ō폜���邽�߂ɑ��
	}
	bool isConnected() const 
	{
		array<rectangle>::iterator it( data );
		if ( ! it ) return true;
		if ( it->bottom <= it->top ) return false;
		if ( it->right <= it->left ) return false;
		rectangle prev = *it; ++it;
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
	////////////////////////////////////////////////////////////////////////////////
	// �̈�_�����Z
	////////////////////////////////////////////////////////////////////////////////
private:
	// �����_���̗�
	void enumDividePointVertical( array<T> & result ) const
	{
		result.reserve( data.size * 2 );
		if ( empty() ) return;
		array<rectangle>::iterator it( data );
		T lastPoint = it->top - 1;
		T lastTop = lastPoint;
		for ( ; it; ++it ) {
			if ( lastTop == it->top ) continue;
			if ( lastPoint < it->top ) lastPoint = result.push_back( it->top );
			if ( lastPoint < it->bottom ) lastPoint = result.push_back( it->bottom );
			lastTop = it->top;
		}
	}
	// �����_���̗�
	void enumDividePointHorizontal( array<T> & result, typename array<rectangle>::iterator & it ) const
	{
		result.reserve( data.size * 2 );
		if ( ! data.size ) return;
		if ( ! it ) return;
		array<rectangle>::iterator start( it );
		T lastPoint = start->left - 1;
		for ( ; it && start->top == it->top; ++it ) {
			if ( lastPoint < it->left ) lastPoint = result.push_back( it->left );
			if ( lastPoint < it->right ) lastPoint = result.push_back( it->right );
		}
	}
	// �������̍���
	static void orDividePoint( array<T> & result, array<T> & src1, array<T> & src2 )
	{
		if ( src1.size == 0 ) { result = src2; return; }
		if ( src2.size == 0 ) { result = src1; return; }
		result.reserve( src1.size + src2.size );
		array<T>::iterator it1( src1 ), it2( src2 );
		while ( it1 && it2 ) {
			const T i1 = it1(), i2 = it2();
			if ( i1 < i2 ) {
				for ( ; it1 && it1() < i2; ++it1 ) {
					result.push_back( it1() );
				}
			} else if ( i2 < i1 ) {
				for ( ; it2 && it2() < i1; ++it2 ) {
					result.push_back( it2() );
				}
			} else {
				result.push_back( i1 ); ++it1; ++it2;
			}
		}
		for ( ; it1; ++it1 ) result.push_back( it1() );
		for ( ; it2; ++it2 ) result.push_back( it2() );
	}
	// �������ɂ��̈�̕���
	void divideVertical( region & result, const array<T> & dividePoint ) const
	{
		result.reserve( data.size * dividePoint.size );

		array<rectangle>::iterator it( data );
		array<T>::iterator itDP( dividePoint );

		region line;
		while ( it ) {
			const rectangle & r = it();
			while ( itDP && itDP() <= r.top ) ++itDP;
			// �w���ʒu���瓯�������̗̈�݂̂����
			line.reserve( data.size );
			for ( rectangle * startRect = it; it; ++it ) {
				if ( it->top != startRect->top ) break;
				line.push_back( *it );
			}
			// divide
			T top = r.top, bottom = itDP ? itDP() : r.bottom;
			while ( bottom <= r.bottom ) {
				for ( array<rectangle>::iterator itLine( line.data ); itLine; ++itLine ) {
                    rectangle l = *itLine;
                    l.top = top;
                    l.bottom = bottom;
					result.push_back( l );
				}
				++itDP;
				if ( ! itDP ) break;
				top = bottom;
				bottom = itDP();
			}
		}
	}
	// �������ɂ��̈�̕���
	void divideHorizontal( region & result, typename array<rectangle>::iterator & it, const array<T> & dividePoint ) const
	{
		T top = it->top;
		result.reserve( data.size * dividePoint.size );
		array<T>::iterator itDP( dividePoint );
		for ( ; it && it->top == top; ++it ) {
			// forward
			while ( itDP && itDP() <= it->left ) ++itDP;
			// divide
			// wk : ���ɓ�����`
			rectangle wk = it();
			for ( ; itDP && itDP() <= wk.right; ++itDP ) {
                rectangle r = wk;//��������`
				//���̕����|�C���g�ō�������`��؂�A���ɓ�����`�̍���ݒ�
                r.right = wk.left = itDP();
				result.push_back( r );
			}
		}
	}
	static void divideVertical( region & result1, region & result2, const region & src1, const region & src2 )
	{
		array<T> src1dp, src2dp, dp;
		src1.enumDividePointVertical( src1dp );
		src2.enumDividePointVertical( src2dp );
		orDividePoint( dp, src1dp, src2dp );
		src1.divideVertical( result1, dp );
		src2.divideVertical( result2, dp );
	}
	static void divideHorizontal( region & result1, region & result2, typename array<rectangle>::iterator & it1, typename array<rectangle>::iterator & it2, const region & src1, const region & src2 )
	{
		const T top = ( ( rectangle * ) it1 )->top;
		array<T> src1dp, src2dp, dp;
		array<rectangle>::iterator bit1( it1 ), bit2( it2 );
		src1.enumDividePointHorizontal( src1dp, it1 );
		src2.enumDividePointHorizontal( src2dp, it2 );
		orDividePoint( dp, src1dp, src2dp );
		src1.divideHorizontal( result1, bit1, dp );
		src2.divideHorizontal( result2, bit2, dp );
	}
	typedef bool ( * operatorFunc )( bool left, bool right );
	static bool orOperator( bool left, bool right ){ return left || right; }
	static bool andOperator( bool left, bool right ){ return left && right; }
	static bool xorOperator( bool left, bool right ){ return ( left && ( ! right ) ) || ( right && ( ! left ) ) ; }
	static bool minusOperator( bool left, bool right ){ return left && ( ! right ); }
	static const region regionOperator( operatorFunc op, const region & src1, const region & src2 )
	{
		//�_�����Z�������Ȃ��D�������C���E���������߁Cfalse�Efalse�̉��Z���ʂ�true�̏ꍇ�͋����Ȃ�
		//�R�̎�蓾�鉉�Z�̌��ʂ��擾
		const bool opTT = op( true, true ), opTF = op( true, false ), opFT = op( false, true );
		//���ʂ̏���
		region result;
		result.reserve( src1.area() + src2.area() );
		region v1, v2, h1, h2;
		//�c�����ɕ�������
		divideVertical( v1, v2, src1, src2 );
		array<rectangle>::iterator itv1( v1.data ), itv2( v2.data );
		rectangle r;
		while ( itv1 && itv2 ) {//�����ɏ����Ώۂ�����ꍇ�ɂ�
			if ( itv1->top < itv2->top ) {//�Е���TOP��菬���������͑S�ď�������
				const T & till = itv2->top;
				if ( opTF ) {
					for ( ; itv1 && itv1->top < till; ++itv1 ) {
						result.push_back( itv1() );
					}
				} else {
					for ( ; itv1 && itv1->top < till; ++itv1 );
				}
			} else if ( itv2->top < itv1->top ) {//�Е���TOP��菬���������͑S�ď�������
				const T & till = itv1->top;
				if ( opFT ) {
					for ( ; itv2 && itv2->top < till; ++itv2 ) {
						result.push_back( itv2() );
					}
				} else {
					for ( ; itv2 && itv2->top < till; ++itv2 );
				}
			} else {
				r.top = itv1->top; r.bottom = itv1->bottom;
				divideHorizontal( h1, h2, itv1, itv2, v1, v2 );
				array<rectangle>::iterator ith1( h1.data ), ith2( h2.data );
				while ( ith1 && ith2 ) {
					if ( ith1->left < ith2->left ) {
						if ( opTF ) {
							for ( ; ith1 && ith1->left < ith2->left; ++ith1 ) {
								result.push_back( ith1() );
							}
						} else {
							for ( ; ith1 && ith1->left < ith2->left; ++ith1 );
						}
					} else if ( ith2->left < ith1->left ) {
						if ( opFT ) {
							for ( ; ith2 && ith2->left < ith1->left; ++ith2 ) {
								result.push_back( ith2() );
							}
						} else {
							for ( ; ith2 && ith2->left < ith1->left; ++ith2 );
						}
					} else {
						if ( opTT ) {
							for ( ; ith1 && ith2 && ith1->left == ith2->left; ++ith1, ++ith2 ) {
								result.push_back( ith1() );
							}
						} else {
							for ( ; ith1 && ith2 && ith1->left == ith2->left; ++ith1, ++ith2 );
						}
					}
				}
				if ( opTF ) for ( ; ith1; ++ith1 ) result.push_back( ith1() );
				if ( opFT ) for ( ; ith2; ++ith2 ) result.push_back( ith2() );
			}
		}
		//�c�蕨�̕Е������ׂď�������
		if ( opTF ) for ( ; itv1; ++itv1 ) result.push_back( itv1() );
		if ( opFT ) for ( ; itv2; ++itv2 ) result.push_back( itv2() );
		result.connect();
		return result;
	}
public:
	const region operator|( const region & src ) const { return regionOperator( orOperator, *this, src ); }
	const region operator&( const region & src ) const { return regionOperator( andOperator, *this, src ); }
	const region operator^( const region & src ) const { return regionOperator( xorOperator, *this, src ); }
	const region operator-( const region & src ) const { return regionOperator( minusOperator, *this, src ); }
	region & operator|=( const region & src ) { return *this = *this | src; }
	region & operator&=( const region & src ) { return *this = *this & src; }
	region & operator^=( const region & src ) { return *this = *this ^ src; }
	region & operator-=( const region & src ) { return *this = *this - src; }

	bool operator==( const region & src ) const
	{
		if ( area() != src.area() ) return false;//�ʐς��r
		if ( bbox != src.bbox ) return false;//�o�E���f�B���O�{�b�N�X���r
		if ( data.size != src.data.size ) return false;//��`�����r
		for ( array<rectangle>::iterator itd( data ), its( src.data ); itd; ++itd, ++its ) {
			if ( itd() != its() ) return false;
		}
		return true;
	}
	bool operator!=( const region & src ) const
	{
		return ! ( *this == src );
	}
/*
	////////////////////////////////////////////////////////////////////////////////
	// �̈�֐�
	static bool collision( const region & src1, const region & src2 )
	{
		if ( ! ( src1.get() & src2.get() ).area() ) return false;
		//���ʂ̏���
		region v1, v2, h1, h2;
		//�c�����ɕ�������
		divideVertical( v1, v2, src1, src2 );
		array<rectangle>::iterator itv1( v1.data ), itv2( v2.data );
		while ( itv1 && itv2 ) {//�����ɏ����Ώۂ�����ꍇ�ɂ�
			if ( itv1->top < itv2->top ) {//�Е���TOP��菬���������͑S�ď�������
				const T & till = itv2->top;
				for ( ; itv1 && itv1->top < till; ++itv1 );
			} else if ( itv2->top < itv1->top ) {//�Е���TOP��菬���������͑S�ď�������
				const T & till = itv1->top;
				for ( ; itv2 && itv2->top < till; ++itv2 );
			} else {
				//r.top = itv1->top; r.bottom = itv1->bottom;
				divideHorizontal( h1, h2, itv1, itv2, v1, v2 );
				array<rectangle>::iterator ith1( h1.data ), ith2( h2.data );
				while ( ith1 && ith2 ) {
					if ( ith1->left < ith2->left ) {
						for ( ; ith1 && ith1->left < ith2->left; ++ith1 );
					} else if ( ith2->left < ith1->left ) {
						for ( ; ith2 && ith2->left < ith1->left; ++ith2 );
					} else {
						for ( ; ith1 && ith2 && ith1->left == ith2->left; ++ith1, ++ith2 ) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}
    */
	////////////////////////////////////////////////////////////////////////////////
	// �����Z�b�g
	void line( T x1, T y1, T x2, T y2 )
	{
		const T ax = sign( x2 - x1 ), ay = sign( y2 - y1 );//�i�s�����̕����̌v�Z
		const T dx = abs( x2 - x1 ), dy = abs( y2 - y1 );//�����̌v�Z
		const T w = dx + 1, h = dy + 1;//�����̌v�Z
		if ( dx + dy == 0 ) {
			*this = region( rectangle::pixel( x1, y1 ) );
			return;
		}
		array2<bool> flag( w, h );
		for ( array2<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		const T ex = dx << 1, ey = dy << 1;//�^�̒l�ɋ߂��悤�Ɍ덷��0.5�̏㉺�Ŕ��肷�邽�߂Ɍ덷�̌v�Z�p�̐��l���v�Z����
		T e = 0, x = x1, y = y1;//�덷�Ə����ʒu��ݒ�
		T bx = minimum( x1, x2 ), by = minimum( y1, y2 );
		if ( dx >= dy ) {
			for ( T t = 0; t <= dx; ++t ) {
				flag( x - bx, y - by ) = true;
				x += ax;
				e += ey;
				if ( e >= dx ) {
					y += ay;
					e -= ex;
				}
			}
		} else {
			for ( T t = 0; t <= dy; ++t ) {
				flag( x - bx, y - by ) = true;
				y += ay;
				e += ex;
				if ( e >= dy ) {
					x += ax;
					e -= ey;
				}
			}
		}
		set( flag, bx, by );
	}
	// �~���Z�b�g
	void circle( T x, T y, T r )
	{
		r = abs( r );
		int size = 2 * r + 1;
		reserve( size * size );
		if ( r == 0 ) {
			*this = region( rectangle::pixel( x, y ) );
			return;
		}
		array2<bool> flag( size, size );
		for ( array2<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		T s = 0, t = r;
		T e = 3 - 2 * r;
		while ( s <= t ) {
			flag( r + s, r + t ) = flag( r - s, r + t ) = flag( r + s, r - t ) = flag( r - s, r - t ) =
			flag( r + t, r + s ) = flag( r - t, r + s ) = flag( r + t, r - s ) = flag( r - t, r - s ) = true;
			if ( e < 0 ) {
				e += 4 * s + 6;
			} else {
				--t;
				e += 4 * ( s - t ) + 6;
			}
			++s;
		}
		set( flag, x, y );
	}
	// ���̋󂫗̈�𖄂߂�
	void fill( region & result ) const
	{
		result.reserve( data.size );
		if ( ! size ) return;
		rectangle bb = bbox;
		bb.expand( 1 );
		region outerBB( bb );
		region other = outerBB - *this;
		region outer; other.select( outer, bb.left, bb.top );
		result = outerBB - outer;
	}
	// �����̈�ɕ�����
	void divideMultiRegion( array<region> & result ) const
	{
		result.reserve( data.size );
		if ( ! data.size ) return;
		array<bool> flag;//�g�p�ς݂̋�`�̃t���O
		array<bool> part;//�����ς݂Ə������̋�`�̃t���O
		list<rectangle*> stack;//�q�����Ă��鏈�����̋�`�̃��X�g
		//�t���O������
		flag.allocate( data.size );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) itf() = false;

		array<bool>::iterator itf( flag );//�t���O�ƑΉ������`�ɂ��Ă܂킷
		region rgn;
		rgn.reserve( data.size );
		for ( array<rectangle>::iterator it( data ); itf; ++it, ++itf ) {
			if ( itf() ) continue;//�����ς̏ꍇ
			const int index = data.index( it() );//���̗v�f�ԍ��𓾂�
			part = flag;//���݂̏�Ԃ��L��
			part[index] = true;//�������̃t���O�𗧂Ă�
			stack.push_back( it );//�ŏ��̋�`���X�^�b�N�ɓ����
			while ( ! stack.empty() ) {//�X�^�b�N�ɋ�`���c���Ă���
				rectangle * r = stack.loot_front();//��`�𓾂�
				int index = data.index( *r );//�Ή�����C���f�b�N�X�𓾂�
				//���E�Ɍq�����Ă��鎖�͂Ȃ��Ɖ��肵�C�㉺�����֌q�����Ă��镨��T���D
				//���̋�`����E�������ɂȂ����T��
				array<rectangle>::iterator its( data, index ); ++its;
				array<bool>::iterator itsp( part, index ); ++itsp;
				for ( ; itsp; ++itsp, ++its ) {
					if ( its->top != r->bottom ) continue;
					if ( ! itsp() &&//�����ςł͂Ȃ�
						its->top == r->bottom &&
						its->left < r->right && r->left < its->right ) {//�ڂ��Ă����
						itsp() = true;//�������̃t���O�𗧂�
						stack.push_back( & its() );//�X�^�b�N�ɒǉ�����
					}
					if ( r->bottom < its->top ) break;//���ʂɉ������͒T���Ȃ�
				}
				//��������ɂ��T��
				its = array<rectangle>::iterator( data, index ); --its;
				itsp = array<bool>::iterator( part, index ); --itsp;
				for ( ; itsp; --itsp, --its ) {
					if ( its->bottom != r->top ) continue;
					if ( ! itsp() &&
						its->left < r->right && r->left < its->right ) {
						itsp() = true;
						stack.push_back( & its() );
					}
					if ( r->top > its->bottom ) break;
				}
			}
			//�V�����̈�����X�g�ɒǉ�
			rgn.initialize();
			//�q�����Ă����`�݂̂��݂��C�ǉ�����
			array<rectangle>::iterator its( data, index );//����ׂ����X�g�̐擪�ʒu�͕������Ă���D
			array<bool>::iterator itsf( flag, index ), itsp( part, index );
			for ( ; itsp; ++itsf, ++itsp, ++its ) {
				if ( itsf() ) continue;
				if ( itsp() ) {
					rgn.push_back( its() );
					itsf() = true;
				}
			}
			rgn.connect();
			result.push_back( rgn );
		}
	}
	// �w��̗̈悾����I������
	void select( region & result, T x, T y ) const
	{
		array<bool> flag;
		flag.allocate( data.size );
		for ( array<bool>::iterator itf( flag ); itf; ++itf ) itf() = false;
		rectangle * r = in( x, y );
		if ( ! r ) {
			result.initialize();
			return;
		}
		list<rectangle*> stack;
		stack.push_back( r );
		flag[ data.index( *r ) ] = true;
		while ( ! stack.empty() ) {
			r = stack.loot_front();
			int index = data.index( *r );
			//���E�ׂ荇���Ă�����̂͂Ȃ��̂ŁA�㉺�����`�F�b�N����B
			array<rectangle>::iterator its( data, index );
			array<bool>::iterator itsf( flag, index );
			for ( ; itsf; ++itsf, ++its ) {
				if ( ! itsf() &&
					( its->bottom == r->top || its->top == r->bottom ) &&
					its->left < r->right && r->left < its->right ) {
					itsf() = true;
					stack.push_back( & its() );
				}
				if ( r->bottom < its->top ) break;
			}
			its = array<rectangle>::iterator( data, index );
			itsf = array<bool>::iterator( flag, index );
			for ( ; itsf; --itsf, --its ) {
				if ( ! itsf() &&
					( its->bottom == r->top || its->top == r->bottom ) &&
					its->left < r->right && r->left < its->right ) {
					itsf() = true;
					stack.push_back( & its() );
				}
				if ( r->top > its->bottom ) break;
			}
		}
		result.reserve( data.size );
		array<rectangle>::iterator its( data );
		for ( array<bool>::iterator itsf( flag ); itsf; ++its, ++itsf ) {
			if ( itsf() ) result.push_back( its() );
		}
		result.connect();
	}
	//d((0,0),(x,y))=d��y,d��^�����Ƃ��̍ő��x��Ԃ�
	//�A���C0<y,0<x�͈͓̔�����
	typedef T ( *distance )( T y, T d );
	static T distance4( T y, T d ){ return y <= d ? d - y: 0 ; }//d=x+y���
	static T distance8( T y, T d ){ return y <= d ? d : 0; }//d=max(x,y)���
	static T distanceEuclidis( T y, T d )//d=sqrt(x^2+y^2)���
	{
		if ( y <= 0 ) return d;
		if ( d <= y ) return 0;
		return round<T,double>( ::sqrt( d * d - y * y ) );
	}
	static T distanceMichener( T y, T d )//�~�b�`�F�[�i�[�̉~����v�Z
	{
		if ( y <= 0 ) return d;
		if ( d <= y ) return 0;
		T s = 0, t = d;
		T e = 3 - 2 * d;
		while ( ! ( s == y || t == y ) ) {
			if ( e < 0 ) {
				e += 4 * s + 6;
			} else {
				--t;
				e += 4 * ( s - t ) + 6;
			}
			++s;
		}
		return ( s == y ) ? t : s;
	}
	// �g��
	void dilation( region & result, int count = 1, distance function = distanceMichener ) const
	{
		result.initialize();
		if ( empty() ) return;
		if ( count < 1 ) return;
		T offsetX = bbox.left - count, offsetY = bbox.top - count, width = bbox.width() + count * 2, height = bbox.height() + count * 2;
		array2<bool> flag( width, height );
		for ( array2<bool>::iterator itf( flag ); itf; ++itf ) *itf = false;
		array<T> distances( count + 1 );
		for ( int i = 0; i <= count; ++i ) distances[i] = function( i, count );
		for ( array<rectangle>::iterator it( data ); it; ++it ) {
			const rectangle & r = it();
			const T w = r.width();
			const T & d = distances[0];
			//��`�����ɍL���ăt���O���Z�b�g
			for ( int i = r.top; i < r.bottom; ++i ) {
				for ( int x = -d; x < w + d; ++x ) {
					flag( r.left - offsetX + x, i - offsetY ) = true;
				}
			}
			//��`�̏㉺�ɑ������������C�p�͂ǂꂾ���������̕����v�Z���C���̕��𑝂₷
			for ( int i = 1; i <= count; ++i ) {
				const T & d = distances[i];
				for ( int x = -d; x < w + d; ++x ) {
					flag( r.left - offsetX + x, r.top - i - offsetY ) = true;
					flag( r.left - offsetX + x, r.bottom - 1 + i - offsetY ) = true;
				}
			}
		}
		result.set( flag, offsetX, offsetY );
	}
	// �k��
	void erosion( region & result, int count = 1, distance function = distanceMichener ) const
	{
		result.initialize();
		if ( empty() ) return;
		rectangle r = bbox;
		r.expand( 1 );
		region outer( r );
		( outer - *this ).dilation( result, count, function );
		result = *this - result;
	}
	// �̈�����
	void closing( region & result, int number, distance function = distanceMichener ) const
	{
		if ( number <= 0 ) { result = *this; return; }
		region wk;
		dilation( wk, number, function );
		wk.erosion( result, number, function );
	}
	// �̈��������
	void opening( region & result, int number, distance function = distanceMichener ) const
	{
		if ( number <= 0 ) { result = *this; return; }
		region wk;
		erosion( wk, number, function );
		wk.dilation( result, number, function );
	}
};
