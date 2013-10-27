//医用画像フレーム　３次元表示用インタフェースなどを含む
class miFrame
{
public:
	static miFrame * instance;
	static void set( miFrame * _instance ){ instance = _instance; }
	static miFrame * get(){ return instance; }
//	array<pixel> regionColor;// miDocument行き．有効で無い場合には全てしろを用いるように

	//３次元表示用データ
	renderInterface * render;
	graphRender gr;
	
	miFrame() : render( NULL )
	{
	}
	~miFrame()
	{
	}
	void paint();
	void initialize();
	void finalize();
};
