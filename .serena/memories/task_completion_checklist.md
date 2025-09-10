# タスク完了時のチェックリスト

## 必須確認事項

### 1. テスト実行
```bash
# すべてのテストが通過することを確認
cmake -B tests/build -S tests
cmake --build tests/build
ctest --test-dir tests/build
```
- ✓ 既存のテストがすべて通過する
- ✓ 新機能に対するテストが追加されている
- ✓ 破壊的変更がないことを確認

### 2. 静的解析
```bash
# clang-tidyによる静的解析
clang-tidy include/argparse/argparse.hpp -- -std=c++11
```
- ✓ 警告が出ていないか確認
- ✓ 新たな警告を追加していないか

### 3. コードフォーマット
```bash
# clang-formatによるフォーマット確認
clang-format --dry-run include/argparse/argparse.hpp
```
- ✓ フォーマットが統一されているか確認

### 4. ビルド確認
```bash
# サンプルプログラムがビルドできることを確認
g++ -std=c++11 -I include examples/basic/hello_world.cpp -o /tmp/test_build
```
- ✓ コンパイルエラーがない
- ✓ 警告が最小限に抑えられている

### 5. Git状態確認
```bash
git status
git diff
```
- ✓ 不要なファイルがステージングされていない
- ✓ untrackedファイルの処理が完了している
- ✓ 意図した変更のみが含まれている

### 6. ドキュメント更新
- ✓ 新機能のドキュメントが追加されている（必要な場合）
- ✓ APIドキュメント（Doxygenコメント）が更新されている
- ✓ README.mdの更新が必要か確認

### 7. spec-workflowタスク更新
```bash
# タスクを完了としてマーク
mcp__spec-workflow__manage-tasks projectPath=/home/hasegawa/code/argparse-cpp specName=argparse-core action=set-status taskId=<id> status=completed
```
- ✓ 実装したタスクのステータスを更新

## 重要な原則
1. **破壊的変更の防止**: 既存のAPIや動作を変更しない
2. **後方互換性**: C++11準拠を維持
3. **Python互換性**: Python argparseとの一貫性を保つ
4. **品質優先**: すべてのチェックが通過するまでコミットしない

## トラブルシューティング
- テストが失敗した場合: 変更を見直し、既存の機能を壊していないか確認
- フォーマットエラー: `clang-format -i`で自動修正
- コンパイルエラー: C++11標準の範囲内で実装されているか確認
- untrackedファイル: `.gitignore`に追加するか、必要なら`git add`する