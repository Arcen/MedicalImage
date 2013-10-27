////////////////////////////////////////////////////////////////////////////////
// Render

class renderInterface
{
public:
	template<class T>
	class cache
	{
	public:
		T data;
		bool enable;
		cache() : enable( false ) {}
		void clear() { enable = false; }
	};

#define colorStructure(name,a,b,c,d); \
	union name { \
		struct { unsigned char a, b, c, d; } element; \
		unsigned long all; \
	};

	colorStructure( charArgb, alpha, red, green, blue );
	colorStructure( charAbgr, alpha, blue, green, red );
	colorStructure( charRgba, red, green, blue, alpha );
	colorStructure( charBgra, blue, green, red, alpha );

	//�f�t�H���g�v���p�e�B
	int width, height;
	int viewX, viewY, viewWidth, viewHeight;
	bool rightHand, culling, cw, fillmode;
	float fieldOfViewY, aspect, zNear, zFar, sizeY;//perspective/ortho��fovy��sizey���O�łȂ����ɂ��ꂼ�ꓮ�삷��
	vector3 frustumMinimum, frustumMaximum;//frustum��fovy��sizey���O�̎��ɓ��삷��
	vector3 eye, at, up;
	//�I�u�W�F�N�g�̐����͂��̃N���X�̉��Œ�`�����֐��ōs��
	renderInterface() 
		: width( 0 ), height( 0 ), viewX( 0 ), viewY( 0 ), viewWidth( 0 ), viewHeight( 0 ), 
		rightHand( true ), culling( true ), cw( true ), fillmode( true ),
		fieldOfViewY( 1 ), aspect( 1 ), zNear( 0.001f ), zFar( 1000.0f ), sizeY( 0 ),
		eye( 0, 0, -1 ), at( 0, 0, 0 ), up( 0, 1, 0 )
	{
	}
	virtual ~renderInterface(){};

	static renderInterface * createDirectX8Render();
	static renderInterface * createOpenGLRender();
	////////////////////////////////////////////////////////////////////////////////
	// ������

	//���C�u�����������ς݂���Ԃ��@���̏����������E�I�������֐��ȊO�͖��������̂Ƃ��ɌĂяo���Ɠ���s��Ƃ���
	virtual bool initialized() = 0;
	//�����������@�^�[�Q�b�g�ƂȂ�̂�Window�̂݁Bwindow�Ńt���X�N���[���ɏo����ꍇ�͍s���B
	virtual bool initialize( void * hwnd, bool window, int width, int height ) = 0;
	//�I������
	virtual void finalize() = 0;

