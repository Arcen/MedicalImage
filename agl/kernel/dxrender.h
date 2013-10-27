#ifdef __GLOBAL__

//DX8
};
//#include <d3d8.h>
//#include <d3dx8tex.h>
//#include <d3dx8math.h>
namespace agl {
#pragma comment( lib, "D3d8.lib" )
#pragma comment( lib, "D3dx8.lib" )

const bool useBackBuffer = false;
const bool useBufferMask = false;
const bool useVertexShader = false;
const bool usePixelShader = false;
const bool forceFlexibleVertex = true;

const int majorVersion = 1;
const int minorVersion = 1;

class DX8VertexBufferVERTEX : public renderInterface::rVertexBuffer
{
public:
	struct VERTEX
	{
		float		x, y, z;
		float		nx, ny, nz;
		float		u1, v1;
	};

	LPDIRECT3DVERTEXBUFFER8 buffer;
	DX8VertexBufferVERTEX( int _size ) : 
		renderInterface::rVertexBuffer( _size, renderInterface::rVertexBuffer::enableVNT0 ), 
		buffer( NULL )
	{
	}
	virtual ~DX8VertexBufferVERTEX()
	{
		if ( buffer ) buffer->Release();
	}
	void create( LPDIRECT3DDEVICE8 device )
	{
		device->CreateVertexBuffer( data.size * sizeof( VERTEX ), D3DUSAGE_WRITEONLY , D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, D3DPOOL_MANAGED, & buffer );
	}
	virtual void update()
	{
		if ( ! buffer ) return;
		BYTE * rawData;
		buffer->Lock( 0, data.size * sizeof( VERTEX ), & rawData, D3DLOCK_NOSYSLOCK );
		const renderInterface::rVertex * src = data;
		for ( VERTEX * dst = ( VERTEX * ) rawData, * sentinel = dst + data.size; dst != sentinel; ++dst, ++src ) {
			dst->x = src->position[0];
			dst->y = src->position[1];
			dst->z = src->position[2];
			dst->nx = src->normal[0];
			dst->ny = src->normal[1];
			dst->nz = src->normal[2];
			dst->u1 = src->texture[0][0];
			dst->v1 = src->texture[0][1];
		}
		buffer->Unlock();
	}
};
class DX8VertexBufferLVERTEX : public renderInterface::rVertexBuffer
{
public:
	struct VERTEX
	{
		float		x, y, z;
		DWORD		diffuse;
		DWORD		specular;
		float		u1, v1;
	};

	LPDIRECT3DVERTEXBUFFER8 buffer;
	DX8VertexBufferLVERTEX( int _size ) : 
		renderInterface::rVertexBuffer( _size, renderInterface::rVertexBuffer::enableVDST0 ), 
		buffer( NULL )
	{
	}
	virtual ~DX8VertexBufferLVERTEX()
	{
		if ( buffer ) buffer->Release();
	}
	void create( LPDIRECT3DDEVICE8 device )
	{
		device->CreateVertexBuffer( data.size * sizeof( VERTEX ), D3DUSAGE_WRITEONLY , D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1, D3DPOOL_MANAGED, & buffer );
	}
	virtual void update()
	{
		if ( ! buffer ) return;
		BYTE * rawData;
		buffer->Lock( 0, data.size * sizeof( VERTEX ), & rawData, D3DLOCK_NOSYSLOCK );
		const renderInterface::rVertex * src = data;
		renderInterface::charBgra color;
		for ( VERTEX * dst = ( VERTEX * ) rawData, * sentinel = dst + data.size; dst != sentinel; ++dst, ++src ) {
			dst->x = src->position[0];
			dst->y = src->position[1];
			dst->z = src->position[2];
			color.element.red = unsigned char( src->diffuse[0] * 255 );
			color.element.green = unsigned char( src->diffuse[1] * 255 );
			color.element.blue = unsigned char( src->diffuse[2] * 255 );
			color.element.alpha = unsigned char( src->diffuse[3] * 255 );
			dst->diffuse = color.all;
			color.element.red = unsigned char( src->specular[0] * 255 );
			color.element.green = unsigned char( src->specular[1] * 255 );
			color.element.blue = unsigned char( src->specular[2] * 255 );
			color.element.alpha = unsigned char( src->specular[3] * 255 );
			dst->specular = color.all;
			dst->u1 = src->texture[0][0];
			dst->v1 = src->texture[0][1];
		}
		buffer->Unlock();
	}
};
class DX8VertexBufferFlexible : public renderInterface::rVertexBuffer
{
	enum {
		offsetXYZ,
		offsetNormal,
		offsetDiffuse,
		offsetSpecular,
		offsetT0,
		offsetT1,
		offsetT2,
		offsetT3,
		offsetT4,
		offsetT5,
		offsetT6,
		offsetT7,
		sizeofOffset
	};
	int offset[sizeofOffset];
	int sizeofTexture[8];
public:
	int sizeofVertex;
	int fvf;
	LPDIRECT3DVERTEXBUFFER8 buffer;
	DX8VertexBufferFlexible( int _size, int flag, int _fvf ) : 
		renderInterface::rVertexBuffer( _size, flag ), 
		buffer( NULL ), fvf( _fvf ), sizeofVertex( 0 )
	{
		for ( int i = 0; i < sizeofOffset; i++ ) offset[i] = -1;
		if ( fvf & D3DFVF_POSITION_MASK ) {
			switch ( fvf & D3DFVF_POSITION_MASK ) {
			case D3DFVF_XYZ: offset[offsetXYZ] = sizeofVertex; sizeofVertex += 3 * sizeof( float ); break;
			}
		}
		if ( fvf & D3DFVF_NORMAL ) { offset[offsetNormal] = sizeofVertex; sizeofVertex += 3 * sizeof( float ); }
		if ( fvf & D3DFVF_DIFFUSE ) { offset[offsetDiffuse] = sizeofVertex; sizeofVertex += 1 * sizeof( DWORD ); }
		if ( fvf & D3DFVF_SPECULAR ) { offset[offsetSpecular] = sizeofVertex; sizeofVertex += 1 * sizeof( DWORD ); }
		if ( fvf & D3DFVF_TEXCOUNT_MASK ) {
			int numTex = ( ( fvf & D3DFVF_TEXCOUNT_MASK ) >> D3DFVF_TEXCOUNT_SHIFT );
			for ( int i = 0; i < numTex; i++ ) {
				switch ( ( fvf >> ( i * 2 + 16 ) ) & 0x3 ) {
				case D3DFVF_TEXTUREFORMAT1: sizeofTexture[i] = 1; offset[offsetT0+i] = sizeofVertex; sizeofVertex += 1 * sizeof( float ); break;
				case D3DFVF_TEXTUREFORMAT2: sizeofTexture[i] = 2; offset[offsetT0+i] = sizeofVertex; sizeofVertex += 2 * sizeof( float ); break;
				case D3DFVF_TEXTUREFORMAT3: sizeofTexture[i] = 3; offset[offsetT0+i] = sizeofVertex; sizeofVertex += 3 * sizeof( float ); break;
				case D3DFVF_TEXTUREFORMAT4: sizeofTexture[i] = 4; offset[offsetT0+i] = sizeofVertex; sizeofVertex += 4 * sizeof( float ); break;
				}
			}
		}
	}
	virtual ~DX8VertexBufferFlexible()
	{
		if ( buffer ) buffer->Release();
	}
	void create( LPDIRECT3DDEVICE8 device )
	{
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		HRESULT hr = device->CreateVertexBuffer( data.size * sizeofVertex, D3DUSAGE_WRITEONLY, fvf, D3DPOOL_MANAGED, & buffer );
		if ( hr == D3D_OK ) return;
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		hr = device->CreateVertexBuffer( data.size * sizeofVertex, 0, fvf, D3DPOOL_MANAGED, & buffer );
		if ( hr == D3D_OK ) return;
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		assert( buffer );
	}
	virtual void update()
	{
		if ( ! buffer ) return;
		BYTE * rawData;
		buffer->Lock( 0, data.size * sizeofVertex, & rawData, D3DLOCK_NOSYSLOCK );
		const renderInterface::rVertex * src = data;
		renderInterface::charBgra color;
		for ( BYTE * dst = rawData, * sentinel = dst + data.size * sizeofVertex; dst != sentinel; ( dst += sizeofVertex ), ++src ) {
			if ( 0 <= offset[offsetXYZ] ) {
				( ( float * ) ( dst + offset[offsetXYZ] ) )[0] = src->position[0];
				( ( float * ) ( dst + offset[offsetXYZ] ) )[1] = src->position[1];
				( ( float * ) ( dst + offset[offsetXYZ] ) )[2] = src->position[2];
			}
			if ( 0 <= offset[offsetNormal] ) {
				( ( float * ) ( dst + offset[offsetNormal] ) )[0] = src->normal[0];
				( ( float * ) ( dst + offset[offsetNormal] ) )[1] = src->normal[1];
				( ( float * ) ( dst + offset[offsetNormal] ) )[2] = src->normal[2];
			}
			if ( 0 <= offset[offsetDiffuse] ) {
				color.element.red = unsigned char( src->diffuse[0] * 255 );
				color.element.green = unsigned char( src->diffuse[1] * 255 );
				color.element.blue = unsigned char( src->diffuse[2] * 255 );
				color.element.alpha = unsigned char( src->diffuse[3] * 255 );
				( ( DWORD * ) ( dst + offset[offsetDiffuse] ) )[0] = color.all;
			}
			if ( 0 <= offset[offsetSpecular] ) {
				color.element.red = unsigned char( src->specular[0] * 255 );
				color.element.green = unsigned char( src->specular[1] * 255 );
				color.element.blue = unsigned char( src->specular[2] * 255 );
				color.element.alpha = unsigned char( src->specular[3] * 255 );
				( ( DWORD * ) ( dst + offset[offsetSpecular] ) )[0] = color.all;
			}
			for ( int i = 0; i < 8; i++ ) {
				if ( offset[offsetT0+i] < 0 ) break;
				for ( int j = 0; j < sizeofTexture[i]; j++ ) {
					( ( float * ) ( dst + offset[offsetT0+i] ) )[j] = src->texture[i][j];
				}
			}
		}
		buffer->Unlock();
	}
};

template<class T,int type>
class DX8IndexBufferInterface : public renderInterface::rIndexBuffer
{
public:
	LPDIRECT3DINDEXBUFFER8 buffer;
	DX8IndexBufferInterface( int _size ) : 
		renderInterface::rIndexBuffer( _size ), 
		buffer( NULL )
	{
	}
	virtual ~DX8IndexBufferInterface()
	{
		if ( buffer ) buffer->Release();
	}
	void create( LPDIRECT3DDEVICE8 device )
	{
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		HRESULT hr = device->CreateIndexBuffer( data.size * sizeof( T ), D3DUSAGE_WRITEONLY, D3DFORMAT( type ), D3DPOOL_MANAGED, & buffer );
		if ( hr == D3D_OK ) return;
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		hr = device->CreateIndexBuffer( data.size * sizeof( T ), 0, D3DFORMAT( type ), D3DPOOL_MANAGED, & buffer );
		if ( hr == D3D_OK ) return;
		if ( buffer ) { buffer->Release(); buffer = NULL; }
		assert( buffer );
	}
	virtual void update()
	{
		if ( ! buffer ) return;
		unsigned char * rawData;
		buffer->Lock( 0, sizeof( T ) * data.size, & rawData, D3DLOCK_NOSYSLOCK );
		if ( sizeof( renderInterface::rIndexBuffer::element ) == sizeof( T ) ) {
			memcpy( rawData, data, sizeof( T ) * data.size );
		} else {
			renderInterface::rIndexBuffer::element * src = data;
			for ( T * dst = ( T * ) rawData, * sentinel = dst + data.size; dst != sentinel; ++dst, ++src ) {
				*dst = T( *src );
			}
		}
		buffer->Unlock();
	}
};

typedef DX8IndexBufferInterface<unsigned short, D3DFMT_INDEX16> DX8IndexBuffer16;
typedef DX8IndexBufferInterface<unsigned int, D3DFMT_INDEX32> DX8IndexBuffer32;

class vertexShaderBuilder
{
public:
	enum {
		DisableLightType,
		PositionalLightType,
		SpotLightType,
		DirectionalLightType,

