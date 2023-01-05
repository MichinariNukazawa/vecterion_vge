'use strict';

let lodash = require('lodash');
var  diff  = require('deep-diff') 
const clonedeep = require('lodash/cloneDeep');
const {Base64} = require('js-base64');
const xmlFormatter = require('xml-formatter');

const PACKAGE = require('./../package.json')
const DOC_DEFAULT = require('./doc_default.json');
const StrCalc = require('../src/strcalc')
const PV = require("../src/pv.js");
const History = require("../src/history.js");
const Strage = require("../src/strage.js");
const Render = require("../src/render.js");
const Tool = require("../src/tool.js");
const TOOLS = Tool.TOOLS();

// documentの最大サイズ（値は適当）
const DOCUMENT_MAX_SIZE_AXIS_PX = 40000;

const EDITOR_MOUSE_OF_MOUSEUP_DEFAULT = {
	'button': 0,
	// select: 選択領域
	// move: 選択item移動
	'mode': '',
	// mousedownからポインタの移動量が一定距離を超えた。
	// 一旦離れてから戻ってきた場合にドラッグ中であることを止めないためにフラグ管理。
	'isMoveLock': true,
	// マウス押下時のShiftKey状態
	'shiftKey': false, // TODO いずれ「マウス押下時情報」をまとめるかどうかを含め、整理をする
	//
	'touchedItemIndex': -1,
	'touchedApIndex': -1,
	'touchedAPAHKind': '',
	// ** 角度にスナップ
	// 移動・拡縮・回転において45度単位ガイドへ操作固定する位置制御（Shiftキー押下による）
	// MEMO AI的には特に名前のない些末機能のようだが「角度にスナップ」としておく。
	'isRegulationalPositionByCenter': false, // 角度にスナップが有効
	'regulationalPositionByCenterDegree': -1, // 使用中のスナップ角度
	'editCenter': undefined,
	// 時間のかかる処理をmousemove中に行わずmouseupするまでサボるフラグ
	// MEMO 現状、これを使うような処理は高速・軽量化しなければならないものではないかと思っている。
	'isLatedFocusChangeInMousemoveCallbacked_': false,
	// **
	'snapAxisXKind': undefined,
	'snapAxisYKind': undefined,
	'snapAxisXTargetPoint': undefined,
	'snapAxisYTargetPoint': undefined,
	'snapItemApCplxX': undefined,
	'snapItemApCplxY': undefined,
};

const EDITOR_HOVER_DEFAULT = {
	'touchedItemIndex': -1,
	'touchedApAhCplx': [-1, -1, undefined],
	// between上の最も近いpointを表示する
	'nearTouchPointOnBetween': undefined,
};

let editor = {
	// ** tool
	'toolIndex': 3,
	// Pキー押下など、APTool選択・再選択した後は、Focusと関係なく新しいBezierを作成
	'isAddAnchorPoindByAAPTool': false,
	// Escapeによるcancel動作を実現するために用いる
	'isHistoryStacked': false,
	//
	'clipboard': {
		'pasteCount': 0,
		'items': [],
	},
	// ** mouse
	'mouse': deepcopy(EDITOR_MOUSE_OF_MOUSEUP_DEFAULT),
	// ** hover 
	// SPEC mousedownした際の編集箇所を示す
	'hover':  deepcopy(EDITOR_HOVER_DEFAULT),
	// ** 各ツール毎固有の変数
	'apAddTool':{
		'isGrowthTailAp': true,
	},
};

let palette = {
	'isFill': false, // fill/stroke
	'colorPair': {
		'fillColor': '#ffffffff',
		'strokeColor': '#000000ff',
	},
};
let border = {
	'width': 8,
	'linecap': 'butt',
	'linejoin': 'miter',
};

let field = {
	'canvas': {
		'scale': 1.0,
	},
	'touch': {
		'pointR': 10,
		'cornerWidth': 25,
	},
};

let strage;
let hist;

// 新しいドキュメントを作成し、canvasとhistoryにそれをセットする。
function startNewDocumentOpen(){
	const doc = deepcopy(DOC_DEFAULT);
	strage.addCurrentDocument(doc, ''); // 無名のドキュメントを追加

	hist.setDoc(doc);
	renderingAll();
	fittingCanvasScale();
}
function startDocoumentOpen(id){
	console.log('opendocument', id);
	if(id === strage.getCurrentDocId()){
		// SPEC DocSelectでOpen中のDocを選択した場合にそのまま（Historyをクリアしないように）
		console.log('document already opened', id);
		return;
	}
	let docconf = strage.loadDocConfig();
	if(! docconf){
		console.error('BUG');
		setMessage('error dont open internal error.')
		return;
	}
	const ix = docconf.docinfos.findIndex(e => e.id === id);
	if(-1 === ix){
		console.error('BUG', docconf);
		setMessage('error dont open internal error.')
		return;
	}
	const [currentdocinfo] = docconf.docinfos.splice(ix, 1);
	docconf.docinfos.unshift(currentdocinfo);
	try{
		localStorage.setItem('vecterion-doc-config', JSON.stringify(docconf, null, '\t'));
	}catch(e){
		console.warn(e);
		alert('error: localStrage full. try delete document.')
	}

	const dockey = `vecterion-doc-${currentdocinfo.id}`;
	const sdoc = localStorage.getItem(dockey)
	if(! sdoc){
		console.error('localStrage loading error.');
		setMessage('error dont open internal error.')
		return;
	}
	const doc = JSON.parse(sdoc);
	console.log(`loaded doc '${dockey}'`, currentdocinfo.id)

	strage.setCurrentDocument(currentdocinfo.id);

	hist.setDoc(doc);
	renderingAll();
	fittingCanvasScale();
}

function strageCurrentDocChangedCallback(currentDocInfo){
	console.log('call strage changed callback.', currentDocInfo);
	const name = ('' == currentDocInfo.name) ? '<untitled>' : currentDocInfo.name;
	document.title = `${name}.vtd - vecterion`;
}

function historyChangedCallback(kind, ix, num) {
	// MEMO ここのコールバックで再描画を呼んでしまうと、現行実装では描画回数が2倍になるだけなのでやらない。呼び出し元でがんばること。
	// TODO mousemove中や例外的なものを除いてむしろここに再描画を集約すべきか？
	console.log('call history changed', kind);
	document.getElementById('history').textContent = `${ix}/${num} - ${kind}`

	updateLayerView();
	checkFocusCurrentTopItemChangedCallback();
	strage.saveCurrentDoc(hist.now());
	updateThumbnail();
}
function updateThumbnail(){
	const RECT_AXIS_PX = 64;
	let svg = document.querySelector("svg");
	let svgData = new XMLSerializer().serializeToString(svg);
	let canvas = document.createElement("canvas");
	canvas.width = RECT_AXIS_PX;
	canvas.height = RECT_AXIS_PX;

	const doc = hist.now();

	let ctx = canvas.getContext("2d");
	let image = new Image;
	image.onload = function(){
		ctx.drawImage(
			image,
			doc.canvas.padding.w,
			doc.canvas.padding.h,
			toField(doc.size.w),
			toField(doc.size.h),
			0, 0, RECT_AXIS_PX, RECT_AXIS_PX);
		let data = canvas.toDataURL("image/png");
		const thumbkey = `vecterion-doc-thumb-${strage.getCurrentDocId()}`;
		try{
			localStorage.setItem(thumbkey, data);
		}catch(e){
			console.warn(e);
			alert('error: localStrage full. try delete document.')
		}
		//var a = document.createElement("a");
		//a.href = data;
		//a.setAttribute("download", "image.png");
		//a.dispatchEvent(new MouseEvent("click"));
		console.log('update thumb', strage.getCurrentDocId(), thumbkey);
	}
	image.src = "data:image/svg+xml;charset=utf-8;base64,"
		+ Base64.encode(svgData);
		//btoa(unescape(encodeURIComponent(svgData))); 
}

const AHKIND = {
	'Symmetry': 'symmetry',
	'Free': 'free',
	'None': 'none',
};

// canvasに使うSVGを掴んでおく
let renderingHandle;
// canvas scale変更した際の表示位置を調整するのに用いる。
let prevEditFieldSizeByRescalingCanvas;
// マウスポインタ位置を保持する
// (callbackのタイミングでしか取得できないため)
let mousedownInEditField
let mousemoveInEditField;
let mousedownInCanvas;
let mousemoveInCanvas;
let mousedownMoreInfo;
let isDisableHistoryUpdate = false;

let docItemSrc;

let confirmOkFunc;
let confirmCancelFunc;

function deepcopy(obj) { return clonedeep(obj); }
// canvas to field scale convert
function toField(v) { return v * field.canvas.scale; }
function toCanvas(v) { return v / field.canvas.scale; }

window.addEventListener('load', (event) => {
	console.log(`appname:'${PACKAGE.name}' ver:'${PACKAGE.version}'`);

	// ** ad
	{
		// AD用のcssを追加することで、ADを表示
		let head = document.getElementsByTagName('head')[0];
		let link = document.createElement('link');
		link.setAttribute('rel','stylesheet');
		link.setAttribute('type','text/css');
		link.setAttribute('href','css/ad.css');
		head.appendChild(link);
	}
	// ** About Dialog
	{
		document.getElementById('about-dialog-button').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			let linkE = document.getElementById('startup-link');
			if(linkE){
				linkE.remove();
			}
			document.getElementById('webmenu-btn-check').checked = false;
			// versionはsetTimeout()トリックなどをかけても起動時に置換されなかったので、
			// 起動中は非表示にしておくことにする。
			document.getElementById('startup-version-number').textContent = PACKAGE.version;
			document.getElementById('startup-version').style.display = 'block';
			document.getElementById('startup-dialog-close-elem').style.display = 'block';
			let dialog = document.getElementById('startup-dialog');
			dialog.style.top = 0;
			dialog.showModal();
		});
	}

	// **
	setTimeout(loadAfterRun, 0);
});

