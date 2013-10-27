////////////////////////////////////////////////////////////////////////////////
// テクスチャ

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
	sgInt valid;//テクスチャが完全透明で無いかどうか
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