		DisableTextureType,
		UVTextureType,
		WorldSpaceReflectionTextureType,
		CameraSpaceNormalTextureType,
		CameraSpacePositionTextureType,
		CameraSpaceReflectionTextureType,
	};
	enum // constant value offset
	{
		cWorldViewProjectionMatrix = 0,//頂点をスクリーン座標系まで変換
		cWorldMatrix = 4,//頂点をワールド座標系まで変換（射影がない）
		cViewMatrix = 8,//ワールド座標系からカメラ座標系まで変更（拡大・射影がない）
		cTextureMatrix = 12,//UVを変換する行列
		cEyePosition, //視点の位置
		cAmbientEmissive,// material ambient * light ambient + material emissive
		cDiffuse,// material diffuse
		cSpecular,// material specular , a : specular power
		cLightParameter, // [6*8]

		// c68-c94 : free

		cZeroHalfOneTwo = 95,
	};
	enum // register offset
	{

		// r0-r6 : free

		rEyeDirection = 7,
		rNormal, 
		rPosition, 
		rD0, 
		rD1
	};
	enum
	{
		vPosition = D3DVSDE_POSITION,
		vBlendWeight = D3DVSDE_BLENDWEIGHT,
		vBlendIndices = D3DVSDE_BLENDINDICES,
		vNormal = D3DVSDE_NORMAL,
		vPointSize = D3DVSDE_PSIZE,
		vDiffuse = D3DVSDE_DIFFUSE,
		vSpecular = D3DVSDE_SPECULAR,
		vTexture0 = D3DVSDE_TEXCOORD0,
		vTexture1 = D3DVSDE_TEXCOORD1,
		vTexture2 = D3DVSDE_TEXCOORD2,
		vTexture3 = D3DVSDE_TEXCOORD3,
		vTexture4 = D3DVSDE_TEXCOORD4,
		vTexture5 = D3DVSDE_TEXCOORD5,
		vTexture6 = D3DVSDE_TEXCOORD6,
		vTexture7 = D3DVSDE_TEXCOORD7,
		vPosition2 = D3DVSDE_POSITION2,
		vNormal2 = D3DVSDE_NORMAL2,
	};
	enum // light parameter offset
	{
		offsetDiffuseColor,
		offsetSpecularColor,
		offsetPositionAtWorld, // xyz for spot/positional light
		offsetDirectionAtWorld, // for directional/spot light
		offsetSpotParam,// cos( outer ), multiple factor, additional factor, falloff

