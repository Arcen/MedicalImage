////////////////////////////////////////////////////////////////////////////////
// カメラ

#ifndef __COMPOSE__

class sgCamera : public sgNode
{
public:
	sgNodeDeclare( sgCamera );
	sgVector3 position;
	sgVector3 interest;
	sgVector3 upper;
	sgMatrix44 matrix;
	sgNodeMember<sgOrtho> ortho;
	sgNodeMember<sgPerspective> perspective;
	sgNodeMember<sgFrustum> frustum;
	const vector3 direction() const
	{
		return vector3::normalize3( interest() - position() );
	}
	const vector3 right() const
	{
		return vector3::normalize3( vector3::cross( direction(), vector3::normalize3( upper() ) ) );
	}
	virtual void update()
	{
		upper( vector3::normalize3( vector3::cross( right(), direction() ) ) );
		const bool rightHand = true;//右手系のカメラ座標行列を作成
		const vector3 zaxis = rightHand ? - direction() : direction();
		const vector3 xaxis = vector3::normalize3( vector3::cross( upper(), zaxis ) );
		const vector3 yaxis = vector3::cross( zaxis, xaxis );
		const vector3 trans = vector3( - vector3::dot3( xaxis, position() ), - vector3::dot3( yaxis, position() ), - vector3::dot3( zaxis, position() ) );
		matrix( matrix44(	xaxis.x, yaxis.x, zaxis.x, 0,
							xaxis.y, yaxis.y, zaxis.y, 0,
							xaxis.z, yaxis.z, zaxis.z, 0,
							trans.x, trans.y, trans.z, 1 ) );
	}
	//左右回転
	void pan( decimal x )
	{
		if ( ! x ) return;
		vector3 look = ( interest() - position() );
		look = look * matrix33::rotation( quaternion( upper(), x * pi2 ) );
		interest( position() + look );
	}
	//上下回転
	void tilt( decimal y )
	{
		if ( ! y ) return;
		vector3 look = ( interest() - position() );
		look = look * matrix33::rotation( quaternion( right(), y * pi2 ) );
		interest( position() + look );
		update();
	}
	//回転
	void pan( decimal x, decimal y )
	{
		if ( ! x && ! y ) return;
		vector3 look = ( interest() - position() );
		vector3 axis = right() * matrix33::rotation( quaternion( direction(), atan2( x, y ) ) );
		look = look * matrix33::rotation( quaternion( axis, vector2::length2( vector2( x, y ) ) * pi2 ) );
		interest( position() + look );
		update();
	}
	//注視点を中心とした回転
	void follow( decimal x, decimal y, decimal z )
	{
		if ( ! x && ! y && ! z ) return;
		vector3 look = ( interest() - position() );
		vector3 axis = right() * matrix33::rotation( quaternion( direction(), atan2( x, y ) ) );
		look = look * matrix33::rotation( quaternion( axis, vector2::length2( vector2( x, y ) ) * pi2 ) );
		position( interest() - look );
		update();
		upper( upper() * matrix33::rotation( quaternion( direction(), z * pi2 ) ) );
	}
	//ズーム
	void zoom( decimal z )
	{
		if ( ! z ) return;
		vector3 look = ( interest() - position() );
		decimal length = vector3::length3( look );
		length *= pow( 2, - z );
		position( interest() - direction() * length );
	}
	//左右移動
	void track( decimal x )
	{
		if ( ! x ) return;
		vector3 look = ( interest() - position() );
		decimal length = vector3::length3( look );
		length *= pow( 2, - x / 2 ) - 1;
		vector3 r = right();
		position( position() + r * length );
		interest( interest() + r * length );
	}
	//上下移動
	void crane( decimal y )
	{
		if ( ! y ) return;
		vector3 look = ( interest() - position() );
		decimal length = vector3::length3( look );
		length *= pow( 2, y / 2 ) - 1;
		position( position() + upper() * length );
		interest( interest() + upper() * length );
	}
	//前後移動
	void dolly( decimal z )
	{
		if ( ! z ) return;
		vector3 look = ( interest() - position() );
		decimal length = vector3::length3( look );
		length *= pow( 2, z / 2 ) - 1;
		vector3 d = direction();
		position( position() + d * length );
		interest( interest() + d * length );
	}
	//移動
	void move( decimal x, decimal y, decimal z )
	{
		track( x );
		crane( y );
		dolly( z );
	}
};

#else

sgNodeDeclareInstance( sgCamera )
{
	sgDeclareThis();
	sgDeclareInitialRetain( position, vector3( 0, 0, -100 ) );
	sgDeclareInitialRetain( interest, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( upper, vector3( 0, 1, 0 ) );
	sgDeclareInitial( matrix, matrix44::identity );
	sgDeclareNode( ortho, sgOrtho );
	sgDeclareNode( perspective, sgPerspective );
	sgDeclareNode( frustum, sgFrustum );
}

#endif
