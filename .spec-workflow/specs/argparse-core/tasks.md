# タスク文書

<!-- AI Instructions: 各タスクに対して、以下の形式で構造化されたAIガイダンスを含む_Promptフィールドを生成：
_Prompt: Role: [専門的な開発者の役割] | Task: [コンテキスト参照を含む明確なタスク説明] | Restrictions: [してはいけないこと、制約] | Success: [具体的な完了基準]_
これにより、単純な「このタスクに取り組む」プロンプトを超えた、より良いAIエージェントガイダンスを提供します。 -->

- [x] 1. ヘッダファイルの基本構造作成
  - File: include/argparse/argparse.hpp
  - pragma onceとネームスペース定義
  - 必要な標準ライブラリのインクルード
  - バージョンマクロの定義
  - Purpose: ヘッダオンリーライブラリの基盤構築
  - _Requirements: 設計書のヘッダファイル構成_
  - _Prompt: Role: C++ライブラリ開発者（ヘッダオンリー設計専門） | Task: argparse.hppの基本構造を作成し、pragma once、必要な標準ライブラリヘッダ（string, vector, map, memory等）のインクルード、argparseネームスペースの定義を行う | Restrictions: 外部依存を追加しない、C++11標準に準拠、過度なマクロを使用しない | Success: ヘッダファイルが正しくコンパイルされ、基本構造が整備される_

- [x] 2. 型消去実装（AnyValue）の作成
  - File: include/argparse/argparse.hpp（detail namespace内）
  - AnyValueクラスの実装
  - BaseHolderとHolder構造の実装
  - テンプレートメソッドの実装
  - Purpose: C++11でstd::any相当の機能を提供
  - _Requirements: 要件6、設計書のAnyValue_
  - _Prompt: Role: C++テンプレートメタプログラミング専門家 | Task: argparse::detail名前空間にAnyValueクラスを実装し、型消去技術を使用して任意の型を保持できるようにする、BaseHolderインターフェースとHolder<T>テンプレートクラスを実装 | Restrictions: C++11準拠、std::anyは使用不可、メモリリークを防ぐ | Success: AnyValueが任意の型を安全に保持・取得でき、型チェックが機能する_

- [x] 3. Argumentクラスの基本実装
  - File: include/argparse/argparse.hpp
  - ArgumentDefinition構造体の定義
  - Argumentクラスの基本メソッド実装
  - 名前、ヘルプ、デフォルト値の設定
  - Purpose: 個々の引数定義を管理
  - _Requirements: 要件2、3、設計書のArgument_
  - _Prompt: Role: C++クラス設計専門家 | Task: ArgumentDefinition構造体とArgumentクラスを実装し、引数の名前、ヘルプテキスト、デフォルト値、タイプ、アクションなどのプロパティを管理できるようにする | Restrictions: Python argparseとAPIの一貫性を保つ、過度に複雑にしない | Success: Argumentクラスが引数定義を適切に保持し、設定メソッドが動作する_

- [x] 4. 型変換機能の実装
  - File: include/argparse/argparse.hpp（detail namespace内）
  - TypeConverterテンプレートの実装
  - int、float、string、boolの変換器
  - カスタム型変換のサポート
  - Purpose: 文字列から各型への安全な変換
  - _Requirements: 要件6、設計書のTypeConverter_
  - _Prompt: Role: C++型システム専門家 | Task: detail名前空間にTypeConverterテンプレートを実装し、文字列からint、float、bool等への変換機能を提供、std::functionを使用してカスタム変換器もサポート | Restrictions: 変換失敗時は適切な例外を投げる、型安全性を確保 | Success: 各型への変換が正しく動作し、エラー時に明確な例外が発生する_

- [ ] 5. ArgumentParserクラスの基本実装
  - File: include/argparse/argparse.hpp
  - コンストラクタとメンバ変数
  - add_argument基本実装
  - 引数の内部管理構造
  - Purpose: パーサーのメインインターフェース
  - _Requirements: 要件1、設計書のArgumentParser_
  - _Leverage: Argument、ArgumentDefinition_
  - _Prompt: Role: C++ APIデザイナー | Task: ArgumentParserクラスの基本構造を実装し、コンストラクタでprog、description、epilogを受け取り、add_argumentメソッドの基本形を実装、内部でArgumentのリストを管理 | Restrictions: Python argparseとAPIシグネチャを一致させる、メモリ管理はRAII原則に従う | Success: ArgumentParserが初期化でき、引数を追加できる_

