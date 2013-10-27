class trackView
{
public:
	static trackView * get()
	{
		static trackView instance;
		return & instance;
	}
	enum {
		original = 0x00,
		vertEdge = 0x01,
		horzEdge = 0x02,
		none = 0x04,
		baseImageMask = 0xFF,
		horizontal = 0x100,
		vertical = 0x200,
		descent = 0x400,//右下下がり
		ascent = 0x800,//右上上がり
		all = 0xF00,
		straight = 0x300,
		oblique = 0xC00,
	};
	int imageScale;//画像表示拡大率
	int mode;//画像表示モード
	trackView()
	{
		imageScale = 2;//画像表示拡大率
		mode = original;//画像表示モード
	}
	void updateImageScale()
	{
		changeImageScale( imageScale );
	}
	void changeImageScale( int t );
};
