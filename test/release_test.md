The release test for Vecterion Vector Graphic Editor
====  
Vecterionリリーステスト  

# About
Vecterionのリリーステストです。  
リリース前にかならず実施しなければなりません。__1回でいいです__。  
本テストで問題の見つかったパッケージをリリースするのはやめましょう。しないと思いますが。  


# Detail
テスト項目は、リストにて記述されます。  
テスト項目以外に、複数テストで共通の指示が普通の文章として記述されていることがあります。  
基本的に、最後の文が期待される結果。その前の文までがテストの手順です。  
環境依存(ex. Linux)や正常系以外(ex.准正常系)のテストは、そのように表記されます。  
未実装機能の将来のテストは、先頭コメントアウト"//"で表記されているのでスキップします。  


完璧な開発者は存在しません。  
完璧なテストは存在しません。  
無限の人数のテスタは用意できません。  
ハイゼンバグを確実に検出できる方法は存在しません。  
永遠にテストされているソフトウェアにリリース版は存在しません。  
だから、私たちは本テストを必ずリリース前に1回だけ実施します。  

それでも不満を感じるあなたへ： Vecterionは銀行の勘定系やペースメーカーのファームウェアではありませんよ！  


# 定義
Current：現在選択しているターゲット(ex. CurrentDocument, CurrentElement, CurrentTool)  


# テスト環境
- 108Keyboard(Keyboard with Tenkey)
- Mouse(with Wheel)


# テスト

## インストール & アプリケーションの起動
アプリケーションをインストールする。(アプリケーションの起動をもってインストール成功とする)  
/*  
 * (Linux環境のみ) ターミナルでソースファイルの.tar.gzを解凍し(`tar zxvf vecteroin-*.tar.gz`)、`sudo make install`を実行する。  
 * (Linux環境のみ) ターミナルで`vecterion_vge`を実行する。  
	vecterionが起動する。  
*/  
* (Linux環境のみ) ターミナルでソースファイルの.tar.gzを解凍し(`tar zxvf vecteroin-*.tar.gz`)、`make run`を実行する。
	vecterionが起動する。  
* (Windows環境のみ) 標準エクスプローラからWindows版配布ファイルのzipを解凍する。  
* (Windows環境のみ) 展開されたディレクトリ内にあるvecterion_vge.exeを実行する。
	Vecterionが起動する。  

## Documentの作成
* Ctrl+Nショートカット押下でDocument作成ダイアログを開き、適当なサイズでDocumentを作成する。
	Documentが作成される。  
* もう一度上記テストの動作を行う。
	2つ目のDocumentが作成される。作成されたDocumentがCurrentDocumentになる。  
* (准正常系) Document作成ダイアログで、Cancelする。
	Documentが作成されない。  

## Documentの編集
ツールに持ち替えてテストを行う。  

### Tool

#### AddAnchorPointTool

##### 開パス
指示がない限り、既存のAnchorPointと重ならない位置にAnchorPointを作成していく。  
* Focusのない状態で、Canvas内をクリックする。
	AnchorPathが作成される。  
* Focusのある状態で、Canvas内をクリックする。
	HandleのないAnchorPointが作成される。  
* Canvas内を適度にドラッグする。
	両端にAnchorPointから点対称なHandleのあるAnchorPointが作成される。  
* 開パスを書いた状態で、Pキーを押下し、Canvas内をクリックする。
	以前のAnchorPathはそのまま、AnchorPathが作成される。  
/*  
* 他のツールに持ち替えてからツールを戻し、Canvas内をクリックする。
	前回のAnchorPointに繋がっていないAnchorPointが作成される。  
*/  

##### 閉パス
* AnchorPointが2つあるElementCurveを作成し、Elementの先頭のAnchorPointをクリックする。
	先頭のAnchorPointのHandleが両端とも消え、ElementCurveが閉パスとなる。  
* AnchorPointが2つあるElementCurveを作成し、Elementの先頭のAnchorPointを適度にドラッグする。
	先頭のAnchorPointのHandleが両端とも上書きされ、ElementCurveが閉パスとなる。  
* 閉パスを作成した後、Canvas内をクリックする。
	以前のAnchorPathはそのまま、AnchorPathが作成される。  

##### 既存AnchorPathへのAnchorPoint追加
* 開パスを作成し、EditAnchorPointToolで先頭のAnchorPointをFocusし、AddAnchorPointToolでAnchorPointを追加する。
	先頭にAnchorPointが追加される。  
* 開パスを作成し、EditAnchorPointToolで末尾のAnchorPointをFocusし、AddAnchorPointToolでAnchorPointを追加する。
	末尾にAnchorPointが追加される。  
