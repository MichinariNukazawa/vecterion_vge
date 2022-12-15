var assert = require("power-assert"); // assertモジュールのinclude
//import { StrCalc } from "../js/strcalc.js";
var StrCalc = require("../src/strcalc.js");

it ("calc", function() {
	assert(2 === StrCalc.calc('2'));
	assert(2 === StrCalc.calc('+2'));
	assert(-2 === StrCalc.calc('-2'));
	assert(-2.1 === StrCalc.calc('-2.1'));
	assert(2 === StrCalc.calc('1 + 1'));
	assert(12.5 === StrCalc.calc('2.4 + 10.1'));
	assert(0.001 > Math.abs(7.7 - StrCalc.calc('10.1 - 2.4')));
	assert(23 === StrCalc.calc('13 + 10'));
	assert(13 === StrCalc.calc('23 - 10'));
	assert(13 === StrCalc.calc('23-10'));
	assert(13 === StrCalc.calc('23 +-10'));
	assert(33 === StrCalc.calc('23 - -10'));
	//assert(33 === StrCalc.calc('23--10')); // これは微妙なところ...
	assert(120 === StrCalc.calc('10 * 12'));
	assert(10 === StrCalc.calc('200 / 20'));
	assert(7 === StrCalc.calc('1 + 2 + 4'));
	assert(4 === StrCalc.calc('2 + 3 - 1'));
	assert(2 === StrCalc.calc('(1 + 1)'));
	assert(3 === StrCalc.calc('(1 + 1) + 1'));
	assert(2 === StrCalc.calc('((1 + 1))'));
	assert(1 === StrCalc.calc('((1))'));
	assert((-1/6) === StrCalc.calc('((2 - 3) / 6)'));
	assert((1.5) === StrCalc.calc('(2 - (3 / 6))'));
	assert(-5 === StrCalc.calc('((7 - 2) * (4 - 5))'));
	assert(-5 === StrCalc.calc('(7 - 2) * (4 - 5)'));
	assert(400 === StrCalc.calc('1 + 21 * 19')); // 1 + 399 = 400 // 駆け引きが先
	assert(0.5 === StrCalc.calc('3/3/2'));
});

it ("calc isNaN", function() {
	assert(isNaN( StrCalc.calc('a') ));
	assert(isNaN( StrCalc.calc('a - 1') ));
	assert(isNaN( StrCalc.calc('((7 - 2) * (4 - 5)') ));
	assert(isNaN( StrCalc.calc('((7 - 2) * 4 - 5))') ));
});