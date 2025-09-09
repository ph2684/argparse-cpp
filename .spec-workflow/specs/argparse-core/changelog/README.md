# Changelog Directory

このディレクトリには、argparse-core仕様に関する変更履歴と修正記録を格納します。

## ディレクトリ構成

- **YYYY-MM-DD-BUGFIX-*.md**: バグ修正の詳細記録
- **YYYY-MM-DD-ENHANCEMENT-*.md**: 機能拡張の記録
- **YYYY-MM-DD-BREAKING-*.md**: 破壊的変更の記録

## ファイル命名規則

日付を先頭に配置してソートを容易にします：

- `YYYY-MM-DD-BUGFIX-[問題の概要].md`
- `YYYY-MM-DD-ENHANCEMENT-[機能名].md` 
- `YYYY-MM-DD-BREAKING-[変更内容].md`

## 記録内容

各変更記録ファイルには以下の情報を含める：

1. 問題/変更の概要
2. 詳細説明と根本原因
3. 修正/変更内容
4. 影響範囲
5. テスト結果
6. 承認ID（該当する場合）

## 注意事項

- 本ディレクトリの内容は仕様書本体（requirements.md, design.md, tasks.md）とは独立した変更履歴です
- 仕様書本体への変更も伴う場合は、両方を更新してください
- 重要な変更は approval request を通じて承認を求めてください