function loadAfterRun(){
	strage = new Strage(strageCurrentDocChangedCallback);
	strage.check();

	hist = new History(historyChangedCallback);

	const doc = strage.loadOrNewCurrentDoc();
	hist.setDoc(doc);
	fittingCanvasScale();

	initContextmenu();

	/* Ctrl+wheelでのページ拡縮を抑制し、canvas内ではスケール変更にする */
	document.body.addEventListener('wheel', event => {
		if (event.ctrlKey) {
			//console.log('')
			event.preventDefault();
		}
	}, { passive: false })

	document.getElementById('edit-field-frame').addEventListener('wheel', event => {
		if (event.altKey) {
			console.log('wheel', event.deltaY, event)
			steppingCanvasScale((0 < event.deltaY) ? -1 : 1); // AI合わせでupで拡大
			event.preventDefault();
			return;
		}
		if (event.ctrlKey) {
			document.getElementById('edit-field-frame').scrollLeft += (0 < event.deltaY) ? 100 : -100; // 値は根拠なし
			event.preventDefault();
			return;
		}
	}, true)
	/* mouse position */
	document.getElementById('edit-field').addEventListener('mousedown', event => {
		console.log('mousedown edit-field', event.offsetX, event.offsetY);
		// **
		mousedownInEditField = {
			'x': event.offsetX,
			'y': event.offsetY,
		};
		mousedownInCanvas = {
			'x': (mousedownInEditField.x - hist.now().canvas.padding.w) / field.canvas.scale,
			'y': (mousedownInEditField.y - hist.now().canvas.padding.h) / field.canvas.scale,
		};
		const fieldFrame = document.getElementById('edit-field-frame');
		mousedownMoreInfo = {
			'button': event.button,
			'point': { 'x': event.offsetX, 'y': event.offsetY },
			'pointInPage': { 'x': event.pageX, 'y': event.pageY },
			'scroll': {
				'top': fieldFrame.scrollTop,
				'left': fieldFrame.scrollLeft,
			},
		};
		console.log('mousedown edit-field-frame', mousedownMoreInfo);
		editor.isHistoryStacked = false;
		editor.mouse.button = event.button;
		editor.mouse.shiftKey = event.shiftKey;
		editor.mouse.isMoveLock = true;
		isDisableHistoryUpdate = true;

		// **
		const point = mousedownInCanvas;

		switch (event.button) {
		case 0:{ // 左ボタン
			switch (editor.toolIndex) {
			case TOOLS.ItemEditTool.Id:
			{
				const mode = getMouseModeByMousePoint(point);
				if('' !== mode && 'move' !== mode){
					// SPEC マウスが拡縮・回転に当たっていた場合、Item選択ではなくそちらのモードへ。
					editor.mouse.mode = mode;
				}else{
					editor.mouse.touchedItemIndex = getTouchedItem(hist.now().items, point);
					const isTouched = (-1 !== editor.mouse.touchedItemIndex);

					if (event.shiftKey) {
						// SPEC Shift押下しながらItemに触れた場合、FocusをON/OFFするモード。
						if(! isTouched){
							// SPEC Shift+Item未接触の場合、何もしない。
							// （Focusクリアしてもよいのだが、操作ミスっぽいのでやめておくことにする）
						}else{ // SPEC Shift+Item接触した場合
							if (! hist.now().focus.focusItemIndexes.includes(editor.mouse.touchedItemIndex)) {
								// SPEC Shift+未FocusItemと接触の場合、ItemをFocusに加える
								Focus_addItemIndex(editor.mouse.touchedItemIndex);
							} else {
								if(hist.now().focus.focusItemIndexes.at(-1) === editor.mouse.touchedItemIndex){
									// SPEC Shift+Focus済み(currentItemである)と接触の場合、ItemからFocusを外す。
									// SPEC ...という判定を、Shiftキー押下による方向スナップ（水平移動等）の誤操作と区別するため、mouseup時に行う。
								}else{
									// SPEC Shift+Focus済み(非currentItem)と接触の場合、currentItemへ変更。
									Focus_addItemIndex(editor.mouse.touchedItemIndex);
								}
							}
						}
					}else{ // Shift押下していない場合
						if(! isTouched){
							// SPCE Itemに触れていない場合、Focusをクリア。
							Focus_clear();
						}else{
							if (! hist.now().focus.focusItemIndexes.includes(editor.mouse.touchedItemIndex)) {
								// SPCE 未FocusItemに触れた場合、触れたItemのみをFocus。
								Focus_clear();									
								Focus_addItemIndex(editor.mouse.touchedItemIndex);
							}else{
								// SPCE FocusずみItemに触れた場合、そのItemをcurrentItemへ変更。
								Focus_addItemIndex(editor.mouse.touchedItemIndex);
								// SPCE FocusずみItemに触れた場合かつ、さらにmousemoveせずupした場合、触れたItemのみをFocus。
								//（もちろんこの処理はup側で実装）
							}
						}
					}

					// ** mousemove中のモード選択
					// SPEC デフォルトではrectselectの条件を満たすまで待つモード。
					editor.mouse.mode = 'notmove';
					if (isTouched && (0 !== hist.now().focus.focusItemIndexes.length)) {
						// SPEC mousedownでItemに当たった場合、mousemoveはItem移動(矩形選択はしない)
						// SPEC （Shiftキー押下しながらで）Focusがすべて外れた場合は、それはItem移動に入らない 
						editor.mouse.mode = 'move';
					}
				}

				console.log('mousedown TOOLS.ItemEditTool', editor.mouse.mode, event.shiftKey);

				renderingAll();
			}
				break;
			case TOOLS.APEditTool.Id:
			{
				const [touchedItemIndex, touchedApIndex, touchedAPAHKind] = getTouchedApAh(hist.now().items, undefined, point);
				editor.mouse.touchedItemIndex = touchedItemIndex;
				editor.mouse.touchedApIndex = touchedApIndex;
				editor.mouse.touchedAPAHKind = touchedAPAHKind;
				const isTouched = (!! touchedAPAHKind);

				if (event.shiftKey) {
					if(! isTouched){
					}else{
						if (! Focus_isExistAP(touchedItemIndex, touchedApIndex)) {
							Focus_addAP(touchedItemIndex, touchedApIndex);
						} else {
							if((hist.now().focus.focusAPCplxes.length - 1) === Focus_apIxFromAPIndexCplx(touchedItemIndex, touchedApIndex)){
								Focus_removeAP(touchedItemIndex, touchedApIndex);
							}else{
								Focus_addAP(touchedItemIndex, touchedApIndex);
							}
						}
					}
				}else{
					if(! isTouched){
						Focus_clear();
					}else{
						if (! Focus_isExistAP(touchedItemIndex, touchedApIndex)) {
							Focus_clear();									
							Focus_addAP(touchedItemIndex, touchedApIndex);
						}else{
							Focus_addAP(touchedItemIndex, touchedApIndex);
						}
					}
				}

				// ** mousemove中のモード選択
				if (isTouched) {
					editor.mouse.mode = 'move';
				}else{
					editor.mouse.mode = 'notmove';
				}

				console.log('mousedown TOOLS.APEditTool', editor.mouse.mode, isTouched, event.shiftKey);
				renderingAll();
			}
				break;
			case TOOLS.APAddTool.Id:
			{
				let currentItem = Focus_currerntItem()

				if ((!! currentItem) && ('Bezier' === currentItem.kind) && (! currentItem.isCloseAp)){
					// ** SPEC currentBezierな開パスの逆端APに触れた場合、パスを閉じる
					if((editor.isAddAnchorPoindByAAPTool) && (2 <= currentItem.aps.length)){
						// SPEC APを伸ばしている反対のAPと触れた場合、パスを閉じる。つまり、
						// SPEC 末尾APをFocusしている場合、先頭APと触れたらパスを閉じる
						// SPEC 先頭APをFocusしている場合、末尾APと触れたらパスを閉じる
						let ap;
						if(editor.apAddTool.isGrowthTailAp){
							ap = Focus_currentFirstAp();
						}else{
							ap = Focus_currentLastAp();
						}
						if (PV.isTouchPointAndPoint(ap.point, point, toCanvas(field.touch.pointR))) {
							console.log('close AP');
							currentItem.isCloseAp = true;
							if(editor.apAddTool.isGrowthTailAp){
								Focus_setCurrentAP(Focus_currerntItemIndex(), 0);
							}else{
								Focus_setCurrentAP(Focus_currerntItemIndex(), (currentItem.aps.length - 1));
							}
							renderingAll();
							break;
						}
					}
					
					// ** SPEC currentBezierな開パスの端APに触れた場合、開パスを延伸（できるFocus状態へ遷移する）
					if(! editor.isAddAnchorPoindByAAPTool){
						// SPEC AnchorPoint追加モードでない状態で、FocusItemの先頭・末尾のAPに触れた場合、触れたAPからAPを生やす状態へ入る。
						// MEMO FocusItemが複数の場合について、処理をどうするかは実装都合の一番いいやつにしておくことにする。
						// MEMO 実装の都合により、当たり判定の優先度はZ順にはしていない。
						// AP自体は未Focusでも含めることとした。
						const isAccepted = !PV.exforReverse(hist.now().focus.focusItemIndexes, (ix, itemIndex) => {
							const itemt = hist.now().items[itemIndex];
							if((! Render.isMetaVisible(itemt)) || Render.isMetaLock(itemt, hist.now())){
								return true;
							}
							const firstAP = itemt.aps.at(0);
							const lastAP = itemt.aps.at(-1);
							if((!! lastAP) && PV.isTouchPointAndPoint(point, lastAP.point, toCanvas(field.touch.pointR))){
								Focus_addAP(itemIndex, (currentItem.aps.length - 1));
								editor.isAddAnchorPoindByAAPTool = true;
								editor.apAddTool.isGrowthTailAp = true;
								console.log('APAddTool touched lastAP', itemIndex, lastAP);
								return false;
							}
							if((!! firstAP) && PV.isTouchPointAndPoint(point, firstAP.point, toCanvas(field.touch.pointR))){
								Focus_addAP(itemIndex, 0);
								editor.isAddAnchorPoindByAAPTool = true;
								editor.apAddTool.isGrowthTailAp = false;
								console.log('APAddTool touched firstAP', itemIndex, firstAP);
								return false;
							}
							return true;
						});
						if(isAccepted){
							renderingEditorUserInterface();
							break;
						}
					}

					// ** SPEC currentBezierと別の開パスの端APに触れた場合、開パス同士を繋げる
					if(editor.isAddAnchorPoindByAAPTool){
						const [touchedItemIndex, touchedApIndex] = getTouchedAp(getNowFocusItems(), point);
						const cplx = hist.now().focus.focusAPCplxes.at(-1);
						if((!! cplx) && (-1 !== touchedApIndex)){
							if(joinItem(cplx[0], cplx[1], touchedItemIndex, touchedApIndex)){
								renderingAll();
								break;
							}
						}
					}
				}

				// ** 新規パス作成
				if((! currentItem)
				 || (! editor.isAddAnchorPoindByAAPTool)
				 || ('Bezier' !== currentItem.kind)
				 || (currentItem.isCloseAp))
				{
					// SPEC 次の場合に、Bezierを新たに作成。
					// SPEC - Focus Itemが無い
					// SPEC - AnchorPoint追加モードでない
					// SPEC - ItemがBezierでない（AP追加できない）
					// SPEC - Itemが閉パスである（すでにCloseしているItemにはAPを追加しない。）
					Focus_clear();
					const newItem = newBezierItem(point)
					hist.now().items.push(newItem);
					currentItem = newItem;
					Focus_addItemIndex(hist.now().items.length - 1);
					Focus_setCurrentAP(Focus_currerntItemIndex(), (currentItem.aps.length - 1));
					editor.isAddAnchorPoindByAAPTool = true;
					setMessage('new Item(Bezier).');
					renderingAll();
					return;
				}

				// ** AP追加
				{
					if(editor.apAddTool.isGrowthTailAp){
						// SPEC 基本的には順当に末尾方向にAPを追加
						currentItem.aps.push(PV.AP_newFromPoint(point));
						Focus_setCurrentAP(Focus_currerntItemIndex(), (currentItem.aps.length - 1));
						console.log('add AP to foot');
					}else{
						// SPEC 先頭APを生やす状態に遷移していた場合のみ、頭方向にAPを追加
						currentItem.aps.unshift(PV.AP_newFromPoint(point));
						Focus_setCurrentAP(Focus_currerntItemIndex(), 0);
						console.log('add AP to head');
					}
				}
				renderingAll();
			}
				break;
			case TOOLS.AHEditTool.Id:
			{
				// SPEC AHEditToolでAPを触っただけの場合、AHを削除する(AHの種別を切り替える)(mouseup時に判定する)
				// SPEC - none以外の場合、noneに切り替える
				// SPEC - noneの場合何もしない
				// SPEC AHEditToolでAHをtouchしただけの場合、touchしたAHを削除する(サイズゼロにする)(mouseup時に判定する)
				// SPEC AHEditToolでAPをtouchして動かした場合、AHをsymmetory種別にして動かす
				// SPEC AHEditToolでAHをtouchして動かした場合、AHをfree種別にして動かす
				// AHの小さいAPを触った場合、touch判定の優先度はAHとする。
				// （小さなAHを触ったつもりでsymmetryやAH削除が起こると混乱するのではと思うので）

				const [touchedItemIndex, touchedApIndex, touchedAPAHKind] = getTouchedApAh(hist.now().items, hist.now().focus.focusItemIndexes, point);
				if(! touchedAPAHKind){
					break;
				}
				editor.mouse.touchedItemIndex = touchedItemIndex;
				editor.mouse.touchedApIndex = touchedApIndex;
				editor.mouse.touchedAPAHKind = touchedAPAHKind;
				Focus_addAP(touchedItemIndex, touchedApIndex);
			}
				break;
			case TOOLS.APInsertTool.Id:{
				insertAp()
				// insert直後のbetween点表示は消す。作成したAPと重なって見ずらくなるので。
				editor.hover.nearTouchPointOnBetween = undefined;
				renderingAll();
			}
				break;
			case TOOLS.APKnifeTool.Id:{
				// SPEC まずAPにtouchしているかを判定し、触れていなければ近いbetweenを探す。
				// SPEC APに触れていればAPで切断を行う。
				const [touchedItemIndex, touchedApIndex] = getTouchedAp(getNowFocusItems(), point);
				if(-1 !== touchedApIndex){
					Item_splitByAp(touchedItemIndex, touchedApIndex);
					renderingAll();
					break;
				}

				// SPEC APに触れていなかったので、近いbetweenを探す。
				// SPEC betweenの接触点にAPを追加し、そのAPで切断を行う。
				const [insertedItemIndex, insertedApIndex] = insertAp();
				if(-1 === insertedApIndex){
					console.log('BUG', insertedItemIndex, insertedApIndex);
					break;
				}
				Item_splitByAp(insertedItemIndex, insertedApIndex);
				renderingAll();

			}
				break;
			case TOOLS.FigureAddTool.Id:{
				const figure = newFigureItem(point);
				hist.now().items.push(figure);
				Focus_clear();
				Focus_addItemIndex(hist.now().items.length - 1);
				setMessage('new Item(Figure).');
				renderingAll();
				}
				break;
			case TOOLS.GuideAddTool.Id:{
				// アイテムを掴んだらそれをmove...したいので実装簡略化のためにToolを切り替えてしまう。
				const [touchedItemIndex, touchedApIndex] = getTouchedAp(hist.now().items, point);
				if(-1 !== touchedApIndex){
					const item = hist.now().items.at(touchedItemIndex);
					if('Guide' === item.kind){
						setToolIndex(TOOLS.ItemEditTool.Id);
						Focus_setCurrentAP(touchedItemIndex, touchedApIndex);
						editor.mouse.mode = 'move';
						renderingAll();
						break;
					}
				}

				const axis = document.getElementById('guide-axis').value;
				const guide = newGuideItem(point, axis);
				hist.now().items.push(guide);
				Focus_clear();
				Focus_addItemIndex(hist.now().items.length - 1);
				setMessage('new Item(Guide).');
				renderingAll();
				}
				break;			
			default:
				console.log('tool not implement', editor.toolIndex);
			}

			switch(editor.mouse.mode){
			case 'move':
			case 'rotate':{
				// TODO AP１つのみの場合などのeditCenter
				if(!! editor.mouse.touchedAPAHKind){
					const ap = Focus_nowCurrerntAP();
					switch(editor.mouse.touchedAPAHKind){
					case 'ap':
						editor.mouse.editCenter = ap.point;
						break;
					case 'front':
						editor.mouse.editCenter = PV.AP_frontAHPoint(ap);
						break;
					case 'back':
						editor.mouse.editCenter = PV.AP_backAHPoint(ap);
						break;
					default:
						console.error('BUG', editor.mouse.touchedAPAHKind);
					}
				}else{
					const focusRect = Render.getFocusingRectByNow(editor, hist.now());
					if(! focusRect){
						console.error('BUG');
						break;
					}
					editor.mouse.editCenter = PV.centerFromRect(focusRect);
				}
			}
				break;
			}
		}
			break;
		case 1: // 中ボタン
			editor.mouse.mode = 'move_field';
			fieldFrame.style.cursor = 'move';
		case 2: // 右ボタン
		default:
			break;
		}
	}, true)
	document.getElementById('edit-field').addEventListener('mouseup', event => {
		console.log('mouseup edit-field');
		mouseupEditFieldFrameCallback();
	}, true)
	const mouseupEditFieldFrameCallback = () => {
		switch (editor.mouse.button) {
		case 0: // 左ボタン
			switch (editor.toolIndex) {
			case TOOLS.ItemEditTool.Id:
			{
				if(editor.mouse.shiftKey){
					if(hist.prev().focus.focusItemIndexes.at(-1) === editor.mouse.touchedItemIndex){
						if(editor.mouse.isMoveLock){
							// SPEC Shift+Focus済み(currentItemである)と接触の場合、ItemからFocusを外す。
							// MEMO 水平移動しようとして先にShiftキーを押した状態でCurrentFocusItemを触る場合が考えられる。
							//      （というか『Shiftキーを押しながらの垂直・水平移動等』、AIの仕様策定のミスな感じある。）
							// MEMO 直前のmousedownでFocusされてcurrentItemになった場合と区別しないと、FocusしたばかりのItemからFocusが失われる不具合が発生する。
							Focus_removeItemIndex(editor.mouse.touchedItemIndex);
						}
					}
				}else{ // Shift押下していない場合
					if(-1 !== editor.mouse.touchedItemIndex){
						if(editor.mouse.isMoveLock){
							// SPCE FocusずみItemに触れた場合かつ、さらにmousemoveせずupした場合、触れたItemのみをFocus。
							// （未FocusItemに触れた場合について、結局触れたItemのみになっているはずなので、ここでの分岐は省略とする。）
							Focus_clear();
							Focus_addItemIndex(editor.mouse.touchedItemIndex);						
						}
					}
				}
			}
				break;
			case TOOLS.APEditTool.Id:
			{
				if(! editor.mouse.shiftKey){
					if(-1 !== editor.mouse.touchedItemIndex){
						if(editor.mouse.isMoveLock){
							Focus_clear();
							Focus_addAP(editor.mouse.touchedItemIndex, editor.mouse.touchedApIndex);
						}
					}
				}
			}
				break;
			case TOOLS.AHEditTool.Id:{
				if(! editor.mouse.isMoveLock){
					break;
				}

				let ap = Focus_nowCurrerntAP();

				// SPEC サイズゼロのFront/BackのAHを触って動かなかった場合、
				// SPEC APをクリックしたことにしてAHを削除する。
				// SPEC (AHが重なっていただけでユーザとしてはAPをクリックしたのだと考えることとする。)
				// MEMO mousedown時点では、サイズゼロのAHを伸ばそうとしている可能性もあるため、
				//      upのここで判定するしかない
				switch(editor.mouse.touchedAPAHKind){
				case 'front':
					if(PV.fuzzyZeroPoint(ap.handle.frontVector)){
						console.log('touch up frontVector zero size, change to ap');
						editor.mouse.touchedAPAHKind = 'ap';
					}
					break;
				case 'back':
					if(PV.fuzzyZeroPoint(PV.AP_backVector(ap))){
						console.log('touch up backVector zero size, change to ap');
						editor.mouse.touchedAPAHKind = 'ap';
					}
					break;
				}

				switch(editor.mouse.touchedAPAHKind){
				case 'ap':{
					ap.handle.kind = AHKIND.None;
					ap.handle.frontVector = PV.ZeroPoint();
					ap.handle.backVector = PV.ZeroPoint();
				}
					break;
				case 'front':{
					if(AHKIND.Symmetry == ap.handle.kind){ // symmetryを外す前にbackVectorに値をセットする
						ap.handle.backVector = PV.pointMul(ap.handle.frontVector, PV.MinusPoint());
					}
					ap.handle.kind = AHKIND.Free
					ap.handle.frontVector = PV.ZeroPoint();	
				}
					break;
				case 'back':{
					ap.handle.kind = AHKIND.Free
					ap.handle.backVector = PV.ZeroPoint();
				}
					break;
				case undefined:
				default:
					console.error('BUG', editor.mouse.touchedAPAHKind);
				}
				const sanitizeAPAH = (ap) => {
					// APのAH編集終了時に呼び出すこと。
					// symmetryの場合のbackVectorの扱いどうしよう感。
					// if(AhKind.Symmetry == ap.handle.kind){
					// 	ap.handle.backVector = PV.pointMul(ap.handle.frontVector, PV.MinusPoint());
					// }
					// AHがゼロサイズになっていた場合に種別noneに切り替える
					if(PV.fuzzyZeroPoint(ap.handle.frontVector) && PV.fuzzyZeroPoint(ap.handle.backVector)){
						ap.handle.kind = AHKIND.None;
						ap.handle.frontVector = PV.ZeroPoint();
						ap.handle.backVector = PV.ZeroPoint();
						return true;
					}
					return false;
				};
				sanitizeAPAH(Focus_nowCurrerntAP());
				renderingAll();
			}
				break;
			}
		}

		if(editor.mouse.isLatedFocusChangeInMousemoveCallbacked_){
			// 対象となる遅い処理は現在のところ無い
		}

		// ** mousemove中のためのキャッシュをクリア
		const fieldFrame = document.getElementById('edit-field-frame');
		fieldFrame.style.cursor = 'auto';
		// ついでに他のmousedownフラグもクリアしておく
		mousedownMoreInfo = undefined;
		mousedownInEditField = undefined;
		mousedownInCanvas = undefined;
		editor.mouse = deepcopy(EDITOR_MOUSE_OF_MOUSEUP_DEFAULT);
		renderingEditorUserInterface();

		isDisableHistoryUpdate = false;
		checkpointStacking(Tool.getToolFromIndex(editor.toolIndex).name);
	};
	document.getElementById('edit-field').addEventListener('mousemove', event => {
		// MEMO mousemoveではevent.buttonはゼロのままなので、中ボタン判定はできない
		//console.log('mousemove', event.offsetX, event.offsetY)

		// **
		mousemoveInEditField = {
			'x': event.offsetX,
			'y': event.offsetY,
		};
		mousemoveInCanvas = {
			'x': (mousemoveInEditField.x - hist.now().canvas.padding.w) / field.canvas.scale,
			'y': (mousemoveInEditField.y - hist.now().canvas.padding.h) / field.canvas.scale,
		};
		const point = mousemoveInCanvas;
		document.getElementById('mouse-position').textContent = `(${mousemoveInCanvas.x.toFixed(1)}, ${mousemoveInCanvas.y.toFixed(1)})`

		// SPEC mousedown,up間のブレで意図しない移動などが起こらないよう、一定距離を動くまで移動操作をロックする
		if (mousedownInCanvas && editor.mouse.isMoveLock) {
			const mouseRect = getMousemoveRectInCanvas();
			if (8 < toField(PV.diagonalLengthFromRect(mouseRect))) {
				editor.mouse.isMoveLock = false;
				if ('notmove' === editor.mouse.mode) {
					// SPEC Item移動モードでない状態で、一定距離をmousemoveした場合、矩形選択へ。
					editor.mouse.mode = 'rectselect';
					console.log('MoveLock unlocked');
				}
			}
		}

		if (! mousedownInCanvas) {
			switch (editor.toolIndex) {
			case TOOLS.ItemEditTool.Id:
			//case TOOLS.APEditTool.Id:
				editor.mouse.mode = getMouseModeByMousePoint(mousemoveInCanvas);
				break;
			}
		}
		{
			const mode = editor.mouse.mode;
			// ** マウスカーソルのセット
			const fieldFrame = document.getElementById('edit-field-frame');
			switch(mode){
			case 'resize_righttop':
			case 'resize_leftbottom':
				fieldFrame.style.cursor = 'nesw-resize';
				break;
			case 'resize_lefttop':
			case 'resize_rightbottom':
				fieldFrame.style.cursor = 'nwse-resize';
				break;
			case 'resize_top':
			case 'resize_bottom':
				fieldFrame.style.cursor = 'ns-resize';
				break;
			case 'resize_right':
			case 'resize_left':
				fieldFrame.style.cursor = 'ew-resize';
				break;
			case 'rotate':
				fieldFrame.style.cursor = 'cell'; // TODO ここのカーソルは仮
				break;
			case 'move_field':
				fieldFrame.style.cursor = 'move';
				break;
			default:
				fieldFrame.style.cursor = 'auto';
				break;	
			}
		}

		// **
		switch (editor.mouse.button) {
		case 0: // 左ボタン
			switch (editor.toolIndex) {
			case TOOLS.ItemEditTool.Id:
				if (! mousedownInCanvas) {
					editor.hover.touchedItemIndex = getTouchedItem(hist.now().items, point);
					renderingEditorUserInterface();
					break;
				}
				if (editor.mouse.isMoveLock) {
					break;
				}

				switch (editor.mouse.mode) {
					case 'move':{
						const [degree, move] = snapMousemoveVectorInCanvas(event.shiftKey);
						//const [degree, move] = angleSnapAndMousemoveVectorInCanvas(event);
						hist.now().focus.focusItemIndexes.forEach(itemIndex => {
							Item_moveWithIndex(itemIndex, move);
						});
						renderingAllReduct();
					}
						break;
					case 'rectselect':
						const mouseRect = getMousemoveRectInCanvas();
						let touchedIndexes = [];
						hist.now().items.forEach((item, index) => {
							if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
								return;
							}
							const itemRect = PV.rectFromRange2d(PV.Item_range2d(item));
							//console.log('rectselect', mouseRect, itemRect)
							if (PV.isTouchRectInRect(mouseRect, itemRect)) {
								touchedIndexes.push(index);
							}
						});
						/*
						# 概要：
						矩形選択中のpaletteUIの表示が変化したりしなかったりする。
						# 再現の手順
						矩形選択で選択範囲を動かして選択Itemを増やしたり減らしたりする
						# 期待する挙動：
						増えたり減ったりする度にpaletteが変化する。
						# 観察されたこと：
						paletteが変化したりしなかったりする。
						# 詳細
						他にもborderUIなどに影響する。
						現行実装上で単純に実装すると高さ順でcurrentItemが決まってしまい、
						画面上でユーザが見た順に選択されていかないため、
						paletteが変化したりしなかったりする。					
						paletteUIの表示に違和感が出てしまうため、focus順を保持するなど、少し複雑な処理で実装した。
						ちなみにAIでは、複数選択するとpaletteが複数選択表示（灰色的な表示）になって、
						選択順が崩れても問題ない。
						本APPでは、各UIにそういった表示状態を追加するのが面倒だったため、場当たり的にこれで処置とした。
							*/
						// 新たにItemが矩形選択範囲に入った場合、それを追加する
						touchedIndexes.forEach(index => {
							if (!hist.now().focus.focusItemIndexes.includes(index)) {
								console.log('rectselect', 'add', index)
								Focus_addItemIndex(index);
							}
						});
						// 矩形選択で範囲が小さくなった場合に、現在の矩形に含まれていないItemがあれば、focusから取り除く
						Focus_filterItemIndexes(touchedIndexes);
						renderingEditorUserInterface();
						break;
					case 'rotate':{
							const [degree, move] = angleSnapAndMousemoveVectorInCanvas(event.shiftKey);
							hist.now().focus.focusItemIndexes.forEach(itemIndex => {
								Item_rotateWithIndex(itemIndex, degree, editor.mouse.editCenter);
							});
							renderingAllReduct();
						}
						break;
					case 'nop':
						break;
					default:
						if(0 === editor.mouse.mode.indexOf('resize_')){
							resize('Item', event);
							break;
						}
						console.error('BUG', editor.mouse.mode);
						return;
					}
					break;
				case TOOLS.APEditTool.Id:
					if (! mousedownInCanvas) {
						editor.hover.touchedApAhCplx = getTouchedApAh(hist.now().items, undefined, point);
						renderingEditorUserInterface();
						break;
					}
					if (editor.mouse.isMoveLock) {
						break;
					}
	
					switch (editor.mouse.mode) {
					case 'move':{
						const [degree, move] = angleSnapAndMousemoveVectorInCanvas(event.shiftKey);
						switch(editor.mouse.touchedAPAHKind){
						case 'ap':{
							hist.now().focus.focusAPCplxes.forEach(([itemIndex, apIndex]) => {
									AP_moveWithIndex(itemIndex, apIndex, move);
							});	
						}
							break;
						case 'front':
						case 'back':{
							// SPEC APEditToolでもAHのハンドル操作ができる。
							// SPEC APEditToolでは、Symmetryハンドルは操作してもSymmetryのまま。Freeにはならない。
							AH_move(
								editor.mouse.touchedItemIndex,
								editor.mouse.touchedApIndex,
								editor.mouse.touchedAPAHKind,
								move,
								false
							);
						}
							break;
						default:
							console.error('BUG', editor.mouse.touchedAPAHKind);
						}

							renderingAllReduct();
						}
						break;
					case 'rectselect':
						const mouseRect = getMousemoveRectInCanvas();
						let touchedCplxes = [];
						hist.now().items.forEach((item, itemIndex) => {
							if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
								return;
							}
							switch(item.kind){
							case 'Bezier':
								item.aps.forEach((ap, apIndex) => {
									if(PV.isTouchPointInRect(mouseRect, ap.point)){
										touchedCplxes.push([itemIndex, apIndex]);
									}
								});
								break;
							case 'Guide':
								if(PV.isTouchPointInRect(mouseRect, item.point)){
									touchedCplxes.push([itemIndex, 0]);
								}
								break;
							default: // NOP
								break;
							}
						});
						// 新たにItemが矩形選択範囲に入った場合、それを追加する
						touchedCplxes.forEach(cplx => {
							if(! Focus_isExistAP(cplx[0], cplx[1])) {
								console.log('rectselect', 'add', cplx)
								Focus_addAP(cplx[0], cplx[1]);
							}
						});
						// 矩形選択で範囲が小さくなった場合に、現在の矩形に含まれていないItemがあれば、focusから取り除く
						Focus_filterAP(touchedCplxes);
						break;
					case 'nop':
						break;
					}

					renderingEditorUserInterface();
					break;
				case TOOLS.APAddTool.Id:{
					if (! mousedownInCanvas) {
						const fieldFrame = document.getElementById('edit-field-frame');

						// SPEC 再パス追加可能をカーソル変化で表示
						if(! editor.isAddAnchorPoindByAAPTool){
							const isTouched = !PV.exfor(hist.now().focus.focusItemIndexes, (ix, itemIndex) => {
								const itemt = hist.now().items[itemIndex];
								if((! Render.isMetaVisible(itemt)) || Render.isMetaLock(itemt, hist.now())){
									return true;
								}
								if(itemt.isCloseAp){
									return true;
								}
								if('Bezier' !== itemt.kind){
									return true;
								}

								const firstAP = itemt.aps.at(0);
								const lastAP = itemt.aps.at(-1);
								if((!! lastAP) && PV.isTouchPointAndPoint(point, lastAP.point, toCanvas(field.touch.pointR))){
									return false;
								}
								if((!! firstAP) && PV.isTouchPointAndPoint(point, firstAP.point, toCanvas(field.touch.pointR))){
									return false;
								}
								return true;
							});
							if (isTouched) {
								// TODO ここのカーソルは仮
								fieldFrame.style.cursor = 'crosshair';
								break;
							}
						}

						// SPEC 閉パス可能をカーソル変化で表示
						{
							const item = Focus_currerntItem();
							if ((!! item)
								&& ('Bezier' === item.kind)
								&& (editor.isAddAnchorPoindByAAPTool)
								&& (! item.isCloseAp)
								&& (2 <= item.aps.length))
							{
								const ap = Focus_currentFirstAp()
								const isTouched = (PV.isTouchPointAndPoint(ap.point, point, toCanvas(field.touch.pointR)));
								const fieldFrame = document.getElementById('edit-field-frame');
								if (isTouched) {
									// TODO ここのカーソルは仮
									fieldFrame.style.cursor = 'crosshair';
									break;
								}
							}
						}

						// SPEC パス連結可能をカーソル変化で表示
						{
							const [touchedItemIndex, touchedApIndex] = getTouchedAp(getNowFocusItems(), point);
							const touchedItem = hist.now().items.at(touchedItemIndex);
							if((-1 !== touchedApIndex)
								&& (!! Item_apTipKind(touchedItem, touchedApIndex))
								&& (! Focus_isCurrentAP(touchedItemIndex, touchedApIndex)))
							{
								const fieldFrame = document.getElementById('edit-field-frame');
								// TODO ここのカーソルは仮
								fieldFrame.style.cursor = 'crosshair';
								break;
							}
						}

						fieldFrame.style.cursor = 'auto';
						break;
					}
					if (editor.mouse.isMoveLock) {
						break;
					}

					// AH操作
					let nowAp = Focus_nowCurrerntAP();
					if (! nowAp) {
						console.error('BUG');
						break;
					}

					if(AHKIND.None == nowAp.handle.kind){
						nowAp.handle.kind = AHKIND.Symmetry;
					}

					// 先頭APを指していた場合、APは逆(ここでは一旦指す方向を逆にしている)
					editor.mouse.editCenter = nowAp.point;
					const [degree, move] = angleSnapAndMousemoveVectorInCanvas(event.shiftKey);
					let vec = move;
					vec.x *= (editor.apAddTool.isGrowthTailAp) ? 1 : -1;
					vec.y *= (editor.apAddTool.isGrowthTailAp) ? 1 : -1;
					nowAp.handle.frontVector = vec;
					renderingAll();
				}
					break;
				case TOOLS.AHEditTool.Id:{
					if (! mousedownInCanvas) {
						editor.hover.touchedApAhCplx = getTouchedApAh(hist.now().items, hist.now().focus.focusItemIndexes, point);
						renderingEditorUserInterface();
						break;
					}
					if (editor.mouse.isMoveLock) {
						break;
					}

					const touchedAPAHKind = editor.mouse.touchedAPAHKind;
					if(! touchedAPAHKind){
						break;
					}

					// AH操作
					const vec = getMousemoveVectorInCanvas();
					const cplx = hist.now().focus.focusAPCplxes.at(-1)
					AH_move(
						cplx[0],
						cplx[1],
						touchedAPAHKind,
						vec,
						true
					);
					renderingAll();
				}
					break;
				case TOOLS.APInsertTool.Id:{
					if (! mousedownInCanvas) {
						// between上の編集pointを描画で示す
						const posi = nearPointPosi(hist.now().items, hist.now().focus.focusItemIndexes, mousemoveInCanvas);
						if(! posi){
							editor.hover.nearTouchPointOnBetween = undefined;
						}else{
							editor.hover.nearTouchPointOnBetween = posi.point;
						}
						renderingEditorUserInterface();
						break;
					}
					// TODO InsertしたAPのAHハンドル操作を入れたい...ような気もするがどうだろう。使いやすいのか？
				}
					break;
				case TOOLS.APKnifeTool.Id:{
					if (! mousedownInCanvas) {
						const fieldFrame = document.getElementById('edit-field-frame');
						fieldFrame.style.cursor = 'auto';
						editor.hover.nearTouchPointOnBetween = undefined;

						// SPEC まずAPにtouchしているかを判定し、触れていなければ近いbetweenを探す。
						// APにtouchしていることをカーソルで表現
						const [touchedItemIndex, touchedApIndex] = getTouchedAp(getNowFocusItems(), point);
						editor.mouse.touchedItemIndex = touchedItemIndex;
						editor.mouse.touchedApIndex = touchedApIndex;
						if((-1 !== touchedApIndex)){
							const item = hist.now().items.at(touchedItemIndex);
							if(! item){
								console.log('BUG', touchedItemIndex, touchedApIndex);
								break;
							}
							if(! item.isCloseAp){
								if((0 === touchedApIndex) || (touchedApIndex === (item.aps.length - 1))){
									// SPEC 開パスの先頭APおよび末尾APは対象としない
									renderingEditorUserInterface();
									break;
								}
							}
							// TODO ここのカーソルは仮
							fieldFrame.style.cursor = 'crosshair';
							renderingEditorUserInterface();
							break;
						}

						// between上の編集pointを描画で示す
						// TODO 直接touchしていない状態では、
						// （先頭APおよび末尾APを含む）APの位置もbetween対象となってしまう
						const posi = nearPointPosi(hist.now().items, hist.now().focus.focusItemIndexes, mousemoveInCanvas);
						if(!! posi){
							editor.hover.nearTouchPointOnBetween = posi.point;
						}
						renderingEditorUserInterface();
						break;
					}
					// SPEC KnifeしたAPは２つに分裂しているのでAHハンドル操作を入れるのは無理・変（なのでしない）
				}
					break;
				default:
					break;
			}
			break;
		case 1: // 中ボタン
			if (! mousedownInCanvas) {
				break;
			}
			let fieldFrame = document.getElementById('edit-field-frame');
			// MEMO: offsetX,offsetYはスクロール位置変更で変化し発火してしまうため、
			// それで計算すると連続でmousemoveイベントが発生し、短い周期で揺れてしまう。
			const diffInField = {
				'x': mousedownMoreInfo.pointInPage.x - event.pageX,
				'y': mousedownMoreInfo.pointInPage.y - event.pageY,
			};
			const movedScroll = {
				'x': fieldFrame.scrollLeft - mousedownMoreInfo.scroll.left,
				'y': fieldFrame.scrollTop - mousedownMoreInfo.scroll.top,
			};
			const diff = { // TODO 計算が正しいが雑？ スクロールによってマウス位置が移動した分をさらに引いている
				'x': diffInField.x - movedScroll.x,
				'y': diffInField.y - movedScroll.y,
			};
			//console.log('move', diffInField.x) // , mousemoveInField, movedScroll, diff )
			fieldFrame.scrollTop += diff.y;
			fieldFrame.scrollLeft += diff.x;
			break;
		}
	}, true);
	document.getElementById('container-wrapper').addEventListener('mousemove', event => {
		// マウスがelementの外に出てしまうと、mouseupが発生しない。
		// そこで外周elementの外へ出た場合に、そこで離した扱いにする。
		// MEMO mouseenter, mouseleave回数をカウントする方法を試してみたが、マウス押下したままelement外へ出ると正常に動作しなかった。
		// TODO AIなどは、fieldやwindowの外まででもマウス押下状態が有効だったり、fieldがある限りスクロールで追いかけたりする。
		// 可能な範囲で、もうちょっとフレンドリィな挙動にできないか？
		if (!mousedownMoreInfo) {
			return;
		}
		const crect = document.getElementById('edit-field-frame').getBoundingClientRect();
		const rect = {
			'x': crect.top,
			'y': crect.left,
			'w': crect.width,
			'h': crect.height,
		};
		const point = { 'x': event.pageX, 'y': event.pageY };
		//console.log('mousemove', PV.isTouchPointInRect(rect, point), event.currentTarget.id, event.pageX, event.screenX, rect);

		if (!PV.isTouchPointInRect(rect, point)) {
			console.log('mouse leave in area');
			mouseupEditFieldFrameCallback();
		}
	});
	/* */
	const resizeable = document.getElementById('edit-field')
	const observer = new ResizeObserver(() => {
		//要素のサイズ確認
		const width = resizeable.getBoundingClientRect().width
		const height = resizeable.getBoundingClientRect().height
		//console.log('size(w,h): ', width, height, prevEditFieldSizeByRescalingCanvas)

		if (prevEditFieldSizeByRescalingCanvas) {
			let fieldFrame = document.getElementById('edit-field-frame')

			// TODO 現行実装は大まかに中央あたりに寄せる程度の実装
			fieldFrame.scrollTop += (height - prevEditFieldSizeByRescalingCanvas.h) / 2
			fieldFrame.scrollLeft += (width - prevEditFieldSizeByRescalingCanvas.w) / 2

			prevEditFieldSizeByRescalingCanvas = undefined;
		}
	})
	observer.observe(resizeable)

	// ** keyboard shortcuts
	document.addEventListener('keydown', (event) => {
		if (event.repeat)
			return;
		const toShortcutKey = (event) => {
			let shortcutKey = ''
			if(event.ctrlKey ){ shortcutKey += 'Ctrl + ';}
			if(event.altKey  ){ shortcutKey += 'Alt + ';}
			if(event.shiftKey){ shortcutKey += 'Shift + ';}
			if(event.metaKey ){ shortcutKey += 'Meta + ';}
			if(1 === event.key.length){
				shortcutKey += event.key.toUpperCase();
			}else{
				shortcutKey += event.key;
			}
			return shortcutKey;
		};
		const shortcutKey = toShortcutKey(event);
		const elem = document.activeElement;
		console.log(`Key "${shortcutKey}", "${event.key}" [event: keydown]`, elem.tagName);

		// ブラウザの標準機能のページ拡縮を抑制
		if(event.ctrlKey && ('+' === event.key || '-' === event.key || '=' === event.key || '_' === event.key)){
			console.log('cancel browser shortcut page resize.');
			// MEMO 英字キーボードでCtrl++押下が次のように報告される。（そして拡大が動作する）
			// `Key "Ctrl + =", "=" repeating [event: keydown] BODY`
			// MEMO 英字キーボードでCtrl+Shift+-押下が次のように報告される。（そして縮小が動作する）
			// `"Ctrl + Shift + _", "_" repeating [event: keydown] BODY`
			event.preventDefault();
			// このあとAPP固有ショートカットがあるのでreturnせず続行
		}

		// MEMO Layer名などの文字列入力欄は、ショートカットとして処理する前に検出し、キー入力できるようreturn
		// TODO Ctrl+Zが入力欄内で入力文字に対して有効になるようにする?
		// 数値のinputの内側でCtrl+Zが効く必要は無いと思う。
		if('INPUT' === elem.tagName){
			const isNotExtraKey = (! event.ctrlKey) && (! event.altKey) && (! event.metaKey);
			if(isNotExtraKey){
				console.log(`normal input ${event.key}`)
				return;
			}
		}

		switch (shortcutKey) {
		case 'Escape':
			// SPEC Escape キーを押下された場合、mousemove状態中のマウス操作があればキャンセルする。
			if(!! mousedownInCanvas){
				event.preventDefault();
				mouseupEditFieldFrameCallback();
				if(editor.isHistoryStacked){
					hist.cancelRoolback();
					editor.isHistoryStacked = false;
				}

				renderingAll();	
				setMessage('cancel by Escape key.')
				return;
			}
		}

		{
			// dialogが開いている場合、ここから下のショートカットキーは処理しない。
			const dialogs = Array.prototype.slice.call(document.getElementsByTagName('dialog'));
			if(dialogs.some(dialog => dialog.open)){
				console.log('dialog now open');
				if('BUTTON' === elem.tagName && 'Enter' === event.key){ // Enterキー押下でフォーカスしているButtonをclick
					return;
				}
				if('Escape' === event.key){ // Escapeキー押下でDialogが閉じる
					return;
				}
				//event.preventDefault();
				return;
			}
		}
	
		if(mousedownInEditField){
			setMessage(`ignore "${event.key}" keydown in mousemove `);
			// マウス操作中はキー入力をキャンセルする。
			// ここから下のショートカットについても無効化。
			// TODO Snapなどはマウス操作中のキー操作がありうるが、それは除く必要があったりするのか？
			event.preventDefault();
			return;
		}

		const moveNowForFocus = (moveVec) => {
			if(Tool.getToolFromIndex(editor.toolIndex).isItem){
				hist.now().focus.focusItemIndexes.forEach(itemIndex => {
					Item_moveNowWithIndex(itemIndex, moveVec);
				});
			}else{
				hist.now().focus.focusAPCplxes.forEach(cplx => {
					AP_moveNowWithIndex(cplx[0], cplx[1], moveVec)
				});
			}
		};

		// ここにあるのはメニュー外のショートカット。
		// TODO なのでメニューが追加されたら、いくつかのものはそちらへ移動する。
		switch (shortcutKey) {
		case 'Delete':
			deleteByFocus();
			renderingAll();
			checkpointStacking('delete');
			event.preventDefault();
			return;
		case 'V':
			setToolIndex(TOOLS.ItemEditTool.Id);
			event.preventDefault();
			return;
		case 'A':
			setToolIndex(TOOLS.APEditTool.Id);
			event.preventDefault();
			return;
		case 'Shift + C':
			setToolIndex(TOOLS.AHEditTool.Id);
			event.preventDefault();
			return;
		case 'P':
			setToolIndex(TOOLS.APAddTool.Id);
			event.preventDefault();
			return;
		case 'Shift + +':
			if('INPUT' === elem.tagName){
				// MEMO 英字キーボードの'+'が'Shift+='になっており、
				// preventDefaultしてしまうと入力できない。
				return;
			}
			setToolIndex(TOOLS.APInsertTool.Id);
			event.preventDefault();
			return;
		case 'K':
			setToolIndex(TOOLS.APKnifeTool.Id);
			event.preventDefault();
			return;
		case 'Ctrl + [':
			for(let i = 0; i < hist.now().focus.focusItemIndexes.length; i++){
				const itemIndex = hist.now().focus.focusItemIndexes[i];
				shiftingItemOrder_(itemIndex, -1)
			};
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('reorder 1down');
			event.preventDefault();
			return;
		case 'Ctrl + ]':
			for(let i = 0; i < hist.now().focus.focusItemIndexes.length; i++){
				const itemIndex = hist.now().focus.focusItemIndexes[i];
				shiftingItemOrder_(itemIndex, +1);
			}
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('reorder 1up');
			event.preventDefault();
			return;
		case 'Ctrl + Shift + {':{ // ('Ctrl + Shift + [')
			// 複数Itemを最前・最後へ移動する場合の固有の都合。
			// A. すべて最前・最後に送ってしまうので、Focus順のままLoopしてItemを移動すると、
			// 選択Items同士の前後関係が崩れてしまう。
			// B. １つずつ順にItem移動を進めようとすると、
			// １つItemを移動するたびにfocus内部のIndex番号を弄りながらLoopしなければならない。
			// (itemIndexは前後順そのものでもある)
			// 
			// 案1. 一旦全部Itemsを取り除いてから、まとめて最前・最後に挿入する方法もある。
			// APのフォーカスを保持するのが面倒だが、APフォーカスを捨ててしまうなら十分あり。
			// 案2. Itemを１つ移動する度に Focus内のitemIndexを採番しなおす。
			// APフォーカスを保持するのが（案1と比べて比較的）楽だが、focusとZ順の２重管理で煩雑になる。
			fullshiftingItemsOrder_(-1);

			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('reorder to bottom');
			event.preventDefault();
		}
			return;
		case 'Ctrl + Shift + }':{ // ('Ctrl + Shift + ]')
			fullshiftingItemsOrder_(+1);

			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('reorder to top');
			event.preventDefault();
		}
			return;
		case 'Ctrl + +':
		case 'Ctrl + shift + +':{
			// キーボード上で'+'キー押下のためにはShiftが必要
			steppingCanvasScale(+1);

			renderingAll();
			event.preventDefault();
		}
			return;
		case 'Ctrl + -':{
			steppingCanvasScale(-1);

			renderingAll();
			event.preventDefault();
		}
			return;
		case 'ArrowUp':{
			const v = (field.canvas.scale > 1.0) ? 1 : 10;
			const moveVec = {'x': 0, 'y': -v};
			moveNowForFocus(moveVec);
			renderingAll();
			checkpointStacking('ArrowMove', 'overwriteDuplicate');
			event.preventDefault();
		}
			return;
		case 'ArrowDown':{
			const v = (field.canvas.scale > 1.0) ? 1 : 10;
			const moveVec = {'x': 0, 'y': v};
			moveNowForFocus(moveVec);
			renderingAll();
			checkpointStacking('ArrowMove', 'overwriteDuplicate');
			event.preventDefault();
		}
			return;
		case 'ArrowRight':{
			const v = (field.canvas.scale > 1.0) ? 1 : 10;
			const moveVec = {'x': v, 'y': 0};
			moveNowForFocus(moveVec);
			renderingAll();
			checkpointStacking('ArrowMove', 'overwriteDuplicate');
			event.preventDefault();
		}
			return;	
		case 'ArrowLeft':{
			const v = (field.canvas.scale > 1.0) ? 1 : 10;
			const moveVec = {'x': -v, 'y': 0};
			moveNowForFocus(moveVec);
			renderingAll();
			checkpointStacking('ArrowMove', 'overwriteDuplicate');
			event.preventDefault();
		}
			return;
		case 'G':
			event.preventDefault();
			showMoveDialog();
			return;	
		case 'S':
			event.preventDefault();
			showResizeDialog();
			return;	
		case 'R':{
			event.preventDefault();
			let rinput = document.getElementById('rotate-input');
			rinput.value = 0;
			showModalWrapper('rotate-dialog')
		}
			return;	
		case 'Ctrl + X':
			cutOrCopyToClipboard('cut');
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('cut');
			event.preventDefault();
			return;	
		case 'Ctrl + C':
			cutOrCopyToClipboard('copy');
			event.preventDefault();
			return;	
		case 'Ctrl + V':
			pasteByClipboard(10);
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('paste');
			return;	
		case 'Ctrl + Shift + V':
			pasteByClipboard(0);
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('paste');
			return;	
		case 'Ctrl + Shift + Z':
			if (!hist.redo()) {
				setMessage('cant redo.')
			} else {
				renderingAll();
			}
			event.preventDefault();
			return;
		case 'Ctrl + Z':
			if (!hist.undo()) {
				setMessage('cant undo.')
			} else {
				renderingAll();
			}
			event.preventDefault();
			return;
		//case 'Ctrl + N': // SPEC Ctrl+Nはブラウザのnew tabなのでさすがに上書きしたくない
		case 'Ctrl + Alt + N':{ // SPEC 新しいドキュメント
			event.preventDefault();
			startNewDocumentOpen();
		}
			return;
		case 'Ctrl + O':{ // SPEC ドキュメントを開く
			event.preventDefault();
			updateDocSelectDialog();
			showModalWrapper('doc-select-dialog');
		}
			return;
		case 'Ctrl + E': // SPEC SVG Export
		case 'Ctrl + Shift + E':{ // SPEC PNG Export
			event.preventDefault();

			// rendering svg on memory
			const doc = hist.now();
			let draw = SVG();
			draw.size(doc.size.w, doc.size.h);
			Render.renderingDocOnCanvas(draw.group(), doc);
			
			// to download file.
			const ssvg = xmlFormatter(draw.svg().replace('>', `><!-- generated by ${PACKAGE.name} ${PACKAGE.version} -->`));
			const svgBase64 = "data:image/svg+xml;charset=utf-8;base64," + Base64.encode(ssvg);
			const currentdocinfo = strage.loadOrNewCurrentDocInfo();
			const name = (!! currentdocinfo.name) ? currentdocinfo.name : 'untitled';

			switch(shortcutKey){
			case 'Ctrl + E':{
				let a = document.createElement("a");
				a.href = svgBase64;
				a.setAttribute("download", `${name}.svg`);
				a.dispatchEvent(new MouseEvent("click"));
			}
				break;
			case 'Ctrl + Shift + E':{
				let canvas = document.createElement("canvas");
				canvas.width = doc.size.w;
				canvas.height = doc.size.h;
			
				let ctx = canvas.getContext("2d");
				let image = new Image;
				image.onload = function(){
					ctx.drawImage(image, 0, 0);
					let data = canvas.toDataURL("image/png");
					var a = document.createElement("a");
					a.href = data;
					a.setAttribute("download", `${name}.png`);
					a.dispatchEvent(new MouseEvent("click"));
				}
				image.src = svgBase64;
			}
				break;	
			}
		}
			return;
		case 'Ctrl + A':{
			event.preventDefault();
			hist.now().items.forEach((item, itemIndex) => {
				item.aps.forEach((ap, apIndex) => {
					item.aps.at(apIndex);
					Focus_addAP(itemIndex, apIndex);	
				});
			});
			renderingEditorUserInterface();
		}
			return;
		case 'Ctrl + Shift + A':{
			event.preventDefault();
			Focus_clear();
			renderingEditorUserInterface();
		}
			return;	
		}
	});

	/* canvas scale */
	{
		const rescallingByInput = () => {
			let par = document.getElementById('canvas-scale').value;
			par = StrCalc.calc(par);

			if (isNaN(par)) {
				// 数値以外だった場合は復元して終了
				document.getElementById('canvas-scale').value = field.canvas.scale * 100;
				return;
			}
			const scale = par / 100;
			if (!rescalingCanvas(scale)) {
				document.getElementById('canvas-scale').value = field.canvas.scale * 100;
				return;
			}
		};
		document.getElementById('canvas-scale-select').addEventListener('change', (e) => {
			console.log('canvas-scale-select', e.target.value)
			document.getElementById('canvas-scale').value = e.target.value.slice(0, -1)
			rescallingByInput();
		});
		document.getElementById('canvas-scale').addEventListener('change', (e) => {
			console.log('canvas-scale', e.target.value)
			rescallingByInput();
		});
	}

	/* axis */
	{
		const axisChangedCallback = (event) => {
			console.log('call axisChangedCallback', event.currentTarget.parentElement.id);
			const focusRect = getFocusingRectByPrev();
			const inputRect = {
				'x': parseInt(document.getElementById('axis-x').children[0].value, 10),
				'y': parseInt(document.getElementById('axis-y').children[0].value, 10),
				'w': parseInt(document.getElementById('axis-w').children[0].value, 10),
				'h': parseInt(document.getElementById('axis-h').children[0].value, 10),
			};
			const tool = Tool.getToolFromIndex(editor.toolIndex);
			const targetKind = tool.isItem ? 'Item' : 'AP';

			const kind = event.currentTarget.parentElement.id.slice(-1);
			switch(kind){
			case 'x':
			case 'y':{
				const vec = PV.pointSub(inputRect, focusRect);
				moveFocusItems(targetKind, vec);
				renderingAll();
				checkpointStacking('axis input move');
			}
				break;
			case 'w':
			case 'h':
				const scale2d = PV.pointDiv(PV.rectSize(inputRect), PV.rectSize(focusRect));
				const editCenter = {'x': focusRect.x, 'y': focusRect.y};
				resizeFocusItems(targetKind, scale2d, editCenter);
				renderingAll();
				checkpointStacking('axis input resize');
				break;
			default:
				console.error('BUG', kind);
			}
		};
		const ps = [
			document.getElementById('axis-x'),
			document.getElementById('axis-y'),
			document.getElementById('axis-w'),
			document.getElementById('axis-h'),
		];
		ps.forEach((parent) => {
			parent.children[1].addEventListener('change', (e) => {
				parent.children[0].value = e.target.value;
				axisChangedCallback(e);
			});
			parent.children[0].addEventListener('change', (e) => {
				console.log('axis-', e.target.value, StrCalc.calc(e.target.value))
				const v = StrCalc.calc(e.target.value);
				if (isNaN(v)) {
					setMessage(`invalid input: '${e.target.value}'`);
					updateAxisInput();
					return;
				}
				e.target.value = v;
				parent.children[1].value = v;
				axisChangedCallback(e);
			});
		});
	}

	/* tool */
	{
		let elems = document.getElementById('tool').getElementsByTagName('button');
		elems.forEach((elem, index) => {
			elem.addEventListener('click', (event) => {
				console.log('toolbutton', event.currentTarget.id, index)
				setToolIndex(index)
			});
		});
		// 初期化
		setToolIndex(TOOLS.APAddTool.Id);
	}
	/* editor setting */
	{
		let dialog = document.getElementById('editor-setting-dialog');
		document.getElementById('editor-setting-dialog-button').addEventListener('click', (event) => {
			console.log('clicked', event.currentTarget.id);
			document.getElementById('editor_resize_is-resize-stroke-width').checked = hist.now().editor.resize.isResizeStrokeWidth;
			document.getElementById('editor_guide_is-lock').checked = hist.now().editor.guide.isLockGuide;
			document.getElementById('editor_guide_is-visible').checked = hist.now().editor.guide.isVisibleGuide;
			document.getElementById('editor_snap_is-snap-for-grid').checked = hist.now().editor.snap.isSnapForGrid;
			document.getElementById('editor_snap_is-snap-for-pixel').checked = hist.now().editor.snap.isSnapForPixel;
			document.getElementById('editor_snap_is-snap-for-item').checked = hist.now().editor.snap.isSnapForItem;
			document.getElementById('editor_snap_is-snap-for-guide').checked = hist.now().editor.snap.isSnapForGuide;

			showModalWrapper(dialog);
		});
		dialog.addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, dialog.returnValue);
			renderingAll();
			checkpointStacking('editor setting');
		});

		document.getElementById('editor_resize_is-resize-stroke-width').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.resize.isResizeStrokeWidth = event.currentTarget.checked;
		});

		document.getElementById('editor_guide_is-lock').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.guide.isLockGuide = event.currentTarget.checked;
			if(hist.now().editor.guide.isLockGuide){
				PV.exforReverse(hist.now().focus.focusItemIndexes, (ix, itemIndex) => {
					const item = hist.now().items.at(itemIndex);
					if('Guide' === item.kind){
						Focus_removeItemIndex(itemIndex);
					}
				});	
			}
			updateLayerView();
			renderingEditorUserInterface();
		});
		document.getElementById('editor_guide_is-visible').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.guide.isVisibleGuide = event.currentTarget.checked;
			if(! hist.now().editor.guide.isVisibleGuide){
				PV.exforReverse(hist.now().focus.focusItemIndexes, (ix, itemIndex) => {
					const item = hist.now().items.at(itemIndex);
					if('Guide' === item.kind){
						Focus_removeItemIndex(itemIndex);
					}
				});	
			}
			updateLayerView();
			renderingEditorUserInterface();
		});
	
		document.getElementById('editor_snap_is-snap-for-grid').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.snap.isSnapForGrid = document.getElementById('editor_snap_is-snap-for-grid').checked;
		});
		document.getElementById('editor_snap_is-snap-for-pixel').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.snap.isSnapForPixel = document.getElementById('editor_snap_is-snap-for-pixel').checked;
		});
		document.getElementById('editor_snap_is-snap-for-item').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.snap.isSnapForItem = document.getElementById('editor_snap_is-snap-for-item').checked;
		});
		document.getElementById('editor_snap_is-snap-for-guide').addEventListener('change', (event)=> {
			console.log('changed', event.currentTarget.id);
			hist.now().editor.snap.isSnapForGuide = document.getElementById('editor_snap_is-snap-for-guide').checked;
		});

	}
	/* document setting */
	{
		let dialog = document.getElementById('document-setting-dialog');
		document.getElementById('document-setting-dialog-button').addEventListener('click', (event) => {
			console.log('clicked', event.currentTarget.id);

			document.getElementById('document-w').children[0].value = hist.now().size.w;
			document.getElementById('document-w').children[1].value = hist.now().size.w;
			document.getElementById('document-h').children[0].value = hist.now().size.h;
			document.getElementById('document-h').children[1].value = hist.now().size.h;
			const bgcolor = hist.now().canvas.background.color;
			const isTransparent = 0 === parseInt(bgcolor.substring(bgcolor.length - 2), 16);
			document.getElementById('editor-background_is-transparent').checked = isTransparent;
			document.getElementById('editor-background_is-showgrids').checked = (0 !== hist.now().canvas.grids.length);
			const gi = (0 === hist.now().canvas.grids.length) ? 100: hist.now().canvas.grids[0].interval;
			document.getElementById('editor-background_grid-interval').value = gi;
			document.getElementById('editor-background_padding-w').value = hist.now().canvas.padding.w;
			document.getElementById('editor-background_padding-h').value = hist.now().canvas.padding.h;
			
			showModalWrapper(dialog);
		});
		dialog.addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, dialog.returnValue);
			if('OK' !== dialog.returnValue){
				console.log('cancel button');
				return;
			}

			const doc_w = parseInt(document.getElementById('document-w').children[0].value, 10);
			const doc_h = parseInt(document.getElementById('document-h').children[0].value, 10);
			const isTransparent = document.getElementById('editor-background_is-transparent').checked;
			const isShowGrids = document.getElementById('editor-background_is-showgrids').checked;
			const gridInterval = parseInt(document.getElementById('editor-background_grid-interval').value, 10);
			const padding_w = parseInt(document.getElementById('editor-background_padding-w').value, 10);
			const padding_h = parseInt(document.getElementById('editor-background_padding-h').value, 10);

			if(doc_w < 1 || DOCUMENT_MAX_SIZE_AXIS_PX < doc_w){
				setMessage(`invalid doc.size.w "${doc_w}" max:${DOCUMENT_MAX_SIZE_AXIS_PX}`);
				return;
			}
			if(doc_h < 1 || DOCUMENT_MAX_SIZE_AXIS_PX < doc_h){
				setMessage(`invalid doc.size.h "${doc_h}" max:${DOCUMENT_MAX_SIZE_AXIS_PX}`);
				return;
			}
			// paddingのサイズは適当、というかもっと小さくて良いと思うが、とりあえず上限を作っておく。
			if(padding_w < 1 || DOCUMENT_MAX_SIZE_AXIS_PX < padding_w){
				setMessage(`invalid padding_w "${padding_w}" max:${DOCUMENT_MAX_SIZE_AXIS_PX}`);
				return;
			}
			if(padding_h < 1 || DOCUMENT_MAX_SIZE_AXIS_PX < padding_h){
				setMessage(`invalid padding_h "${padding_h}" max:${DOCUMENT_MAX_SIZE_AXIS_PX}`);
				return;
			}

			let newCanvas = deepcopy(hist.now().canvas);
			newCanvas.background.color = isTransparent ? '#ffffff00' : '#ffffffff';
			if(! isShowGrids){
				newCanvas.grids = [];
			}else{
				const newGrid = {
					'interval': gridInterval,
					'color': '#3070ffff',
					'strokew': 0.3,
				};
				newCanvas.grids[0] = newGrid;
			}
			newCanvas.padding.w = padding_w;
			newCanvas.padding.h = padding_h;
			console.log('document setting update ', doc_w, doc_h, newCanvas);

			hist.now().size.w = doc_w;
			hist.now().size.h = doc_h;
			hist.now().canvas = newCanvas;

			renderingAll();
			checkpointStacking('document setting');
		});

		const ps = [
			document.getElementById('document-w'),
			document.getElementById('document-h'),
		];
		ps.forEach((parent) => {
			parent.children[1].addEventListener('change', (e) => {
				parent.children[0].value = e.target.value;
				// TODO セット先のchangeは発火しないので、値反映をきちんと行う
			});
			parent.children[0].addEventListener('change', (e) => {
				console.log('document-', e.target.value, StrCalc.calc(e.target.value))
				const v = StrCalc.calc(e.target.value);
				if (isNaN(v)) {
					// TODO 入力値不正の場合、前の値を復元する
					setMessage(`invalid input: '${e.target.value}'`)
					return;
				}
				e.target.value = v;
				parent.children[1].value = v;
			});
		});
	}
	/* palette */
	{
		const changedPaletteElementCallback = (finished) => {
			finished = (false !== finished);
			setPaletteUIForMem()

			// ** itemへ色をセット
			hist.now().focus.focusItemIndexes.forEach(index => {
				hist.now().items[index].colorPair = deepcopy(palette.colorPair);
			});
			if (0 === hist.now().focus.focusItemIndexes.length) {
				return;
			}
			if (finished) { // スライダ操作中はhistoryに積まない
				console.log('changedPaletteElementCallback', finished)
				checkpointStacking('palette');
			}
			renderingAll();
		};
		const colorElemChangedCallback = (finished) => {
			finished = !!(false !== finished);
			const ncolor = "#"
				+ ('00' + parseInt(document.getElementById('r-slider').children[1].value, 10)
					.toString(16)).substr(-2)
				+ ('00' + parseInt(document.getElementById('g-slider').children[1].value, 10)
					.toString(16)).substr(-2)
				+ ('00' + parseInt(document.getElementById('b-slider').children[1].value, 10)
					.toString(16)).substr(-2)
				+ ('00' + Math.round(parseInt(document.getElementById('a-slider').children[1].value, 10) / 100 * 0xff)
					.toString(16)).substr(-2)
				;
			//console.log('color', ncolor);

			//let color = palette.isFill ? palette.colorPair.fillColor : palette.colorPair.strokeColor;
			//if(color === ncolor){
			//	return;
			//}

			if (palette.isFill) {
				palette.colorPair.fillColor = ncolor;
			} else {
				palette.colorPair.strokeColor = ncolor;
			}
			changedPaletteElementCallback(finished);
		};

		let ps = [
			document.getElementById('r-slider'),
			document.getElementById('g-slider'),
			document.getElementById('b-slider'),
			document.getElementById('a-slider'),
		];
		ps.forEach((parent) => {
			parent.children[0].addEventListener('input', (e) => {
				if (parent.children[1].value !== parent.children[0].value) {
					parent.children[1].value = parent.children[0].value;
				}
			});
			// 上の代入だけでは数値入力のchangedが発火しないので両方に仕込む
			parent.children[0].addEventListener('change', colorElemChangedCallback);
			parent.children[1].addEventListener('change', colorElemChangedCallback);
			// スライダ操作中に画面へ反映する
			parent.children[0].addEventListener('input', () => colorElemChangedCallback(false));
		});
		document.getElementById('color-picker').addEventListener('change', (e) => {
			console.log(document.getElementById('color-picker').value)

			let scolor = document.getElementById('color-picker').value;
			if (0 === scolor.indexOf('rgba')) {
				// いまのところスポイトでpicker外を選択した際に(0,0,0,0)が取れた場合しか見たことがない。
				// 単に無視することとする
				console.log('rgba', scolor);
				changedPaletteElementCallback();
				return;
			} else if (0 === scolor.indexOf('rgb')) {
				// いまのところ発生していない。
				console.log('rgb', scolor);
				changedPaletteElementCallback();
				return;
			} else if (7 !== scolor.length) {
				// 念のためチェックしておく
				console.log('rgb', scolor);
				changedPaletteElementCallback();
				return;
			}

			const color = palette.isFill ? palette.colorPair.fillColor : palette.colorPair.strokeColor;
			let a = color.slice(7, 9);
			scolor += a;
			if (palette.isFill) {
				palette.colorPair.fillColor = scolor;
			} else {
				palette.colorPair.strokeColor = scolor;
			}
			changedPaletteElementCallback();
		});

		document.getElementById('palette-pair').addEventListener('click', (e) => {
			// （おそらくCORSにより）object要素のcontentDocumentは取れないためSVG埋め込みで対応した
			palette.isFill = !palette.isFill;
			console.log('palette', palette);
			changedPaletteElementCallback();
		});
		document.getElementById('palette-casring').addEventListener('click', (e) => {
			let c = palette.colorPair.fillColor;
			palette.colorPair.fillColor = palette.colorPair.strokeColor;
			palette.colorPair.strokeColor = c;
			console.log('palette', palette);
			changedPaletteElementCallback();
		});
		document.getElementById('palette-invisible').addEventListener('click', (e) => {
			if (palette.isFill) {
				palette.colorPair.fillColor = palette.colorPair.fillColor.slice(0, -2) + '00';
			} else {
				palette.colorPair.strokeColor = palette.colorPair.strokeColor.slice(0, -2) + '00';
			}
			console.log('palette', palette);
			changedPaletteElementCallback();
		});
		document.getElementById('palette-reset').addEventListener('click', (e) => {
			palette.colorPair.fillColor = '#ffffffff';
			palette.colorPair.strokeColor = '#000000ff';
			console.log('palette', palette);
			changedPaletteElementCallback();
		});
		// 初期化
		setPaletteUIForMem();
		//changedPaletteElementCallback(false);
	}

	/* border */
	{
		const borderChangedCallback = () => {
			border.width = document.getElementById('border-width-input').value;
			border.linecap = document.getElementById('stroke-linecap').value;
			border.linejoin = document.getElementById('stroke-linejoin').value;

			hist.now().focus.focusItemIndexes.forEach(index => {
				hist.now().items[index].border = deepcopy(border);
			});
			if (0 === hist.now().focus.focusItemIndexes.length) {
				return;
			}
			checkpointStacking('border')
			renderingAll();
		};
		document.getElementById('border-width-input').addEventListener('change', (e) => {
			console.log('border-width-input', e.target.value)
			document.getElementById('border-width-select').value = e.target.value;
			borderChangedCallback();
		});
		document.getElementById('border-width-select').addEventListener('change', (e) => {
			console.log('border-width-select', e.target.value)
			document.getElementById('border-width-input').value = e.target.value;
			borderChangedCallback();
		});
		document.getElementById('stroke-linecap').addEventListener('change', (e) => {
			console.log('stroke-linecap', e.target.value)
			borderChangedCallback();
		});
		document.getElementById('stroke-linejoin').addEventListener('change', (e) => {
			console.log('stroke-linejoin', e.target.value)
			borderChangedCallback();
		});
	}
	// ** figure
	{
		// SPEC Figureの角のステップ数（角飛ばしの上限の元）は角数に制約される
		const starryCornerStepNumMaxFromCornerNum = (cornerNum) => {
			let res = Math.floor(cornerNum/2);
			if(0 === (cornerNum % 2)){
				return res - 1; // 整数（ex.６角形が３角形になる等）
			}
			return res;
		};
		document.getElementById('figure-kind').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			let item = Focus_currerntItem();
			if((! item) || ('Figure' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			item.figureKind = e.target.value;
			renderingAll();
			checkpointStacking('figure-kind');
		});
		document.getElementById('figure-corner-num').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			let item = Focus_currerntItem();
			if((! item) || ('Figure' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			item.cornerNum = parseInt(e.target.value, 10);
			const max = starryCornerStepNumMaxFromCornerNum(item.cornerNum)
			if(item.starryCornerStepNum > max){
				// SPEC 角飛ばしの数を合わせて変更
				item.starryCornerStepNum = max;
			}
			renderingAll();
			checkpointStacking('figure-corner-num');
		});
		document.getElementById('figure-starry-corner-step-num').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			let item = Focus_currerntItem();
			if((! item) || ('Figure' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			let v = parseInt(e.target.value, 10);
			const max = starryCornerStepNumMaxFromCornerNum(item.cornerNum)
			if(v > max){
				setMessage(`invalid starryCornerStepNum max(${max}) by cornerNum`);
				// MEMO 元の値の復元が面倒なので、おそらく直前の値っぽいものへ書き換えてしまう。
				v = max;
				document.getElementById('figure-starry-corner-step-num').value = v;
			}
			item.starryCornerStepNum = v;
			renderingAll();
			checkpointStacking('figure-starry-corner-step-num');
		});
		const setCornerPitRParcent = (v) => {
			let item = Focus_currerntItem();
			if((! item) || ('Figure' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			item.starryCornerPitRPercent = v;
			document.getElementById('figure-starry-corner-pit-r-parcent-step').value = v;
			renderingAll();
			checkpointStacking('figure-starry-corner-pit-r-parcent');
		};
		document.getElementById('figure-starry-corner-pit-r-parcent').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			const v = parseFloat(e.target.value);
			setCornerPitRParcent(v)
		});
		document.getElementById('figure-starry-corner-pit-r-parcent-step').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			const v = parseInt(e.target.value);
			setCornerPitRParcent(v)
		});
		document.getElementById('figure-to-bezier').addEventListener('click', (e) => {
			console.log(e.target.id, e.target.value);
			const itemIndex = Focus_currerntItemIndex();
			let item = hist.now().items.at(itemIndex);
			if((! item) || ('Figure' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			const beziers = PV.Item_beziersFromFigure(item);
			hist.now().items.splice(itemIndex, 1, ...beziers);
			for(let i = 0; i < beziers.length; i++){
				// 複数Itemを追加したので、ここでnameの重複を解消
				if(0 !== i){ // ただし自分自身と重複してしまうので最初の要素を除く
					beziers[i].name = newName(beziers[i].name);
				}
				Focus_addItemIndex(itemIndex + i);
				Focus_addAllApFromItemIndex(itemIndex + i);
			}
			renderingAll();
			checkFocusCurrentTopItemChangedCallback();
			checkpointStacking('figure-to-bezier');
		});
		document.getElementById('figure-setting').style.display = 'none';
	}
	// ** guide
	{
		document.getElementById('guide-axis').addEventListener('change', (e) => {
			console.log(e.target.id, e.target.value);
			let item = Focus_currerntItem();
			if((! item) || ('Guide' !== item.kind)){
				console.error('BUG', item);
				return;
			}
			item.axis = e.target.value;
			renderingAll();
			checkpointStacking('figure-kind');
		});
		document.getElementById('guide-setting').style.display = 'none';
	}

	/* layer */
	{
		// layer-itemはHTML上にテンプレートを用意し、コピーして使いまわす。
		// layerItemSrcを非表示へ
		layerItemElemSrc = document.getElementById('layer-item-src').cloneNode(true);
		layerItemElemSrc.style.display = 'none';
		layerItemElemSrc.id = undefined;
		document.getElementById('layer-item-src').remove(); // コピーは取ったので元のelementは消す
		
		let dialog = document.getElementById('layer-setting-dialog');
		dialog.addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, dialog.returnValue);
			if('OK' !== dialog.returnValue){
				console.log('cancel button');
				return;
			}

			const newName = PV.sanitizeName(document.getElementById('layer-name-input').value);
			const isAlreadyExist = (undefined !== getItemFromName(newName));
			if(isAlreadyExist){
				console.log(`"${newName}" is already exist`);
				setMessage(`"${newName}" is already exist`);
				return;
			}

			let item = Focus_currerntItem();
			item.name = newName;
			updateLayerView();
			checkpointStacking('LayerItemSetting')
		});

		updateLayerView();
	}
	// ** confirm
	{
		document.getElementById('confirm-dialog').addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, event.currentTarget.returnValue);
			switch(event.currentTarget.returnValue){
			case 'OK':
				console.log('confirm OK');
				if(!! confirmOkFunc){
					confirmOkFunc();
				}
				break;
			case 'cancel':
				console.log('confirm canceled');
				if(!! confirmCancelFunc){
					confirmCancelFunc();
				}
				break;
			default:
				console.error('BUG', selectDialog.returnValue);
				break;
			}
			confirmOkFunc = undefined;
			confirmCancelFunc = undefined;
		});
	}
	// ** document
	{
		// doc-itemはHTML上にテンプレートを用意し、コピーして使いまわす。
		// docItemSrcを非表示へ
		docItemSrc = document.getElementById('doc-item-src').cloneNode(true);
		docItemSrc.style.display = 'none';
		docItemSrc.id = undefined;

		let saveDialog = document.getElementById('doc-save-dialog');
		saveDialog.addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, event.currentTarget.returnValue);
			switch(event.currentTarget.returnValue){
			case 'Save':{
				const newName = document.getElementById('doc-name-input').value;
				if('' === newName){
					setMessage('error: document name empty.');
					return;
				}
				if(! strage.saveCurrentDocumentName(newName)){
					setMessage(`error: invalid document name "${newName}".`);
					return;
				}
				// docSelectDialogが立っている可能性があるので、
				// その名前表示を更新する。
				updateDocSelectDialog();
			}
				break;
			case 'cancel':
				console.log('cancel button');
				return;
			default:
				console.error('BUG', selectDialog.returnValue);
				break;
			}
		});

		let selectDialog = document.getElementById('doc-select-dialog');
		selectDialog.addEventListener('close', (event) => {
			console.log('closed dialog', event.currentTarget.id, event.currentTarget.returnValue);
			switch(event.currentTarget.returnValue){
			case 'NewDocument':{
				console.log('NewDocument button');
				startNewDocumentOpen();
			}
				break;
			case 'cancel':
				console.log('cancel button');
				return;
			case 'OpenDocument':{
				console.log('document opened.')
			}
				return;
			default:
				console.error('BUG', selectDialog.returnValue);
				break;
			}
		});
	}

	// ** dialog挙動のカスタマイズ
	{
		const dialogs = document.getElementsByTagName('dialog');
		dialogs.forEach(dialog => {
			// MEMO デフォルトではdialog内のinputでEnter押下すると、dialogが閉じてしまう。
			// そのためEnterキーをキャンセルしつつ、入力の確定は行う処理をここで追加。
			dialog.addEventListener('keydown', (event) => {
				const elem = document.activeElement;
				console.log(`dialog keydown. key:"${event.key}" elem:"${elem.id}"`)
				if('INPUT' === elem.tagName && 'Enter' === event.key){
					elem.dispatchEvent( new KeyboardEvent('change'));
					event.preventDefault();
					return;
				}
				// SPEC Escapeキー押下された場合にDialogを閉じる
				if('Escape' === event.key){
					console.log(`Escape key in ${dialog.id} dialog.`);
					dialog.close('cancel');
					event.preventDefault();
					return;
				}
			});
			let inputs = dialog.getElementsByTagName('input');
			inputs.forEach(input => {
				// dialogをdraggableにすると、inputで範囲選択しようとした際に範囲選択でなくdialogが動いてしまう。
				input.draggable = true; // 一旦 draggable="true" を付与しないとそもそもコールバックされない。
				input.addEventListener('dragstart', (event) => {
					console.log('DRAG');
					event.preventDefault();
					event.stopPropagation();
				});
			});
			inputs.forEach(input => {
				// MEMO 入力確定してからのEnterキーではOK(Save等含む)で閉じる処理。
				// MEMO（ですぐ閉じるのはちょっと不安なのでOK Buttonにフォーカスする処理でお茶を濁している）

				// https://blog.utgw.net/entry/2021/06/29/212256

				// SafariっぽいUAのとき、compositionend イベントの直後かどうか判定できるようにする
				const isSafari = navigator.userAgent.includes("Safari/") && navigator.userAgent.includes("Version/");
				let isCompositionFinished = true;

				input.addEventListener("keydown", (e) => {
					if (isSafari && isCompositionFinished) {
						isCompositionFinished = false;
						return;
					  }
					  if (e.key !== "Enter"){
						return;
					  }
					  if(e.isComposing){
						console.log('input composing');
						return;
					  }
					  e.preventDefault();

					  // 最後のButtonがOKであることに期待してそこにFocusする。
					  let buttons = dialog.getElementsByTagName('button');
					  buttons[buttons.length - 1].focus();
					  //dialog.close('cancel');
				});
				input.addEventListener("compositionstart", () => {
					isCompositionFinished = false;
				});
				input.addEventListener("compositionend", () => {
					isCompositionFinished = true;
				});
			});
			// TODO ダイアログの外側をクリックしたら閉じる
		});
	}
	// ** dialogをドラッグ可能にする
	{
		const dialogs = document.getElementsByTagName('dialog');
		dialogs.forEach(dialog => {
			let mouse = {
				x: 0,
				y: 0,
			};
			let pos = {
				x: 0,
				y: 0,
			};
			dialog.addEventListener('dragstart', evt => {
				mouse.y = evt.pageY;
				mouse.x = evt.pageX;
				if(! dialog.style.top){
					dialog.style.top = "0px";
					dialog.style.left = "0px";
					pos.y = 0;
					pos.x = 0;
				}else{
					pos.y = parseInt(dialog.style.top.slice(0, -2), 10);
					pos.x = parseInt(dialog.style.left.slice(0, -2), 10);	
				}
				evt.dataTransfer.setDragImage(document.createElement('div'), 0, 0);
				console.log('dragstart', evt.pageX, evt.pageY, mouse, pos, dialog.style.top, evt.currentTarget.id);
			});
			dialog.addEventListener('drag', evt => {
				if (evt.x === 0 && evt.y === 0) return;
				const movey = (evt.pageY - mouse.y) * 2;
				const movex = (evt.pageX - mouse.x) * 2;
				dialog.style.top  = (pos.y + movey) + 'px';
				dialog.style.left = (pos.x + movex) + 'px';
				console.log('drag', evt.pageX, evt.pageY, mouse, pos, dialog.style.top, movey);
			});
		});
	}
	// ** NumberInputのカスタマイズ
	{
		// MEMO input[type="number"]を'exspinstep'で拡張して、
		// stepによる小数点桁のvalidateとspinクリックのステップ値を分離。
		let numinputs = document.querySelectorAll('input[type="number"]');
		numinputs.forEach(numinput => {
			if(! numinput.hasAttribute('exspinstep')){
				return;
			}
			numinput.addEventListener('mousedown', (event) => {
				console.log(event.currentTarget.id, 'mouse');
				numinput.setAttribute('prevstep', numinput.step);
				numinput.step = numinput.getAttribute('exspinstep')
			});
			numinput.addEventListener('mouseup', (event) => {
				console.log(event.currentTarget.id, 'mouse');
				numinput.step = numinput.getAttribute('prevstep');
			});
		});
	}
	// ** contextmenu
	// ** move,resize,rotate // SPEC 数字指定のmove,resize,rotate
	{
		let input2dDialog = document.getElementById('input2d-dialog');
		let title = document.getElementById('input2d-dialog-title');
		let xinput = document.getElementById('input2d-dialog-x-input');
		let yinput = document.getElementById('input2d-dialog-y-input');
		let xunit = document.getElementById('input2d-dialog-x-input');
		let yunit = document.getElementById('input2d-dialog-y-input');
		let ratio = document.getElementById('input2d-dialog-is-keep-aspect-ratio');
		const targetKind = (Tool.getToolFromIndex(editor.toolIndex).isItem) ? 'Item' : 'AP';

		const execute = () => {
			switch(title.textContent){
			case 'Move':{
				const x = parseFloat(xinput.value);
				const y = parseFloat(yinput.value)
				if(Number.isNaN(x) || Number.isNaN(y)){
					console.log('input not number', xinput.value, yinput.value);
					return false;
				}
				const move = {'x': x, 'y': y};
				moveFocusItems(targetKind, move);
			}
				return true;
			case 'Resize':{
				const x = parseFloat(xinput.value);
				const y = parseFloat(yinput.value)
				if(Number.isNaN(x) || Number.isNaN(y)){
					console.log('input not number', xinput.value, yinput.value);
					return false;
				}
				const scale2d = {'x': x / 100, 'y': y / 100};
				const editCenter = PV.centerFromRect(getFocusingRectByPrev());
				resizeFocusItems(targetKind, scale2d, editCenter);
			}
				return true;
			default:
				console.error('BUG', title.textContent);
				return false;
			}
		};

		xinput.addEventListener('input', (event) => {
			console.log(event.currentTarget.id, event.currentTarget.value);
			if(('none' !== ratio.parentElement.style.display) && ratio.checked){
				yinput.value = xinput.value;
			}
			execute();
			renderingAll();
		});
		yinput.addEventListener('input', (event) => {
			console.log(event.currentTarget.id, event.currentTarget.value);			
			if(('none' !== ratio.parentElement.style.display) && ratio.checked){
				xinput.value = yinput.value;
			}
			execute();
			renderingAll();
		});
		input2dDialog.addEventListener('close', (event) => {
			console.log('close', event.currentTarget.id, event.currentTarget.returnValue);
			if('OK' !== event.currentTarget.returnValue){
				console.log(event.currentTarget.id, 'cancel');
				hist.cancelNow();
				renderingAll();
				return;
			}

			if(! execute()){
				setMessage(`error: ${title.textContent} cant apply`);
				hist.cancelNow();
				renderingAll();
				return;
			}

			renderingAll();
			checkpointStacking(title.textContent);
		});
		input2dDialog.addEventListener('keydown', (event) => {
			// SPEC input2dダイアログはキー押下されたらaxisにfocusする。
			switch(event.key){
			case 'x':
				xinput.select();
				break;
			case 'y':
				yinput.select();
				break;
			}
		});
		document.getElementById('contextmenu-move-button').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			showMoveDialog();
		});
		document.getElementById('contextmenu-resize-button').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			showResizeDialog();
		});
	}
	{
		let rinput = document.getElementById('rotate-input');
		rinput.addEventListener('input', (event) => {
			console.log(event.currentTarget.id, event.currentTarget.value);
			const editCenter = PV.centerFromRect(getFocusingRectByPrev());
			hist.now().focus.focusItemIndexes.forEach(itemIndex => {
				Item_rotateWithIndex(itemIndex, rinput.value, editCenter);
			});
			renderingAll();
		});
		document.getElementById('rotate-dialog').addEventListener('close', (event) => {
			console.log('close', event.currentTarget.id, event.currentTarget.returnValue);
			if('OK' !== event.currentTarget.returnValue){
				console.log(event.currentTarget.id, 'cancel');
				hist.cancelNow();
				renderingAll();
				return;
			}

			hist.now().focus.focusItemIndexes.forEach(itemIndex => {
				Item_rotateWithIndex(itemIndex, rinput.value, PV.centerFromRect(getFocusingRectByPrev()));
			});
			renderingAll();
			checkpointStacking(event.currentTarget.id);
		});
		document.getElementById('contextmenu-rotate-button').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			rinput.value = 0;
			showModalWrapper('rotate-dialog');
		});
	}
	// ** align // SPEC 端揃え(上下前後寄せ・X中央・Y中央)
	{
		const align = (kind) => {
			if(Tool.getToolFromIndex(editor.toolIndex).isItem){
				if(2 > hist.now().focus.focusItemIndexes.length){
					setMessage('need 2 focus items.');
					return;
				}
				const focusBox = Render.getFocusingRectByNow(editor, hist.now());
				if(! focusBox){
					console.error('BUG', hist.now().focus.focusItemIndexes);
					return;
				}

				switch(kind){
				case 'align-left':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': focusBox.x - itemBox.x, 'y': 0};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				case 'align-x-center':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const xcenter = focusBox.x + (focusBox.w/2);
						const itemxcenter = itemBox.x + (itemBox.w/2);
						const moveVec = {'x': xcenter - itemxcenter, 'y': 0};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				case 'align-right':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const xright = focusBox.x + focusBox.w;
						const itemright = itemBox.x + itemBox.w;
						const moveVec = {'x': xright - itemright, 'y': 0};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				case 'align-top':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': 0, 'y': focusBox.y - itemBox.y};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				case 'align-y-center':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const ycenter = focusBox.y + (focusBox.h/2);
						const itemycenter = itemBox.y + (itemBox.h/2);
						const moveVec = {'x': 0, 'y': ycenter - itemycenter};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				case 'align-bottom':{
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						let item = hist.now().items.at(itemIndex);
						const itemBox = PV.rectFromItems([item]);
						const xbottom = focusBox.y + focusBox.h;
						const itembottom = itemBox.y + itemBox.h;
						const moveVec = {'x': 0, 'y': xbottom - itembottom};
						Item_moveNowWithIndex(itemIndex, moveVec);
					})
				}
					break;
				default:
					console.error('BUG', kind);
					return;
				}

				renderingAll();
				checkpointStacking(kind);
			}else{
				if(2 > hist.now().focus.focusAPCplxes.length){
					setMessage('need 2 focus aps.');
					return;
				}
				const focusBox = Render.getFocusingRectByNow(editor, hist.now());
				if(! focusBox){
					console.error('BUG', hist.now().focus.focusAPCplxes);
					return;
				}
				switch(kind){
				case 'align-left':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const moveVec = {'x': focusBox.x - ap.point.x, 'y': 0};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				case 'align-x-center':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const xcenter = focusBox.x + (focusBox.w/2);
						const moveVec = {'x': xcenter - ap.point.x, 'y': 0};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				case 'align-right':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const xright = focusBox.x + focusBox.w;
						const moveVec = {'x': xright - ap.point.x, 'y': 0};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				case 'align-top':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const moveVec = {'x': 0, 'y': focusBox.y - ap.point.y};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				case 'align-y-center':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const ycenter = focusBox.y + (focusBox.h/2);
						const moveVec = {'x': 0, 'y': ycenter - ap.point.y};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				case 'align-bottom':{
					hist.now().focus.focusAPCplxes.forEach(cplx => {
						let ap = PV.apFromItems(hist.now().items, cplx);
						const xbottom = focusBox.y + focusBox.h;
						const moveVec = {'x': 0, 'y': xbottom - ap.point.y};
						ap.point = PV.pointAdd(ap.point, moveVec);
					})
				}
					break;
				default:
					console.error('BUG', kind);
					return;
				}

				renderingAll();
				checkpointStacking(kind);
			}
		};
		document.getElementById('align-left').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
		document.getElementById('align-x-center').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
		document.getElementById('align-right').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
		document.getElementById('align-top').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
		document.getElementById('align-y-center').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
		document.getElementById('align-bottom').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			align(event.currentTarget.id);
		});
	}

	// ** interval // SPEC 等間隔(上下前後基準・X中央基準・Y中央基準)
	{
		const interval = (kind) => {
			if(Tool.getToolFromIndex(editor.toolIndex).isItem){
				if(3 > hist.now().focus.focusItemIndexes.length){
					setMessage('need 3 focus items.');
					return;
				}

				// SPEC Itemの等間隔
				switch(kind){
				case 'interval-x-left':{
					const xAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return aItemBox.x - bItemBox.x;
					});
					const allInterval = PV.rectFromItems([xAxisFocusItems.at(-1)]).x - PV.rectFromItems([xAxisFocusItems.at(0)]).x;
					const interval = allInterval / (xAxisFocusItems.length - 1);
					const start = PV.rectFromItems([xAxisFocusItems.at(0)]).x;
					xAxisFocusItems.forEach((item, ix) => {
						const posx = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': posx - itemBox.x, 'y': 0};
						Item_moveNow(item, moveVec);
					});
				}
					break;
				case 'interval-x-center':{
					const xAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return PV.centerFromRect(aItemBox).x - PV.centerFromRect(bItemBox).x;
					});
					const allInterval = PV.centerFromRect(PV.rectFromItems([xAxisFocusItems.at(-1)])).x - PV.centerFromRect(PV.rectFromItems([xAxisFocusItems.at(0)])).x;
					const interval = allInterval / (xAxisFocusItems.length - 1);
					const start = PV.centerFromRect(PV.rectFromItems([xAxisFocusItems.at(0)])).x;
					xAxisFocusItems.forEach((item, ix) => {
						const posx = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': posx - PV.centerFromRect(itemBox).x, 'y': 0};
						Item_moveNow(item, moveVec);
					});
				}
						break;
				case 'interval-x-right':{
					const xAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return PV.rightBottomFromRect(aItemBox).x - PV.rightBottomFromRect(bItemBox).x;
					});
					const allInterval = PV.rightBottomFromRect(PV.rectFromItems([xAxisFocusItems.at(-1)])).x - PV.rightBottomFromRect(PV.rectFromItems([xAxisFocusItems.at(0)])).x;
					const interval = allInterval / (xAxisFocusItems.length - 1);
					const start = PV.rightBottomFromRect(PV.rectFromItems([xAxisFocusItems.at(0)])).x;
					xAxisFocusItems.forEach((item, ix) => {
						const posx = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': posx - PV.rightBottomFromRect(itemBox).x, 'y': 0};
						Item_moveNow(item, moveVec);
					});
				}
					break;
				case 'interval-y-top':{
					const yAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return aItemBox.y - bItemBox.y;
					});
					const allInterval = PV.rectFromItems([yAxisFocusItems.at(-1)]).y - PV.rectFromItems([yAxisFocusItems.at(0)]).y;
					const interval = allInterval / (yAxisFocusItems.length - 1);
					const start = PV.rectFromItems([yAxisFocusItems.at(0)]).y;
					yAxisFocusItems.forEach((item, ix) => {
						const posy = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': 0, 'y': posy - itemBox.y};
						Item_moveNow(item, moveVec);
					});
				}
					break;
				case 'interval-y-center':{
					const yAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return PV.centerFromRect(aItemBox).y - PV.centerFromRect(bItemBox).y;
					});
					const allInterval = PV.centerFromRect(PV.rectFromItems([yAxisFocusItems.at(-1)])).y - PV.centerFromRect(PV.rectFromItems([yAxisFocusItems.at(0)])).y;
					const interval = allInterval / (yAxisFocusItems.length - 1);
					const start = PV.centerFromRect(PV.rectFromItems([yAxisFocusItems.at(0)])).y;
					yAxisFocusItems.forEach((item, ix) => {
						const posy = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': 0, 'y': posy - PV.centerFromRect(itemBox).y};
						Item_moveNow(item, moveVec);
					});
				}
					break;
				case 'interval-y-bottom':{
					const yAxisFocusItems = PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes).sort((aItem,bItem) => {
						const aItemBox = PV.rectFromItems([aItem]);
						const bItemBox = PV.rectFromItems([bItem]);
						return PV.rightBottomFromRect(aItemBox).y - PV.rightBottomFromRect(bItemBox).y;
					});
					const allInterval = PV.rightBottomFromRect(PV.rectFromItems([yAxisFocusItems.at(-1)])).y - PV.rightBottomFromRect(PV.rectFromItems([yAxisFocusItems.at(0)])).y;
					const interval = allInterval / (yAxisFocusItems.length - 1);
					const start = PV.rightBottomFromRect(PV.rectFromItems([yAxisFocusItems.at(0)])).y;
					yAxisFocusItems.forEach((item, ix) => {
						const posy = start + (ix * interval);
						const itemBox = PV.rectFromItems([item]);
						const moveVec = {'x': 0, 'y': posy - PV.rightBottomFromRect(itemBox).y};
						Item_moveNow(item, moveVec);
					});
				}
					break;
				default:
					console.error('BUG', kind);
					return;
				}

				renderingAll();
				checkpointStacking(kind);
			}else{
				if(3 > hist.now().focus.focusAPCplxes.length){
					setMessage('need 3 focus aps.');
					return;
				}

				// SPEC APの等間隔はX軸・Y軸のみで他は共通
				// MEMO Menuの表示・非表示を切り替える
				switch(kind){
				case 'interval-x-left':
				case 'interval-x-center':
				case 'interval-x-right':{
					const xAxisFocusAps = PV.apsFromItems(hist.now().items, hist.now().focus.focusAPCplxes).sort((aAp,bAp) => {
						return aAp.point.x - bAp.point.x;
					});
					const allInterval = xAxisFocusAps.at(-1).point.x - xAxisFocusAps.at(0).point.x;
					const interval = allInterval / (xAxisFocusAps.length - 1);
					const start = xAxisFocusAps.at(0).point.x;
					xAxisFocusAps.forEach((ap, ix) => {
						const posx = start + (ix * interval);
						const moveVec = {'x': posx - ap.point.x, 'y': 0};
						ap.point = PV.pointAdd(ap.point, moveVec);
					});
				}
					break;
				case 'interval-y-top':
				case 'interval-y-center':
				case 'interval-y-bottom':{
					const yAxisFocusAps = PV.apsFromItems(hist.now().items, hist.now().focus.focusAPCplxes).sort((aAp,bAp) => {
						return aAp.point.y - bAp.point.y;
					});
					const allInterval = yAxisFocusAps.at(-1).point.y - yAxisFocusAps.at(0).point.y;
					const interval = allInterval / (yAxisFocusAps.length - 1);
					const start = yAxisFocusAps.at(0).point.y;
					yAxisFocusAps.forEach((ap, ix) => {
						const posy = start + (ix * interval);
						const moveVec = {'x': 0, 'y': posy - ap.point.y};
						ap.point = PV.pointAdd(ap.point, moveVec);
					});
				}
					break;
				default:
					console.error('BUG', kind);
					return;
				}

				renderingAll();
				checkpointStacking(kind);
			}
		};
		document.getElementById('interval-x-left').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
		document.getElementById('interval-x-center').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
		document.getElementById('interval-x-right').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
		document.getElementById('interval-y-top').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
		document.getElementById('interval-y-center').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
		document.getElementById('interval-y-bottom').addEventListener('click', (event) => {
			console.log(event.currentTarget.id);
			interval(event.currentTarget.id);
		});
	}

	/* */
	let draw = SVG().addTo('#edit-field');
	let rootG = draw.group().id('rootg');
	renderingHandle = {
		'draw': draw,
		'rootG': rootG,
		'backgroundG': rootG.group().id('backgroundg'),
		'canvasParentG': rootG.group().id('canvasparentg'),
		'forgroundG': rootG.group().id('forgroundg'),
	};
	renderingAll();
	fittingCanvasScale();

	checkFocusCurrentTopItemChangedCallback();

	setMessage(`wakeuped.`);

	// ** startup dialog
	{
		let head = document.getElementsByTagName('head')[0];
		let link = document.createElement('link');
		link.id = 'startup-link';
		link.setAttribute('rel','stylesheet');
		link.setAttribute('type','text/css');
		link.setAttribute('href','css/startup.css');
		head.appendChild(link);
	}
	setTimeout(() => {
		document.getElementById('startup-dialog').close();
	}, 2 * 1000);
}