		offsetAttenuation,// a0, a1, a2
		offsetRange = offsetAttenuation, // w : range
	};
	enum {
		lightNumber = 8,
		textureNumber = 8
	};
	char src[64*1024];
	struct Status
	{
		int light[lightNumber];
		int texture[textureNumber];
		bool diffuse;
		bool specular;
		bool textureMatrix;
		bool normal;
		bool range;
		bool attenuation;
		bool backfaceCulling;
		bool lighting;
		DWORD fvf;
		Status()
		{
			memset( this, 0, sizeof( *this ) );
			normal = true;
			backfaceCulling = true;
			lighting = true;
			for ( int i = 0; i < lightNumber; i++ ) light[i] = DisableLightType;
			for ( int i = 0; i < textureNumber; i++ ) texture[i] = DisableTextureType;
		}
		void operator=( const Status & value )
		{
			memcpy( this, & value, sizeof( *this ) );
		}
		bool operator==( const Status & value ) const 
		{
			return ( memcmp( this, & value, sizeof( *this ) ) ) ? false : true;
		}
		bool operator!=( const Status & value ) const 
		{
			return ( *this == value ) ? false : true;
		}
	} status;
	DWORD handle;
	LPDIRECT3DDEVICE8 device;
	vertexShaderBuilder( LPDIRECT3DDEVICE8 _device ) : device( _device )
	{
		handle = 0;
		src[0] = '\0';
	}
	~vertexShaderBuilder()
	{
		if ( handle ) device->DeleteVertexShader( handle );
	}
	void version( int major, int minor )
	{
		sprintf( src, "vs.%d.%d\r\n", major, minor );
	}
	void op( char *fmt, ... )
	{
		char wk[1024];
		va_list args;
		va_start( args, fmt );
		vsprintf( wk, fmt, args );
		va_end( args );

		strcat( src, wk );
		strcat( src, "\r\n" );
	}
	void comment( char *fmt, ... )
	{
		char wk[1024];
		va_list args;
		va_start( args, fmt );
		vsprintf( wk, fmt, args );
		va_end( args );

		strcat( src, "\r\n" );
		strcat( src, ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\r\n" );
		strcat( src, ";" );
		strcat( src, wk );
		strcat( src, "\r\n" );
	}
	void normalize( int rIndex )
	{
		op( "dp3 r%d.w, r%d, r%d", rIndex, rIndex, rIndex ); // .w : d^2
		op( "rsq r%d.w, r%d.w", rIndex, rIndex ); // .w : d
		op( "mul r%d, r%d, r%d.w", rIndex, rIndex, rIndex );
		op( "mov r%d.w, c[%d].z", rIndex, cZeroHalfOneTwo );
	}
	void divideByW( int rIndex )
	{
		op( "rcp r%d.w, r%d.w", rIndex, rIndex );
		op( "mul r%d, r%d, r%d.w", rIndex, rIndex, rIndex );
		op( "mov r%d.w, c[%d].z", rIndex, cZeroHalfOneTwo );
	}
	void calculateLightEffect( const int indexDiffuseColor, const int indexSpecularColor )
	{
		//back face culling
		if ( status.backfaceCulling ) {
			op( "max r0.y, r0.y, c[%d].x", cZeroHalfOneTwo );
		} else {
			op( "max r0.y, r0.y, -r0.y" ); // abs
		}

		// specular cut off
		op( "min r0.z, r0.z, c[%d].z", cZeroHalfOneTwo );

		// diffuse
		op( "mad r%d, c[%d], r0.y, r%d", rD0, indexDiffuseColor, rD0 );
		// specular
		op( "mad r%d, c[%d], r0.z, r%d", rD1, indexSpecularColor, rD1 );
	}
	void addDirectionalLight( const int offset )
	{
		comment( "directional light : parameter index %d", offset );
		// 方向光源
		const int indexDirectionAtWorld = offsetDirectionAtWorld + offset;
		const int indexDiffuseColor = offsetDiffuseColor + offset;
		const int indexSpecularColor = offsetSpecularColor + offset;

		// r1 : half angle vector between eye and direction : normalize( eye + direction )
		op( "add r1, r%d, c[%d]", rEyeDirection, indexDirectionAtWorld );
		normalize( 1 );
		
		// ライティング計算
		op( "dp3 r0.x, -r%d, c[%d]", rNormal, indexDirectionAtWorld );// normal dot light direction
		op( "dp3 r0.y, -r%d, r1", rNormal );// normal dot light and eye half angle direction
		op( "mov r0.w, c[%d].w", cSpecular );
		op( "lit r0, r0" );// 0 < sx ? ( y=sx, ( sy > 0 ? z=sy^sw ) )
		
		calculateLightEffect( indexDiffuseColor, indexSpecularColor );
	}
	void addSpotLight( const int offset )
	{
		comment( "spot light : parameter index %d", offset );
		const int indexPositionAtWorld = offsetPositionAtWorld + offset;
		const int indexDirectionAtWorld = offsetDirectionAtWorld + offset;
		const int indexDiffuseColor = offsetDiffuseColor + offset;
		const int indexSpecularColor = offsetSpecularColor + offset;
		const int indexAttenuation = offsetAttenuation + offset;
		const int indexSpotParam = offsetSpotParam + offset;
		const int indexRange = offsetRange + offset;
		
		// r1 : 点光源から頂点への方向 ( r0 : vector light from vertex )
		op( "add r0, r%d, -c[%d]", rPosition, indexPositionAtWorld );
		op( "dp3 r0.w, r0, r0" ); // r0.w : d^2
		// r3 : distance
		op( "rsq r3.w, r0.w" ); // r3.w : 1/d
		op( "mul r1, r0, r3.w" ); // r1 : normalized direction
		op( "dst r3, r0.w, r3.w" ); // r3 : 距離定数値 1, d, d^2, 1/d

		// r2 : half angle eye and direction normalize( eye + direction )
		op( "add r2, r1, r%d", rEyeDirection );
		normalize( 2 );

		// attenuation r3.w
		if ( status.attenuation ) {
			op( "dp3 r3.w, r3, c[%d]", indexAttenuation ); // ( a0 + a1 * d + a2 * d^2 )
			op( "rcp r3.w, r3.w" ); // 1 / ( a0 + a1 * d + a2 * d^2 )
		} else {
			op( "mov r3.w, c[%d].z", cZeroHalfOneTwo );
		}
		// range r0.w = range >= distance ? 1 : 0
		// ( r0 : flag )
		if ( status.range ) {
			op( "sge r0.w, c[%d].w, r3.y", indexRange );
			op( "mul r3.w, r3.w, r0.w" );
		}

		// spotlight cone attenuation
		// スポットライトの方向ベクトルと光源から頂点へのベクトルの角度を計算
		op( "dp3 r0.y, r1, c[%d]", indexDirectionAtWorld ); // r0.y : cos(α) : ライトの角度とライトへの方向の間の角度α
		// xに範囲外のチェックができるようにcos(outer)を引いた値を入れる
		op( "add r0.x, r0.y, -c[%d].x", indexSpotParam ); // cos( outer )
		// ライティングの計算は( ( cos(α) - cos(outer) ) / ( cos(inner) - cos(outer) ) ) ^ falloffだが、
		// ( cos(α) * multiple factor + additional factor ) ^ falloffにする
		// inner == outerのときに∞が出てしまうのでそれを防ぐため
		// if inner == outer then  ( cos(α) * 0 + 1 ) ^ falloff
		// if inner != outer then  ( cos(α) * ( 1 / (ci-co) ) - co/(ci-co) ) ^ falloff
		op( "mad r0.y, r0.y, c[%d].y, c[%d].z", indexSpotParam, indexSpotParam ); // multipul factor / additional factor
		// 内側の時には１以上になるので、1とのminimumを取る
		// 外部の時にはr0.xが０以下になるので、どうせ結果が０になる
		op( "min r0.y, r0.y, c[%d].z", cZeroHalfOneTwo );
		op( "mov r0.w, c[%d].w", indexSpotParam ); // falloff
		op( "lit r0, r0" );// 0 < sx ? ( y=sx, ( sy > 0 ? z=sy^sw ) )
		op( "mul r3.w, r3.w, r0.z" );//減衰パラメータに乗じておく

		// ライティング計算
		op( "dp3 r0.x, -r%d, r1", rNormal );// normal dot light direction
		op( "dp3 r0.y, -r%d, r2", rNormal );// normal dot light and eye half angle direction
		op( "mov r0.w, c[%d].w", cSpecular );// specular power
		op( "lit r0, r0" );// 0 < sx ? ( y=sx, ( sy > 0 ? z=sy^sw ) )

		op( "mul r0.z, r0.z, r3.w" );//減衰させる
		op( "mov r0.y, r3.w" );// diffuseはライティング計算しない

		calculateLightEffect( indexDiffuseColor, indexSpecularColor );
	}
	void addPositionalLight( const int offset )
	{
		comment( "positional light : parameter index %d", offset );

		const int indexPositionAtWorld = offsetPositionAtWorld + offset;
		const int indexDiffuseColor = offsetDiffuseColor + offset;
		const int indexSpecularColor = offsetSpecularColor + offset;
		const int indexAttenuation = offsetAttenuation + offset;
		const int indexRange = offsetRange + offset;
		
		// r1 : 点光源から頂点への方向 ( r0 : vector light from vertex )
		op( "add r0, r%d, -c[%d]", rPosition, indexPositionAtWorld );
		op( "dp3 r0.w, r0, r0" ); // r0.w : d^2
		// r3 : distance
		op( "rsq r3.w, r0.w" ); // r3.w : 1/d
		op( "mul r1, r0, r3.w" ); // r1 : normalized direction
		op( "dst r3, r0.w, r3.w" ); // r3 : 距離定数値 1, d, d^2, 1/d

		// r2 : half angle eye and direction normalize( eye + direction )
		op( "add r2, r1, r%d", rEyeDirection );
		normalize( 2 );

		// attenuation r3.w
		if ( status.attenuation ) {
			op( "dp3 r3.w, r3, c[%d]", indexAttenuation ); // ( a0 + a1 * d + a2 * d^2 )
			op( "rcp r3.w, r3.w" ); // 1 / ( a0 + a1 * d + a2 * d^2 )
		} else {
			op( "mov r3.w, c[%d].z", cZeroHalfOneTwo );
		}
		// range r0.w = range >= distance ? 1 : 0
		// ( r0 : flag )
		if ( status.range ) {
			op( "sge r0.w, c[%d].w, r3.y", indexRange );
			op( "mul r3.w, r3.w, r0.w" );
		}

		// ライティング計算
		op( "dp3 r0.x, -r%d, r1", rNormal );// normal dot light direction
		op( "dp3 r0.y, -r%d, r2", rNormal );// normal dot light and eye half angle direction
		op( "mov r0.w, c[%d].w", cSpecular );// specular power
		op( "lit r0, r0" );// 0 < sx ? ( y=sx, ( sy > 0 ? z=sy^sw ) )

		op( "mul r0, r0, r3.w" );//減衰させる

		calculateLightEffect( indexDiffuseColor, indexSpecularColor );
	}
	void reflectionVector( int rIndex )
	{
		// reflection vector|world = 2 * ( -eye dot normal ) normal + eye
		// r0 : reflection vector
		op( "dp3 r%d.w, r%d, -r%d", rIndex, rNormal, rEyeDirection );
		op( "mul r%d.w, r%d.w, c[%d].w", rIndex, rIndex, cZeroHalfOneTwo );
		op( "mad r%d, r%d.w, r%d, r%d", rIndex, rIndex, rNormal, rEyeDirection );
	}
	void sphereMap( int rIndex )
	{
		// sphere map m = 2 * sqrt( r.x^2 + r.y^2 + ( r.z + 1 )^2 );
		// uv = r/m + 0.5
		comment( "sphere map" );
		op( "add r%d.z, r%d.z, c[%d].z", rIndex, rIndex, cZeroHalfOneTwo );
		op( "dp3 r%d.w, r%d, r%d", rIndex, rIndex, rIndex );
		op( "rsq r%d.w, r%d.w", rIndex, rIndex );
		op( "mul r%d.xyz, r%d, c[%d].yyxx", rIndex, rIndex, cZeroHalfOneTwo );
		op( "mad r%d, r%d, r%d.w, c[%d].yyxx", rIndex, rIndex, rIndex, cZeroHalfOneTwo );
	}
	bool create()
	{
		int i;
		version( majorVersion, minorVersion );

		//頂点の変換
		comment( "vertex position" );
		// rPosition 位置|screen
		op( "m4x4 oPos, v%d, c[%d]", vPosition, cWorldViewProjectionMatrix );
		// rPosition 位置|world
		op( "m4x4 r%d, v%d, c[%d]", rPosition, vPosition, cWorldMatrix );
		divideByW( rPosition );

		if ( status.normal ) {
			//法線をワールド座標系に変換
			// rNormal 法線|world
			comment( "normal" );
			op( "mov r0, v%d", vNormal );
			normalize( 0 );
			op( "m3x3 r%d, r0, c[%d]", rNormal, cWorldMatrix );
			normalize( rNormal );
			// カメラから頂点へのベクトル
			op( "add r%d, r%d, -c[%d]", rEyeDirection, rPosition, cEyePosition );
			normalize( rEyeDirection );

			// oD0 * texture color + oD1でpixel shaderで使われる
			comment( "color" );
			op( "mov r%d, c[%d].x", rD0, cZeroHalfOneTwo );
			op( "mov r%d, c[%d].x", rD1, cZeroHalfOneTwo );

			if ( status.lighting ) {
				//ライティング（light diffuse/specularの計算）
				comment( "lighting" );

				for ( i = 0; i < lightNumber; i++ ) {
					switch( status.light[i] ) {
						case 0: break;
						case D3DLIGHT_POINT: addPositionalLight( cLightParameter + 6 * i ); break;
						case D3DLIGHT_SPOT: addSpotLight( cLightParameter + 6 * i ); break;
						case D3DLIGHT_DIRECTIONAL: addDirectionalLight( cLightParameter + 6 * i ); break;
					}
				}

				// lighting clamp
				comment( "lighting clamp" );
				op( "max r%d, r%d, c[%d].xxxx", rD0, rD0, cZeroHalfOneTwo );
				op( "max r%d, r%d, c[%d].xxxx", rD1, rD1, cZeroHalfOneTwo );
				op( "min r%d, r%d, c[%d].zzzx", rD0, rD0, cZeroHalfOneTwo );
				op( "min r%d, r%d, c[%d].zzzx", rD1, rD1, cZeroHalfOneTwo );
			}
			// diffuse / specular
			comment( "diffuse/specular" );
			if ( status.diffuse ) {
				op( "mul r%d.xyz, r%d, v%d", rD0, rD0, vDiffuse );
			} else {
				op( "mul r%d.xyz, r%d, c[%d]", rD0, rD0, cDiffuse );
			}
			if ( status.specular ) {
				op( "mul r%d.xyz, r%d, v%d", rD1, rD1, vSpecular );
			} else {
				op( "mul r%d.xyz, r%d, c[%d]", rD1, rD1, cSpecular );
			}

			// ambient / emissive
			comment( "ambient/emissive" );
			op( "add r%d, r%d, c[%d]", rD0, rD0, cAmbientEmissive );

			comment( "color parameter for pixel shader" );
			op( "max r%d, r%d, c[%d].xxxx", rD0, rD0, cZeroHalfOneTwo );
			op( "max r%d, r%d, c[%d].xxxx", rD1, rD1, cZeroHalfOneTwo );
			op( "min oD0, r%d, c[%d].zzzz", rD0, cZeroHalfOneTwo );
			op( "min oD1, r%d, c[%d].zzzz", rD1, cZeroHalfOneTwo );
		} else {
			// ambient / emissive
			comment( "basecolor" );
			if ( status.diffuse ) {
				op( "mov oD0, v%d", vDiffuse );
			} else {
				op( "mov oD0, c[%d]", cDiffuse );
			}
			if ( status.specular ) {
				op( "mov oD1, v%d", vSpecular );
			} else {
				op( "mov oD1, c[%d]", cSpecular );
			}
		}

		comment( "texture" );

		//テクスチャ座標の代入
		for ( i = 0; i < textureNumber; i++ ) {
			comment( "texture %d", i );
			// r0 : original uv
			switch ( status.texture[i] ) {
			case DisableTextureType:
				continue;
			case UVTextureType: 
				op( "mov r0, v%d", i + vTexture0 );
				break;
			case WorldSpaceReflectionTextureType: 
				// reflection vector|world = 2 * ( -eye dot normal ) normal + eye
				reflectionVector( 0 );
				sphereMap( 0 );
				break;
			case CameraSpaceNormalTextureType:
				op( "m3x3 r0, r%d, c[%d]", rNormal, cViewMatrix );
				// [-1,1]なので、[0,1]にマッピングする
				//op( "mad r0, r0, c[%d].y, c[%d].y", cZeroHalfOneTwo, cZeroHalfOneTwo );
				sphereMap( 0 );
				break;
			case CameraSpacePositionTextureType: 
				op( "m4x4 r0, r%d, c[%d]", rPosition, cViewMatrix );
				// normalize
				normalize( 0 );
				sphereMap( 0 );
				break;
			case CameraSpaceReflectionTextureType: 
				// reflection vector|world = 2 * ( -eye dot normal ) normal + eye
				reflectionVector( 1 );
				// to camera space
				op( "m3x3 r0, r1, c[%d]", cViewMatrix );
				sphereMap( 0 );
				break;
			}

			if ( status.textureMatrix ) {
				op( "m4x4 oT%d, r0, c[%d]", i, cTextureMatrix );
			} else {
				op( "mov oT%d, r0", i );
			}
		}
		return true;
	}
	bool assemble()
	{
		LPD3DXBUFFER code;
		HRESULT hr = D3DXAssembleShader( src, strlen( src ), 0, NULL, & code, NULL );
		if ( SUCCEEDED( hr ) ) {
			DWORD declaration[MAX_FVF_DECL_SIZE];
			hr = D3DXDeclaratorFromFVF( status.fvf, declaration );
			/*
			int offset = 0;
			declaration[offset++] = D3DVSD_STREAM( 0 );
			declaration[offset++] = D3DVSD_REG( D3DVSDE_POSITION, D3DVSDT_FLOAT3 );
			if ( normal ) declaration[offset++] = D3DVSD_REG( D3DVSDE_NORMAL, D3DVSDT_FLOAT3 );
			if ( diffuse ) declaration[offset++] = D3DVSD_REG( D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR );
			if ( specular ) declaration[offset++] = D3DVSD_REG( D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR );
			if ( texture[0] ) declaration[offset++] = D3DVSD_REG( D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2 );
			declaration[offset++] = D3DVSD_END();
			*/
			if ( SUCCEEDED( hr ) ) {
				hr = device->CreateVertexShader( declaration, (DWORD*)code->GetBufferPointer(), & handle, 0 );
			}
		}
		if ( code ) { code->Release(); code = NULL; }
		return SUCCEEDED( hr ) ? true : false;
	}
	bool build()
	{
		if ( ! create() ) return false;
		if ( ! assemble() ) return false;
		return true;
	}
};
class pixelShaderBuilder
{
public:
	char src[1024];
	DWORD handle;
	LPDIRECT3DDEVICE8 device;
	int texture;
	bool multiply;
	bool additional;
	// ver 1.1 : same time access t(2), v(2), r(2)
	pixelShaderBuilder( LPDIRECT3DDEVICE8 _device ) : device( _device )
	{
		handle = 0;
		src[0] = '\0';
		texture = 1;
		multiply = true;
		additional = true;
	}
	~pixelShaderBuilder()
	{
		if ( handle ) device->DeletePixelShader( handle );
	}
	void version( int major, int minor )
	{
		sprintf( src, "ps.%d.%d\r\n", major, minor );
	}
	void op( char *fmt, ... )
	{
		char wk[1024];
		va_list args;
		va_start( args, fmt );
		vsprintf( wk, fmt, args );
		va_end( args );

		strcat( src, wk );
		strcat( src, "\r\n" );
	}
	void comment( char *fmt, ... )
	{
		char wk[1024];
		va_list args;
		va_start( args, fmt );
		vsprintf( wk, fmt, args );
		va_end( args );

		strcat( src, "\r\n" );
		strcat( src, ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\r\n" );
		strcat( src, ";" );
		strcat( src, wk );
		strcat( src, "\r\n" );
	}
	bool create()
	{
		version( majorVersion, minorVersion );
		for ( int i = 0; i < texture; i++ ) {
			op( "tex t%d", i );
		}
		const char * target = NULL;
		const char * t0 = "t0";
		const char * r0 = "r0";
		const char * v0 = "v0";
		const char * v1 = "v1";
		if ( texture ) {
			if ( texture == 1 ) {
				target = t0;
			} else {
				target = r0;
				op( "mul r0, t0, t1" );
				for ( int i = 2; i < texture; i++ ) {
					op( "mul r0, r0, t%d", i );
				}
			}
		}
		if ( multiply && additional ) {
			if ( target ) {
				op( "mad_sat r0.rgb, %s, v0, v1", target );
		//		op( "+ mul r0.a, %s.a, v0.a", target );
			} else {
				op( "add r0.rgb, v0, v1" );
				op( "+ mov r0.a, v0" );
			}
			target = r0;
		} else if ( multiply ) {
			if ( target ) {
				op( "mul r0, %s, v0", target );
				target = r0;
			} else {
				target = v0;
			}
		} else if ( additional ) {
			if ( target ) {
				op( "add_sat r0.rgb, %s, v1", target );
				op( "+ mov r0.a, %s.a", target );
				target = r0;
			} else {
				target = v1;
			}
		}
		if ( target && target != r0 ) {//まだr0に結果がない場合
			op( "mov r0, %s", target );
		}
		return true;
	}
	bool assemble()
	{
		LPD3DXBUFFER code;
	    LPD3DXBUFFER pBuffer = NULL;
		HRESULT hr = D3DXAssembleShader( src, strlen( src ), 0, NULL, & code, & pBuffer );
		if ( SUCCEEDED( hr ) ) {
			hr = device->CreatePixelShader( (DWORD*)code->GetBufferPointer(), & handle );
			if ( code ) { code->Release(); code = NULL; }
			if ( pBuffer ) { pBuffer->Release(); pBuffer = NULL; }
			if ( SUCCEEDED( hr ) ) return true;
		}
		if ( code ) { code->Release(); code = NULL; }
		if ( pBuffer ) { pBuffer->Release(); pBuffer = NULL; }
		return false;
	}
	bool build()
	{
		if ( ! create() ) return false;
		if ( ! assemble() ) return false;
		return true;
	}
};

class rDirectX8Version : public renderInterface
{
public:
	enum constantValue
	{
		lightNumber = 8,
		renderStateMaxNumber = D3DRS_BLENDOP + 1,
	};
	LPDIRECT3D8 d3d;
	LPDIRECT3DDEVICE8 device;
	D3DDISPLAYMODE mode;
	D3DPRESENT_PARAMETERS pp;
	LPDIRECT3DSURFACE8 backBuffer;
	LPD3DXMATRIXSTACK stackMatrix;
	D3DCAPS8 caps;