- [ ] 6. 位置引数とオプション引数の判定ロジック
  - File: include/argparse/argparse.hpp（ArgumentParser内）
  - 引数名から位置/オプションを判定
  - 短縮形と長形式の処理
  - 引数の登録と検索
  - Purpose: 引数タイプの自動判定
  - _Requirements: 要件2、3_
  - _Leverage: ArgumentDefinition_
  - _Prompt: Role: パーサー実装専門家 | Task: add_argumentメソッド内で引数名から位置引数（-や--で始まらない）とオプション引数を判定し、短縮形（-v）と長形式（--verbose）を適切に処理する実装 | Restrictions: POSIX準拠の引数形式に従う、重複チェックを行う | Success: 位置引数とオプション引数が正しく分類・登録される_

- [ ] 7. Namespaceクラスの実装
  - File: include/argparse/argparse.hpp
  - 解析結果を保持する構造
  - get<T>()テンプレートメソッド
  - has()、contains()メソッド
  - Purpose: 解析結果への型安全なアクセス
  - _Requirements: 要件4、設計書のNamespace_
  - _Leverage: AnyValue_
  - _Prompt: Role: C++テンプレート設計者 | Task: Namespaceクラスを実装し、解析結果をunordered_mapで保持、get<T>()で型安全な値取得、has()で存在確認を提供 | Restrictions: 存在しないキーへのアクセスは例外を投げる、const正確性を保つ | Success: Namespaceが解析結果を正しく保持し、型安全にアクセスできる_

- [ ] 8. Tokenizerの実装
  - File: include/argparse/argparse.hpp（detail namespace内）
  - コマンドライン引数のトークン化
  - --や=の処理
  - 引用符の処理
  - Purpose: 引数文字列の前処理
  - _Requirements: 設計書のTokenizer_
  - _Prompt: Role: パーサー実装エキスパート | Task: detail名前空間にTokenizerクラスを実装し、argv配列を解析可能なトークンに分割、--key=valueや引用符付き文字列を適切に処理 | Restrictions: POSIX準拠、--で引数終了を認識 | Success: 複雑な引数形式が正しくトークン化される_

- [ ] 9. Parserクラスの基本実装
  - File: include/argparse/argparse.hpp（detail namespace内）
  - トークンと引数のマッチング
  - 位置引数の順序処理
  - オプション引数の検索
  - Purpose: 実際の解析ロジック
  - _Requirements: 要件4、設計書のParser_
  - _Leverage: Tokenizer、ArgumentDefinition_
  - _Prompt: Role: コマンドライン解析専門家 | Task: detail::Parserクラスを実装し、トークン化された引数を定義済みArgumentとマッチング、位置引数は順序通り、オプション引数は名前で検索する処理を実装 | Restrictions: 不明な引数はエラー、必須引数チェック | Success: 引数が正しくマッチングされ、値が抽出される_

- [ ] 10. parse_args()メソッドの実装
  - File: include/argparse/argparse.hpp（ArgumentParser内）
  - argc/argvとvectorの両方をサポート
  - Parserクラスの呼び出し
  - Namespace生成と返却
  - Purpose: メイン解析エントリポイント
  - _Requirements: 要件4_
  - _Leverage: Parser、Namespace、Tokenizer_
  - _Prompt: Role: C++ API実装者 | Task: ArgumentParserクラスにparse_args()メソッドを2つのオーバーロードで実装（argc/argv版とvector<string>版）、内部でParserを使用して解析し、結果をNamespaceで返す | Restrictions: エラー時は例外を投げる、--help処理を含む | Success: コマンドライン引数が正しく解析され、Namespaceが返される_

- [ ] 11. アクション処理の実装
  - File: include/argparse/argparse.hpp（Argument内）
  - store、store_true、store_false
  - append、countアクション
  - カスタムアクション
  - Purpose: 引数値の処理方法を定義
  - _Requirements: 要件3、5_
  - _Prompt: Role: 引数処理ロジック専門家 | Task: Argumentクラスにアクション処理を実装、store（値保存）、store_true/false（フラグ）、append（リスト追加）、count（カウント）を処理する機能 | Restrictions: Python argparseと同じ動作、アクション名は文字列で指定 | Success: 各アクションが期待通りに動作し、値が適切に処理される_

