////////////////////////////////////////////////////////////////////////////////
// “Š‰e

#ifndef __COMPOSE__

class sgOrtho : public sgNode
{
public:
	sgNodeDeclare( sgOrtho );
	sgDecimal nearZ;
	sgDecimal farZ;
	sgDecimal aspect;//width/height
	sgDecimal sizeY;
	sgMatrix44 matrix;
};

class sgPerspective : public sgNode
{
public:
	sgNodeDeclare( sgPerspective );
	sgDecimal nearZ;
	sgDecimal farZ;
	sgDecimal aspect;//width/height
	sgDecimal fieldOfViewY;//[deg]
	sgMatrix44 matrix;
};

class sgFrustum : public sgNode
{
public:
	sgNodeDeclare( sgFrustum );
	sgVector3 minimum;
	sgVector3 maximum;
	sgMatrix44 matrix;
};

#else

sgNodeDeclareInstance( sgOrtho )
{
	sgDeclareThis();
	sgDeclareInitialRetain( nearZ, 0.0009765625 );
	sgDeclareInitialRetain( farZ, 1024 );
	sgDeclareInitialRetain( aspect, decimal( 640.0 / 480.0 ) );
	sgDeclareInitialRetain( sizeY, 480 );
	sgDeclareInitial( matrix, matrix44::identity );
}

sgNodeDeclareInstance( sgPerspective )
{
	sgDeclareThis();
	sgDeclareInitialRetain( nearZ, 0.0009765625 );
	sgDeclareInitialRetain( farZ, 1024 );
	sgDeclareInitialRetain( aspect, decimal( 640.0 / 480.0 ) );
	sgDeclareInitialRetain( fieldOfViewY, 30 );
	sgDeclareInitial( matrix, matrix44::identity );
}

sgNodeDeclareInstance( sgFrustum )
{
	sgDeclareThis();
	sgDeclareInitialRetain( minimum, vector3( -0.0009765625, -0.0009765625, 0.0009765625 ) );
	sgDeclareInitialRetain( maximum, vector3( 0.0009765625, 0.0009765625, 1024 ) );
	sgDeclareInitial( matrix, matrix44::identity );
}

#endif
