////////////////////////////////////////////////////////////////////////////////
// ŽžŠÔ

#ifndef __COMPOSE__

class sgTime : public sgNode
{
public:
	sgNodeDeclare( sgTime );
	sgDecimal current;
	sgDecimal first;
	sgDecimal final;
	sgDecimal fps;
	void elapse( const decimal & time )
	{
		if ( first() < final() ) {
			current( clamp<decimal>( first(), current() + time, final() ) );
		}
	}
};

#else

sgNodeDeclareInstance( sgTime )
{
	sgDeclareThis();
	sgDeclareInitial( current, 0 );
	sgDeclareInitialRetain( first, 0 );
	sgDeclareInitialRetain( final, 0 );
	sgDeclareInitialRetain( fps, 30 );
}

#endif
