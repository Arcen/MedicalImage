//undo,redo�̎���

//�R�}���h�p�^�[��

//�h�L�������g�̓V���O���g���ł���A�O���[�o���Ɏ擾����B

//�e�R�}���h�̌p����Aexecute��h����Ŏ�������B
class miCommand
{
public:
	//�R���X�g���N�^�ŁA�`���̏�Ԃ�ݒ肷��
	miDocument::memento m;
	bool noneedundo;//�A���h�D�������K�v�̂Ȃ��R�}���h
	bool cannotundo;//�A���h�D�������s�\�ȃR�}���h
	miCommand() : noneedundo( false ), cannotundo( false ) {}
	virtual ~miCommand(){}
	virtual bool execute() = 0;//�R�}���h�����s����B
	//�ύX�ӏ����w�肵����h�L�������g����ۑ�����
	void memory()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//�R�}���h�����s�����O��changed�ɂ��ꂩ��ύX�����t���O�������Ă���
		doc->save( m );
		//�`���ɂ͎��s�O�̏�Ԃ��ۑ�����Ă���
	}
	//undo�͎��s���ԁAredo��undo���Ԃł����s���Ȃ����A�`�F�b�N�͏ȗ�����B
	//�A���h�D�����s
	void undo()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//���s��̏�Ԃ�ۑ����A���s�O�̏�Ԃ��h�L�������g�ɍX�V����B
		doc->exchange( m );
		//�`���ɂ͎��s�O�̏�Ԃ��ۑ�����Ă���
	}
	//���h�D�����s
	void redo()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//���s�O�̏�Ԃ�ۑ����A���s��̏�Ԃ��h�L�������g�ɍX�V����B
		//�ύX�ӏ��̓t���O�̌��̂�
		doc->exchange( m );
		//�`���ɂ͎��s��̏�Ԃ��ۑ�����Ă���
	}
};

//�R�}���h�̗����i�V���O���g���j
class miCommands
{
	//�V���O���g���p�^�[��
	static miCommands instance;
	list< smart<miCommand> > undolist, redolist;
public:
	static miCommands & get(){ return instance; }
	//���ׂẴR�}���h���폜
	void clear()
	{
		undolist.release();
		redolist.release();
	}
	//���݂̎��_�łق��̃R�}���h���s�����߁Aredo���폜
	void cancelRedo()
	{
		redolist.release();
	}
	//undolist<current<redolist�̂悤�ɂ���
	void undo()
	{
		if ( ! undolist.empty() ) {
			smart<miCommand> c = undolist.last();
			undolist.pop_back();
			c->undo();
			redolist.push_front( c );
		}
	}
	void redo()
	{
		if ( ! redolist.empty() ) {
			smart<miCommand> c = redolist.first();
			redolist.pop_front();
			c->redo();
			undolist.push_back( c );
		}
	}
	//�R�}���h��o�^���Ď��s����
	void regist( miCommand * command )
	{
		smart<miCommand> c = command;
		if ( ! c->noneedundo ) cancelRedo();
		c->memory();//���s�O��ۑ����Ă���
		if ( c->execute() ) {//���s����
			//���s������ɏI������ꍇ�ɂ́A���������T�|�[�g����
			if ( c->cannotundo ) {
				clear();
				return;
			}
			if ( ! c->noneedundo ) undolist.push_back( c );
		}
	}
};