function initContextmenu(){
	// https://www.cssscript.com/beautiful-multi-level-context-contextmenu-with-pure-javascript-and-css3/ 

	let menu = document.querySelector('.contextmenu');
	function showMenu(x, y){
		menu.style.left = x + 'px';
		menu.style.top = y + 'px';
		menu.classList.add('show-contextmenu');

		isDisableHistoryUpdate = false;
	}
	function hideMenu(){
		menu.classList.remove('show-contextmenu');
	}
	function onContextMenu(e){
		e.preventDefault();
		showMenu(e.pageX, e.pageY);
		document.addEventListener('click', onClick, false);
	}
	function onClick(e){
		hideMenu();
		document.removeEventListener('click', onClick);
	}
	document.getElementById('edit-field-frame').addEventListener('contextmenu', onContextMenu, false);	
}

// ドキュメント内で使われてない名前を生成して返す。
// 最初は渡されたままの名前をチェックし、重複していなければそれをそのまま返す。
// (すでに数字が付いていてもまずはその名前の存在をチェックする。)
function newName(src){
	let head = src;
	let count = -1;
	const m = src.match(/^(.*)([0-9]+)$/);
	if(!! m){
		head = m[1];
		count = parseInt(m[2], 10);
	}
	let name = src;

	while(undefined !== getItemFromName(name)){
		count++;
		name = `${head}${count}`;
	}
	return name;
}
function newFigureItem(point){
	let figure = PV.FigureItemDefault();
	figure.name = newName(figure.name);
	figure.colorPair = deepcopy(palette.colorPair);
	figure.border = deepcopy(border);
	figure.point = deepcopy(point);
	figure.rsize = {'x': toCanvas(100), 'y': toCanvas(100)};
	return figure;
}
function newGuideItem(point, axis){
	let guide = PV.GuideItemDefault();
	guide.name = newName(guide.name);
	guide.point = deepcopy(point);
	guide.axis = axis;
	return guide;
}
function newBezierItem(point){
	const overwrite = {
		'kind': 'Bezier',
		'name': newName('Bezier'),
		'isVisible': true,
		'isLock': false,
		'colorPair': deepcopy(palette.colorPair),
		'border': deepcopy(border),
		//
		'aps': [PV.AP_newFromPoint(point)],
		'isCloseAp': false,
	};
	return lodash.merge(PV.BezierItemDefault(), overwrite)
}