	cache<D3DMATERIAL8> prevMaterial;
	cache<bool> lighting;
	cache<D3DLIGHT8> light[lightNumber];
	cache<BOOL> prevLightEnable[lightNumber];
	cache<DWORD> renderState[renderStateMaxNumber];
	cache<LPDIRECT3DTEXTURE8> prevTexture;
	cache<LPDIRECT3DVERTEXBUFFER8> vertexBuffer;
	cache<LPDIRECT3DINDEXBUFFER8> indexBuffer;
	cache<DWORD> prevDefaultVertexShader;
	cache<vertexShaderBuilder*> prevVertexShader;
	cache<pixelShaderBuilder*> prevPixelShader;
	cache<bool> VertexShaderConstant;
	vector3 ambient;

	D3DXMATRIX unitMatrix;
	rTexture * whiteTexture, * blackTexture;
	unsigned long * bufferTemp;
	rVertexBuffer * vertexBufferTemp;
	list< retainer<vertexShaderBuilder> > shaderList;
	list< retainer<pixelShaderBuilder> > pixelShaderList;

	D3DXMATRIX matrixWorld, matrixView, matrixProjection;

	HWND hwnd;
	rDirectX8Version() 
		// DirectX
		: d3d( NULL ), device( NULL ), stackMatrix( NULL ), 
		whiteTexture( NULL ), blackTexture( NULL ), bufferTemp( NULL ), backBuffer( NULL ), vertexBufferTemp( NULL )
	{
	}
	virtual ~rDirectX8Version()
	{
		finalize();
	}
	virtual void clearCache()
	{
		prevTexture.clear();
		prevMaterial.clear();
		vertexBuffer.clear();
		indexBuffer.clear();
		prevVertexShader.clear();
		prevPixelShader.clear();
		prevDefaultVertexShader.clear();
		VertexShaderConstant.clear();
		lighting.clear();
		int i;
		for ( i = 0; i < lightNumber; i++ ) {
			light[i].clear();
			prevLightEnable[i].clear();
		}
		for ( i = 0; i < renderStateMaxNumber; i++ ) {
			renderState[i].clear();
		}
	}
	virtual bool initialized()
	{
		return device ? true : false;
	}
	virtual bool initialize( void * hwnd, bool window, int width, int height )
	{
		finalize();
		this->width = width; this->height = height;
		this->hwnd = ( HWND ) hwnd;
		d3d = Direct3DCreate8( D3D_SDK_VERSION );
		if ( ! d3d ) return false;

		ZeroMemory( & pp, sizeof( pp ) );

		pp.BackBufferCount = 1;
		pp.EnableAutoDepthStencil = TRUE;
		pp.AutoDepthStencilFormat = D3DFMT_D16;//D3DFMT_D16 D3DFMT_D24S8 D3DFMT_D32
		pp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_FLIP D3DSWAPEFFECT_DISCARD;
		pp.Flags = useBackBuffer ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0;
		if ( ! window ) {
			RECT r;
			GetWindowRect( ( HWND ) hwnd, &r );
			this->width = pp.BackBufferWidth = width < 0 ? r.right - r.left : width;
			this->height = pp.BackBufferHeight = height < 0 ? r.bottom - r.top : height;
			if ( pp.BackBufferWidth <= 640 && pp.BackBufferHeight <= 480 ) {
				pp.BackBufferWidth = 640;
				pp.BackBufferHeight = 480;
			} else if ( pp.BackBufferWidth <= 800 && pp.BackBufferHeight <= 600 ) {
				pp.BackBufferWidth = 800;
				pp.BackBufferHeight = 600;
			} else if ( pp.BackBufferWidth <= 1024 && pp.BackBufferHeight <= 768 ) {
				pp.BackBufferWidth = 1024;
				pp.BackBufferHeight = 768;
			} else if ( pp.BackBufferWidth <= 1280 && pp.BackBufferHeight <= 1024 ) {
				pp.BackBufferWidth = 1280;
				pp.BackBufferHeight = 1024;
			} else {
				window = true;
			}
		}
		if ( window ) {
			d3d->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, & mode );
			pp.Windowed = TRUE;
			pp.BackBufferFormat = mode.Format;//D3DFMT_A8R8G8B8でやるとソフトウェアで初期化される
		} else {
			pp.Windowed = FALSE;
			pp.BackBufferFormat = D3DFMT_X8R8G8B8;//D3DFMT_X8R8G8B8; D3DFMT_A8R8G8B8
			pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;//75;
			pp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		}
		device = NULL;
		D3DDEVTYPE type[3] = { D3DDEVTYPE_HAL, D3DDEVTYPE_REF, D3DDEVTYPE_SW };
		for ( int i = 0; i < 3; i++ ) {
			if ( device ) continue;
			d3d->GetDeviceCaps( D3DADAPTER_DEFAULT, type[i], & caps );
			if ( usePixelShader && caps.PixelShaderVersion < D3DPS_VERSION( majorVersion, minorVersion ) ) continue;
			if ( useVertexShader && caps.VertexShaderVersion < D3DVS_VERSION( majorVersion, minorVersion ) ) continue;
			d3d->CreateDevice( D3DADAPTER_DEFAULT, type[i], ( HWND ) hwnd, D3DCREATE_MULTITHREADED | 
				( caps.VertexShaderVersion < D3DVS_VERSION( 1, 0 ) ? D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING ) 
				, & pp, & device );
		}
		if ( ! device ) return false;
		device->GetDeviceCaps( & caps );

