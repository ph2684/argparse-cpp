# 開発用コマンド一覧

## テスト実行
```bash
# 単体テストのビルドと実行（推奨方法）
cmake -B tests/build -S tests
cmake --build tests/build
ctest --test-dir tests/build

# 個別テストの実行
ctest --test-dir tests/build -R basic_structure_test
ctest --test-dir tests/build -R anyvalue_test

# 詳細なテスト出力
ctest --test-dir tests/build --verbose

# テスト失敗時の詳細表示
ctest --test-dir tests/build --output-on-failure
```

## ビルド
```bash
# サンプルプログラムの手動コンパイル（現在の方法）
g++ -std=c++11 -I include examples/basic/hello_world.cpp -o hello_world

# デバッグビルド
g++ -std=c++11 -I include -g -O0 examples/basic/hello_world.cpp -o hello_world_debug

# リリースビルド
g++ -std=c++11 -I include -O3 -DNDEBUG examples/basic/hello_world.cpp -o hello_world_release
```

## 静的解析とフォーマット
```bash
# clang-tidyによる静的解析
clang-tidy include/argparse/argparse.hpp -- -std=c++11

# clang-formatによるフォーマット（変更を適用）
clang-format -i include/argparse/argparse.hpp

# clang-formatによるフォーマット（差分確認のみ）
clang-format --dry-run include/argparse/argparse.hpp
```

## Git操作
```bash
# ステータス確認
git status

# 差分確認
git diff
git diff --staged

# ブランチ作成（mainブランチでは作業しない）
git checkout -b feature/新機能名

# コミット（日本語メッセージ）
git add .
git commit -m "機能: 新機能の実装"

# プッシュ
git push -u origin feature/新機能名
```

## spec-workflowとの連携
```bash
# 仕様の確認
mcp__spec-workflow__get-spec-context projectPath=/home/hasegawa/code/argparse-cpp specName=argparse-core

# 次のタスク取得
mcp__spec-workflow__manage-tasks projectPath=/home/hasegawa/code/argparse-cpp specName=argparse-core action=next-pending

# タスク一覧表示
mcp__spec-workflow__manage-tasks projectPath=/home/hasegawa/code/argparse-cpp specName=argparse-core action=list

# タスク完了マーク
mcp__spec-workflow__manage-tasks projectPath=/home/hasegawa/code/argparse-cpp specName=argparse-core action=set-status taskId=<id> status=completed
```

## システムコマンド（Linux）
```bash
# ファイル検索
find . -name "*.cpp"
find . -name "*.hpp"

# テキスト検索
grep -r "ArgumentParser" --include="*.hpp"
grep -r "TODO" --include="*.cpp" --include="*.hpp"

# ディレクトリ構造表示
tree -I 'build|.git'

# ファイル/ディレクトリ一覧
ls -la
ls -la tests/
```

## 重要な注意事項
- **cdコマンド禁止**: AIの動作安定化のため、カレントディレクトリの変更は避ける
- **絶対パス使用**: cmake -B, -S オプションを活用
- **untrackedファイル**: コミット前に必ず確認し、適切に処理する