* 開パスを作成し、EditAnchorPointToolで先頭・末尾以外のAnchorPointをFocusし、AddAnchorPointToolでAnchorPointを追加する。
	以前のAnchorPathはそのまま、AnchorPathが作成される。  

#### EditAnchorPointTool
* Focusが無い状態で、AnchorPointをクリックする。AnchorPointがFocusされる。  
* Focusが無い状態で、Shiftを押しながらAnchorPointを2つクリックする。2つのAnchorPointがFocusされる。  

##### AnchorPoint移動
* AnchorPointを1つFocusしてドラッグする。AnchorPointが移動する。  
* AnchorPointを2つFocusしてドラッグする。AnchorPointが移動する。  

##### AnchorPointリサイズ
* AnchorPointを1つFocusして拡大・縮小する。AnchorPointが拡大・縮小しない。  
/*  
* 重なっていないAnchorPointを2つFocusして拡大・縮小する。AnchorPointが拡大・縮小する。  
*/  

##### AnchorPoint削除
* 2つ以上AnchorPointを持つAnchorPathから、先頭の1つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。  
* 2つ以上AnchorPointを持つAnchorPathから、末尾の1つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。  
* 3つ以上AnchorPointを持つAnchorPathから、先頭・末尾以外の1つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。AnchorPathが2つに分断される。  
* 3つ以上AnchorPointを持つAnchorPathから、先頭を含む連続した2つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。  
* 3つ以上AnchorPointを持つAnchorPathから、末尾を含む連続した2つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。  
* 4つ以上AnchorPointを持つAnchorPathから、先頭・末尾以外の連続した2つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。AnchorPathが2つに分断される。  
* 5つ以上AnchorPointを持つAnchorPathから、先頭・末尾以外の連続していない2つのAnchorPointをFocusした状態でDeleteショートカットを押下する。
	FocusされていたAnchorPointが削除される。AnchorPathが3つに分断される。  

#### EditElementTool

##### Element移動
* Elementを1つFocusしてドラッグする。Elementが移動する。  
* Elementを2つFocusしてドラッグする。Elementが移動する。  

##### Elementリサイズ
* Elementを1つFocusして拡大・縮小する。Elementが拡大・縮小する。  
* Elementを2つFocusして拡大・縮小する。Elementが拡大・縮小する。  

##### Element削除
* Elementを1つFocusして、Deleteショートカットを押下する。Elementが削除される。  
* Elementを2つFocusして、Deleteショートカットを押下する。Elementが削除される。  

#### EditAnchorHandleTool
* AnchorHandleのあるAnchorPointのHandleの先端をドラッグする。
	ドラッグした側のHandleがマウスに追随して変形する。  
* AnchorHandleのあるAnchorPointをクリックしてドラッグする。
	ドラッグした側のHandleがマウスに追随して変形し、反対のHandleが点対称に変形する。  

#### KnifeAnchorPathTool
本Toolの実行結果はAnchorPointを移動してみると判断できる。  
本Toolを使用した際、ElementCurveは切れるが、全体のAnchorPathの線の形状は変形しない。  
* 閉パスなCurveElementのAnchorPointをクリックする。
	クリックしたAnchorPointが同じ位置の始点と終点のAnchorPointになり、
	CurveElementが開パスになる。  
* 開パスなCurveElementのAnchorPointをクリックする。
	クリックしたAnchorPointが同じ位置の2点のAnchorPointになり、
	CurveElementがそこから切れて2つのCurveElementになる。  
* 閉パスなCurveElementのAnchorPath上をクリックする。
	クリックした位置が同じ位置の始点と終点のAnchorPointになり、
	CurveElementが開パスになる。  
* 開パスなCurveElementのAnchorPath上をクリックする。
	クリックした位置が同じ位置の2点のAnchorPointになり、
	CurveElementがそこから切れて2つのCurveElementになる。  

#### InsertAnchorPointTool
本Toolを実行した際、AnchorPathの線の形状は変形しない。
* CurveElementのAnchorPath上をクリックする。
	クリックした位置と同じ位置にAnchorPointが追加される。

### ColorPanel・StrokePanel
この項目では以下の属性をテスト対象属性と記述する。  
 ForgroundColor, BackgroundColor, StrokeWidth, StrokeLinecap, StrokeLinejoin  
* CurveElementをFocusした状態で、テスト対象属性を変更する。
	FocusされていたElementの描画に属性が反映される。  
* AddAnchorPointToolで、新しいCurveElementを作成する。
	テスト対象属性が反映されたElementが作成され、ColorPanel・StrokePanelの表示は変化しない。  