	//�����_���[�̗L�����E�������@�ȉ��̃����_���[�R�}���h���Ă񂾂Ƃ��ɏ��������Ɏw�肵���E�B���h�E�֑���悤�A�w�肷��
	virtual void activate( bool active ) = 0;
	//�V�[���̊J�n�@�w�i�������F
	virtual void startScene( unsigned long clearColor = 0x00000000 ) = 0;
	//�V�[���̏I���@�_�u���o�b�t�@�̏ꍇ�X���b�v
	virtual void finishScene( bool swapBuffer ) = 0;
	//�_�u���o�b�t�@���X���b�v
	virtual void swapBuffer() = 0;
	//�����L���b�V���̃N���A
	virtual void clearCache() = 0;
	//�O�E���ʂ���̓ǂݏo��
	virtual void readFrontBuffer( unsigned long * buffer ) = 0;
	virtual void readBackBuffer( unsigned long * buffer ) = 0;
	//�����_�����O�͈͂̐ݒ�
	virtual void setViewport( int x, int y, int width, int height, float zNear = 0, float zFar = 1 ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// �e�X�g

	// reference D3DCMPFUNC�ȂǂƓ����l�ɂ��邱�Ƃ�DX8�ł̓e�[�u�����v��Ȃ�
	enum testFuncType
	{
		testFuncNever = 1,
		testFuncLess, testFuncEqaul, testFuncLessEqual,
		testFuncGreater, testFuncNotEqual, testFuncGreaterEqual, testFuncAlways,
		testFuncTypeNum
	};
	enum stencilOpType
	{
		stencilOpKeep = 1,
		stencilOpZero, stencilOpReplace, stencilOpIncrSat, stencilOpDecrSat,
		stencilOpInvert, stencilOpIncr, stencilOpDecr,
		stencilOpNum
	};
	virtual void depthTest( bool testing, bool writing, int testFunc ) = 0;
	virtual void alphaTest( bool testing, float reference, int testFunc ) = 0;
	virtual void stencilTest( bool testing, unsigned long reference, unsigned long mask, unsigned long writeMask, int opFail, int opZFail, int opPass, int testFunc ) = 0;
	virtual void stencilClear() = 0;
	virtual void cull( bool enable, bool cw ) = 0;
	virtual void colorTest( bool red, bool green, bool blue, bool alpha ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// ��������
	enum blendFactor {
		blendZero = 1, blendOne, 
		blendSourceColor, blendInverseSourceColor, blendSourceAlpha, blendInverseSourceAlpha, 
		blendDestinationAlpha, blendInverseDestinationAlpha, blendDestinationColor, blendInverseDestinationColor,
		blendSourceAlphaSatulate, blendBothSourceAlpha, blendBothInverseSourceAlpha,
		blendFactorNum
	};
	virtual void blend( int destination, int source ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// �e�N�X�`��
	//�S�ĂR�Q�r�b�gRGBA�e�N�X�`���Ƃ��ē����ۑ����Ă���
	class rTexture
	{
	public:
		char * name;
		virtual ~rTexture(){};
		//�e�N�X�`���̃��ɓ����E�����������̌���
		virtual bool isTransparency() = 0;
	};
	//�e�N�X�`���̍쐬
	virtual rTexture * createTexture( const char * tag, int width, int height, unsigned long * pixels, int level ) = 0;
	//�e�N�X�`���̐ݒ�
	virtual void setTexture( int index, rTexture * texture ) = 0;
	virtual void setBlackTexture( int index ) = 0;
	virtual void setWhiteTexture( int index ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// ��������
	virtual void material( const vector3 & ambient, const vector3 & diffuse, const vector3 & specular, const vector3 & emissive, decimal transparency, decimal specularPower ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// ����
	// DX8�ł͊����Ƃ��Đݒ�ADX8�ł͂O�Ԗڂ̌����Ƃ��Đݒ肷��̂ŁA���̌����͂O�ȊO��p�����������
	virtual void lightAmbient( const vector4 & ambient ) = 0;
	//�_����
	virtual void lightPosition( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, float range, const vector3 & attenuation ) = 0;
	//��������
	virtual void lightDirection( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & direction ) = 0;
	//�X�|�b�g
	virtual void lightSpot( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, const vector3 & direction, float range, float fallOff, const vector3 & attenuation, float inside, float outside ) = 0;
	//�����̗L����
	virtual void lightEnable( int index, bool enable ) = 0;
	virtual void lightEnable( bool enable ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// �`��

	//�ʏ풸�_
	struct rVertex
	{
		float		position[4];
		float		normal[4];
		float		diffuse[4];
		float		specular[4];
		float		texture[8][4];
		bool operator==( const rVertex & src ) const { return memcmp( this, & src, sizeof( *this ) ) == 0; }
		bool operator!=( const rVertex & src ) const { return memcmp( this, & src, sizeof( *this ) ) != 0; }
	};
	//���_�`��ێ��p�̃N���X�B�����Ŕh�����Ď��̂�ێ�����
	class rVertexBuffer
	{
	public:
		enum {
			disableAll = 0,
			enableVertex = 0x1,
			enableNormal = 0x2,
			enableDiffuse = 0x4,
			enableSpecular = 0x8,
			enableTexture0 = 0x10,
			enableTexture1 = 0x20,
			enableTexture2 = 0x40,
			enableTexture3 = 0x80,
			enableTexture4 = 0x100,
			enableTexture5 = 0x200,
			enableTexture6 = 0x400,
			enableTexture7 = 0x800,
			enableVNT0 = 0x13,
			enableVDST0 = 0x1d,
		};
		int flag;
		array<rVertex> data;
		rVertexBuffer( int size, int _flag ) : flag( _flag ), data( size ){}
		virtual ~rVertexBuffer(){}
		//���̂Ƀf�[�^��n��
		virtual void update(){};
	};
	class rIndexBuffer
	{
	public:
		typedef unsigned short element;
		array<element> data;
		rIndexBuffer( int size ) : data( size ){}
		virtual ~rIndexBuffer(){}
		//���̂Ƀf�[�^��n��
		virtual void update(){};
	};
	//�쐬
	virtual rVertexBuffer * createVertexBuffer( int size, int flag ) = 0;
	virtual rIndexBuffer * createIndexBuffer( int size ) = 0;

	//�`��
	virtual void drawList( rVertex * vertices, int size, int offset = 0 ) = 0;
	virtual void drawList( rVertexBuffer * vertices, int size, int offset = 0 ) = 0;
	virtual void drawList( rVertexBuffer * vertices, rIndexBuffer * indecies, int size, int offset = 0 ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// �s��

	//Model�s��̐ݒ�
	virtual void initMatrix() = 0;
	virtual void setMatrix( float * elements ) = 0;
	virtual void getMatrix( float * elements ) = 0;
	virtual void multiMatrix( float * elements ) = 0;
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;

	//View�s��̐ݒ�
	virtual void initView() = 0;
	virtual void getViewProjection( float * elements ) = 0;
	virtual void setView( const vector3 & eye, const vector3 & at, const vector3 & up ) = 0;

	//Projection�s��ւ̐ݒ�iOGL�ł͑��݂��Ȃ����߁A�����ۑ���setView���Ŏ��s����j
	virtual void setPerspective( float fieldOfViewY, float aspect, float zNear, float zFar ) = 0;
	virtual void setOrtho( float sizeY, float aspect, float zNear, float zFar ) = 0;
	virtual void setFrustum( const vector3 & frustumMinimum, const vector3 & frustumMaximum ) = 0;

	//�X�v���C�g�������Ƃ���p�[�e�B�N���������Ƃ��̂��߂̊֐�
	//���[���h����r���[�|�[�g�܂ł̕ϊ��s����擾
	virtual void getAllMatrix( float * elements ) = 0;
	//�ϊ��s������������Ă��̂܂܂̈ʒu�ɕ`�悷�邽�߂̐ݒ�
	virtual void initAllMatrix() = 0;
	//���[���h����r���[�|�[�g�܂ł��ꂼ��̕ϊ��s����擾�E�ݒ�
	virtual void getEachMatrix( float * world, float * view, float * projection ) = 0;
	virtual void setEachMatrix( float * world, float * view, float * projection ) = 0;
};

//�g�p���郉�C�u�����̐ݒ�
#pragma comment( lib, "winmm.lib" )
