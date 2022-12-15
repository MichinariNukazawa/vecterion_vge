"use strict";

const { indexOf } = require("lodash");
const clonedeep = require('lodash/cloneDeep');

function deepcopy(obj) { return clonedeep(obj); }
function fuzzyZero(v) { return 0.0001 > Math.abs(v); }
function fuzzyEqual(a, b) { return 0.0001 > Math.abs(a - b); }

const AHKIND = {
	'Symmetry': 'symmetry',
	'Free': 'free',
	'None': 'none',
};

module.exports = class PV{
	// ** Point
	static pointAdd(a, b){
		return {'x': a.x + b.x, 'y': a.y + b.y};
	}
	static pointSub(a, b){
		return {'x': a.x - b.x, 'y': a.y - b.y};
	}
	static pointMul(a, b){
		return {'x': a.x * b.x, 'y': a.y * b.y};
	}
	static pointDiv(a, b){
		const x = (fuzzyZero(b.x)) ? 0 : (a.x / b.x);
		const y = (fuzzyZero(b.y)) ? 0 : (a.y / b.y);
		return {'x': x, 'y': y};
	}
	static MinusPoint(){
		return {'x': -1, 'y': -1};
	}
	static ZeroPoint(){
		return {'x': 0, 'y': 0};
	}
	static toPoint(value){
		return {'x': value, 'y': value};
	}
	static fuzzyZeroPoint(point){
		return fuzzyZero(point.x) && fuzzyZero(point.y);
	}
	// ** rect
	// rect２つを含むrectを返す
	static mergeRect(a, b) {
		const leftTop = {
			'x': Math.min(a.x, b.x),
			'y': Math.min(a.y, b.y),
		};
		const rightBottom = {
			'x': Math.max(a.x + a.w, b.x + b.w),
			'y': Math.max(a.y + a.h, b.y + b.h),
		}
		return {
			'x': leftTop.x,
			'y': leftTop.y,
			'w': rightBottom.x - leftTop.x,
			'h': rightBottom.y - leftTop.y,
		};
	}
	static centerFromRect(rect){
		return {'x': rect.x + (rect.w/2), 'y': rect.y + (rect.h/2),	};
	}
	static rightBottomFromRect(rect){
		return {'x': rect.x + rect.w, 'y': rect.y + rect.h,	};
	}
	static rectSize(rect){
		return {'x': rect.w, 'y': rect.h};
	}
	static rectFromRange2d(range2d) {
		return {
			'x': range2d.x_min,
			'y': range2d.y_min,
			'w': range2d.x_max - range2d.x_min,
			'h': range2d.y_max - range2d.y_min,
		};
	}
	static range2dFromRect(rect) {
		return {
			'x_min': rect.x,
			'y_min': rect.y,
			'x_max': rect.x + rect.w,
			'y_max': rect.y + rect.h,
		};
	}
	// pointを含むように必要に応じてrectを拡張する
	// （loop内で使えるよう、最初のrectがundefinedの場合はゼロサイズのrectを作って返す）
	static Rect_includePoint(rect, point){
		if(! rect){
			return {
				'x': point.x,
				'y': point.y,
				'w': 0,
				'h': 0,
			};
		}
		const srcRange2d = this.range2dFromRect(rect);
		const dstRange2d = {
			'x_min': Math.min(srcRange2d.x_min, point.x),
			'y_min': Math.min(srcRange2d.y_min, point.y),
			'x_max': Math.max(srcRange2d.x_max, point.x),
			'y_max': Math.max(srcRange2d.y_max, point.y),
		};
		return this.rectFromRange2d(dstRange2d);
	}
	// Rectの四隅のPointを返す。順序は左上から時計回り。
	static cornerPointsFromRect(rect){
		return [
			{'x': rect.x, 'y': rect.y, },
			{'x': rect.x + rect.w, 'y': rect.y, },
			{'x': rect.x + rect.w, 'y': rect.y + rect.h, },
			{'x': rect.x, 'y': rect.y + rect.h, },
		];
	}
	// Rectの四辺の中央のPointを返す。順序は上辺から時計回り。
	static sideCentersFromRect(rect){
		return [
			{'x': rect.x + (rect.w/2), 'y': rect.y, },
			{'x': rect.x + rect.w, 'y': rect.y + (rect.h/2), },
			{'x': rect.x + (rect.w/2), 'y': rect.y + rect.h, },
			{'x': rect.x, 'y': rect.y + (rect.h/2), },
		];
	}
	static expandRect(rect, expand) {
		rect.x -= expand;
		rect.y -= expand;
		rect.w += (expand * 2);
		rect.h += (expand * 2);
		return rect;
	}
	// ** Item
	static rectFromItems(items){
		let range2ds = [];
		items.forEach(item => {
			// range2dを収集
			range2ds.push(PV.Item_range2d(item));
		})
		let rect; // itemが無ければ return undefined
		range2ds.forEach((range2d, index) => {
			if (0 === index) {
				rect = PV.rectFromRange2d(range2d);
			} else {
				rect = PV.mergeRect(rect, PV.rectFromRange2d(range2d));
			}
		});
		return rect;
	}
	static apFromItems(items, cplx){
		const item = items.at(cplx[0]);
		if(! item){
			return undefined;
		}
		const ap = item.aps.at(cplx[1]);
		if(! ap){
			return undefined;
		}
		return ap;
	}
	static apsFromItems(items, cplxes){
		let aps = [];
		cplxes.forEach(cplx => {
			const ap = PV.apFromItems(items, cplx);
			if(! ap){
				console.error('BUG', cplx);
				return;
			}
			aps.push(ap);
		});
		return aps;
	}
	static Item_range2d(srcItem) {
		const items = PV.Item_beziersFromItem(srcItem);
		let range2d;
		items.forEach(item => {
			if (0 === item.aps.length) {
				console.error('BUG', item);
				return undefined;
			}
		
			item.aps.forEach((ap, index) => {
				if (0 === index) {
					range2d = {
						'x_min': ap.point.x,
						'x_max': ap.point.x,
						'y_min': ap.point.y,
						'y_max': ap.point.y,
					};
				} else {
					range2d = {
						'x_min': Math.min(range2d.x_min, ap.point.x),
						'x_max': Math.max(range2d.x_max, ap.point.x),
						'y_min': Math.min(range2d.y_min, ap.point.y),
						'y_max': Math.max(range2d.y_max, ap.point.y),
					};
				}
			});
		});
		return range2d;
	}
	static Item_beziersFromItem(item){
		switch(item.kind){
		case 'Bezier':
			return [item];
		case 'Figure':
			return PV.Item_beziersFromFigure(item);
		default:
			console.error('BUG', item);
			return undefined;
		}
	}
	static BezierItem(){
		return {
			'kind': 'Bezier',
			'name': '',
			'isVisible': true,
			'isLock': false,
			'colorPair': '#ffffffff',
			'border': '#000000ff',
			//
			'aps': [],
			'isCloseAp': false,
		};
	}
	static Item_beziersFromFigure(fig){
		const copyBezierFromFigure = (fig) => {
			let bezier = PV.BezierItem();
			bezier.name = fig.name;
			bezier.isVisible = fig.isVisible;
			bezier.isLock = fig.isLock;
			bezier.colorPair = fig.colorPair;
			bezier.border = fig.border;
			bezier.isCloseAp = true;
			return bezier;
		};

		switch(fig.figureKind){
		case 'Polygon':{
			if(0 === (fig.starryCornerStepNum / fig.cornerNum) % 1){
				console.error('BUG', fig.cornerNum, fig.starryCornerStepNum, fig);
				return undefined;
			}
			if(0 === fig.starryCornerStepNum){
				console.error('BUG', fig.cornerNum, fig.starryCornerStepNum, fig);
				return undefined;
			}
			const cornerDegree = 360 / fig.cornerNum;
			const x = (0 === fig.cornerNum%2) ? (1/Math.tan(PV.mathRadianFromFieldDegree(-cornerDegree/2))) : 0;
			const startCornerVec = { // 最初の点（左上）
				'x': x,
				'y': -1,
			};
			let beziers = [];
			let bezier = undefined;
			let exDegree = 0;
			for(let i = 0; i < fig.cornerNum; i++){
				const step = (i * fig.starryCornerStepNum);
				if((!! bezier) && (step === fig.cornerNum)){
					// （最初の点以外で）途中で最初の場所に戻ってきてしまった場合に、
					// 『角度をずらして追加のBezierを作る』ことで六芒星などに対応。
					beziers.push(bezier);
					bezier = undefined;
					exDegree += cornerDegree;
					console.log(`break ${i}/${fig.cornerNum}, ${step}, ${exDegree}`);
				}
				const degree = (cornerDegree * step) + exDegree;
				const rotateVec = PV.vectorRotate(PV.pointMul(fig.rsize, PV.vectorRotate(startCornerVec, degree)), fig.rotate);
				const p = PV.pointAdd(fig.point, rotateVec);
				if(! bezier){
					bezier = copyBezierFromFigure(fig);
				}
				bezier.aps.push(PV.AP_newFromPoint(p));
				if(! fuzzyEqual(100, fig.starryCornerPitRPercent)){
					// 辺のヘコみを作成。
					const v = fig.starryCornerPitRPercent/100;
					const d = (cornerDegree*fig.starryCornerStepNum)/2;
					const t = Math.sin(PV.mathRadianFromFieldDegree(d));
					const pitRotateVec = PV.vectorRotate(PV.pointMul(rotateVec, PV.toPoint(v * t)), d);
					const pitp = PV.pointAdd(fig.point, pitRotateVec);
					bezier.aps.push(PV.AP_newFromPoint(pitp));
				}
			}
			if(!! bezier){
				beziers.push(bezier);
			}
			return beziers;
			}
		case 'Circle':{
			let bezier = copyBezierFromFigure(fig);
			for(let i = 0; i < 4; i++){
				const degree = (i * 90);
				const r = 1;
				const startCornerVec = { // 最初の点（中央上）
					'x': 0,
					'y': r,
				};
				// ベジエで描く曲線は厳密には円にならないそうなのだが、
				// 実装の都合によりBezier変換する前からBezierで描画してしまうことにする。
				// (ここの出力を描画側でも使っている。)
				// https://cat-in-136.github.io/2014/03/bezier-2-diff.html
				const startHandleVec = {
					'x': -0.5522847498,
					'y': 0,
				}
				const rotateVec = PV.vectorRotate(PV.pointMul(fig.rsize, PV.vectorRotate(startCornerVec, degree)), fig.rotate);
				const p = PV.pointAdd(fig.point, rotateVec);
				let ap = PV.AP_newFromPoint(p);
				ap.handle.kind = AHKIND.Symmetry;
				ap.handle.frontVector = PV.vectorRotate(PV.pointMul(fig.rsize, PV.vectorRotate(startHandleVec, degree)), fig.rotate);
				bezier.aps.push(ap);
			}
			return [bezier];
		}
		default:
			console.error('BUG', fig);
			return [];
		}
	}
	// ** diagonalLength
	// 対角線の長さを返す
	static diagonalLengthFromRect(rect) {
		return Math.sqrt(Math.pow(rect.w, 2) + Math.pow(rect.h, 2));
	}
	// 対角線の長さを返す
	static diagonalLengthFromPoint(point) {
		return Math.sqrt(Math.pow(point.x, 2) + Math.pow(point.y, 2));
	}
	// ** touch
	static isTouchPointAndPoint(a, b, r) {
		const l = Math.sqrt(Math.pow(a.x - b.x, 2) + Math.pow(a.y - b.y, 2));
		return (l < r);
	}

	static isTouchPointInRect(rect, point) {
		return (rect.x < point.x && point.x < (rect.x + rect.w))
			&& (rect.y < point.y && point.y < (rect.y + rect.h))
	}

	static isTouchRectInRect(a, b) {
		const ac = { 'x': a.x + (a.w / 2), 'y': a.y + (a.h / 2) };
		const bc = { 'x': b.x + (b.w / 2), 'y': b.y + (b.h / 2) };
		return (
			(Math.abs(ac.x - bc.x) < a.w / 2 + b.w / 2) //横の判定
			&&
			(Math.abs(ac.y - bc.y) < a.h / 2 + b.h / 2) //縦の判定
		);
	}
	// ** radian/degree
	static mathRadianFromFieldDegree(fieldDegree){
		const fieldRadian = (fieldDegree) * (Math.PI / 180.0);
		return -1 * ((fieldRadian) - (Math.PI/2)) % (Math.PI*2);
	}
	
	static fieldDegreeFromMathRadian(mathRadian){
		const mathDegree = ((mathRadian) * 180 / Math.PI);
		return -1 * ((mathDegree) + 270) % 360;
	}
	// 角度ではなく角度の変化量
	static radianDiffFromDegreeDiff(degreeDiff){
		return -1 * (degreeDiff) * (Math.PI / 180.0);
	}
	// ** degree
	// TODO 名前、なにか違うと思う。
	static sanitizeDegree(degree){
		while(0 > degree){
			degree += 360;
		}
		return degree % 360;	
	}
	static absMinDiffByDegreeAndDegree(now, next){
		let d = next - now;
		d -= Math.floor(d / 360.0) * 360.0;
		if (d > 180){
			d -= 360;
		}
		return Math.abs(d);
	}
	static fieldDegreeFromVector(vector){
		if(fuzzyZero(vector.y) && fuzzyZero(vector.x)){ // そのままの場合-270
			return 0;
		}
		return PV.fieldDegreeFromMathRadian(Math.atan2(-vector.y, vector.x))
	}
	// ** rotate
	static vectorFromDegreeAndDiagonalLength(degree, dl){
		return {
			'x':      dl * Math.cos(PV.mathRadianFromFieldDegree(degree)),
			'y': -1 * dl * Math.sin(PV.mathRadianFromFieldDegree(degree)),
		};
	}
	static vectorRotate(vector_, degree){
		if(! vector_){ console.error('BUG'); }
		const toMathPoint = (p) => {
			return {'x': p.x, 'y': -p.y};
		}
		const toFieldPoint = (p) => {
			return {'x': p.x, 'y': -p.y};
		}

		const rad = PV.radianDiffFromDegreeDiff(degree);
		const vector = toMathPoint(vector_);
		const res = {
			'x':  ((vector.x * Math.cos(rad)) - (vector.y * Math.sin(rad))),
			'y':  ((vector.x * Math.sin(rad)) + (vector.y * Math.cos(rad))),
		};
		// console.log('cvt', degree, rad, (Math.PI/2), vector_, vector, res, toFieldPoint(res));
		return toFieldPoint(res);
	}
	static pointRotationByCenter(point, degree, center){
		const sVec = PV.pointSub(point, center);
		const dVec = PV.vectorRotate(sVec, degree);
		return PV.pointAdd(center, dVec);
	}
	static pointFromDegreeAndDiagonalLength(point, degree, dl){
		const vector = PV.vectorFromDegreeAndDiagonalLength(degree, dl)
		return {'x': point.x + vector.x, 'y': point.y + vector.y,};
	}
	// ** AP
	static AP_newFromPoint(point){
		return {
			'point': deepcopy(point),
			'handle': {
				'kind': AHKIND.None,
				'frontVector': { 'x': 0, 'y': 0 },
				'backVector': { 'x': 0, 'y': 0 },
			},
		};
	}
	static AP_frontVector(ap){
		switch(ap.handle.kind){
			case AHKIND.None:
				return {
					'x': 0,
					'y': 0,
				};
				break;
			case AHKIND.Symmetry:
				return {
					'x': ap.handle.frontVector.x,
					'y': ap.handle.frontVector.y,
				};
				break;
			case AHKIND.Free:
				return {
					'x': ap.handle.frontVector.x,
					'y': ap.handle.frontVector.y,
				};
				break;
			default:
				console.error('BUG', ap.handle.kind);
				return undefined;
			}
	}
	static AP_backVector(ap){
		switch(ap.handle.kind){
			case AHKIND.None:
				return {
					'x': 0,
					'y': 0,
				};
				break;
			case AHKIND.Symmetry:
				return {
					'x': -ap.handle.frontVector.x,
					'y': -ap.handle.frontVector.y,
				};
				break;
			case AHKIND.Free:
				return {
					'x': ap.handle.backVector.x,
					'y': ap.handle.backVector.y,
				};
				break;
			default:
				console.error('BUG', ap.handle.kind);
				return undefined;
			}
	}
	static AP_frontAHPoint(ap) {
		switch(ap.handle.kind){
		case AHKIND.None:
			return {
				'x': ap.point.x,
				'y': ap.point.y,
			};
		case AHKIND.Symmetry:
		case AHKIND.Free:
			return {
				'x': ap.point.x + ap.handle.frontVector.x,
				'y': ap.point.y + ap.handle.frontVector.y,
			};
		default:
			console.error('BUG', ap.handle.kind);
			return undefined;
		}
	}
	static AP_backAHPoint(ap) {
		switch(ap.handle.kind){
		case AHKIND.None:
			return {
				'x': ap.point.x,
				'y': ap.point.y,
			};
		case AHKIND.Symmetry:
			return {
				'x': ap.point.x - ap.handle.frontVector.x,
				'y': ap.point.y - ap.handle.frontVector.y,
			};
		case AHKIND.Free:
			return {
				'x': ap.point.x + ap.handle.backVector.x,
				'y': ap.point.y + ap.handle.backVector.y,
			};
		default:
			console.error('BUG', ap.handle.kind);
			return undefined;
		}
	}
	static rectFromAps(aps){
		let rect = undefined;
		aps.forEach(ap => {
			rect = PV.Rect_includePoint(rect, ap.point);
		});
		return rect;
	}
	// ** bezier
	static Bezier_calcAxis(t, P1, P2, P3, P4){
		let P = (Math.pow((1-t), 3) * P1)
			+ (3 * t * Math.pow((1-t), 2) * P2)
			+ (3 * Math.pow(t, 2) * (1-t) * P3)
			+ (Math.pow(t, 3) * P4);
		return P;
	}
	static Bezier_calcPoint(t, P1, P2, P3, P4){
		return {
			'x': this.Bezier_calcAxis(t, P1.x, P2.x, P3.x, P4.x),
			'y': this.Bezier_calcAxis(t, P1.y, P2.y, P3.y, P4.y),
		};
	}
	// https://pomax.github.io/bezierinfo/ja-JP/index.html#abc
	static Bezier_nearTInBezier(bezierps, point){
		let startt = 0;
		let endt = 1;
		let count = 0;
		while(count < 10000){ // 念のためリミットを入れておく
			count++;
			// 探索範囲を５分割して、一番近い分割点を探す
			let winner = 0;
			let windl = Infinity;
			let ts = [];
			let dls = [];
			for(let i = 0; i < 5; i++){
				const t = (((endt - startt)/4) * i) + startt;
				const bezierPoint = this.Bezier_calcPoint(t, bezierps[0], bezierps[1], bezierps[2], bezierps[3]);
				const dl = this.diagonalLengthFromPoint(PV.pointSub(point, bezierPoint));
				ts.push(t);
				dls.push(dl);
				if(i === 0){
					winner = i;
					windl = dl;
				}else{
					if(windl > dl){
						winner = i;
						windl = dl;
					}
				}
			}
			//console.log('range', ts, winner, startt, endt);

			// 十分な精度の結果が得られたら終了する。
			const startp = PV.Bezier_calcPoint(ts[0], bezierps[0], bezierps[1], bezierps[2], bezierps[3])
			const endp = PV.Bezier_calcPoint(ts[4], bezierps[0], bezierps[1], bezierps[2], bezierps[3])
			if(0.000001 > this.diagonalLengthFromPoint(PV.pointSub(startp, endp))){
				console.log('end', count, startp, endp);
				return ts[3];
			}

			// 一番近かった分割点の前後の分割に範囲を絞って、繰り返す。
			if(0 === winner){
				startt = ts[0];
				endt = ts[1];
			}else if(4 === winner){
				startt = ts[3];
				endt = ts[4];
			}else{
				startt = ts[winner - 1];
				endt = ts[winner + 1];
			}
		}
		// 本当は収束しきらなかったとすべきなのだが、
		// 実用的には（チェックが面倒なので）近い値を返せていれば良しとした。
		return endt;
		//return undefined;
	}
	// AP２点間で与えられた点に一番近い線上の点の情報を返す
	// https://pomax.github.io/bezierinfo/ja-JP/index.html#abc
	static Bezier_nearPosiInBezierBetweenFromApPair(ap0, ap1, point){
		const frontAHPoint = PV.AP_frontAHPoint(ap0);
		const backAHPoint = PV.AP_backAHPoint(ap1);

		let nearPosi = undefined;
		let startt = 0;
		let endt = 1;
		let count = 0;
		while(count < 10000){ // 念のためリミットを入れておく
			count++;
			// 探索範囲を５分割して、一番近い分割点を探す

			let winner = 0;
			let windl = Infinity;
			let ts = [];
			let points = [];
			for(let i = 0; i < 5; i++){
				const t = (((endt - startt)/4) * i) + startt;
				const bezierPoint = this.Bezier_calcPoint(t, ap0.point, frontAHPoint, backAHPoint, ap1.point);
				const dl = this.diagonalLengthFromPoint(PV.pointSub(point, bezierPoint));
				ts.push(t);
				points.push(bezierPoint);
				if(windl > dl){
					winner = i;
					windl = dl;
				}
				if(3 === i){
					nearPosi = {
						'point': bezierPoint,
						't': t,
						'diagonalLength': dl,
					};		
				}
			}
			//console.log('range', ts, winner, startt, endt);

			// 十分な精度の結果が得られたら終了する。
			if(0.000001 > this.diagonalLengthFromPoint(PV.pointSub(points[0], points[4]))){
				//console.log('end', count, points[0], points[4]);
				return nearPosi;
			}

			// 一番近かった分割点の前後の分割に範囲を絞って、繰り返す。
			if(0 === winner){
				startt = ts[0];
				endt = ts[1];
			}else if(4 === winner){
				startt = ts[3];
				endt = ts[4];
			}else{
				startt = ts[winner - 1];
				endt = ts[winner + 1];
			}
		}
		// 本当は収束しきらなかったとすべきなのだが、
		// 実用的には（チェックが面倒なので）近い値を返せていれば良しとした。
		return nearPosi;
		//return undefined;
	}
	// エラーチェックの都合があるので戻り値は全部充足かundefined!
	// https://pomax.github.io/bezierinfo/ja-JP/index.html#splitting
	static splitBezierBetween(t, ap0, ap1){
		const frontAHPoint = PV.AP_frontAHPoint(ap0);
		const backAHPoint = PV.AP_backAHPoint(ap1);

		const pointFromT = (t, p0, p1) => {
			return {
				'x': (1-t) * p0.x + t * p1.x,
				'y': (1-t) * p0.y + t * p1.y,
			};
		};
		const p0FrontAhPoint = pointFromT(t, ap0.point, frontAHPoint);
		const p1BackAhPoint = pointFromT(t, backAHPoint, ap1.point);
		const tmpCenter = pointFromT(t, frontAHPoint, backAHPoint);
		const centerBackAhPoint = pointFromT(t, p0FrontAhPoint, tmpCenter);
		const centerFrontAhPoint = pointFromT(t, tmpCenter, p1BackAhPoint);
		const centerPoint = pointFromT(t, centerBackAhPoint, centerFrontAhPoint);
		
		return {
			'preFrontVector': this.pointSub(p0FrontAhPoint, ap0.point),
			'postBackVector': this.pointSub(p1BackAhPoint, ap1.point),
			'preApHandle': {
				'kind': AHKIND.Free,
				'backVector': this.AP_backVector(ap0),
				'frontVector': this.pointSub(p0FrontAhPoint, ap0.point),
			},
			'centerHandle': {
				'kind': AHKIND.Free,
				'backVector':this.pointSub(centerBackAhPoint, centerPoint),
				'frontVector': this.pointSub(centerFrontAhPoint, centerPoint),
			},
			'postApHandle': {
				'kind': AHKIND.Free,
				'backVector': this.pointSub(p1BackAhPoint, ap1.point),
				'frontVector': this.AP_frontVector(ap1),
			},
			'centerPoint': centerPoint,
		};
	}
	// ** name string
	static sanitizeName(str){
		// 改行およびHTML special characterなどを取り除く。
		str = str.replace(/\r?\n/g, '');
		str = str.replace(/\t/g, '');
		return str.replace(/[&<>"']/gi, '');
	}
	// ** array
	/** arrayをindex付きで逆方向にforループ。ラムダでfalseを返すと中断。
	 * 最後まで中断されなかった場合にtrueを返す
	 * */
	static exfor(arr, fun) {
		for (let index = 0; index < arr.length; index++) {
			const res = fun(index, arr[index], arr);
			if (!res) {
				return false;
			}
		}
		return true;
	}

	// 基本機能はPV.exforと同じで、逆方向にforループ。
	static exforReverse(arr, fun) {
		for (let index = arr.length - 1; 0 <= index; index--) {
			const res = fun(index, arr[index], arr);
			if (!res) {
				return false;
			}
		}
		return true;
	}
	// ** obj
	static removeKeysRecursive(obj, keys)
	{
		if(null === obj){
			console.debug("");
		}else if(typeof obj === 'undefined'){
			console.debug("");
		}else if(obj instanceof Array){
			obj.forEach(function(item){
				PV.removeKeysRecursive(item, keys)
			});
		}else if(typeof obj === 'object'){
			Object.getOwnPropertyNames(obj).forEach(function(key){
				if(keys.indexOf(key) !== -1)delete obj[key];
				else PV.removeKeysRecursive(obj[key], keys);
			});
		}
	}
}