* テスト対象属性の値がすべて異なる別のElementをFocusする。
	ColorPanel・StrokePanelの表示がFocusしたElementのテスト対象属性の値になる。  
* テスト対象属性の値がすべて異なるElementを2つFocusする。
	ColorPanel・StrokePanelの表示が、複数のテスト対象属性の値を指している表示に変わる。  
* テスト対象属性の値がすべて異なるElementを2つFocusし、テスト対象属性をすべて変更する。
	ColorPanel・StrokePanelの表示とFocusしている2つのElementの描画に、属性が反映される。  

### Layer (Layer操作・LayerPanel)
Layer操作により変化したLayer構造が、LayerViewに反映されていること。  
* Ctrl+Shift+Nショートカットを押下する。
	FocusしているLayerの下に、新規Layerが作成される。  
* AddNewLayerアイコンをクリックする。
	FocusしているLayerの下に、新規Layerが作成される。  
* CopyLayerアイコンをクリックする。
	FocusしているLayerが複製される。  
* AddNewChildLayerアイコンをクリックする。
	FocusしているLayerの中に、新規Layerが作成される。  
* DeleteLayerアイコンをクリックする。
	FocusしているLayerが削除される。  
* Elementを含むLayerが1つのみ存在するDocumentで、DeleteLayerアイコンをクリックする。
	FocusしているLayerが削除され、空Layerが作成される。  
* (准正常系) 空Layerが1つのみ存在するDocumentで、DeleteLayerアイコンをクリックする。
	FocusしているLayerが削除されない。  

### Layer順序の並び替え
テストデータ：[test/raise_and_lower_release_test.svg](raise_and_lower_release_test.svg)  
LayerのFocusは、Layer内のElementへのFocusと、LeyerへのFocus(ElementをFocusしていない状態)の2通りをテストする。  
Layer順序の並び替えを行った際、Layer内のElementが並び替えられていないことを確認する。  
* 上に姉妹Layerが居るLayerをFocusし、Ctrl+Shift+PageUpショートカットを押下する。
	FocusしているLayerが1つ上に並び替えられる。  
* (准正常系)上に姉妹Layerが居ないLayerをFocusし、Ctrl+Shift+PageUpショートカットを押下する。
	Layerが並び替えられない。  
* 下に姉妹Layerが居るLayerをFocusし、Ctrl+Shift+PageDownショートカットを押下する。
	FocusしているLayerが1つ下に並び替えられる。  
* (准正常系)下に姉妹Layerが居ないLayerをFocusし、Ctrl+Shift+PageDownショートカットを押下する。
	Layerが並び替えられない。  
* 上に姉妹Layerが2つ以上居るLayerをFocusし、Ctrl+Shift+Homeショートカットを押下する。
	FocusしているLayerが姉妹Layerの一番上に並び替えられる。  
* (准正常系)上に姉妹Layerが居ないLayerをFocusし、Ctrl+Shift+Homeショートカットを押下する。
	Layerが並び替えられない。  
* 下に姉妹Layerが2つ以上居るLayerをFocusし、Ctrl+Shift+Endショートカットを押下する。
	FocusしているLayerが姉妹Layerの一番下に並び替えられる。  
* (准正常系)下に姉妹Layerが居ないLayerをFocusし、Ctrl+Shift+Endショートカットを押下する。
	Layerが並び替えられない。  

### Element順序の並び替え
テストデータ：[test/raise_and_lower_release_test.svg](raise_and_lower_release_test.svg)  
* 上に姉妹Elementが居るElementをFocusし、PageUpショートカットを押下する。
	FocusしているElementが1つ上に並び替えられる。
* (准正常系)上に姉妹Elementが居ないElementをFocusし、PageUpショートカットを押下する。
	Elementが並び替えられない。
* 下に姉妹Elementが居るElementをFocusし、PageDownショートカットを押下する。
	FocusしているElementが1つ下に並び替えられる。
* (准正常系)下に姉妹Elementが居ないElementをFocusし、PageDownショートカットを押下する。
	Elementが並び替えられない。
* 上に姉妹Elementが2つ以上居るElementをFocusし、Homeショートカットを押下する。
	FocusしているElementが姉妹Elementの一番上に並び替えられる。
* (准正常系)上に姉妹Elementが居ないElementをFocusし、Homeショートカットを押下する。
	Elementが並び替えられない。
* 下に姉妹Elementが2つ以上居るElementをFocusし、Endショートカットを押下する。
	FocusしているElementが姉妹Elementの一番下に並び替えられる。
