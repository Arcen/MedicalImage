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

	//デフォルトプロパティ
	int width, height;
	int viewX, viewY, viewWidth, viewHeight;
	bool rightHand, culling, cw, fillmode;
	float fieldOfViewY, aspect, zNear, zFar, sizeY;//perspective/orthoはfovyかsizeyが０でない時にそれぞれ動作する
	vector3 frustumMinimum, frustumMaximum;//frustumはfovyとsizeyが０の時に動作する
	vector3 eye, at, up;
	//オブジェクトの生成はこのクラスの下で定義した関数で行う
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
	// 初期化

	//ライブラリが初期済みかを返す　他の初期化処理・終了処理関数以外は未初期化のときに呼び出すと動作不定とする
	virtual bool initialized() = 0;
	//初期化処理　ターゲットとなるのはWindowのみ。windowでフルスクリーンに出来る場合は行う。
	virtual bool initialize( void * hwnd, bool window, int width, int height ) = 0;
	//終了処理
	virtual void finalize() = 0;

	//レンダラーの有効化・無効化　以下のレンダラーコマンドを呼んだときに初期化時に指定したウィンドウへ送るよう、指定する
	virtual void activate( bool active ) = 0;
	//シーンの開始　背景初期化色
	virtual void startScene( unsigned long clearColor = 0x00000000 ) = 0;
	//シーンの終了　ダブルバッファの場合スワップ
	virtual void finishScene( bool swapBuffer ) = 0;
	//ダブルバッファをスワップ
	virtual void swapBuffer() = 0;
	//内部キャッシュのクリア
	virtual void clearCache() = 0;
	//前・後画面からの読み出し
	virtual void readFrontBuffer( unsigned long * buffer ) = 0;
	virtual void readBackBuffer( unsigned long * buffer ) = 0;
	//レンダリング範囲の設定
	virtual void setViewport( int x, int y, int width, int height, float zNear = 0, float zFar = 1 ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// テスト

	// reference D3DCMPFUNCなどと同じ値にすることでDX8ではテーブルが要らない
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
	// 混合処理
	enum blendFactor {
		blendZero = 1, blendOne, 
		blendSourceColor, blendInverseSourceColor, blendSourceAlpha, blendInverseSourceAlpha, 
		blendDestinationAlpha, blendInverseDestinationAlpha, blendDestinationColor, blendInverseDestinationColor,
		blendSourceAlphaSatulate, blendBothSourceAlpha, blendBothInverseSourceAlpha,
		blendFactorNum
	};
	virtual void blend( int destination, int source ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// テクスチャ
	//全て３２ビットRGBAテクスチャとして内部保存しておく
	class rTexture
	{
	public:
		char * name;
		virtual ~rTexture(){};
		//テクスチャのαに透明・半透明部分の検索
		virtual bool isTransparency() = 0;
	};
	//テクスチャの作成
	virtual rTexture * createTexture( const char * tag, int width, int height, unsigned long * pixels, int level ) = 0;
	//テクスチャの設定
	virtual void setTexture( int index, rTexture * texture ) = 0;
	virtual void setBlackTexture( int index ) = 0;
	virtual void setWhiteTexture( int index ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// 質感属性
	virtual void material( const vector3 & ambient, const vector3 & diffuse, const vector3 & specular, const vector3 & emissive, decimal transparency, decimal specularPower ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// 光源
	// DX8では環境光として設定、DX8では０番目の光源として設定するので、他の光源は０以外を用いる方がいい
	virtual void lightAmbient( const vector4 & ambient ) = 0;
	//点光源
	virtual void lightPosition( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, float range, const vector3 & attenuation ) = 0;
	//方向光源
	virtual void lightDirection( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & direction ) = 0;
	//スポット
	virtual void lightSpot( int index, const vector4 & ambient, const vector4 & diffuse, const vector4 & specular, const vector3 & position, const vector3 & direction, float range, float fallOff, const vector3 & attenuation, float inside, float outside ) = 0;
	//光源の有効化
	virtual void lightEnable( int index, bool enable ) = 0;
	virtual void lightEnable( bool enable ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// 形状

	//通常頂点
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
	//頂点形状保持用のクラス。内部で派生して実体を保持する
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
		//実体にデータを渡す
		virtual void update(){};
	};
	class rIndexBuffer
	{
	public:
		typedef unsigned short element;
		array<element> data;
		rIndexBuffer( int size ) : data( size ){}
		virtual ~rIndexBuffer(){}
		//実体にデータを渡す
		virtual void update(){};
	};
	//作成
	virtual rVertexBuffer * createVertexBuffer( int size, int flag ) = 0;
	virtual rIndexBuffer * createIndexBuffer( int size ) = 0;

	//描画
	virtual void drawList( rVertex * vertices, int size, int offset = 0 ) = 0;
	virtual void drawList( rVertexBuffer * vertices, int size, int offset = 0 ) = 0;
	virtual void drawList( rVertexBuffer * vertices, rIndexBuffer * indecies, int size, int offset = 0 ) = 0;

	////////////////////////////////////////////////////////////////////////////////
	// 行列

	//Model行列の設定
	virtual void initMatrix() = 0;
	virtual void setMatrix( float * elements ) = 0;
	virtual void getMatrix( float * elements ) = 0;
	virtual void multiMatrix( float * elements ) = 0;
	virtual void pushMatrix() = 0;
	virtual void popMatrix() = 0;

	//View行列の設定
	virtual void initView() = 0;
	virtual void getViewProjection( float * elements ) = 0;
	virtual void setView( const vector3 & eye, const vector3 & at, const vector3 & up ) = 0;

	//Projection行列への設定（OGLでは存在しないため、内部保存しsetView内で実行する）
	virtual void setPerspective( float fieldOfViewY, float aspect, float zNear, float zFar ) = 0;
	virtual void setOrtho( float sizeY, float aspect, float zNear, float zFar ) = 0;
	virtual void setFrustum( const vector3 & frustumMinimum, const vector3 & frustumMaximum ) = 0;

	//スプライトを書くときやパーティクルを書くときのための関数
	//ワールドからビューポートまでの変換行列を取得
	virtual void getAllMatrix( float * elements ) = 0;
	//変換行列を初期化してそのままの位置に描画するための設定
	virtual void initAllMatrix() = 0;
	//ワールドからビューポートまでそれぞれの変換行列を取得・設定
	virtual void getEachMatrix( float * world, float * view, float * projection ) = 0;
	virtual void setEachMatrix( float * world, float * view, float * projection ) = 0;
};

//使用するライブラリの設定
#pragma comment( lib, "winmm.lib" )