function cutOrCopyToClipboard(kind){
	if(0 === hist.now().focus.focusItemIndexes.length){
		setMessage(`${kind} item not focus.`)
		return;
	}
	editor.clipboard.items = [];
	let fiis = deepcopy(hist.now().focus.focusItemIndexes);
	fiis.sort((a,b)=>{return b-a});

	const func_ = (kind, itemIndex) => {
		switch(kind){
		case 'cut':
			const [item] = hist.now().items.splice(itemIndex, 1);
			return item;
		case 'copy':
			return hist.now().items.at(itemIndex);
		default:
			console.log('BUG', kind);
			return undefined;
		}
	};
	if(Tool.getToolFromIndex(editor.toolIndex).isItem){
		fiis.forEach(itemIndex => {
			const item = func_(kind, itemIndex);
			if(! item){
				console.error('BUG', itemIndex);
				return;
			}
			editor.clipboard.items.push(item);
		});
		editor.clipboard.items.reverse();
	}else{
		// FocusされているAPのみ抽出する。
		fiis.forEach(itemIndex => {
			let item = func_(kind, itemIndex);
			if(! item){
				console.error('BUG', itemIndex);
				return;
			}
			let isAllFocused = true;
			let tmpAps = [];
			let newItems = [];
			item.aps.forEach((ap, apIndex, arr) => {
				if(Focus_isExistAP(itemIndex, apIndex)){
					tmpAps.push(ap);
					if(apIndex !== (arr.length - 1)){ // 最後のAPに到達していたら下の処理へjump
						return;
					}
				}
				isAllFocused = false;
				if(0 === tmpAps.length){ // ここまでFocusされているAPが無ければ次へ
					return;
				}
				// 複製した新しいItemにここまでのAPを入れたものをclipboardに詰める
				let newItem = deepcopy(item);
				newItem.isCloseAp = false;
				newItem.aps = tmpAps;
				tmpAps = [];
				newItems.push(newItem);
			});
			if(isAllFocused){ // 全てのAPがFocusされていた場合、そのままitemを詰める
				editor.clipboard.items.push(item);
			}else{
				// 最後に反転されるのを見越して、ここで一回反転しておく。
				newItems.reverse();
				editor.clipboard.items.push(...newItems);
			}
			editor.clipboard.items.reverse();
		});
	}
	Focus_clear();
	editor.clipboard.pasteCount = 0;
	setMessage(`${kind} to clipboard item(s).`)
}
function pasteByClipboard(vec_){
	if(0 === editor.clipboard.items.length){
		setMessage('paste empty clipboard.')
		return;
	}
	// SPEC Paste先は、現在のFocusItemの位置、なければDocの最前。
	let insertIndex = hist.now().focus.focusItemIndexes.at(-1);
	if(! insertIndex){
		insertIndex = hist.now().items.length;
	}else{
		insertIndex += 1;
	}
	let items = deepcopy(editor.clipboard.items);
	// paste前の位置移動があればそれを行う
	editor.clipboard.pasteCount++;
	const vec = toCanvas(vec_) * (editor.clipboard.pasteCount);
	items.forEach(item => {
		switch(item.kind){
		case 'Bezier':
			item.aps.forEach((ap, ix) => {
				ap.point = PV.pointAdd(ap.point, PV.toPoint(vec));
			});
			break;
		case 'Figure':
		case 'Guide':
			item.point = PV.pointAdd(item.point, PV.toPoint(vec));
			break;
		default:
			console.error('BUG', item);
			return;
		}
	});

	hist.now().items.splice(insertIndex, 0, ...items);

	// itemsに追加したことで効くようになったnewNameをこのタイミングで適用
	for(let i = (items.length - 1); 0 <= i; i--){
		let item = hist.now().items.at(insertIndex + i);
		item.name = newName(item.name);
	}
	// Focusを変更
	const newFiis = [...Array(items.length)].map((_, i) => i + insertIndex);
	hist.now().focus.focusItemIndexes = newFiis;
	hist.now().focus.focusAPCplxes = [];
	newFiis.forEach(itemIndex => {
		Focus_addAllApFromItemIndex(itemIndex);
	});
	setMessage(`paste from clipboard ${items.length} item(s).`)
}

