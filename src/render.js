"use strict";

const PV = require("../src/pv.js");
const Tool = require("../src/tool.js");
const TOOLS = Tool.TOOLS();

const ROTATE_HEIGHT = 20;
const AHKIND = {
	'Symmetry': 'symmetry',
	'Free': 'free',
	'None': 'none',
};

module.exports = class Render{
	static renderingAll(renderingHandle, doc, field, editor, mouse) {
		if (!renderingHandle) {
			return;
		}

		// canvas to field scale convert
		const toField = (v) => { return v * field.canvas.scale; }
		const toCanvas = (v) => { return v / field.canvas.scale; }
	
		const getCanvasRect = function (doc, field) {
			return {
				'x': doc.canvas.padding.w,
				'y': doc.canvas.padding.h,
				'w': doc.size.w * field.canvas.scale,
				'h': doc.size.h * field.canvas.scale,
			};
		}
		const canvasRect = getCanvasRect(doc, field)
	
		let fieldSquare = {
			'w': canvasRect.w + (doc.canvas.padding.w * 2),
			'h': canvasRect.h + (doc.canvas.padding.h * 2),
		};
		// 縮小表示でタッチおよび描画する領域が見切れる場合があるため、下限をフレームサイズまで拡張
		let editFieldFrame = document.getElementById('edit-field-frame')
		fieldSquare.w = Math.max(fieldSquare.w, editFieldFrame.clientWidth)
		fieldSquare.h = Math.max(fieldSquare.h, editFieldFrame.clientHeight)
	
		// 外見上スクロール等は問題ないのだが、中のSVGに連動してサイズ情報が変わってはくれないようなので、ここから直接書き込む。
		let editField = document.getElementById('edit-field')
		editField.style.width = Math.ceil(fieldSquare.w) + 'px';
		editField.style.height = Math.ceil(fieldSquare.h) + 'px';
	
		// setup canvas & field
		let draw = renderingHandle.draw;
		let backgroundG = renderingHandle.backgroundG;
		let canvasParentG = renderingHandle.canvasParentG;
		//draw.clear();
		backgroundG.clear();
		canvasParentG.clear();
		let canvasG = canvasParentG.group();
		renderingHandle.canvasG = canvasG;
	
		draw.size(fieldSquare.w, fieldSquare.h);
	
		const checkerWidth = doc.canvas.background.checkerWidth;
		let pattern = draw.pattern((checkerWidth * 2), (checkerWidth * 2), function (add) {
			add.rect((checkerWidth * 2), (checkerWidth * 2)).fill('#fff')
			add.rect(checkerWidth, checkerWidth).fill('#bbb')
			add.rect(checkerWidth, checkerWidth).fill('#bbb').move(checkerWidth, checkerWidth)
		})
	
		backgroundG.rect(canvasRect.w, canvasRect.h)
			.attr({ fill: pattern }).move(canvasRect.x, canvasRect.y);
		backgroundG.rect(canvasRect.w, canvasRect.h)
			.attr({ fill: doc.canvas.background.color }).move(canvasRect.x, canvasRect.y);
	
	
		canvasG.translate(doc.canvas.padding.w, doc.canvas.padding.h)
		canvasG.scale(field.canvas.scale)
	
		Render.renderingEditorUserInterface(renderingHandle, doc, field, editor, mouse);
		Render.renderingDocOnCanvas(canvasG, doc);
	}

	// ** editor user display
	static renderingEditorUserInterface(renderingHandle, doc, field, editor, mouse) {
		if (!renderingHandle) {
			return;
		}

		// canvas to field scale convert
		const toField = (v) => { return v * field.canvas.scale; }
		const toCanvas = (v) => { return v / field.canvas.scale; }
	
		//console.log(renderingHandle.draw.width())
		const fieldSquare = {
			'w': renderingHandle.draw.width(),
			'h': renderingHandle.draw.height(),
		};

		let forgroundG = renderingHandle.forgroundG;
		forgroundG.clear();

		let forgroundCanG = forgroundG.group();
		let forgroundInG = forgroundG.group();

		forgroundCanG.translate(doc.canvas.padding.w, doc.canvas.padding.h)
		forgroundCanG.scale(field.canvas.scale)

		doc.canvas.grids.forEach((grid) => {
			const wrange = {
				'min': doc.canvas.padding.w,
				'max': doc.canvas.padding.w + toField(doc.size.w),
			};
			for (let i = 0; true; i++) {
				const x = (toField(grid.interval * i) + wrange.min);
				if (wrange.max < x) { break; }
				// 上下に少し足りなくしているのはデバッグのため
				let line = forgroundInG.line(x, 5, x, fieldSquare.h - 5);
				line.stroke({ width: grid.strokew }).attr({ 'stroke': grid.color });
			}
			const hrange = {
				'min': doc.canvas.padding.h,
				'max': doc.canvas.padding.h + toField(doc.size.h),
			};
			for (let i = 0; true; i++) {
				const y = (toField(grid.interval * i) + hrange.min);
				if (hrange.max < y) { break; }
				// 上下に少し足りなくしているのはデバッグのため
				let line = forgroundInG.line(5, y, fieldSquare.w - 5, y);
				line.stroke({ width: grid.strokew }).attr({ 'stroke': grid.color });
			}
		});

		const DASH = `${toCanvas(12)} ${toCanvas(6)}`;
		const GUIDE_DASH = `${(8)} ${(4)}`;

		{
			// 選択item領域の四角形を描画
			const focusRect = this.getFocusingRectByNow(editor, doc);
			if (focusRect) {
				if ((!PV.fuzzyZero(focusRect.w)) && (!PV.fuzzyZero(focusRect.h))) {
					let se = forgroundCanG.rect(focusRect.w, focusRect.h);
					se.move(focusRect.x, focusRect.y);
					se.attr({
						'stroke': '#3070ffff',
						'stroke-width': (1 / field.canvas.scale),
						'fill': 'none',
						'stroke-dasharray': DASH
					});
				}
			}

			const isRotetableTool = (editor.toolIndex === TOOLS.ItemEditTool.Id);
			const isRotatableFocusItems = ! PV.exfor(doc.focus.focusItemIndexes, (ix, itemIndex) => {
				const item = doc.items.at(itemIndex);
				return ('Guide' === item.kind);
			});
			if(focusRect && isRotetableTool && isRotatableFocusItems){
				// rotateハンドルを描画
				const rhPoint = Render.getRotateHandlePoint(focusRect, field);
				const r = toCanvas(5);
				const rootPoint = {'x': focusRect.x + (focusRect.w/2), 'y': focusRect.y};
				let se = forgroundCanG.line(rhPoint.x, rhPoint.y, rootPoint.x, rootPoint.y);
				se.attr({
					'stroke': '#3070ffff',
					'stroke-width': (1 / field.canvas.scale),
				});
				forgroundCanG.circle(r).move(rhPoint.x - (r / 2), rhPoint.y - (r / 2)).attr({ fill: '#333' });
			}
		}

		// ** 編集中心
		const center = editor.mouse.editCenter;
		if(!! center){
			{
				const r = toCanvas(3);
				let se = forgroundCanG.circle(r).move(center.x - (r / 2), center.y - (r / 2))
				se.attr({
					'fill': '#3070ff80',
					'stroke': 'none',
				});
			}
			{
				const r = toCanvas(12);
				let se = forgroundCanG.circle(r).move(center.x - (r / 2), center.y - (r / 2))
				se.attr({
					'fill': 'none',
					'stroke': '#3070ff80',
					'stroke-width': toCanvas(1.5),
				});
			}
		}

		// ** 角度にスナップ
		if(editor.mouse.isRegulationalPositionByCenter){
			const dl = PV.diagonalLengthFromRect(fieldSquare);
			const center = editor.mouse.editCenter;
			if(! center){
				console.error('BUG', editor);
				return;
			}
			for(let degree = 0; degree < 360; degree += 45){
				const isUsedDegree = (degree === editor.mouse.regulationalPositionByCenterDegree);
				const startPoint = PV.pointFromDegreeAndDiagonalLength(center, degree, toCanvas(12));
				const endPoint = PV.pointFromDegreeAndDiagonalLength(center, degree, dl);
				let se = forgroundCanG.line(startPoint.x, startPoint.y, endPoint.x, endPoint.y);
				se.attr({
					'stroke': '#3070ffff',
					'stroke-width': isUsedDegree ? toCanvas(2) : toCanvas(1),
				});
			}
		}

		// ** Axisにスナップ
		if(!! editor.mouse.snapAxisXKind){
			const stp = editor.mouse.snapAxisXTargetPoint;
			const ip = PV.apFromItems(doc.items, editor.mouse.snapItemApCplxX).point;
			let se = forgroundCanG.line(stp.x, stp.y, ip.x, ip.y);
			se.attr({
				'stroke': '#a0f0a0ff',
				'stroke-width': toCanvas(1.2),
			});
		}
		if(!! editor.mouse.snapAxisYKind){
			const stp = editor.mouse.snapAxisYTargetPoint;
			const ip = PV.apFromItems(doc.items, editor.mouse.snapItemApCplxY).point;
			let se = forgroundCanG.line(stp.x, stp.y, ip.x, ip.y);
			se.attr({
				'stroke': '#a0f0a0ff',
				'stroke-width': toCanvas(1.2),
			});
		}	

		const renderingGuideLine = (item) => {
			let line;
			if('Vertical' === item.axis){
	//			const vrange = {
	//				'min': - toCanvas(doc.canvas.padding.h),
	//				'max': toCanvas(doc.canvas.padding.h * 2) + (doc.size.h) + doc.size.h,
	//				// 足りないよりははみ出てくれたほうが都合が良いので適当に延伸
	//			};
	//			line = forgroundCanG.line(item.point.x, vrange.min, item.point.x, vrange.max);
	//
				const x = doc.canvas.padding.w + toField(item.point.x);
				line = forgroundInG.line(x, 5, x, fieldSquare.h - 5);
			}else{
	//			const hrange = {
	//				'min': - toCanvas(doc.canvas.padding.w),
	//				'max': toCanvas(doc.canvas.padding.w * 2) + (doc.size.w) + doc.size.w,
	//				// 足りないよりははみ出てくれたほうが都合が良いので適当に延伸
	//			};
	//			line = forgroundCanG.line(hrange.min, item.point.y, hrange.max, item.point.y);
				const y = doc.canvas.padding.h + toField(item.point.y);
				line = forgroundInG.line(5, y, fieldSquare.w - 5, y);
			}
			return line;
		};

		// Guideを描画
		doc.items.forEach((item) => {
			if(! Render.isMetaVisible(item, doc)){
				return;
			}
			switch(item.kind){
			case 'Guide':{
				let line = renderingGuideLine(item);
				line.attr({
					'stroke': doc.canvas.guide.color,
					'stroke-width': doc.canvas.guide.strokew,
					'fill': 'none',
				});
				const r = toCanvas(6);
				forgroundCanG.circle(r).move(item.point.x - (r / 2), item.point.y - (r / 2)).attr({ fill: '#000' });
			}
				break;
			default:
				return;
			}
		});

		// FocusItemsを描画
		let range2ds = [];
		doc.focus.focusItemIndexes.forEach((itemIndex, ix, arr) => {
			const item = doc.items.at(itemIndex);
			if (! item){
				console.error('BUG', item);
				return;
			}

			const renderingPatharray = (patharray) => {
				let guide = forgroundCanG.path(patharray);
				if(ix === arr.length - 1){ // 最後の要素がcurrent
					guide.attr({
						'stroke': '#3070ffff',
						'stroke-width': (2 / field.canvas.scale),
						'fill': 'none',
						'stroke-dasharray': DASH
					});
				}else{
					guide.attr({
						'stroke': '#5090ffff',
						'stroke-width': (1.2 / field.canvas.scale),
						'fill': 'none',
						'stroke-dasharray': DASH
					});
				}
			};
			const renderingApByBezier = (bezier) => {
				// APを描画
				bezier.aps.forEach((ap, index) => {
					// AHを描画
					// SPEC AIではFocusされているAPのAHだけ描画されているようだが、VEではとりあえず全部描画したままにしておく。
					// (だが矩形選択時によくわからない基準で選んだAHを描画している。まあ無視。)
					{
						const hps = [PV.AP_frontAHPoint(ap), PV.AP_backAHPoint(ap)];
						hps.forEach(hp => {
							// 線を引く(noneは引かない)
							switch (ap.handle.kind) {
							case AHKIND.Symmetry:
							case AHKIND.Free:
								let se = forgroundCanG.line(ap.point.x, ap.point.y, hp.x, hp.y);
								se.attr({
									'stroke': '#3070ffff',
									'stroke-width': (1 / field.canvas.scale),
								});
								break;
							case AHKIND.None:
								return;
							default:
								console.error('BUG', ap, index, item);
								return;
							}
		
							// ハンドル先端を描く
							const r = toCanvas(5);
							switch(ap.handle.kind){
							case AHKIND.Free:
								forgroundCanG.rect(r,r).move(hp.x - (r / 2), hp.y - (r / 2)).attr({ fill: '#33f' });
								break;
							case AHKIND.Symmetry:
								forgroundCanG.circle(r).move(hp.x - (r / 2), hp.y - (r / 2)).attr({ fill: '#33f' });
								break;
							}
						});
					}
					// APを描画
					{
						if(0 === index){ // 先頭APのみ装飾
							const r = 12 / field.canvas.scale;
							let se = forgroundCanG.circle(r).move(ap.point.x - (r / 2), ap.point.y - (r / 2))
							se.attr({
								'fill': 'none',
								'stroke': '#3070ff80',
								'stroke-width': (1 / field.canvas.scale),
							});
						}
						const r = 6 / field.canvas.scale;
						forgroundCanG.circle(r).move(ap.point.x - (r / 2), ap.point.y - (r / 2)).attr({ fill: '#33f' });
					}
				});
			};

			switch(item.kind){
			case 'Bezier':{
				if (0 === item.aps.length) {
					console.error('BUG', item);
					return;
				}
		
				// pathを描画
				const patharray = Render.patharrayFromBezierItem(item);
				renderingPatharray(patharray);
				renderingApByBezier(item);
			}
				break;
			case 'Figure':{
				const beziers = PV.Item_beziersFromFigure(item);
				beziers.forEach(bezier => {
					// pathを描画
					const patharray = Render.patharrayFromBezierItem(bezier);
					renderingPatharray(patharray);
					renderingApByBezier(bezier);
				});
			}
				break;
			case 'Guide':{
				let line = renderingGuideLine(item);
				line.attr({
					'stroke': '#5090ffff',
					'stroke-width': (1.2),
					'fill': 'none',
					'stroke-dasharray': GUIDE_DASH
				});
				const r = toCanvas(6);
				forgroundCanG.circle(r).move(item.point.x - (r / 2), item.point.y - (r / 2)).attr({ fill: '#33f' });
			}
				break;
			default:
				console.error('BUG', item);
				return;
			}
		});

		doc.focus.focusAPCplxes.forEach((cplx, ix, arr) => {
			const ap = PV.apFromItems(doc.items, cplx);
			if(! ap){
				console.log('BUG', cplx);
				return;
			}
			// FocusedAPを描画
			const r = 4 / field.canvas.scale;
			let se = forgroundCanG.circle(r).move(ap.point.x - (r / 2), ap.point.y - (r / 2))
			if(ix === arr.length - 1){ // 最後の要素がcurrent
				se.attr({ fill: '#4c8' });
			}else{
				se.attr({ fill: '#ff2' });
			}
		});

		// ** hover
		const HOVER_COLOR = '#6030ffff';
		if(-1 !== editor.hover.touchedItemIndex){
			const item = doc.items[editor.hover.touchedItemIndex]
			if(! item){
				console.log('BUG',editor.hover.touchedItemIndex);
				return;
			}
			let focusRect = PV.rectFromItems([item]);
			if ((PV.fuzzyZero(focusRect.w)) || (PV.fuzzyZero(focusRect.h))) {
				// ユーザ補助のための表示なので正確さは不要のため、潰れていたら適当に拡張する
				focusRect = PV.expandRect(focusRect, toCanvas(6));
			}
			let se = forgroundCanG.rect(focusRect.w, focusRect.h);
			se.move(focusRect.x, focusRect.y);
			se.attr({
				'stroke': HOVER_COLOR,
				'stroke-width': (1 / field.canvas.scale),
				'fill': '#6030ff10',
				'stroke-dasharray': DASH
			});
			{
				const beziers = PV.Item_beziersFromItem(item);
				beziers.forEach(bezier => {
					// pathを描画
					const patharray = Render.patharrayFromBezierItem(bezier);
					let guide = forgroundCanG.path(patharray);
					guide.attr({
						'stroke': HOVER_COLOR,
						'stroke-width': (0.8 / field.canvas.scale),
						'fill': 'none',
						'stroke-dasharray': DASH
					});
				});
			}
		}
		if(-1 !== editor.hover.touchedApAhCplx[1]){
			const ap = PV.apFromItems(doc.items, editor.hover.touchedApAhCplx);
			if(! ap){
				console.log('BUG', cplx);
				return;
			}

			let point;
			switch(editor.hover.touchedApAhCplx[2]){
			case 'ap':
				point = ap.point;
				break;
			case 'front':
				point = PV.AP_frontAHPoint(ap);
				console.log('f', ap);
				break;
			case 'back':
				point = PV.AP_backAHPoint(ap);
				console.log(ap);
				break;
			default:
				console.log('BUG',editor.hover.touchedApAhCplx);
				return;
			}

			const r = toCanvas(field.touch.pointR);
			let se = forgroundCanG.circle(r).move(point.x - (r / 2), point.y - (r / 2))
			se.attr({
				'stroke': HOVER_COLOR,
				'stroke-width': (1 / field.canvas.scale),
				'fill': 'none',
			});
		}
		if(editor.hover.nearTouchPointOnBetween){
			const p = editor.hover.nearTouchPointOnBetween;
			const r = toCanvas(6);
			let se = forgroundCanG.circle(r).move(p.x - (r / 2), p.y - (r / 2))
			se.attr({ fill: '#d84' });
		}

		if (mouse.mousedownInCanvas && mouse.mousemoveInCanvas) {
			// mousedown,mousemove中
			switch (editor.mouse.mode) {
				case 'rectselect':
					const rect = getMousemoveRectInCanvas();
					let se = forgroundCanG.rect(rect.w, rect.h).move(rect.x, rect.y)
					se.attr({
						'stroke': '#3070ffff',
						'stroke-width': toCanvas(0.5),
						'fill': '#3070ff55',
					});
					break;
			}
		}
	}
 
	static renderingDocOnCanvas(canvasG, doc){
		//console.log('CALL renderingDocOnCanvas');
		// reindering document items
		doc.items.forEach((item) => {
			if(! Render.isMetaVisible(item, doc)){
				return;
			}
	
			switch(item.kind){
			case 'Bezier':{
				if (0 === item.aps.length) {
					console.error('BUG', item);
					return;
				}
				const patharray = Render.patharrayFromBezierItem(item);
				let path = canvasG.path(patharray)
				path.attr({
					'stroke': item.colorPair.strokeColor,
					'stroke-width': item.border.width,
					'stroke-linecap': item.border.linecap,
					'stroke-linejoin': item.border.linejoin,
					'fill': item.colorPair.fillColor
				});
				if(! item.hasOwnProperty('cache')){ item.cache = {}; }
				item['cache']['renderHandles'] = [path];
			}
				break;
			case 'Figure':{
				if(! item.hasOwnProperty('cache')){ item.cache = {}; }
				item['cache']['renderHandles'] = [];
				const beziers = PV.Item_beziersFromFigure(item);
				beziers.forEach(bezier => {
					const patharray = Render.patharrayFromBezierItem(bezier);
					let path = canvasG.path(patharray)
					path.attr({
						'stroke': bezier.colorPair.strokeColor,
						'stroke-width': bezier.border.width,
						'stroke-linecap': bezier.border.linecap,
						'stroke-linejoin': bezier.border.linejoin,
						'fill': bezier.colorPair.fillColor
					});
					if(! item.hasOwnProperty('cache')){ item.cache = {}; }
					item['cache']['renderHandles'].push(path);
				});
			}
				break;
			case 'Guide':{
				// NOP
			}
				break;
			default:
				console.error('BUG', item);
				return;
			}
		});
	}
	static patharrayFromBezierItem(bezier) {
		if('Bezier' !== bezier.kind){
			console.error('BUG', bezier);
			return [];
		}
		let patharray = [];
		bezier.aps.forEach((ap, index) => {
			//patharray.push([(0 === index) ? 'M':'L', ap.point.x, ap.point.y]);
			if (0 === index) {
				patharray.push(['M', ap.point.x, ap.point.y]);
			} else {
				const prevAP = bezier.aps[index - 1];
				const prevFrontAH = PV.AP_frontAHPoint(prevAP);
				const currentBackAH = PV.AP_backAHPoint(ap);
				patharray.push([
					'C',
					prevFrontAH.x, prevFrontAH.y,
					currentBackAH.x, currentBackAH.y,
					ap.point.x, ap.point.y
				]);
			}
		});
		if (2 <= bezier.aps.length && bezier.isCloseAp) {
			// 曲線の閉パスは、開始点と同座標のC点を作ってから閉じ指定。
			const firstAP = bezier.aps.at(0);
			const lastAP = bezier.aps.at(-1);
			const lastFrontAH = PV.AP_frontAHPoint(lastAP);
			const firstBackAH = PV.AP_backAHPoint(firstAP);
			patharray.push(['C', lastFrontAH.x, lastFrontAH.y, firstBackAH.x, firstBackAH.y, firstAP.point.x, firstAP.point.y]);
			patharray.push(['Z']);
		}
		return patharray;
	}

	// **
	static getFocusingRectByNow(editor, doc){
		if(Tool.getToolFromIndex(editor.toolIndex).isItem){
			return PV.rectFromItems(PV.itemsFromIndexes(doc.items, doc.focus.focusItemIndexes));
		}
	
		const aps = PV.apsFromItems(doc.items, doc.focus.focusAPCplxes);
		return PV.rectFromAps(aps);
	}

	static getRotateHandlePoint(focusRect, field){
		const toCanvas = (v) => { return v / field.canvas.scale; }

		const rotatePoint = {
			'x': focusRect.x + (focusRect.w/2),
			'y': focusRect.y - (toCanvas(ROTATE_HEIGHT + (field.touch.cornerWidth/2))),
		};	
		return rotatePoint;
	}

	static isMetaLock(item, doc){
		if('Guide' === item.kind){
			if(doc.editor.guide.isLockGuide){
				return true;
			}
		}
		return item.isLock;
	}
	static isMetaVisible(item, doc){
		if('Guide' === item.kind){
			if(! doc.editor.guide.isVisibleGuide){
				return false;
			}
		}
		return item.isVisible;
	}
}