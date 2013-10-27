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
		descent = 0x400,//�E��������
		ascent = 0x800,//�E��オ��
		all = 0xF00,
		straight = 0x300,
		oblique = 0xC00,
	};
	int imageScale;//�摜�\���g�嗦
	int mode;//�摜�\�����[�h
	trackView()
	{
		imageScale = 2;//�摜�\���g�嗦
		mode = original;//�摜�\�����[�h
	}
	void updateImageScale()
	{
		changeImageScale( imageScale );
	}
	void changeImageScale( int t );
};
