"use strict";

let lodash = require('lodash');
const clonedeep = require('lodash/cloneDeep');
function deepcopy(obj) { return clonedeep(obj); }
const DOC_DEFAULT = require('./doc_default.json');
const PACKAGE = require('./../package.json');
const History = require("../src/history.js");

module.exports = class Strage{
	constructor(callback) {
		console.log('Strage.constructor')
		this.callback = callback;
		// このブラウザタブのcurrentDoc
		// 最初は最後に編集されたDoc（docinfosの先頭）を開く。
		this.currentDocId = -1;

		console.log('Strage', `used:${(Strage.getUsedSpaceOfLocalStorageInBytes() / 1024).toFixed(2)} kib`);
	}

	check(){
		let isOk = true;
		try{
			localStorage.setItem('vecterion-check-ls', 'doubiju');
			isOk = ('doubiju' === localStorage.getItem('vecterion-check-ls'));
			localStorage.removeItem('vecterion-check-ls');
		}catch (error){
			console.error(error);
			isOk = false;
		}
		if(! isOk){
			alert('!! Save not working !!\n try update your browser.');
		}
	}

	getCurrentDocId(){
		return this.currentDocId;
	}

	// べつにファイル名は重複して良い
	newDocName(src){
		if((undefined === src) || ('' === src)){
			src = 'new document';
		}
		const docconf = this.loadDocConfig();
		if(! docconf){
			return src;
		}
		// 重複しないIDを探す
		let head = src;
		let count = -1;
		const m = src.match(/^(.*)([0-9]+)$/);
		if(!! m){
			head = m[1];
			count = parseInt(m[2], 10);
		}
		let name = src;

		while(docconf.docinfos.some(docinfo => docinfo.name === name)){
			count++;
			name = `${head}${count}`;
		}
		return name;
	}
	newDocInfo(){
		let id = 0;
		const docconf = this.loadDocConfig();
		if(!! docconf){ // 重複しないIDを探す
			while(docconf.docinfos.some(docinfo => docinfo.id === id)){
				id++;
			}
		}
		console.log(`new document id: ${id}`);
		return {
			'id': id,
			'name': '',
			'latestUpdate': new Date().getTime(),
		};
	}

	static newDocConf(){
		return {
			'appname': PACKAGE.name,
			'appversion': PACKAGE.version,
			'docinfos': [],
		};
	}

	loadDocConfig(){
		const sdocconf = localStorage.getItem('vecterion-doc-config');
		if(! sdocconf){
			console.log('docconf nothing');
			return undefined;
		}
		let docconf;
		docconf = JSON.parse(sdocconf);
		// validation
		// TODO more validation
		if((! docconf.docinfos) || (0 === docconf.docinfos.length)){
			console.warn('validate error: docconf.docinfos', docconf);
			return undefined;
		}

		console.log('docconf loaded', docconf);
		return docconf;
	}
	loadOrNewDocConfig(){
		let docconf = this.loadDocConfig();
		if(! docconf){
			docconf = Strage.newDocConf();
			docconf.docinfos.unshift(this.newDocInfo());
			console.log('docconf initialized', docconf);
			this.callback(docconf.docinfos.at(0));
		}
		return docconf;
	}
	setCurrentDocument(id){
		let docconf = this.loadDocConfig();
		if(! docconf){
			console.error('BUG');
			return;
		}
		const ix = docconf.docinfos.findIndex(e => e.id === id);
		if(-1 === ix){
			console.error('BUG', docconf);
			return;
		}

		const [currentdocinfo] = docconf.docinfos.splice(ix, 1);
		docconf.docinfos.splice(0, currentdocinfo);

		if(this.currentDocId === id){
			console.log('id is already currentDoc', id);
			return;
		}
		this.currentDocId = id;
		this.callback(currentdocinfo);
	}
	saveCurrentDoc(doc){
		let docconf = this.loadOrNewDocConfig();
		let currentdocinfo = docconf.docinfos.find(e => e.id == this.getCurrentDocId());
		if(! currentdocinfo){
			console.error('BUG', this.getCurrentDocId(), docconf);
			return;
		}
		currentdocinfo.latestUpdate = new Date().getTime();

		try{
			this.setItemDoc_(currentdocinfo.id, doc);
			localStorage.setItem('vecterion-doc-config', JSON.stringify(docconf, null, '\t'));
		}catch(e){
			console.warn(e);
			alert('error: localStrage full. try delete document.')
		}
		this.callback(currentdocinfo);
	}

	saveCurrentDocumentName(name){
		let docconf = this.loadOrNewDocConfig()
		let currentdocinfo = docconf.docinfos.find(e => e.id == this.getCurrentDocId());
		if(! currentdocinfo){
			console.error('BUG', this.getCurrentDocId(), docconf);
			return;
		}
		currentdocinfo.latestUpdate = new Date().getTime();

		currentdocinfo.name = PV.sanitizeName(name);
		try{
				localStorage.setItem('vecterion-doc-config', JSON.stringify(docconf, null, '\t'));
		}catch(e){
			console.warn(e);
			alert('error: localStrage full. try delete document.')
		}
		this.callback(currentdocinfo);
		return true;
	}

	loadOrNewCurrentDocInfo(){
		let docconf = this.loadDocConfig()
		if(! docconf){
			docconf = this.loadOrNewDocConfig()
			this.currentDocId = 0;
			const currentDocInfo = docconf.docinfos.at(0);
			this.callback(currentDocInfo);
			return currentDocInfo;
		}

		if(-1 === this.getCurrentDocId()){
			this.currentDocId = docconf.docinfos.at(0).id;
			console.log(`initialized doc select`, this.getCurrentDocId());
		}
		const currentdocinfo = docconf.docinfos.find(e => e.id == this.getCurrentDocId());
		if(! currentdocinfo){
			console.log('BUG', this.getCurrentDocId(), docconf);
		}
		this.callback(currentdocinfo);
		return currentdocinfo;
	}
	loadOrNewCurrentDoc(){
		const currentdocinfo = this.loadOrNewCurrentDocInfo();
		const dockey = `vecterion-doc-${currentdocinfo.id}`;
		const sdoc = localStorage.getItem(dockey)
		if(! sdoc){
			console.log('localStrage doc empty.', dockey);
			return deepcopy(DOC_DEFAULT);
		}
		const doc = JSON.parse(sdoc);
		console.log(`loaded doc ${dockey}`)
		return doc;
	}

	addCurrentDocument(doc, name){
		let docconf = this.loadDocConfig();
		if(! docconf){
			docconf = Strage.newDocConf();
		}
		let newdocinfo = this.newDocInfo();
		newdocinfo.name = name;
		docconf.docinfos.unshift(newdocinfo);
		const dockey = `vecterion-doc-${newdocinfo.id}`;
		const sdoc = localStorage.getItem(dockey)
		if(!! sdoc){
			console.error('BUG', `already exist '${dockey}'`);
		}
		this.currentDocId = newdocinfo.id;

		try{
			this.setItemDoc_(newdocinfo.id, doc)
			localStorage.setItem('vecterion-doc-config', JSON.stringify(docconf, null, '\t'));
		}catch(e){
			console.warn(e);
			alert('error: localStrage full. try delete document.')
		}
		this.callback(newdocinfo);
	}

	deleteDocId(id){
		const dockey = `vecterion-doc-${id}`;
		localStorage.removeItem(dockey)
		const dockeyThumb = `vecterion-doc-thumb-${id}`;
		localStorage.removeItem(dockeyThumb)
		try{
			localStorage.setItem('vecterion-doc-config', JSON.stringify(docconf, null, '\t'));
		}catch(e){
			console.warn(e);
			alert('error: localStrage full. try delete document.')
		}
	}

	setItemDoc_(id, doc_){
		const dockey = `vecterion-doc-${id}`;
		let doc = History.removeCache(deepcopy(doc_));
		doc.application = {
			"name": PACKAGE.name,
			"subkind": "webapp",
			"version": PACKAGE.version,
		};
		localStorage.setItem(dockey, JSON.stringify(doc));
	}

	static getUsedSpaceOfLocalStorageInBytes() {
		// Returns the total number of used space (in Bytes) of the Local Storage
		var b = 0;
		for (var key in window.localStorage) {
			if (window.localStorage.hasOwnProperty(key)) {
				b += key.length + localStorage.getItem(key).length;
			}
		}
		return b;
	}
}