////////////////////////////////////////////////////////////////////////////////
// sgNode�̐ڑ����
class sgConnection
{
public:
	sgNode & src, & dst;//��񌳂ƑΏ�
	sgAttribute & output, & input;//�o�͏��Ɠ��͏��̑����i�����ł���K�v������j
	//���̂̐錾��nodes.h�ōs��
	//src(output)->dst(input) target�t���O������Ƃ��ɂ�dst���̃|�C���^��src���ŕێ�����
	sgConnection( sgNode & _src, sgAttribute & _output, sgNode & _dst, sgAttribute & _input );//nodes.h
	~sgConnection();//nodes.h
};
