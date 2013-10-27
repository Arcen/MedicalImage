#include "fc.h"

using namespace fc;

list< retainer<base> > base::nodes;
HINSTANCE base::instance = NULL;

base::base( HWND _parent, int _id, HWND _hwnd )
: externalData( NULL ), parent( _parent ), id( _id ), hwnd( _hwnd ), 
	style( 0 ), changed( NULL )
{
	if ( ! hwnd ) hwnd = GetDlgItem( parent, id );
	push( this );
}

base::~base()
{
	pop( this );
}