* (准正常系)下に姉妹Elementが居ないElementをFocusし、Endショートカットを押下する。
	Elementが並び替えられない。

### PosotionPanel
2つ以上のElementをFocusすると、位置はElementsの最も左上と右下を囲った矩形領域となる。  
* Elementを1つFocusする。
	PositionPanelの値にFocusしているElementの位置が反映される。  
* Elementを2つFocusする。
	PositionPanelの値にFocusしているElementの位置が反映される。  
* Elementを1つFocusし、値を増減する。
	PositionPanelの値がFocusしているElementの位置に反映される。  
* Elementを2つFocusし、値を増減する。
	PositionPanelの値がFocusしているElementの位置に反映される。  
* Elementを1つFocusし、値を入力しEnterキーを押下する。
	PositionPanelの値がFocusしているElementの位置に反映される。  

### Undo/Redo (History)
* Documentを編集する。Ctrl+Zショートカットを押下する。
	Undoされる。  
* Undoする。Ctrl+Shift+Zショートカットを押下する。
	Redoされる。  
* Document作成時点までUndoされたDocumentをUndoして、Redoする。
	Redoされる。  
* HistoryにDocument作成時点が残らない回数まで、Documentを編集する。履歴の最過去までUndoする。
	履歴の最過去までUndoされる。  
* (准正常系) Documentを編集する。Document作成時点までUndoする。Undoする。
	Undoされない。  
* (准正常系) Undoする。Documentを編集する。Redoする。
	Redoされない。  

## Documentの表示

### CanvasCollection

#### Document表示(Canvas)のZoom倍率変更
表示の拡大・縮小に合わせて、Document表示倍率が増減する  
* Canvas内にカーソルを配置し、Alt+マウスホイールUpする。
	CurrentCanvasの表示が拡大される。  
* Canvas内にカーソルを配置し、Ctrl++ショートカットを押下する。
	CurrentCanvasの表示が拡大される。  
* Canvas内にカーソルを配置し、Ctrl++(TenKey上の+)ショートカットを押下する。
	CurrentCanvasの表示が拡大される。  
* Canvas内にカーソルを配置し、Alt+マウスホイールDownする。
	CurrentCanvasの表示が縮小される。  
* Canvas内にカーソルを配置し、Ctrl+-ショートカットを押下する。
	CurrentCanvasの表示が縮小される。  
* Canvas内にカーソルを配置し、Ctrl+-(TenKey上の-)ショートカットを押下する。
	CurrentCanvasの表示が縮小される。  

### ThumbnailView
* CucrrentDocumentを変える。
	Thumbnailが変更後のCurrentDocumentに変わる。  

## DocumentのSave
Saveは、Saveで書き込まれたファイルを開いてみて成功を確認する。  
* 新規作成した未SaveのDocumentを開いた状態で、Ctrl+Sショートカットを押下し、
  Save確認ダイアログにて、適当なパスに半角空白と日本語を含む名前でSaveを実行する。
	DocumentがSaveされる。  
* Save済みのDocumentを開いた状態で、Ctrl+Sショートカットを押下する。
	DocumentがSaveされる。  
* (准正常系) Save確認ダイアログで、Cancelする。
	Saveされない。  

## Documentを閉じる
* Save済みでないDocumentを開いた状態で、Ctrl+Wショートカットを押下し、Close確認ダイアログでCloseを実行する。
	DocumentがCloseされる。  
* Save済みのDocumentを開いた状態で、Ctrl+Wショートカットを押下する。
	DocumentがCloseされる。  
* (准正常系) Close確認ダイアログで、Cancelする。
	Closeされない。  

## Documentを開く
* Ctrl+Oショートカットを押下し、ファイルOpenダイアログを開き、「テスト：DocumentのSave」でSaveしたファイルをOpenする。
	SaveしたファイルがOpenされる。  
* (准正常系) ファイルOpenダイアログで、Cancelする。
	Openされない。  

## アプリケーションの終了
* セーブ済みでないのDocumentが有る状態で、Ctrl+Qショートカットを押下する。確認ダイアログでQuitを実行する。
	VecterionがQuitされる。  
* セーブ済みでないファイルが無い状態で、Ctrl+QでアプリケーションをQuitする。
	VecterionがQuitされる。  
* (准正常系) ファイルQuitダイアログで、Cancelする。
	DocumentとVecterionがQuitされない。  

## アンインストール
アプリケーションをアンインストールする。アンインストールされる。  
* (Windows環境のみ) zip解凍して展開したディレクトリを削除する。  
/*  
* (Linux環境のみ) インストール時に展開したディレクトリから、`sudo make uninstall`を実行する。  
*/  


