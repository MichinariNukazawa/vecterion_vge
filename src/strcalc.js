"use strict";

/**
 * TODO regex上の数値判定が甘い(regexでする必要はないが、チェックは追加したい？)。
 */ 

//export class StrCalc{
module.exports = class StrCalc{
	static calc(str){
		//console.log(`call StrCalc.calc '${str}'`)
		return parseFloat(StrCalc.calc_(str));
	}

	static calc_(str){
		//console.debug(`call '${str}'`)
		// 数字のみの場合はそれを返して終了
		if(/^\s*[-+]?[.0-9]+\s*$/.test(str)){
			return str;
		}

		// 括弧があれば中身を数字に置換する
		{
			// いちばん内側の括弧を１つ処理して、同レベル複数や入れ子は再起に任せてしまう
			const reBracket = /^(.*)\(([^\(\)]+)\)(.*)$/;
			const r = reBracket.exec(str);
			if(null !== r){
				//console.debug(`bracket '${r[2]}'`)
				const sres = StrCalc.calc_(r[2]);
				const nstr = r[1] + sres + r[3]
				//console.debug(`bracket replaced '${nstr}'`);
				return StrCalc.calc_(nstr);
			}
		}

		// 先頭を見て、単純な計算式（四則演算）であれば計算して文字列を結果に置換
		// 四則演算はいちおう（+*間など）優先順位があり駆け引きが先。
		{ // 駆け引き
			const reFormula = /^(.*?)([-.0-9]+)\s*([\*\/])\s*([-.0-9]+)(.*?)$/;
			const r = reFormula.exec(str);
			if(null !== r){

				const rv = parseFloat(r[2])
				const lv = parseFloat(r[4])
				let res = NaN;
				switch(r[3]){
				case '*':
					res = rv * lv;
					break;
				case '/':
					res = rv / lv;
					break;
				dafault: // BUG
					console.error(`BUG:'${str}' '${r[3]}'`)
					return NaN;
				}

				const nstr = r[1] + res.toString(10) + r[5];
				//console.debug(`simple formula(kakehiku) replaced '${nstr}'`);
				return StrCalc.calc_(nstr);
			}
		}
		{ // 足す引く
			const reFormula = /^(.*?)([-.0-9]+)\s*([-+])\s*([-.0-9]+)(.*?)$/;
			const r = reFormula.exec(str);
			if(null !== r){
				const rv = parseFloat(r[2])
				const lv = parseFloat(r[4])
				let res = NaN;
				switch(r[3]){
				case '+':
					res = rv + lv;
					break;
				case '-':
					res = rv - lv;
					break;
				dafault: // BUG
					console.error(`BUG:'${str}' '${r[3]}'`)
					return NaN;
				}

				const nstr = r[1] + res.toString(10) + r[5];
				//console.debug(`simple formula(+-) replaced '${nstr}'`);
				return StrCalc.calc_(nstr);
			}
		}

		//console.debug(`not formula '${str}'`)
		return NaN;
	}
}