function shiftingItemOrder_(itemIndex, sign){
	if((0 < sign) && (itemIndex === (hist.now().items.length - 1))){
		console.log('already top item');
		return;
	}
	if((0 > sign) && (itemIndex === 0)){
		console.log('already tail item');
		return;
	}
	const [item] = hist.now().items.splice(itemIndex, 1);
	const newItemIndex = itemIndex + sign;
	hist.now().items.splice(newItemIndex, 0, item);
	renumberingItemsByMovedItemIndex_(itemIndex, newItemIndex);
};

function fullshiftingItemsOrder_(sign){
	// 対象Itemsを収集
	// 1. 最前・最後に関係なく対象Items同士のZ順は保持する。
	// 2. Index順が変化してしまうので、抜き出しは末尾から行う。(抜き出してから反転)
	const fiis = deepcopy(hist.now().focus.focusItemIndexes).sort((a,b) => {return b-a});
	let items = [];
	fiis.forEach(itemIndex => {
		const [item] = hist.now().items.splice(itemIndex, 1);
		if(! item){
			console.error('BUG', itemIndex);
			return;
		}
		items.push(item);
	});
	items.reverse();
	// 挿入はまとめて行う
	const insertIndex = (0 < sign) ? (hist.now().items.length - 1 + 1) : 0;
	hist.now().items.splice(insertIndex, 0, ...items);
	// Focusを変更
	const newFiis = [...Array(items.length)].map((_, i) => i + insertIndex);
	hist.now().focus.focusItemIndexes = newFiis;
	hist.now().focus.focusAPCplxes = [];
}

function dateformat(date){
	const s = date.getFullYear() // yyyy
	+ '/' + ('00' + (date.getMonth() + 1)).slice(-2) // mm
	+ '/' + ('00' + (date.getDate())).slice(-2)// dd
	+ ' '
	+ ('00' + (date.getHours())).slice(-2)// hh
	+ ':' + ('00' + (date.getMonth())).slice(-2)// mm
	+ ':' + ('00' + (date.getSeconds())).slice(-2)// ss

	return s;
}

