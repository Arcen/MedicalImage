//��p�摜�t���[���@�R�����\���p�C���^�t�F�[�X�Ȃǂ��܂�
class miFrame
{
public:
	static miFrame * instance;
	static void set( miFrame * _instance ){ instance = _instance; }
	static miFrame * get(){ return instance; }
//	array<pixel> regionColor;// miDocument�s���D�L���Ŗ����ꍇ�ɂ͑S�Ă����p����悤��

	//�R�����\���p�f�[�^
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
