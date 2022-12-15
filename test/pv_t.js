var assert = require("power-assert"); // assertモジュールのinclude
//import { StrCalc } from "../js/strcalc.js";
const PV = require("../src/pv.js");

function fuzzyEqual(a, b) { return 0.0001 > Math.abs(a - b); }
function fuzzyEqualPoint(a, b) { return fuzzyEqual(a.x, b.x) && fuzzyEqual(a.y, b.y); }
function degSan(degree){
	return PV.sanitizeDegree(degree);
}
function radSan(radian){
	while(0 > radian){
		radian += 2*Math.PI;
	}
	while(2*Math.PI < radian){
		radian -= 2*Math.PI;
	}
	return radian;
}

it ("deg rad", function() {
	assert(fuzzyEqual(0, radSan( PV.mathRadianFromFieldDegree(90) )));
	assert(fuzzyEqual((Math.PI/2), radSan( PV.mathRadianFromFieldDegree(0) )));
	assert(fuzzyEqual(90,  degSan( PV.fieldDegreeFromMathRadian(0) ) ));
	assert(fuzzyEqual(0,   degSan( PV.fieldDegreeFromMathRadian((Math.PI/2)) ) ));
	assert(fuzzyEqual(270, degSan( PV.fieldDegreeFromMathRadian((Math.PI)) ) ));
	assert(fuzzyEqual(0, radSan( PV.radianDiffFromDegreeDiff(0) )));
	assert(fuzzyEqual((Math.PI/2), radSan( PV.radianDiffFromDegreeDiff(-90) )));
});

it ("deg", function() {
	assert(fuzzyEqual(10, PV.sanitizeDegree(370) ));
	assert(fuzzyEqual(350, PV.sanitizeDegree(-10) ));
	assert(fuzzyEqual(15, PV.absMinDiffByDegreeAndDegree(90, 105) ));
	assert(fuzzyEqual(15, PV.absMinDiffByDegreeAndDegree(90, 75) ));
	assert(fuzzyEqual(  0, degSan( PV.fieldDegreeFromVector({'x':    0,'y': -100}) ) ));
	assert(fuzzyEqual( 45, degSan( PV.fieldDegreeFromVector({'x':  100,'y': -100}) ) ));
	assert(fuzzyEqual( 90, degSan( PV.fieldDegreeFromVector({'x':  100,'y':    0}) ) ));
	assert(fuzzyEqual(180, degSan( PV.fieldDegreeFromVector({'x':    0,'y':  100}) ) ));
	assert(fuzzyEqual(  0, degSan( PV.fieldDegreeFromVector({'x':    0,'y':    0}) ) ));
});

it ("rotate", function() {
	assert(fuzzyEqualPoint( {'x':    0, 'y': -100}, PV.vectorFromDegreeAndDiagonalLength(0, 100) ));
	assert(fuzzyEqualPoint( {'x': -100, 'y':    0}, PV.vectorFromDegreeAndDiagonalLength(-90, 100) ));
	assert(fuzzyEqualPoint( {'x':  100, 'y': -100}, PV.vectorFromDegreeAndDiagonalLength(45, 141.421356) ));
	assert(fuzzyEqualPoint( {'x':    0, 'y': -100}, PV.vectorRotate({'x': 0, 'y': -100},0   ) ));
	assert(fuzzyEqualPoint( {'x':    0, 'y':  100}, PV.vectorRotate({'x': 0, 'y':  100},0   ) ));
	assert(fuzzyEqualPoint( {'x':  100, 'y':    0}, PV.vectorRotate({'x': 0, 'y': -100},90  ) ));
	assert(fuzzyEqualPoint( {'x': -100, 'y':    0}, PV.vectorRotate({'x': 0, 'y':  100},90  ) ));
	assert(fuzzyEqualPoint( {'x':    0, 'y': -100}, PV.vectorRotate({'x': 0, 'y':  100},180 ) ));
	assert(fuzzyEqualPoint( {'x':  100, 'y':    0}, PV.vectorRotate({'x': 0, 'y':  100},270 ) ));
});

it ("bezier", function() {
	assert(fuzzyEqual(   0, PV.Bezier_calcAxis(  0, 0, 10, 10, 0) ));
	assert(fuzzyEqual(   0, PV.Bezier_calcAxis(1.0, 0, 10, 10, 0) ));
	assert(fuzzyEqual( 7.5, PV.Bezier_calcAxis(0.5, 0, 10, 10, 0) ));
	const bezierps = [{'x': 0, 'y': 0}, {'x': 0, 'y': 10}, {'x': 10, 'y': 10}, {'x': 10, 'y': 0}, ]
	let t;
	t = PV.Bezier_nearTInBezier(bezierps, {'x': 5, 'y': 20});
	assert(fuzzyEqual( 0.5, t));
	p = PV.Bezier_calcPoint(t, bezierps[0], bezierps[1], bezierps[2], bezierps[3])
	assert(fuzzyEqualPoint( {'x':  5, 'y': 7.5}, p));
});

