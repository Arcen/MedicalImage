////////////////////////////////////////////////////////////////////////////////
// インバースキネマティクス

#ifndef __COMPOSE__

//maya用のIK演算クラス．joint/effectorをリストとしてもち，
//handleの位置にeffectorがくるようにjointの角度を変更した後，
//handleをスナップ(effectorの位置に変更)する
class sgMayaIkHandle : public sgNode
{
	array<sgMayaJointMatrix*> jointsArray;
	array<sgTransformMatrix*> jointMatricies;
public:
	sgNodeDeclare( sgMayaIkHandle );
	sgNodeMember<sgTransformMatrix> parent;//ジョイントの親のノード（最低限の更新を期待する）
	sgNodeMember<sgMayaJointMatrix> joints;//入力出力先のjoint情報．先頭から，effectorまでを入れる．
	sgInt valid;//ハンドルの位置にジョイントをどうさせるかどうか
	sgVector3 position;//ここに向けて，動く グローバル座標
	sgDecimal epsiron;//微分を計算するときの微小角度[rad]
	sgDecimal movability;//ハンドルの動き易さ[rad]
	sgInt numberJointEstimate;//ジョイントの推定回数：単体の回転が早くさせる、１ならば全体の更新角度の大きさが同じになるように動く
	sgInt numberJointsEstimate;//ジョイント全体の推定回数：ジョイント全体のハンドルへの接近が早くなる

	class axisInformation
	{
	public:
		int jointIndex;
		int axisIndex;
		decimal mnmm, mxmm;
		bool limited;
	};
	//ある点に対するjointの回転のみを指定角度で行う
	vector3 rotate( const vector3 & angle, const vector4 & position, sgMayaJointMatrix * joint )
	{
		return ( position * matrix44::rotation( joint->composeRotation( angle ) ) ).getVector3();
	}
	//最急降下法による、ジョイントの回転角度の推定
	bool estimate( const vector4 & handle, const vector4 & effector, sgMayaJointMatrix * joint )
	{
		vector3 & angle = joint->angle();//角度
		const vector3 e = rotate( angle, effector, joint );//エフェクタの回転後の位置
		const vector3 h = ( handle ).getVector3();
		const matrix33 jacobian( matrix33::zero );
		vector3 & freedom = joint->freedom();//回転が自由な軸
		const vector3 epsiron3[3] = { vector3( epsiron(), 0, 0 ), vector3( 0, epsiron(), 0 ), vector3( 0, 0, epsiron() ) };
		vector3 delta( vector3::zero );
		const vector3 h_e = h - e;
		for ( int i = 0; i < 3; ++i ) {
			if ( ! freedom[i] ) continue;
			//最急行化法
			delta[i] = vector3::dot3( h_e, // マイナスを内部に入れて高速化する。本来は- ( e - h ) * transpose(jacobian);
				//ヤコビアン
				//各ベクトルはその要素がepsironだけ正負に動いた時のエフェクタの回転後の位置の差
				// 正規化するので割り算を省略 jacobian = ( plus - minus ) / ( 2 * epsiron );
				rotate( angle + epsiron3[i], effector, joint ) - 
				rotate( angle - epsiron3[i], effector, joint ) );
		}
		//角度が無いなら終わり
		if ( vector3::zero == delta ) return true;
		//正規化
		delta = vector3::normalize3( delta ) * movability();

		vector3 newangle = angle + delta;//新しい角度
		joint->limitAngle( newangle );//制限を行う
		if ( newangle == angle ) return true;//すでに限界
		const vector3 enew = rotate( newangle, effector, joint );
		if ( vector3::length3( e - h ) < vector3::length3( enew - h ) ) {
			return true;//動いた結果遠ざかっている
		}
		//角度の更新
		angle = newangle;
		return false;
	}
	//solverを動作させる
	//上流にあるDAGから始まっているsolverから動作させる必要がある．
	//そして，一つを動作させた後に，dirtyフラグがたった場合には，smartにupdateして，下流のsolverを動作させる
	virtual void update()
	{
		if ( ! valid() ) return;
		if ( joints.count() < 2 ) return;//回転する必要が無い
		//ジョイントの情報
		if ( ! jointsArray.size ) {
			jointsArray.reserve( joints.count() );
			for ( sgNodeMember<sgMayaJointMatrix>::iterator it( joints() ); it; ++it ) {
				jointsArray.push_back( it() );
			}
			jointMatricies.allocate( jointsArray.size );
			for ( int i = 0; i < jointsArray.size; ++i ) {
				jointMatricies[i] = ( sgTransformMatrix * ) jointsArray[i]->getOutput( sgMayaJointMatrix::attributes->matrix, sgTransformMatrix::descriptor );
			}
		}
		//ターゲット先を更新する必要がある
		for ( int i = 0; i < jointMatricies.size; ++i ) {
			jointsArray[i]->update();
			jointMatricies[i]->update();
		}

		for ( int count = 0; count < numberJointsEstimate(); ++count ) {
			matrix44 prerot = jointMatricies.last()->local();//エフェクタの座標系から対象のジョイントまでの修正された変換行列をあらわすようにする
			for ( int i = jointsArray.size - 2; i >= 0; --i ) {
				sgMayaJointMatrix * joint = jointsArray[i];//回転するジョイント
				sgTransformMatrix * tm = jointMatricies[i];//その回転と親情報用
				//回転までのローカル座標への変換
				//目標の位置（ワールド座標系の目標の位置を逆変換でジョイントの回転後の座標系（回転の影響は受けない）に変換）
				vector4 handle = vector4( position() ) * matrix44::inverse( joint->postRotation() * tm->parent() );
				//現在の位置（effectorのローカル位置を回転前の座標系の位置に変換）
				vector4 effector = vector4( 0, 0, 0, 1 ) * prerot * joint->preRotation();
				// effector * rotation == handle になるような回転に変更するようにする

				//最急降下法にて、角度を更新していく。
				bool changed = false;
				for ( int k = 0; k < numberJointEstimate(); ++k ) {//ジョイント単体の更新回数
					if ( estimate( handle, effector, joint ) ) break;
					changed = true;
				}
				if ( changed ) {
					joint->dirty = true;//個別に回転するときはフラグは綺麗にならないが、最後にエンベロープなどにおいて影響が出るように汚しておく
					//joint->graph.update( true );//全体を更新するほうが安全だが、グラフによっては以下の部分だけで、IKにおいては十分
					//ジョイント本体の更新
					jointsArray[i]->update();
					for ( int j = i; j < jointMatricies.size; ++j ) {
						//それ以下のジョイントの行列についてglobalを更新していく
						jointMatricies[j]->update();
					}
				}
				prerot = prerot * tm->local();
			}
		}
	}
};

#else

sgNodeDeclareInstance( sgMayaIkHandle )
{
	sgDeclareThis();
	sgDeclareNodeTarget( parent, sgTransformMatrix );
	sgDeclareNodeTarget( joints, sgMayaJointMatrix );
	sgDeclareInitialRetain( valid, 0 );
	sgDeclareInitialRetain( position, vector3::identity );
	sgDeclareInitialRetain( epsiron, deg2rad( 1.0 ) );
	sgDeclareInitialRetain( movability, deg2rad( 1.0 ) );
	sgDeclareInitialRetain( numberJointEstimate, 1 );
	sgDeclareInitialRetain( numberJointsEstimate, 5 );
}

#endif
