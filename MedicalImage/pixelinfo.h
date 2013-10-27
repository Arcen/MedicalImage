struct pixelInfo
{
//	double gx, gy, gnorm, gangle;
	float gnorm;//勾配ノルム(大きさ)
	pixelInfo() : gnorm( 0 ){}
};
//16bitの画像値から勾配画像の生成.
inline void analyzePixelInformation( const imageInterface< pixelLuminance<int16> > & ctimg, array2<pixelInfo> & info )
{
	info.allocate( ctimg.width, ctimg.height );
	for ( int y = 0; y < info.h; ++y ) {
		for ( int x = 0; x < info.w; ++x ) {
			pixelInfo & pi = info( x, y );
			int16 ct = ctimg.getInternal( x, y ).y;//現在の箇所のCT値.
			//∇F=dF/di･i
			//粒状影では各方向の勾配が平均して分布している
			//回転に依存しないように角度情報自体には重みを置けない。
			//ある程度の勾配の大きさのある角度の分布度は重要だと思われる。
			//pi.gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
			//pi.gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
			const double gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );//一つ左からの傾斜.
			const double gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );//一つ上からの傾斜.
			//勾配の大きさ=||∇F||=√(∇F･∇F)
			//勾配の大きさ自体は濃度情報に多くが含まれているので重点をおかなくてもいい
			//pi.gnorm = sqrt( pi.gx * pi.gx + pi.gy * pi.gy );
            pi.gnorm = ::sqrt( gx * gx + gy * gy );//勾配の大きさ.
			//pi.gnorm = clamp<double>( 0, pi.gnorm, 2048 );
			pi.gnorm = clamp<float>( 0, pi.gnorm, 2048 );//大きさの制限.
			//角度
			//pi.gangle = ( pi.gnorm == 0 ? 0 : atan2( pi.gx, pi.gy ) );

			//線状影ならば角度依存性が多少出てくる。
			//網状影と粒状影なら密度の差が出ると思われる。
			//CT値の方は１次のデータになるが、こちらは２次のデータになる。
			//角度、大きさそれぞれに周辺化し２つの１次データとして相関を取るのが良いか？
			//線状影に関しては角度方向で回転し相関を取らないとあわないと思われる。
			//大きさのみでサンプリング化してみる方が良いかもしれない
		}
	}
}

inline void analyzePixelInformation( const ctImage & ctimg, array2<pixelInfo> & info )
{
	const region<short>::rectangle & bbox = ctimg.shape.get();
	int width = bbox.left + bbox.width(), height = bbox.right + bbox.height();
	info.allocate( width, height );
	for ( ctImage::iterator it( ctimg ); it; ++it ) {
		int x = it.x();
		int y = it.y();
		pixelInfo & pi = info( x, y );
		int16 ct = ctimg.getInternal( it() ).y;
		//∇F=dF/di･i
		//粒状影では各方向の勾配が平均して分布している
		//回転に依存しないように角度情報自体には重みを置けない。
		//ある程度の勾配の大きさのある角度の分布度は重要だと思われる。
		//pi.gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
		//pi.gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
		double gx = 0;
		int addressLeft = ctimg.address( x - 1, y );
		int addressUp = ctimg.address( x, y - 1 );
		if ( addressLeft >= 0 ) gx = ct - ctimg.getInternal( addressLeft ).y;
		double gy = 0;
		if ( addressUp >= 0 ) gy = ct - ctimg.getInternal( addressUp ).y;
		//勾配の大きさ=||∇F||=√(∇F･∇F)
		//勾配の大きさ自体は濃度情報に多くが含まれているので重点をおかなくてもいい
		//pi.gnorm = sqrt( pi.gx * pi.gx + pi.gy * pi.gy );
        pi.gnorm = ::sqrt( gx * gx + gy * gy );
		//pi.gnorm = clamp<double>( 0, pi.gnorm, 2048 );
		pi.gnorm = clamp<float>( 0, pi.gnorm, 2048 );
		//角度
		//pi.gangle = ( pi.gnorm == 0 ? 0 : atan2( pi.gx, pi.gy ) );

		//線状影ならば角度依存性が多少出てくる。
		//網状影と粒状影なら密度の差が出ると思われる。
		//CT値の方は１次のデータになるが、こちらは２次のデータになる。
		//角度、大きさそれぞれに周辺化し２つの１次データとして相関を取るのが良いか？
		//線状影に関しては角度方向で回転し相関を取らないとあわないと思われる。
		//大きさのみでサンプリング化してみる方が良いかもしれない
	}
}