		D3DXMatrixIdentity( ( D3DXMATRIX * ) & unitMatrix );
		matrixWorld = matrixView = matrixProjection = unitMatrix;

		D3DXCreateMatrixStack( 0, & stackMatrix );
		if ( ! stackMatrix ) return false;
		stackMatrix->Push();
		stackMatrix->LoadIdentity();

		//D3DTOP_BLENDTEXTUREALPHA D3DTOP_MODULATE D3DTOP_SELECTARG1
		device->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		device->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		device->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		device->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		device->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

		cull( culling, cw );

		unsigned long whitePixel = 0xffffffff;
		whiteTexture = createTexture( "whiteTexture", 1, 1, ( unsigned long * ) & whitePixel, 0 );
		unsigned long blackPixel = 0xff000000;
		blackTexture = createTexture( "black_texture", 1, 1, ( unsigned long * ) & blackPixel, 0 );

		if ( useBackBuffer ) device->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, & backBuffer );

		vertexBufferTemp = createVertexBuffer( 3, renderInterface::rVertexBuffer::enableVNT0 );

		if ( useBufferMask && ! ( caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE ) ) return false;

		return true;
	}
	virtual void finalize()
	{
		shaderList.release();
		pixelShaderList.release();
		if ( vertexBufferTemp ) { delete vertexBufferTemp; vertexBufferTemp = NULL; }
		if ( bufferTemp ) { delete [] bufferTemp; bufferTemp = NULL; }
		if ( blackTexture ) { delete blackTexture; blackTexture = NULL; }
		if ( whiteTexture ) { delete whiteTexture; whiteTexture = NULL; }
		if ( stackMatrix ) { stackMatrix->Release(); stackMatrix = NULL; }
		if ( backBuffer ) { backBuffer->Release(); backBuffer = NULL; }
		if ( device ) { device->Release(); device = NULL; }
		if ( d3d ) { d3d->Release(); d3d = NULL; }
	}
	int setRenderState( int State, DWORD Value )
	{
		if ( State < renderStateMaxNumber ) {
			if ( renderState[State].enable && renderState[State].data == Value ) return 0;
			renderState[State].enable = true;
			renderState[State].data = Value;
		}
		return device->SetRenderState( ( D3DRENDERSTATETYPE ) State, Value );
	}

	void activate( bool /*active*/ )
	{
	}
	virtual void startScene( DWORD clearColor )
	{
		device->BeginScene();
		device->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0, 0 );

		setRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		setRenderState( D3DRS_SPECULARENABLE, TRUE );

		setRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
		setRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
		setRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
		setRenderState( D3DRS_EMISSIVEMATERIALSOURCE , D3DMCS_MATERIAL );

		setRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		setRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		setRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		setRenderState( D3DRS_NORMALIZENORMALS, TRUE );

		if ( fillmode ) {
			setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		} else {
			setRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		}

		// UV値が０～１以内のデータのときに、１～０へのラッピングを自動的にやるかどうか
	//	setRenderState( D3DRS_WRAP0, D3DWRAPCOORD_0 | D3DWRAPCOORD_1 );
		// UV値が０～１以外のときにテクスチャの値をどのようにするか（ミラーなど）
		device->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
		device->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
		depthTest( true, true, testFuncLess );
		initMatrix();
	}
	virtual void stencilClear()
	{
		device->Clear( 0, NULL, D3DCLEAR_STENCIL , 0x00000000, 1.0, 0 );
	}

	virtual void finishScene( bool swapBuffer )
	{
		device->EndScene();
		if ( swapBuffer ) this->swapBuffer();
	}
	virtual void swapBuffer()
	{
		device->Present( NULL, NULL, NULL, NULL );
	}

	virtual void cull( bool enable, bool cw )
	{
		setRenderState( D3DRS_CULLMODE, enable ? ( cw ? D3DCULL_CW : D3DCULL_CCW ) : D3DCULL_NONE );
	}
	virtual void readFrontBuffer( unsigned long * buffer )
	{
		LPDIRECT3DSURFACE8 surface;
		device->CreateImageSurface( pp.BackBufferWidth, pp.BackBufferHeight, D3DFMT_A8R8G8B8, & surface );
		if ( surface ) {
			device->GetFrontBuffer( surface );
			D3DLOCKED_RECT r;
			surface->LockRect( & r, NULL, D3DLOCK_READONLY );
			if ( width != ( int ) pp.BackBufferWidth || height != ( int ) pp.BackBufferHeight ) {
				for ( int i = 0; i < height; i++ ) {
					memcpy( buffer + i * width, ( int * ) ( r.pBits ) + i * pp.BackBufferWidth, width * sizeof( int ) );
				}
			} else {
				memcpy( buffer, r.pBits, width * height * sizeof( int ) );
			}
			surface->UnlockRect();
			surface->Release();
		}
	}

	virtual void readBackBuffer( unsigned long * buffer )
	{
		if ( ! backBuffer ) return;
		int pitch = 0;
		D3DLOCKED_RECT r;
		HRESULT result = backBuffer->LockRect( & r, NULL, D3DLOCK_READONLY );
		if ( ! result ) {
			if ( int( width * sizeof( unsigned long ) ) == r.Pitch ) {
				memcpy( buffer, r.pBits, r.Pitch * height );
			} else {
				pitch = r.Pitch / sizeof( unsigned long );
				for ( unsigned long * dst = buffer, * sentinel = dst + width * height, * src = ( unsigned long * ) r.pBits; 
					dst != sentinel; dst += width, src += pitch ) {
					memcpy( dst, src, width * sizeof( unsigned long ) );
				}
			}
			result = backBuffer->UnlockRect();
		}
	}

	virtual int getBackBufferPitch()
	{
		if ( ! backBuffer ) return 0;
		int pitch = 0;
		D3DLOCKED_RECT r;
		if ( ! backBuffer->LockRect( & r, NULL, D3DLOCK_READONLY ) ) {
			pitch = r.Pitch / 4;
			backBuffer->UnlockRect();
		}
		return pitch;
	}

	virtual void blend( int destination, int source )
	{
		setRenderState( D3DRS_DESTBLEND, destination );
		setRenderState( D3DRS_SRCBLEND, source );
	}

