# Contract: エディタ拡張 API（US15）

ゲームモジュール内の `#if SPROUT_EDITOR` 区間で定義し、書き出し時は除外される。公開ヘッダは `sprout/editor/`（ImGui を直接は見せず `EditorGui` ラッパを提供）。

```cpp
class CustomInspector { public: virtual void drawInspector(EditorGui& gui, SerializedObject& target) = 0; };
class PropertyDrawer  { public: virtual void draw(EditorGui& gui, SerializedProperty& property) = 0; };
class EditorTool      { public: virtual std::string_view title() const = 0; virtual void draw(EditorGui& gui) = 0; };
class AssetImporter   { public: virtual std::span<const std::string_view> extensions() const = 0; virtual Result<void> import(ImportContext& context) = 0; };

SPROUT_CUSTOM_INSPECTOR(EnemyInspector, Enemy)
SPROUT_EDITOR_TOOL(LevelGenerator, "Tools/Level Generator")
```
- `SerializedObject/Property` 経由の変更は自動で Undo・Prefab 上書き・dirty に反映（FR-802）
