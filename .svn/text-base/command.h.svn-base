//undo,redoの実装

//コマンドパターン

//ドキュメントはシングルトンであり、グローバルに取得する。

//各コマンドの継承基、executeを派生先で実装する。
class miCommand
{
public:
	//コンストラクタで、形見の状態を設定する
	miDocument::memento m;
	bool noneedundo;//アンドゥ処理が必要のないコマンド
	bool cannotundo;//アンドゥ処理が不可能なコマンド
	miCommand() : noneedundo( false ), cannotundo( false ) {}
	virtual ~miCommand(){}
	virtual bool execute() = 0;//コマンドを実行する。
	//変更箇所を指定した後ドキュメントから保存する
	void memory()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//コマンドが実行される前にchangedにこれから変更されるフラグが入っている
		doc->save( m );
		//形見には実行前の状態が保存されている
	}
	//undoは実行後状態、redoはundo後状態でしか行えないが、チェックは省略する。
	//アンドゥを実行
	void undo()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//実行後の状態を保存し、実行前の状態をドキュメントに更新する。
		doc->exchange( m );
		//形見には実行前の状態が保存されている
	}
	//リドゥを実行
	void redo()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//実行前の状態を保存し、実行後の状態をドキュメントに更新する。
		//変更箇所はフラグの個所のみ
		doc->exchange( m );
		//形見には実行後の状態が保存されている
	}
};

//コマンドの履歴（シングルトン）
class miCommands
{
	//シングルトンパターン
	static miCommands instance;
	list< smart<miCommand> > undolist, redolist;
public:
	static miCommands & get(){ return instance; }
	//すべてのコマンドを削除
	void clear()
	{
		undolist.release();
		redolist.release();
	}
	//現在の時点でほかのコマンドを行うため、redoを削除
	void cancelRedo()
	{
		redolist.release();
	}
	//undolist<current<redolistのようにする
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
	//コマンドを登録して実行する
	void regist( miCommand * command )
	{
		smart<miCommand> c = command;
		if ( ! c->noneedundo ) cancelRedo();
		c->memory();//実行前を保存しておき
		if ( c->execute() ) {//実行する
			//実行が正常に終わった場合には、取り消しをサポートする
			if ( c->cannotundo ) {
				clear();
				return;
			}
			if ( ! c->noneedundo ) undolist.push_back( c );
		}
	}
};
