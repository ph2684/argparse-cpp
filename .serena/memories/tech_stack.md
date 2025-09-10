# テクノロジースタック

## プログラミング言語
- **C++11**: メイン実装言語（C++11標準準拠）
- **ヘッダオンリーライブラリ**: 単一ファイル実装

## ビルドシステム
- **CMake**: ビルド管理システム（バージョン3.14以上）
  - FetchContentでGoogle Testを自動ダウンロード
  - テストビルドに使用

## テストフレームワーク
- **Google Test (gtest)**: 単体テストフレームワーク
  - バージョン: v1.14.0
  - FetchContentで自動ダウンロード・ビルド

## 静的解析ツール
- **clang-tidy**: 静的コード解析
- **clang-format**: コードフォーマッター

## コンパイラ
- **g++**: GNU C++コンパイラ（C++11対応）
- **clang++**: Clang C++コンパイラ（C++11対応）

## 依存関係
- **標準ライブラリのみ**: 外部ライブラリ依存なし
- C++11標準ライブラリ機能:
  - `<string>`, `<vector>`, `<map>`, `<memory>`
  - `<algorithm>`, `<iostream>`, `<sstream>`
  - `<stdexcept>`, `<typeinfo>`

## 開発方法論
- **Spec Driven Development (SDD)**: spec-workflow-mcpを使用
- 仕様書ベースの開発アプローチ