#ifdef __GLOBAL__

//OGL
};
#include <gl/gl.h>
#include <gl/glu.h>
//#include <gl/glaux.h>
namespace agl {
#pragma comment( lib, "Opengl32.lib" )
#pragma comment( lib, "Glu32.lib" )
//#pragma comment( lib, "GLAUX.lib" )

class rOpenGLVersion : public renderInterface
{
public:
	enum constantValue
	{
		lightNumber = 8,
		renderStateMaxNumber = 0xffff + 1,
	};
	class Material
	{
	public:
		float ambient[4], diffuse[4], specular[4], emissive[4], power;
	};
	class Light
	{
	public:
		int type;
		float ambient[4], diffuse[4], specular[4];
		float position[4], direction[4];
		float range, fallOff;
		float attenuation[3];
		float theta, phi;
	};
	typedef GLuint GLTexture;

	cache<Material> prevMaterial;
	cache<Light> light[lightNumber];
	cache<bool> renderState[renderStateMaxNumber];
	cache<GLTexture> prevTexture;

	rTexture * whiteTexture, * blackTexture;

	HDC hdc;
	HGLRC hrc;
	HWND hwnd;

	rOpenGLVersion() 
		: whiteTexture( NULL ), blackTexture( NULL ),
		hdc( NULL ), hrc( NULL ), hwnd( NULL )
	{
	}
	virtual ~rOpenGLVersion()
	{
		finalize();
	}
	virtual void clearCache()
	{
		prevTexture.enable = prevMaterial.enable = false;
		int i;
		for ( i = 0; i < lightNumber; i++ ) {
			light[i].enable = false;
		}
		for ( i = 0; i < renderStateMaxNumber; i++ ) {
			renderState[i].enable = false;
		}
	}
	virtual bool initialized()
	{
		return hrc ? true : false;
	}
	void SetCurrent( bool set )
	{
		glFinish();
		if ( set ) { wglMakeCurrent( hdc, hrc );
		} else { wglMakeCurrent( NULL, NULL ); }
		glFinish();
	}
	bool SetupFormat()
	{
		PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR) };
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | // replaces PFD_DRAW_TO_BITMAP PFD_DRAW_TO_WINDOW
					  PFD_SUPPORT_OPENGL;// | PFD_DOUBLEBUFFER;// not need PFD_SUPPORT_GDI; PFD_DOUBLEBUFFER
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE; 
		int pixelformat;
		if ( ( pixelformat = ChoosePixelFormat( hdc, &pfd ) ) == 0 ) return false;
		if ( SetPixelFormat( hdc, pixelformat, &pfd ) == FALSE ) return false;
		DescribePixelFormat( hdc, GetPixelFormat( hdc ), sizeof( PIXELFORMATDESCRIPTOR ), &pfd );

