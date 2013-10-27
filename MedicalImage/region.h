#pragma once
#include <algorithm>
#include <deque>
#include <vector>
#include <list>
#include <assert.h>

//�o�����L�q�q�p�񋓎q
template<typename C>
class enumrator {
    typedef C container_type;
    typedef typename container_type::iterator iterator;
    iterator firstPosition;
    iterator currentPosition;
    iterator sentinel;
public:
    typedef typename container_type::value_type value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    enumrator()
    {
    }
    explicit enumrator( container_type & container )
        : firstPosition( container.begin() )
        , currentPosition( firstPosition )
        , sentinel( container.end() )
    {
    }
    enumrator( container_type & container, size_t index )
        : firstPosition( container.begin() )
        , currentPosition( firstPosition )
        , sentinel( container.end() )
    {
        std::advance( currentPosition, index );
    }
	enumrator( const enumrator & rhs )
        : firstPosition( rhs.firstPosition )
        , currentPosition( rhs.currentPosition )
        , sentinel( rhs.sentinel )
	{
	}
	enumrator & operator=( const enumrator & rhs )
    {
        if ( this != & rhs ) {
            firstPosition = rhs.firstPosition;
            currentPosition = rhs.currentPosition;
            sentinel = rhs.sentinel;
        }
        return *this;
    }
	////////////////////////////////////////////////////////////////////////////////
	//�ړ�
	enumrator & operator++()
	{
        if ( currentPosition != sentinel ) ++currentPosition;
		return *this;
	}
	enumrator & operator--()
	{
        if ( currentPosition != sentinel ) {
            if ( currentPosition == firstPosition ) {
                currentPosition = sentinel;
            } else {
                --currentPosition;
            }
        }
		return *this;
	}
	enumrator operator++(int) { enumrator wk(*this); ++(*this); return wk; }
	enumrator operator--(int) { enumrator wk(*this); --(*this); return wk; }
	////////////////////////////////////////////////////////////////////////////////
	//�^�ϊ�
    /*
	reference operator()()
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
	const_reference operator()() const
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
    */
	reference operator*()
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
	const_reference operator*() const
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
	pointer operator->()
    {
        assert( currentPosition != sentinel );
        return &*currentPosition;
    }
    const_pointer operator->() const
    {
        assert( currentPosition != sentinel );
        return &*currentPosition;
    }
	operator pointer()
    {
        if ( currentPosition == sentinel ) return 0;
        return &*currentPosition;
    }
	operator const_pointer() const
    {
        if ( currentPosition == sentinel ) return 0;
        return &*currentPosition;
    }
	size_t index() const
    {
        return std::distance( firstPosition, currentPosition );
    }
    iterator begin() { return firstPosition; }
    iterator end() { return sentinel; }
    iterator it() { return currentPosition; }
};
template<typename C>
class const_enumrator {
    typedef C container_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    const_iterator firstPosition;
    const_iterator currentPosition;
    const_iterator sentinel;
public:
    typedef typename container_type::value_type value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    const_enumrator()
    {
    }
    explicit const_enumrator( const container_type & container )
        : firstPosition( container.begin() )
        , currentPosition( firstPosition )
        , sentinel( container.end() )
    {
    }
    const_enumrator( const container_type & container, size_t index )
        : firstPosition( container.begin() )
        , currentPosition( firstPosition )
        , sentinel( container.end() )
    {
        std::advance( currentPosition, index );
    }
	const_enumrator( const const_enumrator & rhs )
        : firstPosition( rhs.firstPosition )
        , currentPosition( rhs.currentPosition )
        , sentinel( rhs.sentinel )
	{
	}
	const_enumrator( const enumrator<C> & rhs )
        : firstPosition( rhs.firstPosition )
        , currentPosition( rhs.currentPosition )
        , sentinel( rhs.sentinel )
	{
	}
	const_enumrator & operator=( const const_enumrator & rhs )
    {
        if ( this != & rhs ) {
            firstPosition = rhs.firstPosition;
            currentPosition = rhs.currentPosition;
            sentinel = rhs.sentinel;
        }
        return *this;
    }
	const_enumrator & operator=( const enumrator<C> & rhs )
    {
        if ( this != & rhs ) {
            firstPosition = rhs.firstPosition;
            currentPosition = rhs.currentPosition;
            sentinel = rhs.sentinel;
        }
        return *this;
    }
	////////////////////////////////////////////////////////////////////////////////
	//�ړ�
	const_enumrator & operator++()
	{
        if ( currentPosition != sentinel ) ++currentPosition;
		return *this;
	}
	const_enumrator & operator--()
	{
        if ( currentPosition != sentinel ) {
            if ( currentPosition == firstPosition ) {
                currentPosition = sentinel;
            } else {
                --currentPosition;
            }
        }
		return *this;
	}
	const_enumrator operator++(int) { const_enumrator wk(*this); ++(*this); return wk; }
	const_enumrator operator--(int) { const_enumrator wk(*this); --(*this); return wk; }
	////////////////////////////////////////////////////////////////////////////////
	//�^�ϊ�
    /*
	const_reference operator()() const
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
    */
	const_reference operator*() const
    {
        assert( currentPosition != sentinel );
        return *currentPosition;
    }
    const_pointer operator->() const
    {
        assert( currentPosition != sentinel );
        return &*currentPosition;
    }
	operator const_pointer() const
    {
        if ( currentPosition == sentinel ) return 0;
        return &*currentPosition;
    }
	size_t index() const
    {
        return std::distance( firstPosition, currentPosition );
    }
    const_iterator begin() { return firstPosition; }
    const_iterator end() { return sentinel; }
    const_iterator it() { return currentPosition; }
};

