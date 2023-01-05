"use strict";

let lodash = require('lodash');
const clonedeep = require('lodash/cloneDeep');
function deepcopy(obj) { return clonedeep(obj); }
const DOC_DEFAULT = require('./doc_default.json');

module.exports = class History {
	constructor(callback, doc) {
		console.log('History.constructor')
		this.callback = callback;
		this.MAX_HIST = 10;
		this.setDoc(doc);
	}

	setDoc(doc_){
		const doc = (!! doc_) ? doc_ : DOC_DEFAULT;
		this.currentDoc = deepcopy(doc);

		this.currentDoc = History.conversionDoc_(this.currentDoc);

		this.hist = [{ 'cause': (!! doc_) ? 'load doc':'new doc', 'doc': History.removeCache(deepcopy(this.currentDoc)), },];
		this.histIndex = 0;
		History.updatePrevItemCache(this.currentDoc);

		setTimeout(() => { // コンストラクタ内で直接呼び出すと、メンバの初期化が完了しておらずエラーを起こすため
			this.callback((!! doc_) ? 'init load':'init', this.histIndex + 1, this.hist.length);
		}, 0);
	}

	now() {
		return this.currentDoc;
	}
	prev() {
		return deepcopy(this.hist[this.histIndex].doc);
	}
	stackNum(){
		return this.histIndex;
	}

	static conversionDoc_(doc){
		// 現行Document構造に新規追加されたキーを、docに付与する。
		let def = deepcopy(DOC_DEFAULT);
		delete def.items;
		return lodash.merge({}, def, doc);
	}

	static updatePrevItemCache(doc){
		doc.items.forEach(item => {
			if(! item.hasOwnProperty('cache')){
				item.cache = {};
			}
			item.cache.prevItems = [deepcopy(item)];
		});
	}
	static removeCache(doc){
		doc.items.forEach(item => {
			this.removeKeysRecursive(item, ['cache']);
		});
		return doc;
	}

	static removeKeysRecursive(obj, keys)
	{
		if(null === obj){
			console.debug("");
		}else if(typeof obj === 'undefined'){
			console.debug("");
		}else if(obj instanceof Array){
			obj.forEach(function(item){
				History.removeKeysRecursive(item, keys)
			});
		}else if(typeof obj === 'object'){
			Object.getOwnPropertyNames(obj).forEach(function(key){
				if(keys.indexOf(key) !== -1)delete obj[key];
				else{
					History.removeKeysRecursive(obj[key], keys);
				}
			});
		}
	}

	stacking(cause) {
		// drop redo hists.
		this.hist.length = (this.histIndex + 1);
		// stacking	
		this.histIndex++;
		this.hist.push({ 'cause': cause, 'doc': History.removeCache(deepcopy(this.currentDoc)) });
		History.updatePrevItemCache(this.currentDoc);
		// TODO histが規定数を超えたら古い履歴を削除する
		if (this.MAX_HIST < this.hist.length) {
			this.hist.shift(); // this.hist.length = this.MAX_HIST;
			this.histIndex = this.MAX_HIST - 1;
			if ((this.hist.length - 1) !== this.histIndex) { console.error('BUG', this.histIndex, this.hist.length) }
		}
		console.log(`History.stacking: ${cause}`);
		this.callback('stacking', this.histIndex + 1, this.hist.length);
	}
	// 直前のHistoryと同じ要因でのstackの場合、Historyに積まずにDocだけ更新する(ex. テンキー押下での移動)
	stackingCheckDup(cause) {
		if(this.hist.at(this.histIndex).cause !== cause){
			this.stacking(cause)
			return;
		}
		// drop redo hists.
		this.hist.length = (this.histIndex + 1);
		// stacking dup
		this.hist[this.histIndex] = { 'cause': cause, 'doc': History.removeCache(deepcopy(this.currentDoc)) };
		console.log(`History.stacking dup: ${cause}`);
		this.callback('stacking', this.histIndex + 1, this.hist.length);
	}
	// Redoスタックの残らないUndo
	// (中断において状態をクリンアップする処理を実装するために呼び出し側で一旦完了処理をするのが簡素で、
	// その都合からHistory側からは巻き戻し有無の判断がつかないため、呼び出し側でStackingしたかをチェックしてこれを呼び出す。)
	cancelRoolback() {
		if (0 == this.histIndex) {
			console.log('undo: history empty')
			this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
			History.updatePrevItemCache(this.currentDoc);
			return true;
		}
		this.histIndex--;
		this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
		History.updatePrevItemCache(this.currentDoc);
		this.hist.length = hist.histIndex + 1; // 上に乗っているRedo１回分を削除
		console.log(`History.undo: ${this.histIndex + 1}/${this.hist.length}`)
		this.callback('undo', this.histIndex + 1, this.hist.length);
		return true;
	}
	cancelNow(){
		this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
		History.updatePrevItemCache(this.currentDoc);
	}
	undo() {
		if (0 == this.histIndex) {
			console.log('undo: history empty')
			this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
			History.updatePrevItemCache(this.currentDoc);
			return true;
		}
		this.histIndex--;
		this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
		History.updatePrevItemCache(this.currentDoc);
		console.log(`History.undo: ${this.histIndex + 1}/${this.hist.length}`)
		this.callback('undo', this.histIndex + 1, this.hist.length);
		return true;
	}
	redo() {
		if (!((this.hist.length - 1) > this.histIndex)) {
			console.log(`redo: history empty ${this.histIndex}/${this.hist.length}`)
			return false;
		}
		this.histIndex++;
		this.currentDoc = deepcopy(this.hist[this.histIndex].doc);
		History.updatePrevItemCache(this.currentDoc);
		console.log(`History.redo: ${this.histIndex + 1}/${this.hist.length}`)
		this.callback('redo', this.histIndex + 1, this.hist.length);
		return true;
	}
};