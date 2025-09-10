# argparse-cpp プロジェクト概要

## プロジェクトの目的
argparse-cppは、PythonのargparseモジュールのコアAPIをC++で実装したヘッダオンリーライブラリです。Python開発者が慣れ親しんだAPIを提供することで、C++でのコマンドライン引数解析を直感的かつ効率的に行えるようにすることを目的としています。

## 主要な特徴
- **Python argparse API互換性**: Pythonユーザーが学習コストゼロで使用開始可能
- **ヘッダオンリー設計**: 単一ファイルインクルードで即座に使用可能
- **型安全な解析**: C++の型システムを活用した安全な実装
- **C++11準拠**: 現代的なC++機能を活用しながら幅広い環境で利用可能
- **外部依存なし**: 標準ライブラリのみを使用

## プロジェクト構造
```
argparse-cpp/
├── include/argparse/argparse.hpp  # メインヘッダファイル（全実装）
├── tests/                          # テストディレクトリ
│   ├── unit/                      # 単体テスト
│   ├── integration/               # 統合テスト
│   └── CMakeLists.txt            # テストビルド設定
├── examples/                      # 使用例
│   ├── basic/                    # 基本的な使用例
│   └── advanced/                 # 高度な使用例
├── .spec-workflow/               # Spec Driven Development用ディレクトリ
│   ├── steering/                 # プロダクト方針・技術スタック定義
│   └── specs/argparse-core/     # 要件定義・設計書・タスク管理
├── README.md                     # プロジェクト説明
├── CLAUDE.md                     # AI開発ガイドライン
└── .gitignore                    # Git無視設定
```

## 重要なファイル
- `include/argparse/argparse.hpp`: すべての実装を含む単一ヘッダファイル
- `CLAUDE.md`: Claude AIで開発する際のガイドライン（日本語）
- `.spec-workflow/`: Spec Driven Developmentのための仕様書ディレクトリ