// ���G�ȗ̈��\������N���X
// �ȉ��̃f�[�^������L����悤�Ɉ���
// �f�[�^�͕K���㉺�Ƀ\�[�g����A���E�Ƀ\�[�g����Ă���B
// ����top�������͓̂���bottom�������̂����Ȃ��B
// �e��`�͏d�Ȃ�Ȃ��B
// �e��`�͍��E�ɗׂ荇������connecte�֐��ɂ��Ȃ���D
template<typename T>
class region
{
public:
    template<typename U> static bool between( const U & minimum, const U & v, const U & maximum )
    { return ( minimum <= v ) && ( v <= maximum ) && ( minimum <= maximum ); }

    template<typename U> static const U & clamp( const U & minimum, const U & v, const U & maximum )
    { return v < minimum ? minimum : ( maximum < v ? maximum : v ); }

    template<typename U> static const U & minimum( const U & v1, const U & v2 )
    { return v1 < v2 ? v1 : v2; }

    template<typename U> static const U & maximum( const U & v1, const U & v2 )
    { return v1 > v2 ? v1 : v2; }

    template<typename U, typename V> static U round( const V & v )
    { return U( v + 0.5f ); }

    template<typename U> static U sign( const U & v )
    { return v == U( 0 ) ? U( 0 ) : v < U( 0 ) ? U( -1 ) : U( 1 ); }

    typedef T value_type;
    class point
    {
    public:
	    value_type x, y;
	    point( value_type _x = 0, value_type _y = 0 ) : x( _x ), y( _y ) {}
	    point( const point & p ) : x( p.x ), y( p.y ) {}
	    point & operator=( const point & p ) { x = p.x; y = p.y; return *this; }
	    bool operator==( const point & p ) const { return ( x == p.x && y == p.y ); }
	    bool operator!=( const point & p ) const { return ! ( *this == p ); }
	    point operator-() const { return point( - x, - y ); }
	    point operator+( const point & p ) const { return point( x + p.x, y + p.y ); }
	    point operator-( const point & p ) const { return point( x - p.x, y - p.y ); }
	    point operator*( const point & p ) const { return point( x * p.x, y * p.y ); }
	    point operator/( const point & p ) const { return point( x / p.x, y / p.y ); }
	    point & operator+=( const point & p ) { x += p.x; y += p.y; return *this; }
	    point & operator-=( const point & p ) { x -= p.x; y -= p.y; return *this; }
	    point & operator*=( const point & p ) { x *= p.x; y *= p.y; return *this; }
	    point & operator/=( const point & p ) { x /= p.x; y /= p.y; return *this; }
	    point operator+( const value_type & p ) const { return point( x + p, y + p ); }
	    point operator-( const value_type & p ) const { return point( x - p, y - p ); }
	    point operator*( const value_type & p ) const { return point( x * p, y * p ); }
	    point operator/( const value_type & p ) const { return point( x / p, y / p ); }
	    point & operator+=( const value_type & p ) { x += p; y += p; return *this; }
	    point & operator-=( const value_type & p ) { x -= p; y -= p; return *this; }
	    point & operator*=( const value_type & p ) { x *= p; y *= p; return *this; }
	    point & operator/=( const value_type & p ) { x /= p; y /= p; return *this; }
    };
    class rectangle
    {
    public:
	    value_type left, top, right, bottom;
	    rectangle() : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) {}
	    rectangle( value_type _left, value_type _top, value_type _right, value_type _bottom ) : left( _left ), top( _top ), right( _right ), bottom( _bottom ) {}
	    rectangle & operator=( const rectangle & r ) { left = r.left; top = r.top; right = r.right; bottom = r.bottom; return *this; }
	    bool operator==( const rectangle & r ) const { return( left == r.left && right == r.right && top == r.top && bottom == r.bottom ); }
	    bool operator!=( const rectangle & r ) const { return ! ( *this == r ); }
	    void initialize() { left = top = right = bottom = 0; }
	    value_type width() const { return right - left; }
	    value_type height() const { return bottom - top; }
	    double area() const { return double( width() ) * height(); }
        bool empty() const { return ( right <= left ) || ( bottom <= top ); }
	    void first( value_type l, value_type t ){ left = l; top = t; }
	    void second( value_type w, value_type h ){ right = left + w; bottom = top + h; }
	    void single(){ right = left + 1; bottom = top + 1; }
	    void expand( value_type band ){ left -= band; right += band; top -= band; bottom += band; }
	    void offset( value_type x, value_type y ) { left += x; right += x; top += y; bottom += y; }
	    bool in( value_type x, value_type y ) const { return between<value_type>( left, x, right - 1 ) && between<value_type>( top, y, bottom - 1 ); }
	    static rectangle pixel( value_type x, value_type y ){ rectangle r; r.left = x; r.top = y; r.single(); return r; }
	    static rectangle size( value_type w, value_type h ){ return rectangle( 0, 0, w, h ); }
	    const rectangle operator|( const rectangle & src )
	    {
		    rectangle r;
		    r.left = minimum( left, src.left );
		    r.right = maximum( right, src.right );
		    r.top = minimum( top, src.top );
		    r.bottom = maximum( bottom, src.bottom );
		    return r;
	    }
	    const rectangle operator&( const rectangle & src )
	    {
		    rectangle r;
		    if ( src.right <= left || right <= src.left ||
			    src.bottom <= top || bottom <= src.top ) return r;
		    r.left = maximum( left, src.left );
		    r.right = minimum( right, src.right );
		    r.top = maximum( top, src.top );
		    r.bottom = minimum( bottom, src.bottom );
		    if ( r.right <= r.left || r.bottom <= r.top ) return rectangle();
		    return r;
	    }
    };