- [ ] 12. nargs処理の実装
  - File: include/argparse/argparse.hpp（Parser内）
  - 固定数、?、*、+の処理
  - remainderの処理
  - エラーチェック
  - Purpose: 引数が受け取る値の数を制御
  - _Requirements: 要件2_
  - _Prompt: Role: 引数カウント処理専門家 | Task: Parserクラスにnargs処理を実装、固定数値、'?'（0または1）、'*'（0以上）、'+'（1以上）、remainder（残り全て）をサポート | Restrictions: 値の数が不正な場合はエラー、位置引数での動作も考慮 | Success: nargsが正しく機能し、指定された数の値を処理できる_

- [ ] 13. HelpGeneratorクラスの実装
  - File: include/argparse/argparse.hpp
  - usage文字列の生成
  - 引数リストのフォーマット
  - グループごとの表示
  - Purpose: ヘルプメッセージの自動生成
  - _Requirements: 要件5、設計書のHelpGenerator_
  - _Leverage: ArgumentDefinition、ArgumentGroup_
  - _Prompt: Role: ドキュメント生成専門家 | Task: HelpGeneratorクラスを実装し、usage行の生成、description表示、引数リストのフォーマット（位置引数、オプション引数をグループ分け）、epilog表示を実装 | Restrictions: Python argparseと同じフォーマット、80文字幅を考慮 | Success: 見やすく整形されたヘルプが生成される_

- [ ] 14. --helpオプションの自動処理
  - File: include/argparse/argparse.hpp（ArgumentParser内）
  - add_help引数の処理
  - --help/-hの自動追加
  - ヘルプ表示と終了
  - Purpose: ヘルプオプションの自動化
  - _Requirements: 要件5_
  - _Leverage: HelpGenerator_
  - _Prompt: Role: CLI UX専門家 | Task: ArgumentParserにadd_helpパラメータを追加（デフォルトtrue）、--helpと-hを自動的に追加し、指定時にヘルプを表示してexit(0)する処理を実装 | Restrictions: add_help=falseで無効化可能、他の引数より優先 | Success: --helpが自動的に機能し、ヘルプが表示される_

- [ ] 15. ArgumentGroupクラスの実装
  - File: include/argparse/argparse.hpp
  - グループの作成と管理
  - グループへの引数追加
  - ヘルプでのグループ表示
  - Purpose: 関連引数のグループ化
  - _Requirements: 要件7、設計書のArgumentGroup_
  - _Leverage: Argument_
  - _Prompt: Role: API構造設計者 | Task: ArgumentGroupクラスを実装し、タイトルと説明を持ち、add_argument()でグループ内に引数を追加、HelpGeneratorと連携してグループごとの表示を実現 | Restrictions: デフォルトで位置引数とオプション引数グループを用意 | Success: 引数がグループごとに整理され、ヘルプで分かりやすく表示される_

- [ ] 16. add_argument_group()メソッドの実装
  - File: include/argparse/argparse.hpp（ArgumentParser内）
  - グループの作成と登録
  - グループへの参照返却
  - デフォルトグループの管理
  - Purpose: ユーザー定義グループの追加
  - _Requirements: 要件7_
  - _Leverage: ArgumentGroup_
  - _Prompt: Role: APIインターフェース実装者 | Task: ArgumentParserにadd_argument_group()メソッドを実装、新しいArgumentGroupを作成して管理、グループへの参照を返してadd_argumentを可能にする | Restrictions: グループ名の重複チェック、メモリ管理に注意 | Success: カスタムグループが作成でき、引数を追加できる_

- [ ] 17. choices検証の実装
  - File: include/argparse/argparse.hpp（Argument内）
  - 選択肢リストの設定
  - 値の検証ロジック
  - エラーメッセージ生成
  - Purpose: 値を特定の選択肢に制限
  - _Requirements: 要件6_
  - _Prompt: Role: 入力検証専門家 | Task: Argumentクラスにchoicesパラメータを実装、設定された選択肢リストに対して入力値を検証、不正な値の場合は選択肢を含むエラーメッセージを生成 | Restrictions: 型安全性を保つ、選択肢は任意の型をサポート | Success: choicesが正しく機能し、不正な値が拒否される_