	virtual void depthTest( bool testing, bool writing, int testFunc )
	{
		setRenderState( D3DRS_ZENABLE, testing ? D3DZB_TRUE : D3DZB_FALSE );
		setRenderState( D3DRS_ZFUNC, testFunc );
		setRenderState( D3DRS_ZWRITEENABLE, writing ? TRUE : FALSE );
	}
	virtual void alphaTest( bool testing, float reference, int testFunc )
	{
		setRenderState( D3DRS_ALPHATESTENABLE, testing ? TRUE : FALSE );
		setRenderState( D3DRS_ALPHAREF, DWORD( reference * 255.0f ) );
		setRenderState( D3DRS_ALPHAFUNC, testFunc );
	}
	virtual void stencilTest( bool testing, unsigned long reference, unsigned long mask, unsigned long writeMask, int opFail, int opZFail, int opPass, int testFunc )
	{
		setRenderState( D3DRS_STENCILENABLE, testing ? TRUE : FALSE );
		setRenderState( D3DRS_STENCILFAIL, opFail );
		setRenderState( D3DRS_STENCILZFAIL, opZFail );
		setRenderState( D3DRS_STENCILPASS, opPass );
		setRenderState( D3DRS_STENCILFUNC, testFunc );
		setRenderState( D3DRS_STENCILREF, reference );
		setRenderState( D3DRS_STENCILMASK, mask );
		setRenderState( D3DRS_STENCILWRITEMASK, writeMask );
	}
	virtual void colorTest( bool red, bool green, bool blue, bool alpha )
	{
		setRenderState( D3DRS_COLORWRITEENABLE, 
			( red ? D3DCOLORWRITEENABLE_RED : 0 ) |
			( green ? D3DCOLORWRITEENABLE_GREEN : 0 ) |
			( blue ? D3DCOLORWRITEENABLE_BLUE : 0 ) |
			( alpha ? D3DCOLORWRITEENABLE_ALPHA : 0 ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// TEXTURE
	class DX8Texture : public renderInterface::rTexture
	{
	public:
		LPDIRECT3DTEXTURE8 texture;
		bool transparency;
		char filename[MAX_PATH];
		DX8Texture() : texture( NULL ), transparency( false )
		{
			filename[0] = '\0';
			name = filename;
		}
		~DX8Texture()
		{
			if ( texture ) texture->Release(); texture = NULL;
		}
		bool isTransparency(){ return transparency; }
	};
	void createTexture( LPDIRECT3DTEXTURE8 & texture, int width, int height, unsigned long * pixels )
	{
		HRESULT hr;
		hr = D3DXCreateTexture( device, width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, & texture );
		if ( hr == D3D_OK ) {
			D3DLOCKED_RECT r;
			hr = texture->LockRect( 0, &r, NULL, 0 );
			for ( int y = 0; y < height; y++ ) {
				char * dst = ( ( char * ) ( r.pBits ) ) + r.Pitch * y;
				const char * src = ( ( const char * ) ( pixels + width * y ) );
				memcpy( dst, src, minimum<int>( width * 4, r.Pitch ) );
			}
			hr = texture->UnlockRect( 0 );
			assert( hr == D3D_OK );
			return;
		}
		hr = D3DXCreateTexture( device, width, height, 1, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED, & texture );
		if ( hr == D3D_OK ) {
			D3DLOCKED_RECT r;
			hr = texture->LockRect( 0, &r, NULL, 0 );
			for ( int y = 0; y < height; y++ ) {
				char * dst = ( ( char * ) ( r.pBits ) ) + r.Pitch * y;
				const char * src = ( ( const char * ) ( pixels + width * y ) );
				memcpy( dst, src, minimum<int>( width * 3, r.Pitch ) );
			}
			hr = texture->UnlockRect( 0 );
			assert( hr == D3D_OK );
			return;
		}
		assert( hr == D3D_OK );
	}
	
	virtual rTexture * createTexture( const char * tag, int width, int height, unsigned long * pixels, int /*level*/ )
	{
		DX8Texture * result = new DX8Texture();
		strcpy( result->filename, tag );
		createTexture( result->texture, width, height, pixels );
		for ( charBgra * src = ( charBgra * ) pixels, * sentinel = src + width * height; src != sentinel; ++src ) {
			if ( src->element.alpha != 0xff ) {
				result->transparency = true;
				break;
			}
		}
		return result;
	}
	void setTextureInternal( int index, LPDIRECT3DTEXTURE8 texture )
	{
		if ( prevTexture.enable && prevTexture.data == texture ) return;
		prevTexture.enable = true;
		prevTexture.data = texture;
		HRESULT hr = device->SetTexture( index, texture );
		if ( hr != D3D_OK ) {
			DX8Texture * dxt = ( DX8Texture * ) whiteTexture;
			if ( texture != dxt->texture ) {
				setTextureInternal( index, dxt->texture );
			}
		}
	}
	virtual void setTexture( int index, rTexture * texture )
	{
		if ( ! texture ) texture = whiteTexture;
		DX8Texture * dxt = ( DX8Texture * ) texture;
		setTextureInternal( index, dxt ? dxt->texture : NULL );
	}
	virtual void setWhiteTexture( int index )
	{
		setTexture( index, whiteTexture );
	}
	virtual void setBlackTexture( int index )
	{
		setTexture( index, blackTexture );
	}
	////////////////////////////////////////////////////////////////////////////////
	// Tool
	inline bool compareF4( const D3DCOLORVALUE & left, const float right[4] )
	{ return ( left.r == right[0] && left.g == right[1] && left.b == right[2] && left.a == right[3] ) ? true : false; }
	inline void copyF4( D3DCOLORVALUE & dst, const float src[4] )
	{ dst.r = src[0]; dst.g = src[1]; dst.b = src[2]; dst.a = src[3]; }
	inline bool compareF3( const D3DCOLORVALUE & left, const float right[3] )
	{ return ( left.r == right[0] && left.g == right[1] && left.b == right[2] ) ? true : false; }
	inline bool compareF3( const D3DVECTOR & left, const float right[3] )
	{ return ( left.x == right[0] && left.y == right[1] && left.z == right[2] ) ? true : false; }
	inline void copyF3( D3DVECTOR & dst, const float src[3] )
	{ dst.x = src[0]; dst.y = src[1]; dst.z = src[2]; }

	inline bool compareF4( const D3DCOLORVALUE & left, const vector4 & right )
	{ return ( left.r == right[0] && left.g == right[1] && left.b == right[2] && left.a == right[3] ) ? true : false; }
	inline void copyF4( D3DCOLORVALUE & dst, const vector4 & src )
	{ dst.r = src[0]; dst.g = src[1]; dst.b = src[2]; dst.a = src[3]; }
	inline bool compareF3( const D3DCOLORVALUE & left, const vector4 & right )
	{ return ( left.r == right[0] && left.g == right[1] && left.b == right[2] ) ? true : false; }
	inline bool compareF3( const D3DCOLORVALUE & left, const vector3 & right )
	{ return ( left.r == right[0] && left.g == right[1] && left.b == right[2] ) ? true : false; }
	inline bool compareF3( const D3DVECTOR & left, const vector3 & right )
	{ return ( left.x == right[0] && left.y == right[1] && left.z == right[2] ) ? true : false; }
	inline bool compareF3( const D3DVECTOR & left, const vector4 & right )
	{ return ( left.x == right[0] && left.y == right[1] && left.z == right[2] ) ? true : false; }
	inline void copyF3( D3DVECTOR & dst, const vector4 & src )
	{ dst.x = src[0]; dst.y = src[1]; dst.z = src[2]; }
	inline void copyF3( D3DVECTOR & dst, const vector3 & src )
	{ dst.x = src[0]; dst.y = src[1]; dst.z = src[2]; }
	inline void copyF3( D3DCOLORVALUE & dst, const vector3 & src )
	{ dst.r = src[0]; dst.g = src[1]; dst.b = src[2]; }
	////////////////////////////////////////////////////////////////////////////////
	// MATERIAL

	virtual void material( const vector3 & ambient, const vector3 & diffuse, const vector3 & specular, const vector3 & emissive, decimal transparency, decimal specularPower )
	{
		D3DMATERIAL8 & m = prevMaterial.data;
		if ( prevMaterial.enable && 
			compareF3( m.Ambient, ambient ) && 
			compareF3( m.Diffuse, diffuse ) && 
			compareF3( m.Specular, specular ) && 
			compareF3( m.Emissive, emissive ) && 
			m.Ambient.a == 1 - transparency &&
			m.Power == specularPower ) 
			return;
		copyF3( m.Ambient, ambient );
		copyF3( m.Diffuse, diffuse );
		copyF3( m.Specular, specular );
		copyF3( m.Emissive, emissive );
		m.Power = specularPower;
		m.Ambient.a = 1 - transparency;
		m.Diffuse.a = 1 - transparency;
		m.Specular.a = 0;
		m.Emissive.a = 0;
		prevMaterial.enable = true;
		if ( ! useVertexShader ) device->SetMaterial( & m );
		VertexShaderConstant.enable = false;
	}
	////////////////////////////////////////////////////////////////////////////////
	// LIGHT

	virtual void lightEnable( bool enable )
	{
		lighting.enable = true;
		lighting.data = enable;
		if ( ! useVertexShader ) setRenderState( D3DRS_LIGHTING, enable ? TRUE : FALSE );
	}
	virtual void lightEnable( int index, bool enable )
	{
		BOOL & e = prevLightEnable[index].data;
		if ( prevLightEnable[index].enable && 
			e == ( enable ? TRUE : FALSE ) ) 
			return;
		e = ( enable ? TRUE : FALSE );
		prevLightEnable[index].enable = true;
		if ( ! useVertexShader ) device->LightEnable( index, enable );
		VertexShaderConstant.enable = false;
	}
	virtual void lightAmbient( const vector4 & ambient )
	{
		this->ambient = ambient.getVector3();
		charBgra temp;
		temp.element.red = ( unsigned char ) ( 255 * ambient.x );
		temp.element.green = ( unsigned char ) ( 255 * ambient.y );
		temp.element.blue = ( unsigned char ) ( 255 * ambient.z );
		temp.element.alpha = 255;
		if ( ! useVertexShader ) setRenderState( D3DRS_AMBIENT, temp.all );
		VertexShaderConstant.enable = false;
	}
	virtual void lightPosition( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, float range, const vector3 & attenuation )
	{
		D3DLIGHT8 & l = light[index].data;
		if ( light[index].enable && 
			l.Type == D3DLIGHT_POINT &&
			compareF4( l.Ambient, ambient ) && 
			compareF4( l.Diffuse, diffuse ) && 
			compareF4( l.Specular, specular ) && 
			compareF3( l.Position, position ) && 
			l.Range == range && 
			l.Attenuation0 == attenuation.x && l.Attenuation1 == attenuation.y && l.Attenuation2 == attenuation.z )
			return;
		l.Type = D3DLIGHT_POINT;
		copyF4( l.Ambient, ambient );
		copyF4( l.Diffuse, diffuse );
		copyF4( l.Specular, specular );
		copyF3( l.Position, position );
		l.Range = range; 
		l.Attenuation0 = attenuation.x; l.Attenuation1 = attenuation.y; l.Attenuation2 = attenuation.z;
		light[index].enable = true;
		if ( ! useVertexShader ) device->SetLight( index, & l );
		VertexShaderConstant.enable = false;
	}
	virtual void lightDirection( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & direction )
	{
		D3DLIGHT8 & l = light[index].data;
		l.Range = 1.0f;
		if ( light[index].enable && 
			l.Type == D3DLIGHT_DIRECTIONAL &&
			compareF4( l.Ambient, ambient ) && 
			compareF4( l.Diffuse, diffuse ) && 
			compareF4( l.Specular, specular ) && 
			compareF3( l.Direction, direction ) )
			return;
		l.Type = D3DLIGHT_DIRECTIONAL;
		copyF4( l.Ambient, ambient );
		copyF4( l.Diffuse, diffuse );
		copyF4( l.Specular, specular );
		copyF3( l.Direction, direction );
		light[index].enable = true;
		if ( ! useVertexShader ) device->SetLight( index, & l );
		VertexShaderConstant.enable = false;
	}
	virtual void lightSpot( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, const vector3 & direction, float range, float fallOff, const vector3 & attenuation, float inside, float outside )
	{
		D3DLIGHT8 & l = light[index].data;
		if ( light[index].enable && 
			l.Type == D3DLIGHT_SPOT &&
			compareF4( l.Ambient, ambient ) && 
			compareF4( l.Diffuse, diffuse ) && 
			compareF4( l.Specular, specular ) && 
			compareF3( l.Direction, direction ) &&
			compareF3( l.Position, position ) && 
			l.Range == range && l.Falloff == fallOff &&
			l.Attenuation0 == attenuation.x && l.Attenuation1 == attenuation.y && l.Attenuation2 == attenuation.z &&
			l.Theta == inside && l.Phi == outside )
			return;
		l.Type = D3DLIGHT_SPOT;
		copyF4( l.Ambient, ambient );
		copyF4( l.Diffuse, diffuse );
		copyF4( l.Specular, specular );
		copyF3( l.Position, position );
		copyF3( l.Direction, direction );
		l.Range = range; l.Falloff = fallOff;
		l.Attenuation0 = attenuation.x; l.Attenuation1 = attenuation.y; l.Attenuation2 = attenuation.z;
		l.Theta = inside; l.Phi = outside;
		light[index].enable = true;
		if ( ! useVertexShader ) device->SetLight( index, & l );
		VertexShaderConstant.enable = false;
	}
	////////////////////////////////////////////////////////////////////////////////
	// MATRIX

	virtual void setViewport( int x, int y, int width, int height, float zNear, float zFar )
	{
		D3DVIEWPORT8 vp = { x, y, width, height, zNear, zFar };
		viewWidth = width; viewHeight = height;
		viewX = x; viewY = this->height - y - viewHeight;
		device->SetViewport( & vp );
		VertexShaderConstant.enable = false;
	}

	virtual void setMatrix( float * elements )
	{
		stackMatrix->LoadMatrix( ( D3DXMATRIX * ) elements );
		if ( useVertexShader ) {
			matrixWorld = * ( D3DXMATRIX * ) elements;
		} else {
			device->SetTransform( D3DTS_WORLD, ( D3DXMATRIX * ) elements );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void getMatrix( float * elements )
	{
		if ( useVertexShader ) {
			*(( D3DXMATRIX * ) elements) = matrixWorld;
		} else {
			*(( D3DXMATRIX * ) elements) = *(stackMatrix->GetTop());
		}
	}
	virtual void initMatrix()
	{
		stackMatrix->LoadIdentity();
		if ( useVertexShader ) {
			matrixWorld = unitMatrix;
		} else {
			device->SetTransform( D3DTS_WORLD, & unitMatrix );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void multiMatrix( float * elements )
	{
		stackMatrix->MultMatrix( ( D3DXMATRIX * ) elements );
		if ( useVertexShader ) {
			matrixWorld = *(stackMatrix->GetTop());
		} else {
			device->SetTransform( D3DTS_WORLD, stackMatrix->GetTop() );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void pushMatrix()
	{
		if ( useVertexShader ) {
			matrixWorld = *(stackMatrix->GetTop());
			stackMatrix->Push();
			stackMatrix->LoadMatrix( & matrixWorld );
		} else {
			D3DXMATRIX wk = *(stackMatrix->GetTop());
			stackMatrix->Push();
			stackMatrix->LoadMatrix( & wk );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void popMatrix()
	{
		stackMatrix->Pop();
		if ( useVertexShader ) {
			matrixWorld = *(stackMatrix->GetTop());
		} else {
			device->SetTransform( D3DTS_WORLD, stackMatrix->GetTop() );
		}
		VertexShaderConstant.enable = false;
	}

	virtual void initView()
	{
		if ( useVertexShader ) {
			matrixView = unitMatrix;
		} else {
			device->SetTransform( D3DTS_VIEW, & unitMatrix );
		}
		VertexShaderConstant.enable = false;
	}

	virtual void setView( const vector3 & eye, const vector3 & at, const vector3 & up )
	{
		( rightHand ? D3DXMatrixLookAtRH : D3DXMatrixLookAtLH )( & matrixView, ( const D3DXVECTOR3 * ) ( const float * ) eye, ( const D3DXVECTOR3 * ) ( const float * ) at, ( const D3DXVECTOR3 * ) ( const float * ) up );
		if ( useVertexShader ) {
		} else {
			device->SetTransform( D3DTS_VIEW, & matrixView );
		}
		this->eye = eye;
		this->at = at;
		this->up = up;
		VertexShaderConstant.enable = false;
	}
	virtual void getViewProjection( float * elements )
	{
		if ( useVertexShader ) {
			* ( ( D3DXMATRIX * ) elements ) = matrixView * matrixProjection;
		} else {
			D3DXMATRIX v, p;
			device->GetTransform( D3DTS_VIEW, & v );
			device->GetTransform( D3DTS_PROJECTION, & p );
			* ( ( D3DXMATRIX * ) elements ) = v * p;
		}
	}

	virtual void setPerspective( float fieldOfViewY, float aspect, float zNear, float zFar )
	{
		this->fieldOfViewY = fieldOfViewY;//[deg]設定はRadでおこなう
		this->sizeY = 0;
		this->aspect = aspect;
		this->zNear = zNear;
		this->zFar = zFar;
		( rightHand ? D3DXMatrixPerspectiveFovRH : D3DXMatrixPerspectiveFovLH )( & matrixProjection, deg2rad( fieldOfViewY ), aspect, zNear, zFar );
		if ( useVertexShader ) {
		} else {
			device->SetTransform( D3DTS_PROJECTION, & matrixProjection );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void setOrtho( float sizeY, float aspect, float zNear, float zFar )
	{
		this->fieldOfViewY = 0;
		this->sizeY = sizeY;
		this->aspect = aspect;
		this->zNear = zNear;
		this->zFar = zFar;
		( rightHand ? D3DXMatrixOrthoRH : D3DXMatrixOrthoLH )( & matrixProjection, sizeY * aspect, sizeY, zNear, zFar );
		if ( useVertexShader ) {
		} else {
			device->SetTransform( D3DTS_PROJECTION, & matrixProjection );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void setFrustum( const vector3 & frustumMinimum, const vector3 & frustumMaximum )
	{
		this->frustumMinimum = frustumMinimum;
		this->frustumMaximum = frustumMaximum;
		this->fieldOfViewY = 0;
		this->sizeY = 0;
		( rightHand ? D3DXMatrixPerspectiveOffCenterRH : D3DXMatrixPerspectiveOffCenterLH )
			( & matrixProjection, frustumMinimum.x, frustumMaximum.x, frustumMinimum.y, frustumMaximum.y, frustumMinimum.z, frustumMaximum.z );
		if ( useVertexShader ) {
		} else {
			device->SetTransform( D3DTS_PROJECTION, & matrixProjection );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void getAllMatrix( float * elements )
	{
		if ( useVertexShader ) {
		} else {
			device->GetTransform( D3DTS_WORLD, & matrixWorld );
			device->GetTransform( D3DTS_VIEW, & matrixView );
			device->GetTransform( D3DTS_PROJECTION, & matrixProjection );
		}
		* ( ( D3DXMATRIX * ) elements ) = matrixWorld * matrixView * matrixProjection;
	}
	virtual void initAllMatrix()
	{
		if ( useVertexShader ) {
			matrixWorld = unitMatrix;
			matrixView = unitMatrix;
			matrixProjection = unitMatrix;
		} else {
			device->SetTransform( D3DTS_WORLD, & unitMatrix );
			device->SetTransform( D3DTS_VIEW, & unitMatrix );
			device->SetTransform( D3DTS_PROJECTION, & unitMatrix );
		}
		VertexShaderConstant.enable = false;
	}
	virtual void getEachMatrix( float * world, float * view, float * projection )
	{
		if ( useVertexShader ) {
			if ( world ) * ( D3DXMATRIX * ) world = matrixWorld;
			if ( view ) * ( D3DXMATRIX * ) view = matrixView;
			if ( projection ) * ( D3DXMATRIX * ) projection = matrixProjection;
		} else {
			if ( world ) device->GetTransform( D3DTS_WORLD, ( D3DXMATRIX * ) world );
			if ( view ) device->GetTransform( D3DTS_VIEW, ( D3DXMATRIX * ) view );
			if ( projection ) device->GetTransform( D3DTS_PROJECTION, ( D3DXMATRIX * ) projection );
		}
	}
	virtual void setEachMatrix( float * world, float * view, float * projection )
	{
		if ( useVertexShader ) {
			if ( world ) matrixWorld = * ( D3DXMATRIX * ) world;
			if ( view ) matrixView = * ( D3DXMATRIX * ) view;
			if ( projection ) matrixProjection = * ( D3DXMATRIX * ) projection;
		} else {
			if ( world ) device->SetTransform( D3DTS_WORLD, ( D3DXMATRIX * ) world );
			if ( view ) device->SetTransform( D3DTS_VIEW, ( D3DXMATRIX * ) view );
			if ( projection ) device->SetTransform( D3DTS_PROJECTION, ( D3DXMATRIX * ) projection );
		}
		VertexShaderConstant.enable = false;
	}
	////////////////////////////////////////////////////////////////////////////////
	// Primitive

	virtual rVertexBuffer * createVertexBuffer( int size, int flag )
	{
		rVertexBuffer * result = NULL;
		if ( forceFlexibleVertex ) {
			int _flag = 0;
			if ( flag & renderInterface::rVertexBuffer::enableVertex ) _flag |= D3DFVF_XYZ;
			if ( flag & renderInterface::rVertexBuffer::enableNormal ) _flag |= D3DFVF_NORMAL;
			if ( flag & renderInterface::rVertexBuffer::enableDiffuse ) _flag |= D3DFVF_DIFFUSE;
			if ( flag & renderInterface::rVertexBuffer::enableSpecular ) _flag |= D3DFVF_SPECULAR;
			if ( flag & renderInterface::rVertexBuffer::enableTexture7 ) _flag |= D3DFVF_TEX8;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture6 ) _flag |= D3DFVF_TEX7;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture5 ) _flag |= D3DFVF_TEX6;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture4 ) _flag |= D3DFVF_TEX5;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture3 ) _flag |= D3DFVF_TEX4;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture2 ) _flag |= D3DFVF_TEX3;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture1 ) _flag |= D3DFVF_TEX2;
			else if ( flag & renderInterface::rVertexBuffer::enableTexture0 ) _flag |= D3DFVF_TEX1;
			result = new DX8VertexBufferFlexible( size, flag, _flag );
			( ( DX8VertexBufferFlexible * ) result )->create( device );
		} else if ( flag == renderInterface::rVertexBuffer::enableVNT0 ) {
			result = new DX8VertexBufferVERTEX( size );
			( ( DX8VertexBufferVERTEX * ) result )->create( device );
		} else if ( flag == renderInterface::rVertexBuffer::enableVDST0 ) {
			result = new DX8VertexBufferLVERTEX( size );
			( ( DX8VertexBufferLVERTEX * ) result )->create( device );
		}
		return result;
	}
	virtual rIndexBuffer * createIndexBuffer( int size )
	{
		DX8IndexBuffer16 * result = new DX8IndexBuffer16( size );
		result->create( device );
		return result;
	}
	inline void set( LPDIRECT3DVERTEXBUFFER8 vb, int size )
	{
		if ( vertexBuffer.enable && vertexBuffer.data == vb ) return;
		vertexBuffer.enable = true;
		vertexBuffer.data = vb;
		device->SetStreamSource( 0, vb, size );
	}
	inline void set( LPDIRECT3DINDEXBUFFER8 ib )
	{
		if ( indexBuffer.enable && indexBuffer.data == ib ) return;
		indexBuffer.enable = true;
		indexBuffer.data = ib;
		device->SetIndices( ib, 0 );
	}
	void setShader( DWORD fvf )
	{
		if ( useVertexShader ) {
			vertexShaderBuilder * shader = NULL;
			vertexShaderBuilder::Status status;
			status.fvf = fvf;
			status.texture[0] = ( fvf & D3DFVF_TEX1 ) ? vertexShaderBuilder::UVTextureType : vertexShaderBuilder::DisableTextureType;
			status.normal = ( fvf & D3DFVF_NORMAL ) ? true : false;
			status.diffuse = ( fvf & D3DFVF_DIFFUSE ) ? true : false;
			status.specular = ( fvf & D3DFVF_SPECULAR ) ? true : false;
			for ( int i = 0; i < 8; i++ ) {
				if ( light[i].enable && light[i].data.Type && prevLightEnable[i].enable && prevLightEnable[i].data && lighting.data ) {
					status.light[i] = ( int ) light[i].data.Type;
				}
			}
			for ( list< retainer<vertexShaderBuilder> >::iterator it( shaderList ); it; ++it ) {
				vertexShaderBuilder & vsb = it()();
				if ( status == vsb.status ) {
					shader = & vsb;
					break;
				}
			}
			if ( ! shader ) {
				shader = new vertexShaderBuilder( device );
				shaderList.push_back( shader );
				shader->status = status;
				shader->build();
				VertexShaderConstant.enable = false;
			}
			if ( prevVertexShader.enable == false || prevVertexShader.data != shader ) {
				prevVertexShader.enable = true;
				prevVertexShader.data = shader;
				device->SetVertexShader( shader->handle );
				VertexShaderConstant.enable = false;
			}
			if ( ! VertexShaderConstant.enable ) {
				D3DXVECTOR4 data[vertexShaderBuilder::cLightParameter];
				D3DXMATRIX & cWorldViewProjectionMatrix = * ( D3DXMATRIX * ) ( & data[vertexShaderBuilder::cWorldViewProjectionMatrix] );
				D3DXMATRIX & cWorldMatrix = * ( D3DXMATRIX * ) ( & data[vertexShaderBuilder::cWorldMatrix] );
				D3DXMATRIX & cViewMatrix = * ( D3DXMATRIX * ) ( & data[vertexShaderBuilder::cViewMatrix] );
				D3DXMATRIX & cTextureMatrix = * ( D3DXMATRIX * ) ( & data[vertexShaderBuilder::cTextureMatrix] );
				D3DXVECTOR4 & cEyePosition = data[vertexShaderBuilder::cEyePosition];
				D3DXVECTOR4 & cAmbientEmissive = data[vertexShaderBuilder::cAmbientEmissive];
				D3DXVECTOR4 & cDiffuse = data[vertexShaderBuilder::cDiffuse];
				D3DXVECTOR4 & cSpecular = data[vertexShaderBuilder::cSpecular];

				D3DXMATRIX worldViewProjection = matrixWorld * matrixView * matrixProjection;
				D3DXMatrixTranspose(&cWorldViewProjectionMatrix, &worldViewProjection);
				D3DXMatrixTranspose(&cWorldMatrix, &matrixWorld);
				D3DXMatrixTranspose(&cViewMatrix, &matrixView);
				cTextureMatrix = unitMatrix;

				cEyePosition = D3DXVECTOR4( eye[0], eye[1], eye[2], 1 );
				D3DMATERIAL8 & m = prevMaterial.data;
				cAmbientEmissive = D3DXVECTOR4( ambient[0] * m.Ambient.r + m.Emissive.r, ambient[1] * m.Ambient.g + m.Emissive.g, ambient[2] * m.Ambient.b + m.Emissive.b, maximum( m.Ambient.a, m.Emissive.a ) );
				cAmbientEmissive.x = clamp( 0.0f, cAmbientEmissive.x, 1.0f );
				cAmbientEmissive.y = clamp( 0.0f, cAmbientEmissive.y, 1.0f );
				cAmbientEmissive.z = clamp( 0.0f, cAmbientEmissive.z, 1.0f );
				cAmbientEmissive.w = clamp( 0.0f, cAmbientEmissive.w, 1.0f );
				cDiffuse = * ( D3DXVECTOR4 * ) & m.Diffuse;
				cSpecular = D3DXVECTOR4( m.Specular.r, m.Specular.g, m.Specular.b, m.Power );

				device->SetVertexShaderConstant( vertexShaderBuilder::cWorldViewProjectionMatrix, ( float * ) data[0], vertexShaderBuilder::cLightParameter );

				for ( int i = 0; i < 8; i++ ) {
					if ( ! ( light[i].enable && light[i].data.Type && prevLightEnable[i].enable && prevLightEnable[i].data && lighting.data ) ) continue;

					D3DLIGHT8 & l = light[i].data;
					D3DXVECTOR4 data[6];
					D3DXVECTOR4 & position = data[vertexShaderBuilder::offsetPositionAtWorld];
					D3DXVECTOR4 & direction = data[vertexShaderBuilder::offsetDirectionAtWorld];
					float & range = data[vertexShaderBuilder::offsetRange].w;
					D3DXVECTOR4 & attenuation = data[vertexShaderBuilder::offsetAttenuation];
					D3DXVECTOR4 & spot = data[vertexShaderBuilder::offsetSpotParam];
					D3DXVECTOR4 ld( l.Direction.x, l.Direction.y, l.Direction.z, 0 );
					D3DXVECTOR4 lp( l.Position.x, l.Position.y, l.Position.z, 1 );
					D3DXVec4Normalize( & ld, & ld );
					ld.w = 1;
					if ( l.Type == D3DLIGHT_DIRECTIONAL ) {
						direction = ld;
					} else if ( l.Type == D3DLIGHT_SPOT ) {
						direction = ld;
						position = lp;
						const float co = ( float ) ::cos( l.Phi / 2 );
						spot.x = co;
						if ( l.Phi == l.Theta ) {
							spot.y = 0;
							spot.z = 1;
						} else {
							const float ci = ( float ) ::cos( l.Theta  / 2 );
							const float factor = 1 / ( ci - co );
							spot.y = factor;
							spot.z = - co * factor;
						}
						spot.w = l.Falloff;
					} else if ( l.Type == D3DLIGHT_POINT ) {
						position = lp;
					}
					attenuation.x = l.Attenuation0;
					attenuation.y = l.Attenuation1;
					attenuation.z = l.Attenuation2;
					range = l.Range;
					D3DXVECTOR4 & diffuse = data[vertexShaderBuilder::offsetDiffuseColor];
					diffuse = * ( ( D3DXVECTOR4 * ) & l.Diffuse );
					D3DXVECTOR4 & specular = data[vertexShaderBuilder::offsetSpecularColor];
					specular = * ( ( D3DXVECTOR4 * ) & l.Specular );
					device->SetVertexShaderConstant( vertexShaderBuilder::cLightParameter + i * 6, ( float * ) data[0], 6 );
				}
				float cZeroHalfOneTwo[4] = { 0, 0.5, 1, 2 };
				device->SetVertexShaderConstant( vertexShaderBuilder::cZeroHalfOneTwo, cZeroHalfOneTwo, 1 );
				VertexShaderConstant.enable = true;
			}

			if ( usePixelShader ) {
				pixelShaderBuilder * shader = NULL;
				int texture = ( fvf & D3DFVF_TEXCOUNT_MASK ) >> D3DFVF_TEXCOUNT_SHIFT;
				for ( list< retainer<pixelShaderBuilder> >::iterator it( pixelShaderList ); it; ++it ) {
					pixelShaderBuilder & psb = it()();
					bool found = true;
					if ( psb.texture != texture ) { found = false; }
					if ( found ) {
						shader = & psb;
						break;
					}
				}
				if ( ! shader ) {
					shader = new pixelShaderBuilder( device );
					pixelShaderList.push_back( shader );
					shader->texture = texture;
					shader->build();
				}
				if ( prevPixelShader.enable == false || prevPixelShader.data != shader ) {
					prevPixelShader.enable = true;
					prevPixelShader.data = shader;
					device->SetPixelShader( shader->handle );
				}
			}
		} else {
			if ( prevDefaultVertexShader.enable && prevDefaultVertexShader.data == fvf ) return;
			prevVertexShader.enable = false;
			prevDefaultVertexShader.enable = true;
			prevDefaultVertexShader.data = fvf;
			device->SetVertexShader( fvf );
		}
	}
	virtual void drawList( rVertex * vertices, int size, int offset = 0 )
	{
		for ( int i = 0; i < size; i++ ) {
			memcpy( vertexBufferTemp->data, vertices + offset + i * 3, sizeof( rVertex ) * 3 ); 
			vertexBufferTemp->update();
			drawList( vertexBufferTemp, 1 );
		}
	}
	virtual void drawList( rVertex * vertices, int size, rIndexBuffer::element * indices, int offset = 0 )
	{
		for ( int i = 0; i < size; i++ ) {
			memcpy( vertexBufferTemp->data.data + 0, vertices + indices[offset+i*3+0], sizeof( rVertex ) ); 
			memcpy( vertexBufferTemp->data.data + 1, vertices + indices[offset+i*3+1], sizeof( rVertex ) ); 
			memcpy( vertexBufferTemp->data.data + 2, vertices + indices[offset+i*3+2], sizeof( rVertex ) ); 
			vertexBufferTemp->update();
			drawList( vertexBufferTemp, 1 );
		}
	}
	virtual void drawList( rVertexBuffer * vertices, int size, int offset = 0 )
	{
		if ( forceFlexibleVertex ) {
			DX8VertexBufferFlexible * vb = ( DX8VertexBufferFlexible * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) 
					drawList( vertices->data, vertices->data.size );
				return;
			}
			lightEnable( ( vb->fvf & D3DFVF_NORMAL ) ? true : false );
			setShader( vb->fvf );
			set( vb->buffer, vb->sizeofVertex );
			device->DrawPrimitive( D3DPT_TRIANGLELIST, offset, size );
		} else if ( vertices->flag == rVertexBuffer::enableVNT0 ) {
			DX8VertexBufferVERTEX * vb = ( DX8VertexBufferVERTEX * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) 
					drawList( vertices->data, vertices->data.size );
				return;
			}
			setShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );
			set( vb->buffer, sizeof( DX8VertexBufferVERTEX::VERTEX ) );
			device->DrawPrimitive( D3DPT_TRIANGLELIST, offset, size );
		} else if ( vertices->flag == rVertexBuffer::enableVDST0 ) {
			DX8VertexBufferLVERTEX * vb = ( DX8VertexBufferLVERTEX * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) 
					drawList( vertices->data, vertices->data.size );
				return;
			}
			lightEnable( false );
			setShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 );
			set( vb->buffer, sizeof( DX8VertexBufferLVERTEX::VERTEX ) );
			device->DrawPrimitive( D3DPT_TRIANGLELIST, offset, size );
		}
	}
	virtual void drawList( rVertexBuffer * vertices, rIndexBuffer * indecies, int size, int offset = 0 )
	{
		if ( forceFlexibleVertex ) {
			DX8VertexBufferFlexible * vb = ( DX8VertexBufferFlexible * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) {
					drawList( vertices->data, vertices->data.size, indecies->data );
				}
				return;
			}
			lightEnable( ( vb->fvf & D3DFVF_NORMAL ) ? true : false );
			setShader( vb->fvf );
			set( vb->buffer, vb->sizeofVertex );
		} else if ( vertices->flag == rVertexBuffer::enableVNT0 ) {
			DX8VertexBufferVERTEX * vb = ( DX8VertexBufferVERTEX * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) {
					drawList( vertices->data, vertices->data.size, indecies->data );
				}
				return;
			}
			setShader( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );
			set( vb->buffer, sizeof( DX8VertexBufferVERTEX::VERTEX ) );
		} else if ( vertices->flag == rVertexBuffer::enableVDST0 ) {
			DX8VertexBufferLVERTEX * vb = ( DX8VertexBufferLVERTEX * ) vertices;
			if ( ! vb->buffer ) {
				if ( vertices != vertexBufferTemp ) {
					drawList( vertices->data, vertices->data.size, indecies->data );
				}
				return;
			}
			lightEnable( false );
			setShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 );
			set( vb->buffer, sizeof( DX8VertexBufferLVERTEX::VERTEX ) );
		} else {
			return;
		}
		if ( ! ( ( DX8IndexBuffer16 * ) indecies )->buffer ) return;
		set( ( ( DX8IndexBuffer16 * ) indecies )->buffer );

		UINT MinIndex = 0;
		UINT NumVertices = vertices->data.size;
		UINT StartIndex = offset;
		device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, MinIndex, NumVertices, StartIndex, size );
	}
};

renderInterface * renderInterface::createDirectX8Render() { return new rDirectX8Version(); }

#endif
