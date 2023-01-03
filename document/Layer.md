Layer
====

## Layerの仕様
Layerとは、複数のItemをまとめて扱う、またDocument内のItemにツリー構造を導入する機能。
ItemをまとめてLock,Visibleする、表示順を変える、LayerModifierをかける等。
（ModifierはAIにおけるAppearance。Modifierの項を参照。（そんなものは現在存在しないが））


Layerの設計において、
- Group
- Bool(等のGroup派生)
を考慮する必要がある。
GroupはLayerと扱いが異なるものの、同じくItemをまとめて扱う機能であり、ツリー構造を作る。
そのためLayerとGroupで、コード共通化と処理分岐をどの程度とするか、考慮が必要となる。

追加要望として許されるならば、
- Git
も考慮したい。

- TopLevelは必ずLayerである必要があるか？(RootLayer, TopLevelLayerの子)
- Layer同階層に子LayerとItemの混載を許すか？

VECではLayer構造の制約について、以下の通りとする。
- RootLayerは必ず存在し削除できない
- RootLayerはユーザに操作できず、TreeViewからは非表示
- Layerの子にはLayerとItemが混在して良い
- RootLayerは他LevelのLayerと同じくLayerとItemが混在して良い
(この仕様はAIとは異なる。AIではLayerVeiw上で最上位レベルはLayerのみの配列となっている)
- ItemはLayerTree上の子を持つことはできない
- Layerの親は必ずLayerでなければならない
（つまり、子Layerを持てるのはLayerのみであり、GroupとItemは子Layerを持つことはできない）

## Layerの実装

データ構造
- メモリ上にあるdocumentのObjectをJSONファイル出力したものをファイル書き出し・ファイルフォーマットとしている都合から、引き続きオンメモリとファイルフォーマットは共通としたい。

- 素直にツリー
- Layer構造まで配列で持ち、メモリ上で参照またはIDを手繰る
- Layer構造のみツリーで持ち、Itemは別途配列
 - Layerに小要素

