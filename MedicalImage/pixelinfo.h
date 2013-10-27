struct pixelInfo
{
//	double gx, gy, gnorm, gangle;
	float gnorm;//���z�m����(�傫��)
	pixelInfo() : gnorm( 0 ){}
};
//16bit�̉摜�l������z�摜�̐���.
inline void analyzePixelInformation( const imageInterface< pixelLuminance<int16> > & ctimg, array2<pixelInfo> & info )
{
	info.allocate( ctimg.width, ctimg.height );
	for ( int y = 0; y < info.h; ++y ) {
		for ( int x = 0; x < info.w; ++x ) {
			pixelInfo & pi = info( x, y );
			int16 ct = ctimg.getInternal( x, y ).y;//���݂̉ӏ���CT�l.
			//��F=dF/di�i
			//����e�ł͊e�����̌��z�����ς��ĕ��z���Ă���
			//��]�Ɉˑ����Ȃ��悤�Ɋp�x��񎩑̂ɂ͏d�݂�u���Ȃ��B
			//������x�̌��z�̑傫���̂���p�x�̕��z�x�͏d�v���Ǝv����B
			//pi.gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
			//pi.gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
			const double gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );//�������̌X��.
			const double gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );//��ォ��̌X��.
			//���z�̑傫��=||��F||=��(��F���F)
			//���z�̑傫�����͔̂Z�x���ɑ������܂܂�Ă���̂ŏd�_�������Ȃ��Ă�����
			//pi.gnorm = sqrt( pi.gx * pi.gx + pi.gy * pi.gy );
            pi.gnorm = ::sqrt( gx * gx + gy * gy );//���z�̑傫��.
			//pi.gnorm = clamp<double>( 0, pi.gnorm, 2048 );
			pi.gnorm = clamp<float>( 0, pi.gnorm, 2048 );//�傫���̐���.
			//�p�x
			//pi.gangle = ( pi.gnorm == 0 ? 0 : atan2( pi.gx, pi.gy ) );

			//����e�Ȃ�Ίp�x�ˑ����������o�Ă���B
			//�ԏ�e�Ɨ���e�Ȃ疧�x�̍����o��Ǝv����B
			//CT�l�̕��͂P���̃f�[�^�ɂȂ邪�A������͂Q���̃f�[�^�ɂȂ�B
			//�p�x�A�傫�����ꂼ��Ɏ��Ӊ����Q�̂P���f�[�^�Ƃ��đ��ւ����̂��ǂ����H
			//����e�Ɋւ��Ă͊p�x�����ŉ�]�����ւ����Ȃ��Ƃ���Ȃ��Ǝv����B
			//�傫���݂̂ŃT���v�����O�����Ă݂�����ǂ���������Ȃ�
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
		//��F=dF/di�i
		//����e�ł͊e�����̌��z�����ς��ĕ��z���Ă���
		//��]�Ɉˑ����Ȃ��悤�Ɋp�x��񎩑̂ɂ͏d�݂�u���Ȃ��B
		//������x�̌��z�̑傫���̂���p�x�̕��z�x�͏d�v���Ǝv����B
		//pi.gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
		//pi.gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
		double gx = 0;
		int addressLeft = ctimg.address( x - 1, y );
		int addressUp = ctimg.address( x, y - 1 );
		if ( addressLeft >= 0 ) gx = ct - ctimg.getInternal( addressLeft ).y;
		double gy = 0;
		if ( addressUp >= 0 ) gy = ct - ctimg.getInternal( addressUp ).y;
		//���z�̑傫��=||��F||=��(��F���F)
		//���z�̑傫�����͔̂Z�x���ɑ������܂܂�Ă���̂ŏd�_�������Ȃ��Ă�����
		//pi.gnorm = sqrt( pi.gx * pi.gx + pi.gy * pi.gy );
        pi.gnorm = ::sqrt( gx * gx + gy * gy );
		//pi.gnorm = clamp<double>( 0, pi.gnorm, 2048 );
		pi.gnorm = clamp<float>( 0, pi.gnorm, 2048 );
		//�p�x
		//pi.gangle = ( pi.gnorm == 0 ? 0 : atan2( pi.gx, pi.gy ) );

		//����e�Ȃ�Ίp�x�ˑ����������o�Ă���B
		//�ԏ�e�Ɨ���e�Ȃ疧�x�̍����o��Ǝv����B
		//CT�l�̕��͂P���̃f�[�^�ɂȂ邪�A������͂Q���̃f�[�^�ɂȂ�B
		//�p�x�A�傫�����ꂼ��Ɏ��Ӊ����Q�̂P���f�[�^�Ƃ��đ��ւ����̂��ǂ����H
		//����e�Ɋւ��Ă͊p�x�����ŉ�]�����ւ����Ȃ��Ƃ���Ȃ��Ǝv����B
		//�傫���݂̂ŃT���v�����O�����Ă݂�����ǂ���������Ȃ�
	}
}