- [ ] 18. required引数の処理
  - File: include/argparse/argparse.hpp（Parser内）
  - オプション引数のrequired設定
  - 必須チェックロジック
  - エラーメッセージ
  - Purpose: オプション引数を必須にする
  - _Requirements: 要件3_
  - _Prompt: Role: 引数検証エキスパート | Task: Parserクラスに必須引数チェックを実装、requiredフラグがtrueの引数が指定されていない場合にエラーを報告、分かりやすいエラーメッセージを生成 | Restrictions: 位置引数は常に必須、デフォルト値がある場合は必須でない | Success: 必須引数が正しくチェックされ、未指定時にエラーになる_

- [ ] 19. エラーメッセージシステムの実装
  - File: include/argparse/argparse.hpp（detail namespace内）
  - ArgumentError例外クラス
  - エラーメッセージフォーマット
  - usage付きエラー表示
  - Purpose: 一貫性のあるエラー報告
  - _Requirements: 設計書のエラー処理_
  - _Leverage: HelpGenerator_
  - _Prompt: Role: エラー処理設計者 | Task: detail名前空間にArgumentError例外クラスを実装、エラータイプごとのメッセージテンプレート、usage行を含むフォーマット済みエラーメッセージを生成 | Restrictions: Python argparseと同様のメッセージ形式、実用的で分かりやすい | Success: エラーが明確で、ユーザーが問題を理解・修正できる_

- [ ] 20. metavar処理の実装
  - File: include/argparse/argparse.hpp（Argument内）
  - metavarの設定と取得
  - ヘルプでの表示処理
  - デフォルトmetavar生成
  - Purpose: ヘルプでの引数表示名制御
  - _Requirements: 要件5_
  - _Leverage: HelpGenerator_
  - _Prompt: Role: ヘルプ表示カスタマイズ専門家 | Task: Argumentクラスにmetavarパラメータを実装、ヘルプメッセージで引数値の表示名を制御、未指定時は引数名から自動生成 | Restrictions: 大文字変換などPython互換の動作 | Success: metavarがヘルプで正しく表示される_

- [ ] 21. 基本的な単体テストの作成
  - File: tests/unit/parser_test.cpp
  - ArgumentParserの初期化テスト
  - add_argumentの基本テスト
  - parse_argsの基本テスト
  - Purpose: 基本機能の動作確認
  - _Requirements: 全要件_
  - _Prompt: Role: C++テストエンジニア | Task: Google Testを使用してArgumentParserの基本的な単体テストを作成、初期化、引数追加、基本的な解析の動作を確認 | Restrictions: テストは独立して実行可能、エッジケースも考慮 | Success: 基本機能のテストがすべてパスする_

- [ ] 22. Python互換性テストの作成
  - File: tests/integration/python_compat_test.cpp
  - Python argparseと同じ入力での比較
  - 出力メッセージの一致確認
  - エラーケースの互換性
  - Purpose: Python argparseとの互換性確認
  - _Requirements: 全要件_
  - _Prompt: Role: 互換性テストエンジニア | Task: Python argparseと同じ引数定義・入力で同じ結果になることを確認するテストスイートを作成、ヘルプメッセージやエラーメッセージも比較 | Restrictions: 主要なユースケースをカバー、差異は文書化 | Success: Python argparseと高い互換性が確認される_

- [ ] 23. サンプルプログラムの作成
  - File: examples/basic/hello_world.cpp
  - 最小限の使用例
  - 位置引数とオプション引数
  - ヘルプの確認
  - Purpose: クイックスタート用の例
  - _Requirements: 要件1-5_
  - _Prompt: Role: デベロッパーアドボケート | Task: argparse-cppの最も簡単な使用例を作成、位置引数1つとオプション引数2-3個を含む実用的なhello worldプログラム | Restrictions: 5分で理解できる簡潔さ、コメント付き | Success: 初心者が即座に使い始められる_

- [ ] 24. ドキュメントの作成
  - File: README.md
  - インストール方法
  - 基本的な使い方
  - APIリファレンスへのリンク
  - Purpose: ライブラリの導入ガイド
  - _Requirements: 全要件_
  - _Prompt: Role: テクニカルライター | Task: argparse-cppのREADMEを作成、インストール（単一ヘッダのコピー）、基本的な使用例、主要なAPIの説明、Python argparseとの違いを記載 | Restrictions: 簡潔で実用的、コード例を多用 | Success: ユーザーがREADMEだけで使い始められる_