private:
    std::vector<rectangle> rectangles;//��`���X�g
	rectangle bbox;//�o�E���f�B���O�{�b�N�X
	double size;//�ʐ�

    typedef enumrator<std::vector<rectangle> > rectangle_enumrator;
public:
    typedef const_enumrator<std::vector<rectangle> > rectangle_const_enumrator;
private:
    //��`�p�o�b�N�C���T�[�^�[
    class back_inserter
    {
        region * target;
    public:
        back_inserter(back_inserter& rhs) : target(rhs.target){}
        back_inserter & operator=( const back_inserter & rhs ) {
            target = rhs.target;
            return *this;
        }
        explicit back_inserter(region& target_) : target(&target_){}
        back_inserter & operator=( const rectangle & value )
        {
            target->push_back(value);
            return (*this);
        }
        back_inserter & operator*() { return (*this); }
        back_inserter & operator++() { return (*this); }
        back_inserter & operator++(int) { return (*this); }
        back_inserter & operator()( const rectangle & r )
        {
            target->push_back( r );
            return (*this);
        }
    };

public:
	//�_�p��
	class point_enumrator
	{
        rectangle_const_enumrator itr, currentLineTop;
		point p;
	public:
		point_enumrator( const point_enumrator & it ) : itr( it.itr ), currentLineTop( it.currentLineTop ), p( it.p ) {}
		point_enumrator( const region & base ) : itr( base.rectangles ), currentLineTop( itr )
		{
			if ( itr ) {
				p.x = itr->left;
				p.y = itr->top;
			}
		}
		point_enumrator & operator=( const point_enumrator & it )
        {
            itr = it.itr;
            currentLineTop = it.currentLineTop;
            p = it.p;
            return *this;
        }
		operator const rectangle * () const { return ( const rectangle * ) itr; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		//const point & operator()() const { assert( itr ); return p; }
		const point & operator*() const { assert( itr ); return p; }
		const point * operator->() const { assert( itr ); return & p; }
		////////////////////////////////////////////////////////////////////////////////
		//���� ���P���C�����Ƃɉ��ɐi��ł����A�̈���̈�_���w��
		point_enumrator & operator++()
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
		point_enumrator & operator--()
		{
			if ( ! itr ) return *this;
			if ( itr->left < p.x ) {
				--p.x;
			} else {
				--itr;
				if ( ( ! itr ) || ( itr->top != currentLineTop->top ) ) {
					if ( currentLineTop->top < p.y ) {
						itr = currentLineTop;
						rectangle_const_enumrator next( itr );
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
							rectangle_const_enumrator prev( currentLineTop );
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
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		point_enumrator operator++(int) { point_enumrator wk(*this); ++(*this); return wk; }
		point_enumrator operator--(int) { point_enumrator wk(*this); --(*this); return wk; }
	};
	friend class point_enumrator;
public:
	region() : size( 0 ){}
	region( const rectangle & r ) : size( 0 )
	{
        if ( ! r.empty() ) push_back( r );
	}
	region( const point & r ) : size( 0 )
	{
		push_back( rectangle::pixel( r.x, r.y ) );
	}
	region( const region & r ) : size( 0 )
	{
		*this = r;
	}
	region & operator=( const region & rhs )
	{
        if ( this != & rhs )
        {
            //�e�ʍ팸
            if ( rectangles.capacity() > rhs.rectangles.size() )
            {
                rectangles.swap( std::vector<rectangle>() );
            }
            rectangles = rhs.rectangles;
            bbox = rhs.bbox;
            size = rhs.size;
        }
		return *this;
	}
	void swap( region & r )
    {
        rectangles.swap( r.rectangles );
        std::swap( bbox, r.bbox );
        std::swap( size, r.size );
    }
    //�f�[�^������
	void initialize()
	{
        rectangles.resize( 0 );
		size = 0;
		bbox.initialize();
	}
    //�f�[�^�J��
	void release()
	{
		initialize();
        rectangles.swap( std::vector<rectangle>() );
	}
	// �������p��(vector�Ƃ͈���āA���݂̏�Ԃ�j������)
	void reserve( int index )
	{
		initialize();
		rectangles.reserve( index );
	}
	//��`�̗񋓎q
	rectangle_const_enumrator range() const 
	{
		return rectangle_const_enumrator( rectangles );
	}
	//�o�E���f�B���O�{�b�N�X�i���`�j
	const rectangle & get() const { return bbox; }
	//�ʐ�
	double area() const { return size; }
	//�ʐς���
	bool empty() const
	{
		return size == 0;
	}
	//��`��
	size_t rectSize() const { return rectangles.size(); }
	// ���ɂ��āC�ʐςƃo�E���f�B���O�{�b�N�X���v�Z
	const rectangle & push_back( const rectangle & value )
	{
        rectangles.push_back( value );
		if ( bbox.empty() ) {
			bbox = value;
		} else {
			bbox.left = minimum( bbox.left, value.left );
			bbox.right = maximum( bbox.right, value.right );
			bbox.top = minimum( bbox.top, value.top );
			bbox.bottom = maximum( bbox.bottom, value.bottom );
		}
		size += value.area();
		return rectangles.back();
	}
	const rectangle & push_back( const value_type x, const value_type y )
	{
		return push_back( rectangle::pixel( x, y ) );
	}
	void push_back( const region & value )
	{
        for ( rectangle_const_enumrator it( value.rectangles ); it; ++it ) {
			push_back( *it );
		}
	}
	//�ŏ��ƍŌ�̋�`��Ԃ�
	const rectangle & first() const
	{
		return rectangles.front();
	}
	const rectangle & last() const
	{
		return rectangles.back();
	}
	//�_���܂܂�Ă����`��Ԃ�
	const rectangle * in( value_type x, value_type y ) const
	{
		if ( ! bbox.in( x, y ) ) return NULL;//�o�E���f�B���O�{�b�N�X�O�Ȃ炠�肦�Ȃ�
		for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			if ( it->in( x, y ) ) return &*it;
		}
		return NULL;
	}
private:
	//�_���܂܂�Ă����`��Ԃ�
    static const size_t npos = ~ static_cast<size_t>( 0 );
	size_t inIndex( value_type x, value_type y ) const
	{
		if ( bbox.in( x, y ) )
        {
            for ( size_t i = 0, n = rectangles.size(); i < n; ++i ) {
			    if ( rectangles[i].in( x, y ) ) return i;
		    }
        }
        return npos;//�O
	}
	size_t rectangleIndex( const rectangle * r ) const
    {
        return std::distance( & rectangles.front(), r );
    }
public:
	// �d�S�v�Z
	point center() const
	{
		point2<double> result( 0, 0 );
        if ( size )
        {
		    for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			    const double a = it->area();
			    result += point2<double>( ( it->left + it->right - 1 ) * a / 2.0,
									      ( it->top + it->bottom - 1 ) * a / 2.0 );
		    }
            result.x /= size;
            result.y /= size;
        }
		return point( static_cast<value_type>( result.x ), static_cast<value_type>( result.y ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// �̈�ύX
	////////////////////////////////////////////////////////////////////////////////
	//�̈�S�̂��I�t�Z�b�g����
	void offset( value_type x, value_type y )
	{
		for ( rectangle_enumrator it( rectangles ); it; ++it ) 
			it->offset( x, y );
	}
	void offset( region & result, value_type x, value_type y ) const 
	{
		result = *this;
		result.offset( x, y );
	}
	// x, y�����ւ���
	void exchange( region & result ) const
	{
		result.reserve( rectangles.size() );
        size_t width = bbox.width();
        size_t height = bbox.height();
        if ( width && height ) {
            std::vector<char> flags( width * height, 0 );
		    get( flags, width, height, bbox.left, bbox.top, true );
		    result.set( flags, height, width, bbox.top, bbox.left );
        }
	}
	// flag : width * height�̑傫���̐^�U�l
	void set( const std::vector<char> & flag, size_t width, size_t height, value_type offsetX, value_type offsetY )
	{
        if ( width * height != flag.size() ) throw std::runtime_error( "flag is not enough error" );
		reserve( flag.size() );
		std::vector<char>::const_iterator it = flag.begin();
		rectangle r;
		for ( value_type y = 0; y < static_cast<value_type>( height ); y++ ) {
			r.top = y + offsetY; r.bottom = r.top + 1;
			bool valid = false;
			for ( value_type x = 0; x < static_cast<value_type>( width ); x++ ) {
				if ( *it ) {
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
	void get( std::vector<char> & flag, size_t width, size_t height, T offsetX, T offsetY, bool exchange = false ) const
	{
        flag.assign( width * height, 0 );
	    const value_type flag_left = offsetX;
	    const value_type flag_top = offsetY;
	    const value_type flag_right = offsetX + width;
	    const value_type flag_bottom = offsetY + height;
        if ( ! exchange ) {
		    for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			    rectangle r = *it;
			    if ( r.left < flag_left ) r.left = flag_left;
			    if ( flag_right < r.right ) r.right = flag_right;
			    if ( r.top < flag_top ) r.top = flag_top;
			    if ( flag_bottom < r.bottom ) r.bottom = flag_bottom;
                r.offset( - offsetX, - offsetY );
			    for ( value_type y = r.top; y < r.bottom; y++ ) {
				    for ( value_type x = r.left; x < r.right; x++ ) {
					    flag[x+y*width] = true;
				    }
			    }
		    }
        } else {
		    for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			    rectangle r = *it;
			    if ( r.left < flag_left ) r.left = flag_left;
			    if ( flag_right < r.right ) r.right = flag_right;
			    if ( r.top < flag_top ) r.top = flag_top;
			    if ( flag_bottom < r.bottom ) r.bottom = flag_bottom;
                r.offset( - offsetX, - offsetY );
			    for ( value_type y = r.top; y < r.bottom; y++ ) {
				    for ( value_type x = r.left; x < r.right; x++ ) {
					    flag[x*height+y] = true;
				    }
			    }
		    }
        }
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	// �̈�ڑ�
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
    //�Q�s���Ȃ���
    //@param[in] line ���݂̏����s�̋�`���X�g
    //@param[in,out] prevLine ��O�̏����s�̋�`���X�g�A�Ȃ���ꍇ�ɂ͊g�������
    //@note �ʐς͐��m�łȂ��Ȃ�
    static bool connect( const std::vector<rectangle> & line, std::vector<rectangle> & prevLine )
	{
		if ( prevLine.size() != line.size() ) return false;
		if ( prevLine.front().bottom != line.front().top ) return false;
		rectangle_const_enumrator itpl( prevLine );
		rectangle_const_enumrator itl( line );
 		for ( ; itl && itpl; ++itl, ++itpl ) {
			if ( itl->left != itpl->left ||
				itl->right != itpl->right ) return false;
		}
        //prevLine�̉��[���g������.
		value_type newBottom = line.front().bottom;
		for ( rectangle_enumrator it( prevLine ); it; ++it ) {
			it->bottom = newBottom;
		}
		return true;
	}
	//�Q�s���q���邩�e�X�g���āC�q����Ȃ��Ȃ�Ό��ʂɈȑO�̍s��ǉ�
	static void connect( std::vector<rectangle> * & now, std::vector<rectangle> * & prev, region & result )
	{
		if ( prev->empty() ) {//�O�̍s�������ꍇ�ɂ́C��񂵂ɂ��Ď���Ă���
            std::swap( prev, now );//���O�̍s��ݒ�
		} else if ( ! connect( *now, *prev ) ) {//�q���邩���ׂāC�q����悤�Ȃ�΁C�ȑO�̃��C�������������Ă���
			//�Ȃ���Ȃ�����
            std::for_each( prev->begin(), prev->end(), back_inserter( result ) );//���ʂɈȑO�̋�`��ǉ���
			std::swap( prev, now );//���O�̍s��ݒ�
		}
		now->clear();
	}
public:
	// �ڑ� ���ʂɌq�����Ă��Ȃ����������ď����𖞂����Ă����Ԃ𐮓ڂ���
	void connect()
	{
		region result;
		result.reserve( rectangles.size() );

		std::vector<rectangle> lines[2];
		std::vector<rectangle> * prev = & lines[0], * now = & lines[1];
		prev->reserve( rectangles.size() );
		now->reserve( rectangles.size() );
		for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			if ( now->empty() ) {//�ŏ��̋�`�͖������ǉ�
				now->push_back( *it );
			} else {
                rectangle & prevrect = now->back();//�Ō�̋�`�����o��
				if ( prevrect.top == it->top ) {//���݂Əオ��v���Ă��邩
					if ( prevrect.right == it->left ) {//���łȂ���
						prevrect.right = it->right;//@note �ʐς̒l�͂��������Ȃ�.
					} else {//����Ă��āC���������Ȃ̂Œǉ�
						now->push_back( *it );
					}
				} else {//���̍s�̋�`�����݂̃^�[�Q�b�g�ɂȂ��Ă���
					connect( now, prev, result );
					now->push_back( *it );
				}
			}
		}
		if ( ! now->empty() ) {
			connect( now, prev, result );
            std::for_each( prev->begin(), prev->end(), back_inserter( result ) );
		}
		*this = result;//���ʂȗe�ʂ���x�m�ۂ��Ă���̂ō폜���邽�߂ɑ��
	}
	bool isConnected() const 
	{
		rectangle_const_enumrator it( rectangles );
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
    void enumDividePointVertical( std::deque<value_type> & result ) const
	{
        result.clear();
		if ( rectangles.empty() ) return;
		rectangle_const_enumrator it( rectangles );
		value_type lastPoint = it->top - 1;
		value_type lastTop = lastPoint;
		for ( ; it; ++it ) {
			if ( lastTop == it->top ) continue;
			if ( lastPoint < it->top ) result.push_back( lastPoint = it->top );
			if ( lastPoint < it->bottom ) result.push_back( lastPoint = it->bottom );
			lastTop = it->top;
		}
	}
	// �����_���̗�
	static void enumDividePointHorizontal( std::deque<T> & result, rectangle_const_enumrator & it )
	{
        result.clear();
		if ( ! it ) return;
		rectangle_const_enumrator start( it );
		T lastPoint = start->left - 1;
		for ( ; it && start->top == it->top; ++it ) {
			if ( lastPoint < it->left ) result.push_back( lastPoint = it->left );
			if ( lastPoint < it->right ) result.push_back( lastPoint = it->right );
		}
	}
	// �������̍���
	static void orDividePoint( std::deque<value_type> & result, std::deque<value_type> & src1, std::deque<value_type> & src2 )
	{
        std::set_union( src1.begin(), src1.end(), src2.begin(), src2.end(), std::back_inserter( result ) );
	}
	// �������ɂ��̈�̕���
    void divideVertical( std::vector<rectangle> & result, const std::deque<T> & dividePoint ) const
	{
        result.clear();
		//result.reserve( rectangles.size() * dividePoint.size() );

		rectangle_const_enumrator it( rectangles );
		const_enumrator<std::deque<value_type> > itDP( dividePoint );

		region line;
		while ( it ) {
			const rectangle & r = *it;
			while ( itDP && *itDP <= r.top ) ++itDP;
			// �w���ʒu���瓯�������̗̈�݂̂����
			line.reserve( rectangles.size() );
			for ( ; it && it->top == r.top; ++it ) {
				line.push_back( *it );
			}
			// divide
			T top = r.top, bottom = itDP ? *itDP : r.bottom;
			while ( bottom <= r.bottom ) {
				for ( rectangle_const_enumrator itLine( line.rectangles ); itLine; ++itLine ) {
                    rectangle l = *itLine;
                    l.top = top;
                    l.bottom = bottom;
					result.push_back( l );
				}
				++itDP;
				if ( ! itDP ) break;
				top = bottom;
				bottom = *itDP;
			}
		}
	}
	// �������ɂ��̈�̕���
	static void divideHorizontal( std::vector<rectangle> & result, rectangle_const_enumrator & it, const std::deque<T> & dividePoint )
	{
		T top = it->top;
        result.clear();
        //result.reserve( std::distance( it.begin(), it.end() ) * dividePoint.size() );
		const_enumrator<std::deque<value_type> >itDP( dividePoint );
		for ( ; it && it->top == top; ++it ) {
			// forward
			while ( itDP && *itDP <= it->left ) ++itDP;
			// divide
			// wk : ���ɓ�����`
			rectangle wk = *it;
			for ( ; itDP && *itDP <= wk.right; ++itDP ) {
                rectangle r = wk;//��������`
				//���̕����|�C���g�ō�������`��؂�A���ɓ�����`�̍���ݒ�
                r.right = wk.left = *itDP;
				result.push_back( r );
			}
		}
	}
	static void divideVertical( std::vector<rectangle> & result1, std::vector<rectangle> & result2, const region & src1, const region & src2 )
	{
		std::deque<T> src1dp, src2dp, dp;
		src1.enumDividePointVertical( src1dp );
		src2.enumDividePointVertical( src2dp );
		orDividePoint( dp, src1dp, src2dp );
		src1.divideVertical( result1, dp );
		src2.divideVertical( result2, dp );
	}
	static void divideHorizontal( std::vector<rectangle> & result1, std::vector<rectangle> & result2, rectangle_const_enumrator & it1, rectangle_const_enumrator & it2 )
	{
		value_type top = it1->top;
		std::deque<T> src1dp, src2dp, dp;
		rectangle_const_enumrator bit1( it1 ), bit2( it2 );
		enumDividePointHorizontal( src1dp, it1 );
		enumDividePointHorizontal( src2dp, it2 );
		orDividePoint( dp, src1dp, src2dp );
		divideHorizontal( result1, bit1, dp );
		divideHorizontal( result2, bit2, dp );
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
        size_t total_size = static_cast<size_t>( src1.area() + src2.area() );
		result.reserve( total_size );
		std::vector<rectangle> v1, v2, h1, h2;
        v1.reserve( total_size );
        v2.reserve( total_size );
        h1.reserve( total_size );
        h2.reserve( total_size );
		//�c�����ɕ�������
		divideVertical( v1, v2, src1, src2 );
		rectangle_const_enumrator itv1( v1 ), itv2( v2 );
		rectangle r;
		while ( itv1 && itv2 ) {//�����ɏ����Ώۂ�����ꍇ�ɂ�
			if ( itv1->top < itv2->top ) {//�Е���TOP��菬���������͑S�ď�������
				const value_type till = itv2->top;
				if ( opTF ) {
					for ( ; itv1 && itv1->top < till; ++itv1 ) {
						result.push_back( *itv1 );
					}
				} else {
					for ( ; itv1 && itv1->top < till; ++itv1 );
				}
			} else if ( itv2->top < itv1->top ) {//�Е���TOP��菬���������͑S�ď�������
				const value_type till = itv1->top;
				if ( opFT ) {
					for ( ; itv2 && itv2->top < till; ++itv2 ) {
						result.push_back( *itv2 );
					}
				} else {
					for ( ; itv2 && itv2->top < till; ++itv2 );
				}
			} else {//�����������ꍇ
				r.top = itv1->top; r.bottom = itv1->bottom;//�㉺�̈ʒu��ݒ�.
				divideHorizontal( h1, h2, itv1, itv2 );//���������̋�`�����𐅕������ɕ���.
				rectangle_const_enumrator ith1( h1 ), ith2( h2 );
				while ( ith1 && ith2 ) {
					if ( ith1->left < ith2->left ) {//�d�Ȃ荇���Ă��Ȃ��ꍇ
						if ( opTF ) {
							for ( ; ith1 && ith1->left < ith2->left; ++ith1 ) {
								result.push_back( *ith1 );
							}
						} else {
							for ( ; ith1 && ith1->left < ith2->left; ++ith1 );
						}
					} else if ( ith2->left < ith1->left ) {//�d�Ȃ荇���Ă��Ȃ��ꍇ
						if ( opFT ) {
							for ( ; ith2 && ith2->left < ith1->left; ++ith2 ) {
								result.push_back( *ith2 );
							}
						} else {
							for ( ; ith2 && ith2->left < ith1->left; ++ith2 );
						}
					} else {//�d�Ȃ��Ă���ꍇ
						if ( opTT ) {
							for ( ; ith1 && ith2 && ith1->left == ith2->left; ++ith1, ++ith2 ) {
								result.push_back( *ith1 );
							}
						} else {
							for ( ; ith1 && ith2 && ith1->left == ith2->left; ++ith1, ++ith2 );
						}
					}
				}
                if ( opTF ) std::for_each( ith1.it(), ith1.end(), back_inserter( result ) );
                if ( opFT ) std::for_each( ith2.it(), ith2.end(), back_inserter( result ) );
			}
		}
		//�c�蕨�̕Е������ׂď�������
        if ( opTF ) std::for_each( itv1.it(), itv1.end(), back_inserter( result ) );
        if ( opFT ) std::for_each( itv2.it(), itv2.end(), back_inserter( result ) );
        //�A��.
		result.connect();
        //�߂�Ƃ��ɁA�T�C�Y���k�������.
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
		if ( rectangles.size() != src.rectangles.size() ) return false;//��`�����r
		for ( rectangle_const_enumrator itd( rectangles ), its( src.rectangles ); itd; ++itd, ++its ) {
			if ( itd() != its() ) return false;
		}
		return true;
	}
	bool operator!=( const region & src ) const
	{
		return ! ( *this == src );
	}
	////////////////////////////////////////////////////////////////////////////////
	// �����Z�b�g
	void line( T x1, T y1, T x2, T y2 )
	{
		const T ax = sign( x2 - x1 ), ay = sign( y2 - y1 );//�i�s�����̕����̌v�Z
		const T dx = abs( x2 - x1 ), dy = abs( y2 - y1 );//�����̌v�Z
		const size_t w = dx + 1, h = dy + 1;//�����̌v�Z
		if ( dx + dy == 0 ) {
			*this = region( rectangle::pixel( x1, y1 ) );
			return;
		}
        std::vector<char> flag( w * h, 0 );
		const T ex = dx << 1, ey = dy << 1;//�^�̒l�ɋ߂��悤�Ɍ덷��0.5�̏㉺�Ŕ��肷�邽�߂Ɍ덷�̌v�Z�p�̐��l���v�Z����
		T e = 0, x = x1, y = y1;//�덷�Ə����ʒu��ݒ�
		T bx = minimum( x1, x2 ), by = minimum( y1, y2 );
		if ( dx >= dy ) {
			for ( T t = 0; t <= dx; ++t ) {
				flag[(x - bx)+w*(y - by)] = true;
				x += ax;
				e += ey;
				if ( e >= dx ) {
					y += ay;
					e -= ex;
				}
			}
		} else {
			for ( T t = 0; t <= dy; ++t ) {
				flag[(x - bx)+w*(y - by)] = true;
				y += ay;
				e += ex;
				if ( e >= dy ) {
					x += ax;
					e -= ey;
				}
			}
		}
		set( flag, w, h, bx, by );
	}
	// �~���Z�b�g
	void circle( T x, T y, T r )
	{
		r = abs( r );
		size_t size = 2 * r + 1;
		reserve( size * size );
		if ( r == 0 ) {
			*this = region( rectangle::pixel( x, y ) );
			return;
		}
        std::vector<char> flag( size * size, 0 );
		T s = 0, t = r;
		T e = 3 - 2 * r;
		while ( s <= t ) {
			flag[ ( r + s ) + size * ( r + t ) ] =
			flag[ ( r - s ) + size * ( r + t ) ] =
			flag[ ( r + s ) + size * ( r - t ) ] =
			flag[ ( r - s ) + size * ( r - t ) ] =
			flag[ ( r + t ) + size * ( r + s ) ] =
			flag[ ( r - t ) + size * ( r + s ) ] =
			flag[ ( r + t ) + size * ( r - s ) ] =
			flag[ ( r - t ) + size * ( r - s ) ] = true;
			if ( e < 0 ) {
				e += 4 * s + 6;
			} else {
				--t;
				e += 4 * ( s - t ) + 6;
			}
			++s;
		}
		set( flag, size, size, x, y );
	}
	// ���̋󂫗̈�𖄂߂�
	void fill( region & result ) const
	{
		result.reserve( rectangles.size() );
		if ( ! size ) return;
		rectangle bb = bbox;
		bb.expand( 1 );
		region outerBB( bb );
		region other = outerBB - *this;
		region outer; other.select( outer, bb.left, bb.top );
		result = outerBB - outer;
	}
	// �����̈�ɕ�����
	void divideMultiRegion( std::list<region> & result ) const
	{
		result.clear();
        const size_t n = rectangles.size();
		if ( ! n ) return;
		std::vector<char> part;//�����ς݂Ə������̋�`�̃t���O
        std::list<size_t> stack;//�q�����Ă��鏈�����̋�`�̃C���f�b�N�X���X�g
		std::vector<char> flag( n, 0 );//�g�p�ς݂̋�`�̃t���O
		region rgn;
		rgn.reserve( n );
        for ( size_t i = 0; i < n; ++i ) {
			if ( flag[i] ) continue;//�����ς̏ꍇ
			part = flag;//���݂̏�Ԃ��L��
			part[i] = true;//�������̃t���O�𗧂Ă�
            // part && ! flag�̏�Ԃ��Ȃ����Ă���̈�
			stack.push_back( i );//�ŏ��̋�`���X�^�b�N�ɓ����
			while ( ! stack.empty() ) {//�X�^�b�N�ɋ�`���c���Ă���
                int index = stack.front();//�Ή�����C���f�b�N�X�𓾂�
                stack.pop_front();
				const rectangle & r = rectangles[index];//��`�𓾂�
				//���E�Ɍq�����Ă��鎖�͂Ȃ��Ɖ��肵�C�㉺�����֌q�����Ă��镨��T���D
				//���̋�`����E�������ɂȂ����T��
                for ( size_t j = index + 1; j < n; ++j ) {
                    const rectangle & s = rectangles[j];
                    char & sp = part[j];
                    if ( s.top == r.bottom &&
					    ! sp &&//�����ςł͂Ȃ�
					    s.left < r.right && r.left < s.right )//�ڂ��Ă����
                    {
					    sp = true;//�������̃t���O�𗧂�
					    stack.push_back( j );//�X�^�b�N�ɒǉ�����
                    }
                    if ( r.bottom < s.top ) break;//���ʂɉ������͒T���Ȃ�
				}
				//��������ɂ��T��
                for ( ptrdiff_t j = index - 1; 0 <= j; --j ) {
                    const rectangle & s = rectangles[j];
                    char & sp = part[j];
					if ( s.bottom == r.top &&
					    ! sp &&//�����ςł͂Ȃ�
					    s.left < r.right && r.left < s.right )
                    {
					    sp = true;
					    stack.push_back( j );
                    }
                    if ( r.top > s.bottom ) break;
				}
			}
			//�V�����̈�����X�g�ɒǉ�
			rgn.initialize();
			//�q�����Ă����`�݂̂��݂��C�ǉ�����
            for ( size_t j = i; j < n; ++j ) {
                char & sf = flag[j];
				if ( ! sf && part[j] ) {
					rgn.push_back( rectangles[j] );
					sf = true;
				}
			}
			rgn.connect();
			result.push_back( rgn );
		}
	}
	// �w��̗̈悾����I������
	void select( region & result, T x, T y ) const
	{
		size_t index = inIndex( x, y );
		if ( index == npos ) {
			result.initialize();
			return;
		}
        std::list<size_t> stack;
		stack.push_back( index );
        const size_t n = rectangles.size();
		std::vector<char> flag( n, 0 );
		flag[index] = true;
        size_t count = 1;
		while ( ! stack.empty() ) {
			size_t index = stack.front();
			stack.pop_front();
            const rectangle & r = rectangles[index];
			//���E�ׂ荇���Ă�����̂͂Ȃ��̂ŁA�㉺�����`�F�b�N����B
            for ( size_t i = index + 1; i < n; ++i )
            {
                const rectangle & s = rectangles[i];
                char & sf = flag[i];
			    if ( ! sf &&
				    ( s.bottom == r.top || s.top == r.bottom ) &&
				    s.left < r.right && r.left < s.right ) {
				    sf = true;
				    stack.push_back( i );
                    ++count;
			    }
			    if ( r.bottom < s.top ) break;
            }
            for ( ptrdiff_t i = index - 1; 0 <= i; --i )
            {
                const rectangle & s = rectangles[i];
                char & sf = flag[i];
			    if ( ! sf &&
				    ( s.bottom == r.top || s.top == r.bottom ) &&
				    s.left < r.right && r.left < s.right ) {
				    sf = true;
				    stack.push_back( i );
                    ++count;
			    }
			    if ( r.top > s.bottom ) break;
            }
		}
		result.reserve( count );
        for ( size_t i = 0; i < n; ++i ) {
			if ( flag[i] ) result.push_back( rectangles[i] );
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
		T offsetX = bbox.left - count, offsetY = bbox.top - count;
		size_t width = bbox.width() + count * 2, height = bbox.height() + count * 2;
        std::vector<char> flag( width * height, 0 );
		std::vector<T> distances( count + 1 );
		for ( int i = 0; i <= count; ++i ) distances[i] = function( i, count );
		for ( rectangle_const_enumrator it( rectangles ); it; ++it ) {
			const rectangle & r = *it;
			const T w = r.width();
			const T & d = distances[0];
			//��`�����ɍL���ăt���O���Z�b�g
			for ( int i = r.top; i < r.bottom; ++i ) {
				for ( int x = -d; x < w + d; ++x ) {
					flag[( r.left - offsetX + x) + width*(i - offsetY )] = true;
				}
			}
			//��`�̏㉺�ɑ������������C�p�͂ǂꂾ���������̕����v�Z���C���̕��𑝂₷
			for ( int i = 1; i <= count; ++i ) {
				const T & d = distances[i];
				for ( int x = -d; x < w + d; ++x ) {
					flag[( r.left - offsetX + x) + width*(r.top - i - offsetY )] = true;
					flag[( r.left - offsetX + x) + width*(r.bottom - 1 + i - offsetY )] = true;
				}
			}
		}
		result.set( flag, width, height, offsetX, offsetY );
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