// modalDialogを開いて指定elementがあればそこにfocusするwrapper
// 開発中の確認では、起動後初回表示時にdoc-select-dialogにて先頭docの先頭ボタンにfocusが当たって描画が変化しており、
// 何か操作した次以降はFocusが移動したのかFocus描画はされなかったように見える。
// 引数はdialogDOMElementとidに対応する。
// MEMO dialogElementにcloseEventはあってもopenEventはない。
function showModalWrapper(dialogElementTarget){
	let dialog;
	if('string' === typeof dialogElementTarget){
		dialog = document.getElementById(dialogElementTarget);
	}else{
		dialog = dialogElementTarget;
	}
	dialog.showModal();
	let dfes = dialog.getElementsByClassName('default-focus-in-dialog');
	if(0 !== dfes.length){
		if(!! dfes[0].select){
			// INPUTなどの場合にFocusするだけでなく入力文字を全選択しておく
			dfes[0].select();
		}else{
			dfes[0].focus();
		}
		return;
	}
}
function updateDocSelectDialog(){
	const newDocItem = (docinfo) => {
		let newDocItem = docItemSrc.cloneNode(true);
		newDocItem.id = `doc_item-${docinfo.id}`;
		newDocItem.style.display = 'flex';

		let thumbElem = newDocItem.getElementsByClassName('doc-item-thumb')[0];
		const thumbkey = `vecterion-doc-thumb-${docinfo.id}`;
		const thumb = localStorage.getItem(thumbkey);
		if(!! thumb){
			thumbElem.src = thumb;
		}

		let nameElem = newDocItem.getElementsByClassName('doc-name')[0];
		nameElem.textContent = ('' === docinfo.name) ? '<untitled document>':docinfo.name;
		let date = new Date(docinfo.latestUpdate);
		let dlut = newDocItem.getElementsByClassName('doc-latest-update-time')[0];
		dlut.textContent = dateformat(date);

		let dbs = newDocItem.getElementsByClassName('doc-bytesize')[0];
		const dockey = `vecterion-doc-${docinfo.id}`;
		if(! localStorage.getItem(dockey)){
			console.error('BUG', dockey);
			dbs.textContent = '   ?';
		}else{
			const v = localStorage.getItem(dockey).length / 1024;
			dbs.textContent = ('    ' + (v).toFixed(0)).slice(-4);
		}

		newDocItem.getElementsByClassName('doc-item-tag')[0].addEventListener('click', (event) => {
			startDocoumentOpen(docinfo.id);
			let selectDialog = document.getElementById('doc-select-dialog');
			selectDialog.close('OpenDocument'); // 文字というかdivクリックではdialogが閉じないので閉じる処理
		});
		newDocItem.getElementsByClassName('doc-item-edit')[0].addEventListener('click', (event) => {
			console.log('doc-item-edit', docinfo.id);
			strage.setCurrentDocument(docinfo.id);
			const name = ('' !== docinfo.name) ? docinfo.name : strage.newDocName();
			document.getElementById('doc-name-input').value = name;
			showModalWrapper('doc-save-dialog');
			event.preventDefault(); // button押下でdialogが閉じるのを抑制
		})
		newDocItem.getElementsByClassName('doc-item-delete')[0].addEventListener('click', (event) => {
			console.log('doc-item-delete', docinfo.id);
			confirmOkFunc = () => {
				let docconf = strage.loadOrNewDocConfig();
				const ix = docconf.docinfos.findIndex(e => e.id === docinfo.id);
				if(-1 === ix){
					console.error('BUG', docconf);
					updateDocSelectDialog();
					return;
				}
				const [removedocinfo] = docconf.docinfos.splice(ix, 1);
				if(! removedocinfo){
					console.error('BUG', ix, docconf);
				}

				strage.deleteDocId(removedocinfo.id);

				updateDocSelectDialog();
				console.log('deleted doc', dockey);
			};
			document.getElementById('confirm-title').textContent = 'Delete?';
			showModalWrapper('confirm-dialog')
			event.preventDefault(); // button押下でdialogが閉じるのを抑制
		})
		return newDocItem;
	};

	let docListElem = document.getElementById('doc-list');
	while (docListElem.firstChild) {
		docListElem.removeChild(docListElem.firstChild);
	}

	const docconf = strage.loadOrNewDocConfig();
	docconf.docinfos.forEach(docinfo => {
		docListElem.appendChild(newDocItem(docinfo));
	});
}
function showMoveDialog(){
	let title = document.getElementById('input2d-dialog-title');
	let xinput = document.getElementById('input2d-dialog-x-input');
	let yinput = document.getElementById('input2d-dialog-y-input');
	let xunit = document.getElementById('input2d-dialog-x-input');
	let yunit = document.getElementById('input2d-dialog-y-input');
	let ratio = document.getElementById('input2d-dialog-is-keep-aspect-ratio');

	title.textContent = 'Move';
	xinput.value = 0;
	yinput.value = 0;
	xinput.setAttribute('exspinstep', 100);
	yinput.setAttribute('exspinstep', 100);
	xunit.textContent = 'px';
	yunit.textContent = 'px';
	ratio.parentElement.style.display = 'none';
	showModalWrapper('input2d-dialog');
}
function showResizeDialog(){
	let title = document.getElementById('input2d-dialog-title');
	let xinput = document.getElementById('input2d-dialog-x-input');
	let yinput = document.getElementById('input2d-dialog-y-input');
	let xunit = document.getElementById('input2d-dialog-x-input');
	let yunit = document.getElementById('input2d-dialog-y-input');
	let ratio = document.getElementById('input2d-dialog-is-keep-aspect-ratio');

	title.textContent = 'Resize';
	xinput.value = 100;
	yinput.value = 100;
	xinput.setAttribute('exspinstep', 10);
	yinput.setAttribute('exspinstep', 10);
	xunit.textContent = '%';
	yunit.textContent = '%';
	ratio.parentElement.style.display = 'block';
	showModalWrapper('input2d-dialog');
}

let layerItemElemSrc;
function updateLayerView(){
	// TODO 高速化！
	// HTML Elementをすべて捨てて再構築するのでとてつもなく遅い！
	// (mousemove中に処理できないのでサボりと遅延実行の仕組みを入れている。)

	const newLayerItem = () => {
		let layerItemElem = layerItemElemSrc.cloneNode(true);
		layerItemElem.style.display = 'block';

		let isVisibleElem = layerItemElem.getElementsByClassName('layer_is-visible')[0];
		let isLockElem = layerItemElem.getElementsByClassName('layer_is-lock')[0];
		let isChildsOpenElem = layerItemElem.getElementsByClassName('layer_is-childs-open')[0];
		let isLevelSpacerElem = layerItemElem.getElementsByClassName('layer_level-spacer')[0];
		let layerNameElem = layerItemElem.getElementsByClassName('layer_name')[0];

		const nameFromId = (id) => {
			return id.substring('layer_item-'.length);
		};

		isVisibleElem.addEventListener('click', (event) => {
			console.log('clicked layer-is-visible', event.currentTarget.parentElement.id);

			let [item, itemIndex] = getItemAndIndexFromName(nameFromId(event.currentTarget.parentElement.id));
			item.isVisible = (! item.isVisible);
			if((! Render.isMetaVisible(item, hist.now()))){
				Focus_removeItemIndex(itemIndex);
			}

			renderingAll();
			updateLayerView();
			checkpointStacking('visibility changed.')
		});
		isLockElem.addEventListener('click', (event) => {
			console.log('clicked layer-is-lock', event.currentTarget.parentElement.id);

			let [item, itemIndex] = getItemAndIndexFromName(nameFromId(event.currentTarget.parentElement.id));
			item.isLock = (! item.isLock);
			if(Render.isMetaLock(item, hist.now())){
				Focus_removeItemIndex(itemIndex);
			}

			renderingAll();
			updateLayerView();
			checkpointStacking('lock changed.')
		});
		isChildsOpenElem.addEventListener('click', (event) => {
			console.log('clicked layer-is-childs-open', event.currentTarget.parentElement.id);
			
			let [item, itemIndex] = getItemAndIndexFromName(nameFromId(event.currentTarget.parentElement.id));
			item.isChildView = (! item.isChildView);
			
			updateLayerView();
			// TODO 本当はdocumenttreeに入れておきたくない気もするのだが、良い案が浮かばなければそのままにする。
			// せめてもの対応として、これを契機にHistoryを呼ばない。
			// （それはそれで別タイミングに保存が発火しそうで怖いのだが。）
			// checkpointStacking('lock changed.')
		});

		layerNameElem.addEventListener('click', (event) => {
			console.log('clicked layer-name-item', event.currentTarget.id);
			let [item, itemIndex] = getItemAndIndexFromName(nameFromId(event.currentTarget.parentElement.id));

			hist.now().focus.focusItemIndexes = [itemIndex];
			hist.now().focus.focusAPCplxes = [];
			checkFocusCurrentTopItemChangedCallback();
			renderingEditorUserInterface();

			updateLayerView();
		});
		layerNameElem.addEventListener('dblclick', (event) => {
			console.log('dblclicked layer-name-item', event.currentTarget.id);
			let [item, itemIndex] = getItemAndIndexFromName(nameFromId(event.currentTarget.parentElement.id));
			document.getElementById('layer-name-input').value = item.name;

			hist.now().focus.focusItemIndexes = [itemIndex];
			hist.now().focus.focusAPCplxes = [];
			checkFocusCurrentTopItemChangedCallback();
			renderingEditorUserInterface();

			showModalWrapper('layer-setting-dialog');
		});	

		return layerItemElem;
	};

	const updateLayerItem = (layerItemElem, layerItem, level, focusKind) => {
		layerItemElem.id = `layer_item-${layerItem.name}`;
		layerItemElem.classList = layerItemElemSrc.classList;
		if(focusKind){
			layerItemElem.classList.add(focusKind);
		}

		let isVisibleElem = layerItemElem.getElementsByClassName('layer_is-visible')[0];
		let isLockElem = layerItemElem.getElementsByClassName('layer_is-lock')[0];
		let isChildsOpenElem = layerItemElem.getElementsByClassName('layer_is-childs-open')[0];
		let isLevelSpacerElem = layerItemElem.getElementsByClassName('layer_level-spacer')[0];
		let layerNameElem = layerItemElem.getElementsByClassName('layer_name')[0];

		isVisibleElem.children[0].style.display = (layerItem.isVisible) ? 'block' : 'none';
		isVisibleElem.children[1].style.display = (! layerItem.isVisible) ? 'block' : 'none';
		isLockElem.children[0].style.display = (! layerItem.isLock) ? 'block' : 'none';
		isLockElem.children[1].style.display = (layerItem.isLock) ? 'block' : 'none';

		if(Render.isMetaVisible(layerItem, hist.now())){
			isVisibleElem.children[0].classList.remove('layer_is-meta-disable');
			isVisibleElem.children[1].classList.remove('layer_is-meta-disable');
		}else{
			isVisibleElem.children[0].classList.add('layer_is-meta-disable');
			isVisibleElem.children[1].classList.add('layer_is-meta-disable');
		}
		if(Render.isMetaLock(layerItem, hist.now())){
			isLockElem.children[0].classList.add('layer_is-meta-disable');;
			isLockElem.children[1].classList.add('layer_is-meta-disable');;
		}else{
			isLockElem.children[0].classList.remove('layer_is-meta-disable');
			isLockElem.children[1].classList.remove('layer_is-meta-disable');
		}

		isLevelSpacerElem.style.width = 18 * level;
		isChildsOpenElem.children[0].style.display = layerItem.isChildView ? 'block' : 'none';
		isChildsOpenElem.children[1].style.display = (! layerItem.isChildView) ? 'block' : 'none';
		isChildsOpenElem.style.visibility = ('Layer' === layerItem.kind) ? 'visible' : 'hidden';
		layerNameElem.textContent = layerItem.name;
	};

	let layerTreeElem = document.getElementById('layer-tree');
	// 余るようなら消す
	while(layerTreeElem.children.length > hist.now().items.length){
		layerTreeElem.children[layerTreeElem.children.length - 1].remove();
	}
	// 足りないようなら足す
	while(layerTreeElem.children.length < hist.now().items.length){
		layerTreeElem.appendChild(newLayerItem());
	}

	const focusKindFromItemIndex = (itemIndex) => {
		if(! hist.now().focus.focusItemIndexes.includes(itemIndex)){
			return undefined;
		}
		if(itemIndex === hist.now().focus.focusItemIndexes.at(-1)){
			return 'current-focus-item';
		}
		return 'focus-item';
	};
	let i = 0;
	PV.exforReverse(hist.now().items, (itemIndex, item) => {
		const focusKind = focusKindFromItemIndex(itemIndex);
		updateLayerItem(layerTreeElem.children[i], item, 0, focusKind);
		i++;
		return true;
	});
}

function getItemAndIndexFromName(name){
	const items = hist.now().items;
	for(let i = 0; i < items.length; i++){
		const item = items.at(i);
		if(name === item.name){
			return [item, i];
		}
	}

	return [undefined, -1];
}
function getItemFromName(name){
	const r = getItemAndIndexFromName(name);
	return r[0];
}

function checkpointStacking(cause, isDup) {
	if(isDisableHistoryUpdate){
		console.log('checkpointStacking isDisableHistoryUpdate');
		return;
	}
	// mouse,tool操作については、差分があれば更新という仕組みで手抜きする。
	const now = History.removeCache(deepcopy(hist.now()));
	if (!lodash.isEqual(now, hist.prev())) {
		console.log('document changed.')
		//console.log(hist.now());
		//console.log(diff(hist.prev(), now));
		editor.isHistoryStacked = true;
		if(isDup){
			hist.stackingCheckDup(cause);
		}else{
			hist.stacking(cause);
		}
	}
}

function nearPointPosi(items, itemIndexes, point){
	let winnerPosi = undefined;
	itemIndexes.forEach(itemIndex => {
		const item = items.at(itemIndex);
		if(! item){
			console.error('BUG', itemIndex, items);
			return;
		}
		if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
			return;
		}
		if('Bezier' !== item.kind){
			return;
		}

		for(let i = 0; i < item.aps.length; i++){
			const ap = Item_apAtRoundable(item, i);
			const nextAp = Item_apAtRoundable(item, i+1);
			if(! nextAp){
				return;
			}
			let posi = PV.Bezier_nearPosiInBezierBetweenFromApPair(ap, nextAp, point);
			posi['itemIndex'] = itemIndex;
			posi['between'] = [i, i+1];	// roundしたindexになる場合があるが、それで良しとする。		
			if(! winnerPosi){
				winnerPosi = posi;
			}else{
				if(posi.diagonalLength < winnerPosi.diagonalLength){
					winnerPosi = posi;
				}
			}
		}
	});
	return winnerPosi;
}

// Item,APの末尾からの順に並び替える
function APCplx_sortToDesc(cplxes_){
	let cplxes = deepcopy(cplxes_).sort((a,b) => {
		if (a[0] < b[0]) return 1;
		if (a[0] > b[0]) return -1;
		if (a[1] < b[1]) return 1;
		if (a[1] > b[1]) return -1;
		return 0;
	});
	return cplxes;
}

function deleteByFocus(){
	// SPEC Delete キーを押下された場合、ItemまたはAPを削除する(TOOL選択状態による)。
	if(Tool.getToolFromIndex(editor.toolIndex).isItem){
		hist.now().focus.focusItemIndexes.sort((a,b) => {return a-b;}).reverse().forEach(itemIndex => {
			console.log('delete', itemIndex);
			hist.now().items.splice(itemIndex, 1);
		});
		Focus_clear();
		return;
	}

	const cplxes = APCplx_sortToDesc(hist.now().focus.focusAPCplxes);
	
	const removeAPFromCplx_ = (cplx) => {
		const itemIndex = cplx[0];
		const apIndex = cplx[1];
	
		// SPEC AP削除ではAPを中抜きするだけでBezierを切断しない。
		let item = hist.now().items.at(itemIndex);
		if(! item){
			console.error('BUG');
			return;
		}
	
		item.aps.splice(apIndex, 1);
		if(2 >= item.aps.length){ // SPEC APを失って2つ以下になったBezierはパスを開く
			item.isCloseAp = false;
		}
		if(0 === item.aps.length){
			// SPEC APを全て失ったBezierは削除する
			hist.now().items.splice(itemIndex, 1);
			console.log('delete Item in removeAP', itemIndex);
			return false;
		}
		// Focus_removeAP(itemIndex, apIndex); // 呼び出し元でまとめてやる
		return true;
	}
	
	cplxes.forEach(cplx => {
		console.log('delete', cplx);
		if(! removeAPFromCplx_(cplx)){
			// 全てのAPを失ったItemは削除されたのでFocusも外す
			hist.now().focus.focusItemIndexes
				= hist.now().focus.focusItemIndexes.filter(e => e !== cplx[0]);
		}
	});
	hist.now().focus.focusAPCplxes = [];
}

function Item_apAtRoundable(item, apIndex){
	// 閉パスでindexが+1方向にはみ出す場合に、先頭APを返す。
	const ap = item.aps.at(apIndex);
	if(ap){
		return ap;
	}
	if(item.isCloseAp && item.aps.length === apIndex){
		return item.aps.at(0);
	}
	return undefined;
}
function Item_apTipKind(item, apIndex){
	if('Bezier' !== item.kind){
		return undefined;
	}
	if(item.isCloseAp){
		return undefined;
	}
	if(0 === apIndex){
		return 'head';
	}
	if(apIndex === (item.aps.length - 1)){
		return 'tail';
	}
	return undefined;
}

function joinItem(itemIndex0, apIndex0, itemIndex1, apIndex1){
	if((-1 === itemIndex0) || (-1 === itemIndex1) || (itemIndex0 === itemIndex1)){
		console.log('BUG', itemIndex0, itemIndex1);
		return false;
	}
	let item0 = hist.now().items.at(itemIndex0);
	const item1 = hist.now().items.at(itemIndex1);
	if((! item0) || (! item1)){
		console.log('BUG', itemIndex0, itemIndex1);
		return false;
	}

	if(item0.isCloseAp || item1.isCloseAp){
		console.log('not open path', itemIndex0, itemIndex1, item0, item1);
		return false;
	}

	const tip0 = Item_apTipKind(item0, apIndex0);
	const tip1 = Item_apTipKind(item1, apIndex1);
	if((! tip0) || (! tip1)){
		console.log('not tipAp', itemIndex0, itemIndex1, apIndex0, apIndex1);
		return false;
	}

	const reverseAps = (aps) => {
		aps = aps.reverse();
		aps.forEach(ap => {
			switch(ap.handle.kind){
			case AHKIND.Symmetry:
				ap.handle.frontVector = PV.pointMul(ap.handle.frontVector, PV.MinusPoint());
				break;
			case AHKIND.Free:
				const vec = ap.handle.frontVector;
				ap.handle.frontVector = ap.handle.backVector;
				ap.handle.backVector = vec;
			}
		});
		return aps;
	};
	let newApIndex;
	if('tail' === tip0){
		const aps = ('head' === tip1) ? item1.aps : reverseAps(item1.aps);
		newApIndex = item0.aps.length;
		item0.aps.push(...aps);
	}else{
		const aps = ('tail' === tip1) ? item1.aps : reverseAps(item1.aps);
		newApIndex = aps.length;
		item0.aps.unshift(...aps);
	}

	hist.now().items.splice(itemIndex1, 1);

	// Item削除によるIndex変更に合わせてFocusを再構築する
	Focus_restructByFocusRemovedItem_(itemIndex1)
	const newItemIndex0 = (itemIndex0 < itemIndex1) ? itemIndex0 : (itemIndex0 - 1);
	Focus_addAP(newItemIndex0, newApIndex);

	console.log('joined Item', newItemIndex0, newApIndex);
	return true;
}

function getTouchedItem(items, point){
	let touchedItemIndex = -1;
	PV.exforReverse(items, (index, item) => {
		if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
			return true;
		}

		let rect = PV.rectFromRange2d(PV.Item_range2d(item));
		// SPEC Itemサイズが小さい場合は当たり判定を広げる。
		if (rect.w < toCanvas(20) || rect.h < toCanvas(20)) {
			// サイズゼロでない場合、指定値より大きい当たり判定になるわけだが、恐らく問題にならないので良しとする。
			rect = PV.expandRect(rect, toCanvas(20));
		}
		if (PV.isTouchPointInRect(rect, point)) {
			console.log('touched Item', index);
			touchedItemIndex = index;
			return false;
		}
		return true;
	});

	return touchedItemIndex;
}

function getTouchedAp(items, mousePoint){
	let res = [-1, -1];
	const isTouched = !PV.exforReverse(items, (itemIndex, item) => {
		if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
			return true;
		}

		switch(item.kind){
		case 'Bezier':
			break;
		case 'Guide':{
			if(PV.isTouchPointAndPoint(mousePoint, item.point, toCanvas(field.touch.pointR))){
				res[0] = itemIndex;
				res[1] = 0;
				return false;
			}
		}
			return true;
		default:
			return true;
		}

		let isTouchedAP = !PV.exforReverse(item.aps, (apIndex, ap) => {
			if(PV.isTouchPointAndPoint(mousePoint, ap.point, toCanvas(field.touch.pointR))){
				console.log('touched AP', itemIndex, apIndex);
				res[0] = itemIndex;
				res[1] = apIndex;
				return false;
			}
			return true;
		});
		if(isTouchedAP){
			return false;
		}
		return true;
	});

	return res;
}

function getTouchedApAh(items, enableItemIndexes, mousePoint){
	let res = [-1, -1, undefined];
	PV.exforReverse(items, (itemIndex, item) => {
		if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
			return true;
		}

		// MEMO itemIndexを得るために、focusedItemから探す場合もdocのitemsの配列を使う必要がある。
		if(undefined !== enableItemIndexes){
			if(!enableItemIndexes.includes(itemIndex)){
				return true;
			}
		}
		
		switch(item.kind){
		case 'Bezier':
			break;
		case 'Guide':{
			if(PV.isTouchPointAndPoint(mousePoint, item.point, toCanvas(field.touch.pointR))){
				res[0] = itemIndex;
				res[1] = 0;
				res[2] = 'ap';
				return false;
			}
		}
			return true;
		default:
			return true;
		}

		let isTouchedAP = !PV.exforReverse(item.aps, (apIndex, ap) => {
			const frontAHPoint = PV.AP_frontAHPoint(ap);
			const backAHPoint = PV.AP_backAHPoint(ap);
			if(AHKIND.None !== ap.handle.kind){
				if(PV.isTouchPointAndPoint(mousePoint, frontAHPoint, toCanvas(field.touch.pointR))){
					//console.log('touched frontAH', itemIndex, apIndex);
					res[0] = itemIndex;
					res[1] = apIndex;
					res[2] = 'front';
					return false;
				}
				if(PV.isTouchPointAndPoint(mousePoint, backAHPoint, toCanvas(field.touch.pointR))){
					//console.log('touched backAH', itemIndex, apIndex);
					res[0] = itemIndex;
					res[1] = apIndex;
					res[2] = 'back';
					return false;
				}	
			}
			if(PV.isTouchPointAndPoint(mousePoint, ap.point, toCanvas(field.touch.pointR))){
				//console.log('touched AP', itemIndex, apIndex);
				res[0] = itemIndex;
				res[1] = apIndex;
				res[2] = 'ap';
				return false;
			}
			return true;
		});
		if(isTouchedAP){
			return false;
		}
		return true;
	});

	return res;
}

function insertAp(){
	const posi = nearPointPosi(hist.now().items, hist.now().focus.focusItemIndexes, mousemoveInCanvas);
	if(! posi){
		console.log('posi not detected');
		return undefined;
	}
	let item = hist.now().items.at(posi.itemIndex);
	if(! item){
		console.error('BUG')
		return undefined;
	}

	// AH調整
	let preAp = Item_apAtRoundable(item, posi.between[0]);
	let postAp = Item_apAtRoundable(item, posi.between[1]);
	if(! preAp || !postAp){
		console.error('BUG')
		return undefined;
	}
	const splited = PV.splitBezierBetween(posi.t, preAp, postAp);
	if(! splited){
		console.error('BUG')
		return undefined;
	}
	preAp.handle = splited.preApHandle;
	postAp.handle = splited.postApHandle;
	// AP追加
	let newAp = PV.AP_newFromPoint(posi.point);
	newAp.handle = splited.centerHandle;
	item.aps.splice(posi.between[1], 0, newAp);
	// MEMO apIndexがズレるのを、他のAPからfocusを外すことで誤魔化している。
	hist.now().focus.focusAPCplxes = []
	hist.now().focus.focusAPCplxes.push([posi.itemIndex, posi.between[1]]);

	return [posi.itemIndex, posi.between[1]];
}

function moveFocusItems(targetKind, vec){
	let apCplxes = [];
	switch(targetKind){
	case 'Item':
		hist.now().focus.focusItemIndexes.forEach(itemIndex => {
			Item_moveWithIndex(itemIndex, vec);
		})
		break;
	case 'AP':
		apCplxes = hist.now().focus.focusAPCplxes;
		break;
	default:
		console.error('BUG', targetKind);
		return;
	}

	apCplxes.forEach(([itemIndex, apIndex]) => {
		let ap = hist.now().items.at(itemIndex).aps.at(apIndex);
		ap.point = PV.pointAdd(ap.point, vec);
	});
}

