////////////////////////////////////////////////////////////////////////////////
// �e�N�X�`��

#ifndef __COMPOSE__

class sgTexture : public sgNode
{
public:
	sgNodeDeclare( sgTexture );
	enum {
		wrap,
		clamp,
		mirror,
	};
	sgString url;
	sgInt mapping;
	sgInt valid;//�e�N�X�`�������S�����Ŗ������ǂ���
	image texture;
};

#else

sgNodeDeclareInstance( sgTexture )
{
	static const string wk = "";
	sgDeclareThis();
	sgDeclareInitialRetain( url, wk );
	sgDeclareInitialRetain( mapping, sgTexture::wrap );
	sgDeclareInitialRetain( valid, 1 );
}

#endif
