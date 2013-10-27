////////////////////////////////////////////////////////////////////////////////
// Container

#define OperatorOpEqualElement(Type,Element,OpEqual) \
		Type & operator OpEqual( const Element & index ) { \
			for ( iterator it( *this ); it; ++it ) *it OpEqual index; \
			return *this; \
		}
#define OperatorOpEqualType(Type,Element,OpEqual) \
		Type & operator OpEqual( const Type & index ) { \
			if ( this == &index ) return *this OpEqual Type( *this ); \
			if ( size != index.size ) return *this; \
			for ( iterator itdst( *this ), itsrc( index ); itdst; ++itdst, ++itsrc ) *itdst OpEqual *itsrc; \
			return *this; \
		}

#define OperatorOpElement(Type,Element,Op,OpEqual) \
		Type operator Op( const Element & index ) const { return Type( *this ) OpEqual index; }

#define OperatorOpType(Type,Element,Op,OpEqual) \
		Type operator Op( const Type & index ) const { return Type( *this ) OpEqual index; }

#define OperatorMinus(Type,Element) \
		Type operator-() const { \
			Type wk( *this ); \
			for ( iterator it( wk ); it; ++it ) *it = - *it; \
			return wk; \
		}

#define OperatorEqualElement(Type,Element) \
		OperatorOpEqualElement(Type,Element,=)

#define OperatorCompElement(Type,Element) \
		bool operator==( const Element & index ) const { \
			for ( iterator it( *this ); it; ++it ) if ( *it != index ) return false; \
			return true; \
		} \
		bool operator!=( const Element & index ) const { return ! ( *this == index ); }

#define OperatorCompType(Type,Element) \
		bool operator==( const Type & index ) const { \
			if ( size != index.size ) return false; \
			for ( iterator itdst( *this ), itsrc( index ); itdst; ++itdst, ++itsrc ) if ( *itdst != *itsrc ) return false; \
			return true; \
		} \
		bool operator!=( const Type & index ) const { return ! ( *this == index ); }

#define OperatorOpEqual(Type,Element,OpEqual) \
		OperatorOpEqualElement(Type,Element,OpEqual) \
		OperatorOpEqualType(Type,Element,OpEqual)

#define OperatorOp(Type,Element,Op,OpEqual) \
		OperatorOpElement(Type,Element,Op,OpEqual) \
		OperatorOpType(Type,Element,Op,OpEqual)

#define OperatorEqual(Type,Element) \
		OperatorEqualElement(Type,Element)

#define OperatorComp(Type,Element) \
		OperatorCompElement(Type,Element) \
		OperatorCompType(Type,Element)

#define OperatorOpAndOpEqual(Type,Element,Op,OpEqual) \
		OperatorOpEqual(Type,Element,OpEqual) \
		OperatorOp(Type,Element,Op,OpEqual)

#define OperatorSet(Type,Element) \
		OperatorEqual(Type,Element) \
		OperatorOpAndOpEqual(Type,Element,+,+=) \
		OperatorOpAndOpEqual(Type,Element,*,*=) \
		OperatorOpAndOpEqual(Type,Element,-,-=) \
		OperatorOpAndOpEqual(Type,Element,/,/=) \
		OperatorOpAndOpEqual(Type,Element,%,%=) \
		OperatorOpAndOpEqual(Type,Element,&,&=) \
		OperatorOpAndOpEqual(Type,Element,|,|=) \
		OperatorOpAndOpEqual(Type,Element,^,^=) \
		OperatorOpAndOpEqual(Type,Element,<<,<<=) \
		OperatorOpAndOpEqual(Type,Element,>>,>>=) \
		OperatorComp(Type,Element) \
		OperatorMinus(Type,Element)

#include "array.h"
#include "list.h"
#include "tree.h"
#include "btree.h"

#undef OperatorOpEqualElement
#undef OperatorOpEqualType
#undef OperatorOpElement
#undef OperatorOpType
#undef OperatorMinus
#undef OperatorEqualElement
#undef OperatorCompElement
#undef OperatorCompType
#undef OperatorReference
#undef OperatorOpEqual
#undef OperatorOp
#undef OperatorEqual
#undef OperatorComp
#undef OperatorOpAndOpEqual
#undef OperatorSet