function resize(targetKind, event){
	// resizeの場合、move, rotateのようにはいかない。
	// resizeハンドルによる方向の選択とctrlKeyによる起点の変化があるため。

	const focusRect = getFocusingRectByPrev();
	if(! focusRect){
		console.error('BUG');
		return;
	}
	const corners = PV.cornerPointsFromRect(focusRect);
	const centers = PV.sideCentersFromRect(focusRect);
	let editCenter; // 拡縮の中央はresizeハンドルの対向またはctrlKeyがあれば中央
	let srcPoint; // 操作の起点はresizeハンドル
	switch(editor.mouse.mode){
	case 'resize_lefttop':
		editCenter = corners[2];
		srcPoint = corners[0];
		break;
	case 'resize_righttop':
		editCenter = corners[3];
		srcPoint = corners[1];
		break;
	case 'resize_rightbottom':
		editCenter = corners[0];
		srcPoint = corners[2];
		break;
	case 'resize_leftbottom':
		editCenter = corners[1];
		srcPoint = corners[3];
		break;
	case 'resize_top':
		editCenter = centers[2];
		srcPoint = centers[0];
		break;
	case 'resize_right':
		editCenter = centers[3];
		srcPoint = centers[1];
		break;
	case 'resize_bottom':
		editCenter = centers[0];
		srcPoint = centers[2];
		break;
	case 'resize_left':
		editCenter = centers[1];
		srcPoint = centers[3];
		break;
	default:
		console.error('BUG', mode);
		return;
	}
	if(event.ctrlKey){
		editCenter = PV.centerFromRect(focusRect);
	}
	editor.mouse.editCenter = deepcopy(editCenter);

	// resizeハンドル点とマウスのpointはずれているはずなので、
	// 差分をキャンセルしてresizeの矩形の変形を計算する。
	const diff = PV.pointSub(mousedownInCanvas, srcPoint);
	const dstPoint = PV.pointSub(mousemoveInCanvas, diff);
	const srcVec = PV.pointSub(srcPoint, editCenter);
	const dstVec = PV.pointSub(dstPoint, editCenter);
	// editCenter---srcVec間のスケールをこれで算出。
	let scale2d = PV.pointDiv(dstVec, srcVec);

	// SPEC shiftKeyされていたらスケール変化の縦横比を固定
	if(event.shiftKey){
		const v = Math.max(Math.abs(scale2d.x), Math.abs(scale2d.y));
		scale2d.x = ((0 > scale2d.x) ? -1:1) * v;
		scale2d.y = ((0 > scale2d.y) ? -1:1) * v;
	}else{
		switch(editor.mouse.mode){
			case 'resize_lefttop':
			case 'resize_righttop':
			case 'resize_rightbottom':
			case 'resize_leftbottom':
				break;
			case 'resize_top':
			case 'resize_bottom':
				scale2d.x = 1;
				break;
			case 'resize_right':
			case 'resize_left':
				scale2d.y = 1;
				break;
			default:
				console.error('BUG', mode);
				return;
			}
	}

	resizeFocusItems(targetKind, scale2d, editCenter);
	renderingAll();
}

function resizeFocusItems(targetKind, scale2d, editCenter){
	const convPoint = (point, editCenter, resize) => {
		// resizeはeditCenterからの相対座標で行う
		const prevVec = PV.pointSub(point, editCenter);
		const padd = PV.pointMul(prevVec, resize);
		return PV.pointAdd(editCenter, padd);
	};

	let apCplxes = [];
	switch(targetKind){
	case 'Item':
		hist.now().focus.focusItemIndexes.forEach(itemIndex => {
			const item = hist.now().items.at(itemIndex);
			if(! item){
				console.error('BUG', itemIndex);
			}

			if(hist.now().editor.resize.isResizeStrokeWidth){
				const scale1d = (Math.abs(scale2d.x) + Math.abs(scale2d.y)) / 2; // SPEC: リサイズの縦横比が異なる場合に中間値を取る
				item.border.width = item.cache.prevItems[0].border.width * scale1d;
			}

			switch(item.kind){
			case 'Bezier':{
				for(let apIndex = 0; apIndex < item.aps.length; apIndex++){
					apCplxes.push([itemIndex, apIndex]);
				}
			}
				break;
			case 'Figure':{
				const prevItem = item.cache.prevItems[0];
				item.point = convPoint(prevItem.point, editCenter, scale2d);
				item.rsize = PV.pointMul(prevItem.rsize, scale2d);
			}
				break;
			case 'Guide':{
				const prevItem = item.cache.prevItems[0];
				item.point = convPoint(prevItem.point, editCenter, scale2d);
			}
				break;	
			default:
				console.error('BUG', item);
			}
		})
		break;
	case 'AP':
		apCplxes = hist.now().focus.focusAPCplxes;
		break;
	default:
		console.error('BUG', targetKind);
		return;
	}
	apCplxes.forEach(apCplx => {
		const itemIndex = apCplx[0];
		const apIndex = apCplx[1];
		let item = hist.now().items.at(itemIndex); 
		let nowAp = item.aps.at(apIndex);
		let prevAp = item.cache.prevItems[0].aps.at(apIndex);
		nowAp.point = convPoint(prevAp.point, editCenter, scale2d);
		nowAp.handle.frontVector = PV.pointMul(prevAp.handle.frontVector, scale2d);
		nowAp.handle.backVector = PV.pointMul(prevAp.handle.backVector, scale2d);
	});
}

function updateAxisInputState(){
	const isFocusItemEmpty = (0 === hist.now().focus.focusItemIndexes.length);
	const tool = Tool.getToolFromIndex(editor.toolIndex);

	const axises = [
		document.getElementById('axis-x'),
		document.getElementById('axis-y'),
		document.getElementById('axis-w'),
		document.getElementById('axis-h'),
	];
	axises.forEach((axis, ix) => {
		axis.children[0].disabled = isFocusItemEmpty;
		axis.children[1].disabled = isFocusItemEmpty;
		if(2 <= ix){ // w,h はItem操作のToolでなければDisableにする
			if(! tool.isItem){
				axis.children[0].disabled = true;
				axis.children[1].disabled = true;
			}
		}
	});
}
function updateAxisInput(){
	updateAxisInputState();
	const focusRect = Render.getFocusingRectByNow(editor, hist.now());
	if(! focusRect){
		document.getElementById('axis-x').children[0].value = '';
		document.getElementById('axis-y').children[0].value = '';
		document.getElementById('axis-w').children[0].value = '';
		document.getElementById('axis-h').children[0].value = '';
		return;
	}
	document.getElementById('axis-x').children[0].value = focusRect.x.toFixed(1);
	document.getElementById('axis-x').children[1].value = focusRect.x.toFixed(1);
	document.getElementById('axis-y').children[0].value = focusRect.y.toFixed(1);
	document.getElementById('axis-y').children[1].value = focusRect.y.toFixed(1);
	document.getElementById('axis-w').children[0].value = focusRect.w.toFixed(1);
	document.getElementById('axis-w').children[1].value = focusRect.w.toFixed(1);
	document.getElementById('axis-h').children[0].value = focusRect.h.toFixed(1);
	document.getElementById('axis-h').children[1].value = focusRect.h.toFixed(1);
}

function getFocusingRectByPrev(){
	if(Tool.getToolFromIndex(editor.toolIndex).isItem){
		let cacheItems = [];
		getNowFocusItems().forEach(item => {cacheItems.push(...item.cache.prevItems)});
		return PV.rectFromItems(cacheItems);
	}

	let cacheItems = [];
	hist.now().items.forEach(item => {cacheItems.push(item.cache.prevItems[0])});
	const aps = PV.apsFromItems(cacheItems, hist.now().focus.focusAPCplxes);
	return PV.rectFromAps(aps);
}

// Focus先頭のアイテム(内部表現的には末尾だが...)が変更された(かもしれない)場合に呼び出す
// UI上の他の更新はここで行うが、再描画(renderingEditorUserInterface)はしないので上位で呼び出すこと！
function checkFocusCurrentTopItemChangedCallback() {
	updateAxisInput();
	updateLayerView();
	if((!! mousedownInCanvas) && (! editor.mouse.isMoveLock)){
		editor.mouse.isLatedFocusChangeInMousemoveCallbacked_ = true;
	}else{
		// 対象となる遅い処理は現在のところ無い
	}

	document.getElementById('figure-setting').style.display = 'none';
	document.getElementById('guide-setting').style.display = 'none';

	const isFocusItemEmpty = (0 === hist.now().focus.focusItemIndexes.length);
	if (isFocusItemEmpty) {
		return;
	}

	const index = hist.now().focus.focusItemIndexes.at(-1)
	const item = hist.now().items[index];
	if(!! item.colorPair){
		palette.colorPair = deepcopy(item.colorPair);
		setPaletteUIForMem();	
	}
	if(!! item.border){
		border = deepcopy(item.border);
		setBorderUIFromMem();	
	}
	switch(item.kind){
	case 'Figure':{
		if((1 === hist.now().focus.focusItemIndexes.length)){
			document.getElementById('figure-setting').style.display = 'block';
			document.getElementById('figure-kind').value = item.figureKind;
			document.getElementById('figure-corner-num').value = item.cornerNum;
			document.getElementById('figure-starry-corner-step-num').value = item.starryCornerStepNum;
			document.getElementById('figure-starry-corner-pit-r-parcent').value = item.starryCornerPitRPercent;
			switch(item.figureKind){
			case 'Polygon':{
				document.getElementById('figure-polygon-setting').style.display = 'block';
			}
				break;
			default:{
				document.getElementById('figure-polygon-setting').style.display = 'none';
			}
			}
		}	
	}
		break;
	case 'Guide':{
		if((1 === hist.now().focus.focusItemIndexes.length)){
			document.getElementById('guide-setting').style.display = 'block';
			document.getElementById('guide-axis').value = item.axis;
		}
	}
		break;
	default: // NOP
		break;
	}
}

function Item_splitByAp(itemIndex, apIndex){
	let item = hist.now().items.at(itemIndex);
	if(! item){
		console.error('BUG', itemIndex, apIndex, item);
		return;
	}

	// ** 閉パスの場合、splitしたAPでパスを開く
	if(item.isCloseAp){
		// 開いた際に先頭APと末尾APとして重複するAPを複製
		const newAp = deepcopy(item.aps.at(apIndex));
		if(! newAp){
			console.error('BUG', itemIndex, apIndex, item);
			return;	
		}
		const tops = item.aps.splice(0, apIndex);
		item.aps.push(...tops);
		item.aps.push(newAp);
		item.isCloseAp = false;
		// ** indexが崩れたのでFocusを書き換え（リセットして誤魔化す）
		hist.now().focus.focusAPCplxes = [];
		Focus_addItemIndex(itemIndex);
		hist.now().focus.focusItemIndexes = [itemIndex];
		checkFocusCurrentTopItemChangedCallback();
		return;
	}

	// ** 開パスの場合、本機能はsplitなので、先頭末尾のAPは操作しない
	if((apIndex === 0) || (apIndex === (item.aps.length - 1))){
		console.error('BUG', itemIndex, apIndex, item);
		return;
	}

	// ** Itemをcopyしてそれぞれapを削ってからdocumentに挿入することで、APによる分割を実現
	let newItem = Item_copy(item);
	item.aps.length = apIndex + 1;
	newItem.aps.splice(0, apIndex);
	hist.now().items.splice(itemIndex, 0, newItem);

	// ** indexが崩れたのでFocusを書き換え（リセットして誤魔化す）
	hist.now().focus.focusAPCplxes = [];
	hist.now().focus.focusItemIndexes = [itemIndex, itemIndex + 1];
	checkFocusCurrentTopItemChangedCallback();
}

function Item_copy(item){
	return deepcopy(item);
}

function Item_moveWithIndex(itemIndex, move){
	let item = hist.now().items.at(itemIndex);
	const prevItem = item.cache.prevItems[0];
	if(undefined === item || undefined === prevItem){
		console.error('BUG', itemIndex, item, prevItem);
		return;
	}
	switch(item.kind){
	case 'Bezier':{
		if(item.aps.length !== prevItem.aps.length){
			console.error('BUG', itemIndex, item, prevItem);
			return;
		}
		item.aps.forEach((ap, ix) => {
			const pap = prevItem.aps.at(ix);
			ap.point.x = pap.point.x + move.x;
			ap.point.y = pap.point.y + move.y;
		});
	}
		break;
	case 'Figure':
	case 'Guide':{
		item.point = PV.pointAdd(prevItem.point, move);
	}
		break;
	default:
		console.error('BUG', item);
	}
}
function Item_moveNowWithIndex(itemIndex, move){
	let item = hist.now().items.at(itemIndex);
	if(undefined === item){
		console.error('BUG', itemIndex);
		return;
	}
	Item_moveNow(item, move);
}
function Item_moveNow(item, move){
	switch(item.kind){
		case 'Bezier':{
		item.aps.forEach((ap) => {
			ap.point = PV.pointAdd(ap.point, move);
		});
	}
		break;
	case 'Figure':
	case 'Guide':{
		item.point = PV.pointAdd(item.point, move);
	}
		break;
	default:
		console.error('BUG', item);
	}
}

function Item_rotateWithIndex(itemIndex, degree, center){
	let item = hist.now().items.at(itemIndex);
	const prevItem = item.cache.prevItems[0];
	if(undefined === item || undefined === prevItem){
		console.error('BUG', itemIndex, item, prevItem);
		return;
	}
	switch(item.kind){
	case 'Bezier':{
		if(item.aps.length !== prevItem.aps.length){
			console.error('BUG', itemIndex, item, prevItem);
			return;
		}
		item.aps.forEach((newAp, ix) => {
			const prevAp = prevItem.aps.at(ix);
			newAp.point = PV.pointRotationByCenter(prevAp.point, degree, center);
			newAp.handle.frontVector = PV.vectorRotate(prevAp.handle.frontVector, degree);
			newAp.handle.backVector = PV.vectorRotate(prevAp.handle.backVector, degree);
		});	
	}
		break;
	case 'Figure':{
		// TODO rotateの際に縦横比が違っていたらBezierに変換しないと回転できない
		item.point = PV.pointRotationByCenter(prevItem.point, degree, center);
		item.rotate = prevItem.rotate + degree;
	}
		break;
	case 'Guide':{
		// NOP
	}
	break;
	default:
		console.error('BUG', item);
	}
}

function AP_moveWithIndex(itemIndex, apIndex, move){
	let item = hist.now().items.at(itemIndex);
	const prevItem = item.cache.prevItems[0];
	if(undefined === item || undefined === prevItem){
		console.error('BUG', itemIndex, item, prevItem);
		return;
	}
	switch(item.kind){
	case 'Guide':
		item.point = PV.pointAdd(prevItem.point, move);
		break;
	default:{
		let ap = item.aps.at(apIndex);
		const pap = prevItem.aps.at(apIndex);
		if(undefined === ap || undefined === pap){
			console.error('BUG', itemIndex, apIndex, item, prevItem);
			return;
		}
		ap.point = PV.pointAdd(pap.point, move);
	}
		break;
	}
}
function AP_moveNowWithIndex(itemIndex, apIndex, move){
	let item = hist.now().items.at(itemIndex);
	if(undefined === item){
		console.error('BUG', itemIndex);
		return;
	}
	let ap = item.aps.at(apIndex);
	if(undefined === ap){
		console.error('BUG', itemIndex, apIndex, item);
		return;
	}
	ap.point = PV.pointAdd(ap.point, move);
}

function AH_move(itemIndex, apIndex, apAhKind, vec, isToFree){
	let item = hist.now().items.at(itemIndex);
	let nowAp = item.aps.at(apIndex);
	if (! nowAp) {
		console.error('BUG');
		return;
	}
	const prevAp = item.cache.prevItems[0].aps.at(apIndex);
	if (! prevAp) {
		console.error('BUG');
		return;
	}

	switch(nowAp.handle.kind){
	case AHKIND.None:
		nowAp.handle.kind = AHKIND.Symmetry;
		nowAp.handle.frontVector = vec;
		break;
	case AHKIND.Symmetry:
		switch(apAhKind){
		case 'ap':
			nowAp.handle.frontVector = vec;
			break;
		case 'front':{
			if(isToFree){
				nowAp.handle.kind = AHKIND.Free;
				nowAp.handle.frontVector = PV.pointAdd(prevAp.handle.frontVector, vec);
				nowAp.handle.backVector = PV.AP_backVector(prevAp)	
			}else{
				nowAp.handle.frontVector = PV.pointAdd(prevAp.handle.frontVector, vec);
			}
		}
			break;
		case 'back':{
			if(isToFree){
				nowAp.handle.kind = AHKIND.Free;
				nowAp.handle.backVector = PV.pointAdd(PV.AP_backVector(prevAp), vec);
			}else{
				nowAp.handle.frontVector = PV.pointAdd(prevAp.handle.frontVector, PV.pointMul(vec, PV.MinusPoint()));
			}
		}
			break;
		default:
			console.error('bug', apAhKind)
			break;
		}
	case AHKIND.Free:
		switch(apAhKind){
		case 'ap':
			nowAp.handle.kind = AHKIND.Symmetry;
			nowAp.handle.frontVector = vec;
			nowAp.handle.backVector = PV.ZeroPoint();
			break;
		case 'front':
			nowAp.handle.frontVector = PV.pointAdd(prevAp.handle.frontVector, vec);
			break;
		case 'back':
			nowAp.handle.backVector = PV.pointAdd(PV.AP_backVector(prevAp), vec);
			break;
		default:
			console.error('bug', apAhKind)
			break;
		}
	default:
		break;
	}
}

function Focus_clear(){
	hist.now().focus.focusItemIndexes = [];
	hist.now().focus.focusAPCplxes = [];
	checkFocusCurrentTopItemChangedCallback();
}
// 追加アイテムは（すでにfocus済みであっても）常にcurrentItemとして並び替える。
// 並び替えたくない場合は呼び出し側でチェックすること。
function Focus_addItemIndex(itemIndex) {
	const isAddNow = Focus_addItemIndexOnly_(itemIndex);
	const isExistInAP = Focus_isExistInAPItemIndex_(itemIndex)
	if(isAddNow && (! isExistInAP)){
		// 新規追加かつAPがすべて未Focusの場合、すべてのAPをFocusする。
		// 順番は末尾APがcurrentAPになるよう正順。
		const item = hist.now().items[itemIndex];
		if((! Render.isMetaVisible(item, hist.now())) || Render.isMetaLock(item, hist.now())){
			console.log('BUG', itemIndex, item);
		}
		if('Bezier' === item.kind){
			for(let apIndex = 0; apIndex < item.aps.length; apIndex++){
				hist.now().focus.focusAPCplxes.push([itemIndex, apIndex]);	
		}
		}
	}
	checkFocusCurrentTopItemChangedCallback();
	return isAddNow;
}

function Focus_addAllApFromItemIndex(itemIndex){
	const item = hist.now().items.at(itemIndex);
	if('Bezier' !== item.kind){
		return;
	}

	item.aps.forEach((ap, apIndex) => {
		Focus_addAP(itemIndex, apIndex);
	});
}

// ItemのFocusのみ操作、AP側Focusには触らない。
function Focus_addItemIndexOnly_(index) {
	if (-1 === index) {
		console.error('BUG');
		return false;
	}
	const ix = hist.now().focus.focusItemIndexes.indexOf(index);
	if (-1 !== ix) {
		hist.now().focus.focusItemIndexes.splice(ix, 1);
	}
	hist.now().focus.focusItemIndexes.push(index);
	console.log('Focus_addItemIndex', index, typeof index, ix, hist.now().focus.focusItemIndexes);
	return (-1 === ix);
}

function Focus_removeItemIndex(itemIndex) {
	console.log('Focus_removeItemIndex', itemIndex)
	const ix = hist.now().focus.focusItemIndexes.indexOf(itemIndex);
	if (-1 === ix) {
		console.log('Focus_removeItemIndex not focused', itemIndex);
		return;
	}
	hist.now().focus.focusItemIndexes.splice(ix, 1);
	hist.now().focus.focusAPCplxes
		= hist.now().focus.focusAPCplxes.filter(e => {return e[0] !== itemIndex});
	checkFocusCurrentTopItemChangedCallback();
}

function Focus_restructByFocusRemovedItem_(removedItemIndex){
	hist.now().focus.focusItemIndexes
	= hist.now().focus.focusItemIndexes.filter(e => {return e !== removedItemIndex});
hist.now().focus.focusAPCplxes
	= hist.now().focus.focusAPCplxes.filter(e => {return e[0] !== removedItemIndex});
hist.now().focus.focusItemIndexes
	= hist.now().focus.focusItemIndexes.map(e => {return (e < removedItemIndex) ? e : (e - 1)});
hist.now().focus.focusAPCplxes
	= hist.now().focus.focusAPCplxes.map(e => {return (e[0] < removedItemIndex) ? e : [(e[0] - 1), e[1]]});
}

// あくまで採番変更の反映のみ行うUtility.
// itemsから取り除かれたitemはFocusできないはずだが、そのフォーカス外しは呼び出し元で行う。
function renumberingItemsByEjectedItemIndex_(deletedItemIndex){
hist.now().focus.focusItemIndexes
	= hist.now().focus.focusItemIndexes.map(e => {return (e > deletedItemIndex) ? (e - 1):e});
hist.now().focus.focusAPCplxes
	= hist.now().focus.focusAPCplxes.map(e => {return (e[0] > deletedItemIndex) ? [(e[0] - 1), e[1]] : e});
}
function renumberingItemsByInsertedItemIndex_(insertedItemIndex){
	hist.now().focus.focusItemIndexes
		= hist.now().focus.focusItemIndexes.map(e => {return (e > insertedItemIndex) ? (e + 1) : e});
	hist.now().focus.focusAPCplxes
		= hist.now().focus.focusAPCplxes.map(e => {return (e[0] > insertedItemIndex) ? [(e[0] + 1), e[1]] : e});
}
	
function renumberingItemsByMovedItemIndex_(oldItemIndex, newItemIndex){
	hist.now().focus.focusItemIndexes
	= hist.now().focus.focusItemIndexes.map(e => {return (e === oldItemIndex) ? NaN:e});
hist.now().focus.focusAPCplxes
	= hist.now().focus.focusAPCplxes.map(e => {return (e[0] === oldItemIndex) ? [NaN, e[1]]:e});
	renumberingItemsByEjectedItemIndex_(oldItemIndex);
	renumberingItemsByInsertedItemIndex_(newItemIndex);
	hist.now().focus.focusItemIndexes
	= hist.now().focus.focusItemIndexes.map(e => {return isNaN(e) ? newItemIndex:e});
hist.now().focus.focusAPCplxes
	= hist.now().focus.focusAPCplxes.map(e => {return isNaN(e[0]) ? [newItemIndex, e[1]]:e});
}

function Focus_filterItemIndexes(itemIndexes){
	const r = hist.now().focus.focusItemIndexes.filter((index) => { return itemIndexes.includes(index); });
	hist.now().focus.focusItemIndexes = r;
	//console.log('d', r, touchedIndexes);
	hist.now().focus.focusAPCplxes
		= hist.now().focus.focusAPCplxes.filter((e) => {return itemIndexes.includes(e[0])});
	// currentAPの属するItemをcurrentItemへ
	const cplx = hist.now().focus.focusAPCplxes.at(-1);
	if (undefined !== cplx) {
		Focus_addItemIndex(cplx[0]);
	}
	checkFocusCurrentTopItemChangedCallback();	
}

function Focus_filterAP(touchedCplxes){
	let itemIndexes = [];
	let newCplx = [];
	hist.now().focus.focusAPCplxes.forEach((oldCplx) => {
		const isExist = !PV.exfor(touchedCplxes, (nIx, touchCplx) => {
			if(oldCplx[0] === touchCplx[0] && oldCplx[1] === touchCplx[1]){
				itemIndexes.push(touchCplx[0]);
				newCplx.push(touchCplx);
				return false;
			}
			return true;
		});
		if(! isExist){ console.log('remove AP', oldCplx)}
	});
	hist.now().focus.focusAPCplxes = newCplx;
	// currentAPの属するItemをcurrentItemへ
	const cplx = hist.now().focus.focusAPCplxes.at(-1);
	if (undefined !== cplx) {
		Focus_addItemIndex(cplx[0]);
	}
	//console.log('d', r, touchedIndexes);
	Focus_filterItemIndexes(itemIndexes);
}

