////////////////////////////////////////////////////////////////////////////////
// sgNodeの接続情報
class sgConnection
{
public:
	sgNode & src, & dst;//情報元と対象
	sgAttribute & output, & input;//出力情報と入力情報の属性（同じである必要がある）
	//実体の宣言はnodes.hで行う
	//src(output)->dst(input) targetフラグがあるときにはdst側のポインタをsrc側で保持する
	sgConnection( sgNode & _src, sgAttribute & _output, sgNode & _dst, sgAttribute & _input );//nodes.h
	~sgConnection();//nodes.h
};