		glFinish();
		return true;
	}
	virtual bool initialize( void * hwnd, bool /*window*/, int width, int height )
	{
		finalize();
		this->width = width;
		this->height = height;
		this->hwnd = ( HWND ) hwnd;
		hdc = GetDC( ( HWND ) hwnd );

		if ( ! SetupFormat() ) return false;

		hrc = wglCreateContext( hdc );
		if ( ! hrc ) return false;

		activate( true );

		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();

		initMatrix();
		pushMatrix();
		initMatrix();

		setRenderState( GL_TEXTURE_2D, true );
		setTextureParameter();

	//	float one[4]={1,1,1,1};
	//	glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, one );// GL_TEX_ENV_COLOR
		
	//	setRenderState( GL_COLOR_MATERIAL, true );// GL_COLOR_MATERIAL
	//	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );// GL_COLOR_MATERIAL_PARAMETER. GL_COLOR_MATERIAL_FACE

		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);


		if ( fillmode ) {
			glPolygonMode( GL_FRONT, GL_FILL );//GL_FILL
			glPolygonMode( GL_BACK, GL_FILL );//GL_LINE
		} else {
			glPolygonMode( GL_FRONT, GL_LINE );//GL_FILL
			glPolygonMode( GL_BACK, GL_LINE );//GL_LINE
		}
		glShadeModel( GL_SMOOTH );
		setRenderState( GL_BLEND, true );
		setRenderState( GL_SCISSOR_TEST, true );

		cull( culling, false );

		unsigned long whitePixel = 0xffffffff;
		whiteTexture = createTexture( "whiteTexture", 1, 1, ( unsigned long * ) & whitePixel, 0 );
		unsigned long blackPixel = 0xff000000;
		blackTexture = createTexture( "black_texture", 1, 1, ( unsigned long * ) & blackPixel, 0 );

		activate( false );
		return true;
	}
	virtual void finalize()
	{
		activate( false );
        delete whiteTexture; whiteTexture = NULL;
        delete blackTexture; blackTexture = NULL;
		if ( hrc ) { wglDeleteContext( hrc ); hrc = NULL; }
		if ( hdc ) { ReleaseDC( hwnd, hdc ); hwnd = NULL; hdc = NULL; }
	}
	int setRenderState( int State, bool Value )
	{
		if ( State < renderStateMaxNumber ) {
			if ( renderState[State].enable && renderState[State].data == Value ) return 0;
			renderState[State].enable = true;
			renderState[State].data = Value;
		}
		if ( Value ) {
			glEnable( State );
		} else {
			glDisable( State );
		}
		return 0;
	}

	virtual void startScene( DWORD clearColor )
	{
		depthTest( true, true, testFuncLess );
		glClearColor( ( clearColor & 0xff ) / 255.0f, 
			( ( clearColor >> 8 ) & 0xff ) / 255.0f, 
			( ( clearColor >> 16 ) & 0xff ) / 255.0f, 
			( ( clearColor >> 24 ) & 0xff ) / 255.0f );

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glFinish();
		initMatrix();
	}
	void activate( bool active )
	{
		SetCurrent( active );
	}

	virtual void stencilClear()
	{
		glClearStencil( 0 );
	}
	virtual void finishScene( bool swapBuffer )
	{
		if ( swapBuffer ) SwapBuffers( hdc );
	}
	virtual void swapBuffer()
	{
		SwapBuffers( hdc );
	}

	virtual void cull( bool enable, bool cw )
	{
		setRenderState( GL_CULL_FACE, enable );
		if ( enable ) {
			glFrontFace( cw ? GL_CW : GL_CCW ); // GL_CW
			glCullFace( GL_BACK );
		}
	}
	virtual void colorTest( bool red, bool green, bool blue, bool alpha )
	{
		glColorMask( unsigned char( red ? GL_TRUE : GL_FALSE ), 
			unsigned char( green ? GL_TRUE : GL_FALSE ),
			unsigned char( blue ? GL_TRUE : GL_FALSE ),
			unsigned char( alpha ? GL_TRUE : GL_FALSE ) );
	}

	virtual void readFrontBuffer( unsigned long * buffer )
	{
		glReadBuffer( GL_FRONT );
		glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
 	}

	virtual void readBackBuffer( unsigned long * buffer )
	{
		glReadBuffer( GL_BACK );
		glReadPixels( 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	}

	virtual int getBackBufferPitch()
	{
		return 0;
	}

	virtual void blend( int destination, int source )
	{
		static int table[blendFactorNum] = { 0,
			GL_ZERO, GL_ONE, 
			GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
			GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
			GL_SRC_ALPHA_SATURATE, 0, 0
		};

		glBlendFunc( table[source], table[destination] );
	}

	virtual void depthTest( bool testing, bool writing, int testFunc )
	{
		static int table[testFuncTypeNum] = { 0,
			GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL,
			GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
		setRenderState( GL_DEPTH_TEST, testing );
		glDepthFunc( table[testFunc] );
		glDepthMask( writing );
	}
	virtual void alphaTest( bool testing, float reference, int testFunc )
	{
		static int table[testFuncTypeNum] = { 0,
			GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL,
			GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
		setRenderState( GL_ALPHA_TEST, testing ? true : false );
		glAlphaFunc( table[testFunc], reference );
	}
	virtual void stencilTest( bool /*testing*/, unsigned long reference, unsigned long mask, unsigned long writeMask, int opFail, int opZFail, int opPass, int testFunc )
	{
		static int funcTable[testFuncTypeNum] = { 0,
			GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL,
			GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS };
		static int opTable[stencilOpNum] = { 0,
			GL_KEEP, GL_ZERO, GL_REPLACE, 0, 0,
			GL_INVERT, GL_INCR, GL_DECR };
		glStencilFunc( funcTable[testFunc], reference, mask );
		glStencilOp( opTable[opFail], opTable[opZFail], opTable[opPass] );
		glStencilMask( writeMask );
	}
	////////////////////////////////////////////////////////////////////////////////
	// TEXTURE
	class OGLTexture : public renderInterface::rTexture
	{
	public:
		GLTexture texture;
		bool transparency;
		char filename[MAX_PATH];
		OGLTexture() : transparency( false )
		{
			glGenTextures( 1, & texture );
			filename[0] = '\0';
			name = filename;
		}
		~OGLTexture()
		{
			glDeleteTextures( 1, & texture );
		}
		virtual bool isTransparency() { return transparency; }
	};
	void setTextureParameter()
	{
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glPixelStorei( GL_PACK_ALIGNMENT, 1 );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	}
	void createTexture( GLTexture & texture, int width, int height, unsigned long * pixels )
	{
		glBindTexture( GL_TEXTURE_2D, texture );
		setTextureParameter();
		array<unsigned long> wk( width * height );
		for ( int y = 0; y < height; y++ ) memcpy( wk.data + width * ( height - y - 1 ), pixels + width * y, width * 4 );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, wk.data );//GL_BITMAP
	}

	virtual rTexture * createTexture( const char * tag, int width, int height, unsigned long * pixels, int /*level*/ )
	{
		OGLTexture * result = new OGLTexture();
		strcpy( result->filename, tag );
		//charBgra charRgba
		charBgra * srcHead = ( charBgra * ) pixels, * src;
		charRgba * dst, * sentinel, * temp = new charRgba[width*height];
		for ( src = srcHead, dst = temp, sentinel = dst + width * height; dst != sentinel; ++src, ++dst ) {
			dst->element.red = src->element.red;
			dst->element.green = src->element.green;
			dst->element.blue = src->element.blue;
			dst->element.alpha = src->element.alpha;
			if ( dst->element.alpha != 255 ) result->transparency = true;
		}
		createTexture( result->texture, width, height, ( unsigned long * ) temp );
		delete [] temp;
		return result;
	}

	void setTexture( int /*index*/, GLTexture texture )
	{
		if ( prevTexture.enable && prevTexture.data == texture ) return;
		prevTexture.enable = true;
		prevTexture.data = texture;
		glBindTexture( GL_TEXTURE_2D, texture );
		setTextureParameter();
	}
	virtual void setTexture( int index, rTexture * texture )
	{
		OGLTexture * oglt = ( OGLTexture * ) texture;
		if ( ! texture ) texture = whiteTexture;

		setTexture( index, oglt ? oglt->texture : 0 );
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
	inline bool compareF4( const float left[4], const float right[4] )
	{ return ( left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3] ) ? true : false; }
	inline void copyF4( float dst[4], const float src[4] )
	{ dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; }
	inline bool compareF3( const float left[3], const float right[3] )
	{ return ( left[0] == right[0] && left[1] == right[1] && left[2] == right[2] ) ? true : false; }
	inline void copyF3( float dst[3], const float src[3] )
	{ dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }

	inline bool compareF4( const float left[4], const vector4 & right )
	{ return ( left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3] ) ? true : false; }
	inline void copyF4( float dst[4], const vector4 & src )
	{ dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; }
	inline bool compareF3( const float left[3], const vector4 & right )
	{ return ( left[0] == right[0] && left[1] == right[1] && left[2] == right[2] ) ? true : false; }
	inline void copyF3( float dst[3], const vector4 & src )
	{ dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }
	inline bool compareF3( const float left[3], const vector3 & right )
	{ return ( left[0] == right[0] && left[1] == right[1] && left[2] == right[2] ) ? true : false; }
	inline void copyF3( float dst[3], const vector3 & src )
	{ dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }
	////////////////////////////////////////////////////////////////////////////////
	// MATERIAL
	virtual void material( const vector3 & ambient, const vector3 & diffuse, const vector3 & specular, const vector3 & emissive, decimal transparency, decimal specularPower )
	{
		Material & m = prevMaterial.data;
		if ( prevMaterial.enable && 
			compareF3( m.ambient, ambient ) &&
			compareF3( m.diffuse, diffuse ) &&
			compareF3( m.specular, specular ) &&
			compareF3( m.emissive, emissive ) &&
			m.diffuse[3] == 1 - transparency &&
			m.power == specularPower ) 
			return;
		copyF3( m.ambient, ambient );
		copyF3( m.diffuse, diffuse );
		copyF3( m.specular, specular );
		copyF3( m.emissive, emissive );
		m.power = specularPower;
		m.ambient[3] = 1 - transparency;
		m.diffuse[3] = 1 - transparency;
		m.specular[3] = 0;
		m.emissive[3] = 0;
		prevMaterial.enable = true;
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, m.ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, m.diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, m.specular );
		glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, m.emissive );
		glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, m.power );
	}
	////////////////////////////////////////////////////////////////////////////////
	// LIGHT

	virtual void lightEnable( bool enable )
	{
		setRenderState( GL_LIGHTING, enable );
	}
	virtual void lightEnable( int index, bool enable )
	{
		setRenderState( GL_LIGHT0 + index, enable );
	}
	virtual void lightAmbient( const vector4 & ambient )
	{
		glLightfv( GL_LIGHT0 + 0, GL_AMBIENT, ambient );
	}
	virtual void lightPosition( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, float range, const vector3 & attenuation )
	{
		Light & l = light[index].data;
		if ( light[index].enable && 
			l.type == 0x1111 &&
			compareF4( l.ambient, ambient ) &&
			compareF4( l.diffuse, diffuse ) &&
			compareF4( l.specular, specular ) &&
			compareF3( l.position, position ) &&
			l.range == range && 
			l.attenuation[0] == attenuation.x && l.attenuation[1] == attenuation.y && l.attenuation[2] == attenuation.z )
			return;
		l.type = 0x1111;
		copyF4( l.ambient, ambient );
		copyF4( l.diffuse, diffuse );
		copyF4( l.specular, specular );
		copyF3( l.position, position );
		l.range = range; 
		l.attenuation[0] = attenuation.x; l.attenuation[1] = attenuation.y; l.attenuation[2] = attenuation.z;
		light[index].enable = true;

		l.position[3] = 1;

		glLightfv( GL_LIGHT0 + index, GL_AMBIENT, l.ambient );
		glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, l.diffuse );
		glLightfv( GL_LIGHT0 + index, GL_SPECULAR, l.specular );
		glLightfv( GL_LIGHT0 + index, GL_POSITION, l.position );
		glLightf( GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION, l.attenuation[0] );
		glLightf( GL_LIGHT0 + index, GL_LINEAR_ATTENUATION, l.attenuation[1] );
		glLightf( GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, l.attenuation[2] );
	}
	virtual void lightDirection( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & direction )
	{
		Light & l = light[index].data;
		l.range = 1.0f;
		if ( light[index].enable && 
			l.type == 0x2222 &&
			compareF4( l.ambient, ambient ) &&
			compareF4( l.diffuse, diffuse ) &&
			compareF4( l.specular, specular ) &&
			compareF3( l.direction, direction ) )
			return;
		l.type = 0x2222;
		copyF4( l.ambient, ambient );
		copyF4( l.diffuse, diffuse );
		copyF4( l.specular, specular );
		copyF3( l.direction, direction );
		light[index].enable = true;

		//float d[4] = { -l.direction[0], -l.direction[1], -l.direction[2], 0 };
		float d[4] = { l.direction[0], l.direction[1], l.direction[2], 0 };

		glLightfv( GL_LIGHT0 + index, GL_AMBIENT, l.ambient );
		glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, l.diffuse );
		glLightfv( GL_LIGHT0 + index, GL_SPECULAR, l.specular );
		glLightfv( GL_LIGHT0 + index, GL_POSITION, d );
	}
	virtual void lightSpot( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, const vector3 & direction, float range, float fallOff, const vector3 & attenuation, float inside, float outside )
	{
		Light & l = light[index].data;
		if ( light[index].enable && 
			l.type == 0x3333 &&
			compareF4( l.ambient, ambient ) &&
			compareF4( l.diffuse, diffuse ) &&
			compareF4( l.specular, specular ) &&
			compareF3( l.direction, direction ) &&
			compareF3( l.position, position ) &&
			l.range == range && l.fallOff == fallOff &&
			l.attenuation[0] == attenuation.x && l.attenuation[1] == attenuation.y && l.attenuation[2] == attenuation.z &&
			l.theta == inside && l.phi == outside )
			return;
		l.type = 0x3333;
		copyF4( l.ambient, ambient );
		copyF4( l.diffuse, ambient );
		copyF4( l.specular, ambient );
		copyF3( l.position, position );
		copyF3( l.direction, direction );
		l.range = range; l.fallOff = fallOff;
		l.attenuation[0] = attenuation.x; l.attenuation[1] = attenuation.y; l.attenuation[2] = attenuation.z;
		l.theta = inside; l.phi = outside;
		light[index].enable = true;

		//float d[4] = { -l.direction[0], -l.direction[1], -l.direction[2], 0 };
		float d[4] = { l.direction[0], l.direction[1], l.direction[2], 0 };
		l.position[3] = 1;

		glLightfv( GL_LIGHT0 + index, GL_AMBIENT, l.ambient );
		glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, l.diffuse );
		glLightfv( GL_LIGHT0 + index, GL_SPECULAR, l.specular );
		glLightfv( GL_LIGHT0 + index, GL_POSITION, l.position );
		glLightfv( GL_LIGHT0 + index, GL_SPOT_DIRECTION, d );

		glLightf( GL_LIGHT0 + index, GL_SPOT_EXPONENT, l.fallOff );
		glLightf( GL_LIGHT0 + index, GL_SPOT_CUTOFF, l.range );
		glLightf( GL_LIGHT0 + index, GL_CONSTANT_ATTENUATION, l.attenuation[0] );
		glLightf( GL_LIGHT0 + index, GL_LINEAR_ATTENUATION, l.attenuation[1] );
		glLightf( GL_LIGHT0 + index, GL_QUADRATIC_ATTENUATION, l.attenuation[2] );
	}
	////////////////////////////////////////////////////////////////////////////////
	// MATRIX

	virtual void setViewport( int x, int y, int width, int height, float zNear, float zFar )
	{
		viewWidth = width; viewHeight = height;
		viewX = x; viewY = this->height - y - viewHeight;
		glViewport( viewX, viewY, viewWidth, viewHeight );
		glScissor( viewX, viewY, viewWidth, viewHeight );
		glDepthRange( zNear, zFar );
 	}
	void transpose( float * dst )
	{
		float src[16];
		memcpy( src, dst, 16 * sizeof( float ) );
							dst[ 4] = src[ 1];	dst[ 8] = src[ 2];	dst[12] = src[ 3]; 
		dst[ 1] = src[ 4];						dst[ 9] = src[ 6];	dst[13] = src[ 7]; 
		dst[ 2] = src[ 8];	dst[ 6] = src[ 9];						dst[14] = src[11]; 
		dst[ 3] = src[12];	dst[ 7] = src[13];	dst[11] = src[14];
	}
	virtual void setMatrix( float * elements )
	{
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( elements );
	}
	virtual void getMatrix( float * elements )
	{
		glMatrixMode( GL_MODELVIEW );
		glGetFloatv( GL_MODELVIEW_MATRIX, elements );
	}
	virtual void initMatrix()
	{
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
	}
	virtual void multiMatrix( float * elements )
	{
		glMatrixMode( GL_MODELVIEW );
		glMultMatrixf( elements );
	}
	virtual void pushMatrix()
	{
		float elements[16];
		glMatrixMode( GL_MODELVIEW );
		glGetFloatv( GL_MODELVIEW_MATRIX, elements );
		glPushMatrix();
		glLoadMatrixf( elements );
	}
	virtual void popMatrix()
	{
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
	}

	virtual void initView()
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
	}

	virtual void setView( const vector3 & eye, const vector3 & at, const vector3 & up )
	{
		this->eye = eye;
		this->at = at;
		this->up = up;
		setViewProjectionMatrix();
	}
	virtual void getViewProjection( float * elements )
	{
		glMatrixMode( GL_PROJECTION );
		glGetFloatv( GL_PROJECTION_MATRIX, elements );
	}
	virtual void setPerspective( float fieldOfViewY, float aspect, float zNear, float zFar )
	{
		this->fieldOfViewY = fieldOfViewY;//[deg]
		this->sizeY = 0;
		this->aspect = aspect;
		this->zNear = zNear;
		this->zFar = zFar;
		setViewProjectionMatrix();
	}
	virtual void setOrtho( float sizeY, float aspect, float zNear, float zFar )
	{
		this->fieldOfViewY = 0;
		this->sizeY = sizeY;
		this->aspect = aspect;
		this->zNear = zNear;
		this->zFar = zFar;
		setViewProjectionMatrix();
	}
	virtual void setFrustum( const vector3 & frustumMinimum, const vector3 & frustumMaximum )
	{
		this->frustumMinimum = frustumMinimum;
		this->frustumMaximum = frustumMaximum;
		this->fieldOfViewY = 0;
		this->sizeY = 0;
		setViewProjectionMatrix();
	}
	virtual void getAllMatrix( float * elements )
	{
		float wk[16];
		glMatrixMode( GL_MODELVIEW );
		glGetFloatv( GL_MODELVIEW_MATRIX, elements );
		glMatrixMode( GL_PROJECTION );
		glGetFloatv( GL_PROJECTION_MATRIX, wk );
		glMultMatrixf( elements );
		glGetFloatv( GL_PROJECTION_MATRIX, elements );
		glLoadMatrixf( wk );
	}
	virtual void initAllMatrix()
	{
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
	}
	void setViewProjectionMatrix()
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		if ( fieldOfViewY ) {
			gluPerspective( fieldOfViewY, aspect, zNear, zFar );
		} else if ( sizeY ) {
			const float sizeX = sizeY * aspect;
			glOrtho( - sizeX / 2, sizeX / 2, - sizeY / 2, sizeY / 2, zNear, zFar );
		} else {
			glFrustum( frustumMinimum.x, frustumMaximum.x, frustumMinimum.y, frustumMaximum.y, frustumMinimum.z, frustumMaximum.z );
		}
		gluLookAt( eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1], up[2] );
		if ( fieldOfViewY ) {
			float elements[16] = { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			glMultMatrixf( elements );
		}
	}
	virtual void getEachMatrix( float * world, float * view, float * projection )
	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		if ( fieldOfViewY ) {
			gluPerspective( fieldOfViewY, aspect, zNear, zFar );
		} else if ( sizeY ) {
			const float sizeX = sizeY * aspect;
			glOrtho( - sizeX / 2, sizeX / 2, - sizeY / 2, sizeY / 2, zNear, zFar );
		} else {
			glFrustum( frustumMinimum.x, frustumMaximum.x, frustumMinimum.y, frustumMaximum.y, frustumMinimum.z, frustumMaximum.z );
		}

		if ( projection ) glGetFloatv( GL_PROJECTION_MATRIX, projection );

		glLoadIdentity();
		gluLookAt( eye[0], eye[1], eye[2], at[0], at[1], at[2], up[0], up[1], up[2] );
		if ( fieldOfViewY ) {
			float elements[16] = { -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			glMultMatrixf( elements );
		}
		if ( view ) glGetFloatv( GL_PROJECTION_MATRIX, view );
	
		setViewProjectionMatrix();

		glMatrixMode( GL_MODELVIEW );
		if ( world ) glGetFloatv( GL_MODELVIEW_MATRIX, world );
	}
	virtual void setEachMatrix( float * world, float * view, float * projection )
	{
		glMatrixMode( GL_MODELVIEW );
		if ( world ) glLoadMatrixf( world );
		glMatrixMode( GL_PROJECTION );
		if ( view && projection ) {
			glLoadMatrixf( view );
			glMultMatrixf( projection );
		} else if ( view ) {
			glLoadMatrixf( view );
		} else if ( projection ) {
			glLoadMatrixf( projection );
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	// Primitive
	virtual rVertexBuffer * createVertexBuffer( int size, int flag )
	{
		return new rVertexBuffer( size, flag );
	}
	virtual rIndexBuffer * createIndexBuffer( int size )
	{
		return new rIndexBuffer( size );
	}
	virtual void drawList( rVertex * vertices, int size, int offset = 0 )
	{
		if ( 0 ) {
			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 4, GL_FLOAT, sizeof( rVertex ), & vertices->diffuse[0] );
		}
		if ( 1 ) {
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glTexCoordPointer( 2, GL_FLOAT, sizeof( rVertex ), & vertices->texture[0][0] );
		}
		if ( 1 ) {
			glEnableClientState( GL_NORMAL_ARRAY );
			glNormalPointer( GL_FLOAT, sizeof( rVertex ), & vertices->normal[0] );
		}
		if ( 1 ) {
			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 3, GL_FLOAT, sizeof( rVertex ), & vertices->position[0] );
		}

		glDrawArrays( GL_TRIANGLES, offset, size * 3 );

		glDisableClientState( GL_COLOR_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
	}
	virtual void drawList( rVertexBuffer * vertices, int size, int offset = 0 )
	{
		if ( vertices->flag & rVertexBuffer::enableDiffuse ) {
			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 4, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].diffuse[0] );
		}
		if ( vertices->flag & rVertexBuffer::enableTexture0 ) {
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glTexCoordPointer( 2, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].texture[0][0] );
		}
		if ( vertices->flag & rVertexBuffer::enableNormal ) {
			glEnableClientState( GL_NORMAL_ARRAY );
			glNormalPointer( GL_FLOAT, sizeof( rVertex ), & vertices->data[0].normal[0] );
		}
		if ( vertices->flag & rVertexBuffer::enableVertex ) {
			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 3, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].position[0] );
		}

		glDrawArrays( GL_TRIANGLES, offset, size * 3 );

		glDisableClientState( GL_COLOR_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
	}
	virtual void drawList( rVertexBuffer * vertices, rIndexBuffer * indecies, int size, int offset = 0 )
	{
		if ( vertices->flag & rVertexBuffer::enableDiffuse ) {
			glEnableClientState( GL_COLOR_ARRAY );
			glColorPointer( 4, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].diffuse[0] );
		}
		if ( vertices->flag & rVertexBuffer::enableTexture0 ) {
			glEnableClientState( GL_TEXTURE_COORD_ARRAY );
			glTexCoordPointer( 2, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].texture[0][0] );
		}
		if ( vertices->flag & rVertexBuffer::enableNormal ) {
			glEnableClientState( GL_NORMAL_ARRAY );
			glNormalPointer( GL_FLOAT, sizeof( rVertex ), & vertices->data[0].normal[0] );
		}
		if ( vertices->flag & rVertexBuffer::enableVertex ) {
			glEnableClientState( GL_VERTEX_ARRAY );
			glVertexPointer( 3, GL_FLOAT, sizeof( rVertex ), & vertices->data[0].position[0] );
		}

		glBegin( GL_TRIANGLES );
		for ( int i = 0; i < size * 3; i++ ) {
			glArrayElement( indecies->data[i+offset] );
		}
		glEnd();

		glDisableClientState( GL_COLOR_ARRAY );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
		glDisableClientState( GL_NORMAL_ARRAY );
		glDisableClientState( GL_VERTEX_ARRAY );
	}
};

renderInterface * renderInterface::createOpenGLRender() { return new rOpenGLVersion(); }

#endif