function Focus_setCurrentAP(itemIndex, apIndex) {
	if (-1 === itemIndex || -1 === apIndex) {
		console.error('BUG', itemIndex, apIndex);
		return;
	}
	const isAddNow = (-1 !== Focus_apIxFromAPIndexCplx(itemIndex, apIndex));

	hist.now().focus.focusItemIndexes = [];
	hist.now().focus.focusAPCplxes = [];
	hist.now().focus.focusItemIndexes.push(itemIndex);
	hist.now().focus.focusAPCplxes.push([itemIndex, apIndex]);
	console.log('Focus_setCurrentAP', itemIndex, apIndex, typeof apIndex);
	checkFocusCurrentTopItemChangedCallback();
	return isAddNow;
}

function Focus_apIxFromAPIndexCplx(itemIndex, apIndex){
	let apIx = -1;
	hist.now().focus.focusAPCplxes.forEach((cplx, ix) => {
		if(cplx[0] === itemIndex && cplx[1] === apIndex){
			apIx = ix;
		}
	});
	return apIx;
}

function Focus_isExistInAPItemIndex_(itemIndex){
	return hist.now().focus.focusAPCplxes.some((cplx) => {
		return cplx[0] === itemIndex;
	});
}

function Focus_currerntItem() {
	const ix = hist.now().focus.focusItemIndexes.at(-1);
	return hist.now().items[ix];
}
function Focus_currentFirstAp() {
	let item = Focus_currerntItem();
	if (!item) {
		return undefined;
	}
	return item.aps.at(0);
}
function Focus_currentLastAp() {
	let item = Focus_currerntItem();
	if (!item) {
		return undefined;
	}
	return item.aps.at(-1); // TODO 本当はfocusの当たっているものを返す？
}
function Focus_isCurrentAP(itemIndex, apIndex){
	const cplx = hist.now().focus.focusAPCplxes.at(-1);
	if(! cplx){
		return false;
	}
	if((cplx[0] === itemIndex) && (cplx[1] === apIndex)){
		return true;
	}
	return false;
}

function Focus_addAP(itemIndex, apIndex){
	if (-1 === itemIndex || -1 === apIndex){
		console.error('BUG', itemIndex, apIndex);
		return;
	}

	const ix = Focus_apIxFromAPIndexCplx(itemIndex, apIndex);
	if (-1 !== ix) {
		hist.now().focus.focusAPCplxes.splice(ix, 1);
	}
	hist.now().focus.focusAPCplxes.push([itemIndex, apIndex]);
	console.log('Focus_addAP', itemIndex, apIndex);
	// currentAPの属するItemをcurrentItemへ
	Focus_addItemIndex(itemIndex);
	return (-1 === ix);
}

function Focus_removeAP(itemIndex, apIndex){
	if (-1 === itemIndex || -1 === apIndex){
		console.error('BUG', itemIndex, apIndex);
		return;
	}
	const ix = Focus_apIxFromAPIndexCplx(itemIndex, apIndex);
	if (-1 === ix) {
		console.error('BUG', itemIndex, apIndex);
		return;
	}
	hist.now().focus.focusAPCplxes.splice(ix, 1);
	// すでにAPのフォーカスがないItemがあればそれを取り除く。
	if(! Focus_isExistInAPItemIndex_(itemIndex)){
		Focus_removeItemIndex(itemIndex);
		return;
	}
	checkFocusCurrentTopItemChangedCallback();
}

function Focus_isExistAP(itemIndex, apIndex){
	return (-1 !== Focus_apIxFromAPIndexCplx(itemIndex, apIndex));
}
function Focus_currerntItemIndex() {
	const index = hist.now().focus.focusItemIndexes.at(-1);
	return (undefined === index)? -1 : index;
}
function Focus_nowCurrerntAP() {
	const cplx = hist.now().focus.focusAPCplxes.at(-1);
	if (undefined === cplx) {
		return undefined;
	}
	return PV.apFromItems(hist.now().items, cplx);
}
function Focus_prevCurrerntAP() {
	const cplx = hist.now().focus.focusAPCplxes.at(-1);
	if (undefined === cplx) {
		return undefined;
	}
	const item = hist.now().items.at(cplx[0]);
	const beziers = PV.Item_beziersFromItem(item.cache.prevItems[0]);
	return beziers[0].aps.at(cplx[1]);
}

function setPaletteUIForMem() {
	// ** update UI palette
	// strokeColor, fillColorを入れ替える
	let p = document.getElementById('pallette-pair-parent')
	let bg = document.getElementById('palette-pair-border-group')
	let fg = document.getElementById('palette-pair-fill-group')
	p.appendChild(palette.isFill ? fg : bg);
	// TODO 透明のとき、border/fillの区別すらつかないためわかりずらい。
	// というかfillを透明にした時点でpair選択状態がわからなくなる。
	let b = document.getElementById('palette-pair-border')
	let f = document.getElementById('palette-pair-fill')
	b.style.fill = palette.colorPair.strokeColor;
	f.style.fill = palette.colorPair.fillColor;
	// RGB要素ごとのinput elementsへ反映
	let color = palette.isFill ? palette.colorPair.fillColor : palette.colorPair.strokeColor;
	let av = Math.round(parseInt(color.slice(7, 9), 16) / 0xff * 100);
	document.getElementById('r-slider').children[0].value = parseInt(color.slice(1, 3), 16);
	document.getElementById('g-slider').children[0].value = parseInt(color.slice(3, 5), 16);
	document.getElementById('b-slider').children[0].value = parseInt(color.slice(5, 7), 16);
	document.getElementById('a-slider').children[0].value = av;
	document.getElementById('r-slider').children[1].value = parseInt(color.slice(1, 3), 16);
	document.getElementById('g-slider').children[1].value = parseInt(color.slice(3, 5), 16);
	document.getElementById('b-slider').children[1].value = parseInt(color.slice(5, 7), 16);
	document.getElementById('a-slider').children[1].value = av;
	document.getElementById('color-hex').textContent = color;
	document.getElementById('color-picker').value = color.slice(0, 7);
}

function setBorderUIFromMem() {
	document.getElementById('border-width-input').value = border.width;
	document.getElementById('stroke-linecap').value = border.linecap;
	document.getElementById('stroke-linejoin').value = border.linejoin;
}

function setToolIndex(toolIndex) {
	console.log('setToolIndex', toolIndex);
	const tool = Tool.getToolFromIndex(toolIndex);
	setMessage(`tool change ${tool.name}`)

	const fieldFrame = document.getElementById('edit-field-frame');
	fieldFrame.style.cursor = 'auto';

	editor.toolIndex = toolIndex;
	editor.isAddAnchorPoindByAAPTool = false;
	//
	editor.apAddTool.isGrowthTailAp = true;
	editor.hover = deepcopy(EDITOR_HOVER_DEFAULT);

	let elems = document.getElementById('tool').getElementsByTagName('button');
	elems.forEach((elem, index) => {
		elem.classList.remove('tool-selected');
		if (index === toolIndex) {
			elem.classList.add('tool-selected');
		}
	});

	document.getElementById('interval-x-left').style.display = tool.isItem ? 'block' : 'none';
	document.getElementById('interval-x-right').style.display = tool.isItem ? 'block' : 'none';
	document.getElementById('interval-y-top').style.display = tool.isItem ? 'block' : 'none';
	document.getElementById('interval-y-bottom').style.display = tool.isItem ? 'block' : 'none';

	updateAxisInput();
	renderingEditorUserInterface();
}


function validCanvasScale(scale) {
	if (scale < (1 / 100.0)) {
		setMessage(`scale is min:${(1 / 100.0)}`) // TODO ちゃんとした値の表示に直す
		return false;
	}
	if ((4000 / 100.0) < scale) {
		setMessage(`scale is max:${(500 / 100.0)}`) // TODO ちゃんとした値の表示に直す
		return false;
	}

	return true;
}
function steppingCanvasScale(step) {
	let frame = document.getElementById('edit-field-frame')
	console.log('scr', frame.scrollTop, frame.scrollLeft, frame.clientWidth, frame.clientHeight, frame.scrollWidth, frame.scrollHeight)

	let diff = 0.1;

	let [_, digitPer] = (field.canvas.scale * 100).toExponential().split('e')
	if(2 <= digitPer){ // 100%以上では
		digitPer -= 1;
	}
	let digPer = 10 ** digitPer;
	let scale = field.canvas.scale;
	let scaleSanPer = Math.round((field.canvas.scale * 100) / digPer)*digPer;
	console.log('dig', digitPer, digPer, scaleSanPer)
	if((0 > step) && (digPer >= scaleSanPer)){
		digPer = digPer / 10;
	}
	scale = (scaleSanPer + (digPer * step)) / 100;

	//scale = Math.round(scale * 10) / 10;
	//scale += (diff * step);

	rescalingCanvas(scale);
}

function rescalingCanvas(scale) {
	if (!validCanvasScale(scale)) {
		return false;
	}

	let editField = document.getElementById('edit-field')
	prevEditFieldSizeByRescalingCanvas = {
		'w': editField.clientWidth,
		'h': editField.clientHeight,
	}

	field.canvas.scale = scale;
	document.getElementById('canvas-scale').value = (field.canvas.scale * 100).toFixed(1);
	renderingAll();
	return true;
}

function fittingCanvasScale() {
	let frame = document.getElementById('edit-field-frame')
	const ableCanvasSquare = {
		'w': frame.clientWidth,
		'h': frame.clientHeight,
	};
	const scaleW = ableCanvasSquare.w / hist.now().size.w;
	const scaleH = ableCanvasSquare.h / hist.now().size.h;

	let scale = (scaleW < scaleH) ? scaleW : scaleH;

	rescalingCanvas(scale);

	prevEditFieldSizeByRescalingCanvas = undefined;
	frame.scrollTop = hist.now().canvas.padding.h;
	frame.scrollLeft = hist.now().canvas.padding.w;
}

function getMouseModeByMousePoint(mousePoint){
	const focusRect = PV.rectFromItems(getNowFocusItems());
	if(! focusRect){
		return '';
	}

	// サイズゼロの場合にもmoveできるようにexpandしておく。値は根拠なし。
	const touchRect = PV.expandRect(focusRect, toCanvas(2));
	const inRect = PV.isTouchPointInRect(touchRect, mousePoint);
	if(inRect){
		return 'move';
	}
	// Guideのみの場合はresize, rotateしない。
	let isGuideOnly = true;
	PV.exfor(hist.now().focus.focusItemIndexes, (index, itemIndex) => {
		const item = hist.now().items.at(itemIndex);
		if('Guide' !== item.kind){
			isGuideOnly = false;
			return false;
		}
		return true;
	});
	if(isGuideOnly){
		return 'move';
	}

	// Rectがタッチ範囲より小さいと、判定領域が隣や逆側などにはみ出して当たってしまうが
	// ...Rectが小さすぎるせいなので拡大してくれということで良しとする。
	const corners = PV.cornerPointsFromRect(focusRect);
	if(PV.isTouchPointAndPoint(corners[0], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_lefttop';
	}
	if(PV.isTouchPointAndPoint(corners[1], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_righttop';
	}
	if(PV.isTouchPointAndPoint(corners[2], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_rightbottom';
	}
	if(PV.isTouchPointAndPoint(corners[3], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_leftbottom';
	}
	const sideCenters = PV.sideCentersFromRect(focusRect)
	if(PV.isTouchPointAndPoint(sideCenters[0], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_top';
	}
	if(PV.isTouchPointAndPoint(sideCenters[1], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_right';
	}
	if(PV.isTouchPointAndPoint(sideCenters[2], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_bottom';
	}
	if(PV.isTouchPointAndPoint(sideCenters[3], mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'resize_left';
	}
	const rotatePoint = Render.getRotateHandlePoint(focusRect, field)
	if(PV.isTouchPointAndPoint(rotatePoint, mousePoint, toCanvas(field.touch.cornerWidth))){
		return 'rotate';
	}
	
	return ''; // none
}

function getMousemoveRectInCanvas() {
	const range2d = {
		'x_min': Math.min(mousemoveInCanvas.x, mousedownInCanvas.x),
		'y_min': Math.min(mousemoveInCanvas.y, mousedownInCanvas.y),
		'x_max': Math.max(mousemoveInCanvas.x, mousedownInCanvas.x),
		'y_max': Math.max(mousemoveInCanvas.y, mousedownInCanvas.y),
	};
	return PV.rectFromRange2d(range2d);
}

function getMousemoveVectorInCanvas() {
	return {
		'x': mousemoveInCanvas.x - mousedownInCanvas.x,
		'y': mousemoveInCanvas.y - mousedownInCanvas.y,
	};
}

function snapMousemoveVectorInCanvas(isSnapForAngle){
	let move = getMousemoveVectorInCanvas();

	editor.mouse.snapAxisXKind = undefined;
	editor.mouse.snapAxisYKind = undefined;
	editor.mouse.snapItemApCplxX = undefined;
	editor.mouse.snapItemApCplxY = undefined;
	editor.mouse.snapAxisXTargetPoint = undefined;
	editor.mouse.snapAxisYTargetPoint = undefined;
	let snapAxisXItemPoint, snapAxisYItemPoint;
	let dbgTi, dbgIi, dbgTapi, dbgIapi;

	const SNAPW = hist.now().editor.snap.snapForAxisWidth;
	let betweenX = toCanvas(SNAPW);
	let betweenY = toCanvas(SNAPW);

	if(hist.now().editor.snap.isSnapForGrid){
		hist.now().focus.focusItemIndexes.forEach(itemIndex => {
			const prevItem = hist.now().items.at(itemIndex).cache.prevItems[0];
			const bezierItems = PV.Item_beziersFromItem(prevItem);
			bezierItems.forEach(bezierItem => {
				bezierItem.aps.forEach((ap, apIndex) => {
					const p = PV.pointAdd(ap.point, move);
					hist.now().canvas.grids.forEach(grid => {
						{
							const txmin = p.x - (p.x % grid.interval);
							const txmax = (txmin + ((0 > txmin ? -1:1) * grid.interval));
							//console.log('snap for grid', txmin, txmax, p.x);
							[txmin, txmax].forEach(tx => {
								const bw = Math.abs(p.x - tx);
								if(bw < betweenX){
									editor.mouse.snapItemApCplxX = [itemIndex, apIndex];
									snapAxisXItemPoint = ap.point;
									editor.mouse.snapAxisXTargetPoint = {'x': tx, 'y': 0};
									betweenX = bw;
									editor.mouse.snapAxisXKind = 'Grid';
								}
							});
						}
						{
							const tymin = p.x - (p.x % grid.interval);
							const tymax = (tymin + ((0 > tymin ? -1:1) * grid.interval));
							//console.log('snap for grid y', tymin, tymax, p.y);
							[tymin, tymax].forEach(ty => {
								const bw = Math.abs(p.y - ty);
								if(bw < betweenY){
									editor.mouse.snapItemApCplxY = [itemIndex, apIndex];
									snapAxisYItemPoint = ap.point;
									editor.mouse.snapAxisYTargetPoint = {'x': 0, 'y': ty};
									betweenY = bw;
									editor.mouse.snapAxisYKind = 'Grid';
								}
							});
						}
					});
				});
			});
		});
	}

	if(hist.now().editor.snap.isSnapForGuide || hist.now().editor.snap.isSnapForItem){
		hist.now().items.forEach((targetItem, targetItemIndex) =>{
			if(! Render.isMetaVisible(targetItem)){
				return;
			}

			const targetBezierItems = PV.Item_beziersFromItem(targetItem.cache.prevItems[0]);
			targetBezierItems.forEach(targetBezierItem => {
				targetBezierItem.aps.forEach((targetAp, targetApIndex) => {
					hist.now().focus.focusItemIndexes.forEach(itemIndex => {
						if(targetItemIndex === itemIndex){
							return;
						}
						const prevItem = hist.now().items.at(itemIndex).cache.prevItems[0];
						const bezierItems = PV.Item_beziersFromItem(prevItem);
						bezierItems.forEach(bezierItem => {
							bezierItem.aps.forEach((ap, apIndex) => {
								const p = PV.pointAdd(ap.point, move);
								const dlx = Math.abs(targetAp.point.x - p.x);
								const dly = Math.abs(targetAp.point.y - p.y);

								switch(targetItem.kind){
								case 'Guide':
									if(hist.now().editor.snap.isSnapForGuide){
										if('Vertical' === targetItem.axis){
											if(dlx < betweenX){
												editor.mouse.snapItemApCplxX = [itemIndex, apIndex];
												snapAxisXItemPoint = ap.point;
												editor.mouse.snapAxisXTargetPoint = targetAp.point;
												betweenX = dlx;
												editor.mouse.snapAxisXKind = 'Guide';
											}
										}else{ // Horizontal
											if(dly < betweenY){
												editor.mouse.snapItemApCplxY = [itemIndex, apIndex];
												snapAxisYItemPoint = ap.point;
												editor.mouse.snapAxisYTargetPoint = targetAp.point;
												betweenY = dly;
												editor.mouse.snapAxisYKind = 'Guide';
											}	
										}
									}
									break;
								default:
									if(hist.now().editor.snap.isSnapForItem){
										//console.log('aa:', dlx, dly);
										if(dlx < betweenX){
											editor.mouse.snapItemApCplxX = [itemIndex, apIndex];
											snapAxisXItemPoint = ap.point;
											editor.mouse.snapAxisXTargetPoint = targetAp.point;
											betweenX = dlx;
											editor.mouse.snapAxisXKind = 'Item';
											//
											dbgTi = targetItemIndex;
											dbgTapi = targetApIndex;
											dbgIi = itemIndex;
											dbgIapi = apIndex;
											console.log(targetAp.point.x, ap.point.x);
										}
										if(dly < betweenY){
											editor.mouse.snapItemApCplxY = [itemIndex, apIndex];
											snapAxisYItemPoint = ap.point;
											editor.mouse.snapAxisYTargetPoint = targetAp.point;
											betweenY = dly;
											editor.mouse.snapAxisYKind = 'Item';
										}
									}
									break;
								}
							});
						});
					});
				});
			});
		});
	}

	if(!! editor.mouse.snapAxisXKind){
		if(! snapAxisXItemPoint){
			console.error('BUG');
		}
		const diffx = (editor.mouse.snapAxisXTargetPoint.x - snapAxisXItemPoint.x);
		//console.log('snap x', move.x.toFixed(2), diffx.toFixed(2), dbgTi, dbgIi, dbgTapi, dbgIapi);
		move.x = diffx;
	}
	if(!! editor.mouse.snapAxisYKind){
		const diffy = (editor.mouse.snapAxisYTargetPoint.y - snapAxisYItemPoint.y);
		move.y = diffy;
	}

	if(hist.now().editor.snap.isSnapForPixel){
		const ap = Focus_prevCurrerntAP();
		if((! editor.mouse.snapAxisXKind) && (!! ap)){			
			const p = PV.pointAdd(ap.point, move);
			const diffx = (Math.round(p.x) - p.x);
			console.log('snapForPixel', p.x, diffx, move.x);
			move.x += diffx;
			//editor.mouse.snapAxisXKind = 'Point';
		}
		if((! editor.mouse.snapAxisYKind) && (!! ap)){
			const p = PV.pointAdd(ap.point, move);
			move.y += (Math.round(p.y) - p.y);
		}
	}

	// 
	if(! isSnapForAngle){
		editor.mouse.isRegulationalPositionByCenter = false;
		const vec = PV.pointSub(mousemoveInCanvas, editor.mouse.editCenter);
		const degree = PV.fieldDegreeFromVector(vec);
		return [degree, move];
	}
	const isSnapForAxis = ((!! editor.mouse.snapAxisXKind) || (!! editor.mouse.snapAxisYKind))
	if(! isSnapForAxis){
		return angleSnapAndMousemoveVectorInCanvas(isSnapForAngle);
	}

	// TODO AngleとAxisの並立
	// ---- 一時的なコード（Axisが効いているとき単にAngleを無視する）
	editor.mouse.isRegulationalPositionByCenter = false;
	const vec = PV.pointSub(mousemoveInCanvas, editor.mouse.editCenter);
	const degree = PV.fieldDegreeFromVector(vec);
	return [degree, move];
	// ----

}

// mousemove中の「角度にsnap」に必要な計算とglobal変数のセット
function angleSnapAndMousemoveVectorInCanvas(isSnapForAngle){
	let move = getMousemoveVectorInCanvas();
	let degree;
	if(isSnapForAngle){
		// SPEC ShiftKey押下しながらmousemoveの場合、角度にsnap
		[degree, move] = regulationalPositionByCenter(move);
		//if(! editor.mouse.isRegulationalPositionByCenter){
		//	console.log('RegulationalPosition enabled', degree);
		//}else{
		//	if(editor.mouse.regulationalPositionByCenterDegree !== degree){
		//		console.log('RegulationalPosition changed', degree);
		//	}
		//}
		editor.mouse.isRegulationalPositionByCenter = true;
		editor.mouse.regulationalPositionByCenterDegree = degree;
	}else{
		if(! editor.mouse.editCenter){
			console.error('BUG', editor.mouse);
		}
		if(! mousemoveInCanvas){
			console.error('BUG');
		}
		//if(editor.mouse.isRegulationalPositionByCenter){
		//	console.log('RegulationalPosition disabled');
		//}
		editor.mouse.isRegulationalPositionByCenter = false;
		const vec = PV.pointSub(mousemoveInCanvas, editor.mouse.editCenter);
		degree = PV.fieldDegreeFromVector(vec);
	}
	return [degree, move];
}

function getNowFocusItems(){
	return PV.itemsFromIndexes(hist.now().items, hist.now().focus.focusItemIndexes);
}

let renderingReductCount = 0;
let renderingReductTimerId;
function renderingAllReduct(){
	if(0 === renderingReductCount){
		renderingReductTimerId = setTimeout(() => {
			//if(1 !== renderingReductCount){
			//	console.log('cached', renderingReductCount);
			//}
			renderingReductCount = 0;
			renderingReductTimerId = undefined;
			renderingAll();
			//renderingUpdateDiff(hist.now().focus.focusItemIndexes);
		}, 10);
	}
	renderingReductCount++;
}

function renderingAll(){
	const mouse = {
		'mousedownInCanvas': mousedownInCanvas,
		'mousemoveInCanvas': mousemoveInCanvas
	};
	Render.renderingAll(renderingHandle, hist.now(), field, editor, mouse);
}

function renderingEditorUserInterface(){
	const mouse = {
		'mousedownInCanvas': mousedownInCanvas,
		'mousemoveInCanvas': mousemoveInCanvas
	};
	Render.renderingEditorUserInterface(renderingHandle, hist.now(), field, editor, mouse);
}

function regulationalPositionByCenter(move){
	const targetDegree = PV.fieldDegreeFromVector(move);
	let prevDiff = 45;
	let nearDegree = 0;
	for(let deg = 0; deg < 360; deg += 45){
		let diff = PV.absMinDiffByDegreeAndDegree(targetDegree, deg);
		if(diff < prevDiff){
			prevDiff = diff;
			nearDegree = deg;
		}
	}
	if(0 === nearDegree % 90){
		if(0 === nearDegree % 180){
			move.x = 0; // 垂直
		}else{
			move.y = 0; // 水平
		}
	}else{
		const dl = PV.diagonalLengthFromPoint(move);
		move = PV.vectorFromDegreeAndDiagonalLength(nearDegree, dl);
	}
	return [nearDegree, move];
}

function setMessage(msg) {
	// TODO 時間が経つなどしたら消す
	document.getElementById('message').textContent = msg;
}