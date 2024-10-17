let tests = require("tests");
let math = require("math");

// math.EPSILON on Flipper Zero is 2.22044604925031308085e-16

// basics
tests.assert_float_close(5, math.abs(-5), math.EPSILON);
tests.assert_float_close(0.5, math.abs(-0.5), math.EPSILON);
tests.assert_float_close(5, math.abs(5), math.EPSILON);
tests.assert_float_close(0.5, math.abs(0.5), math.EPSILON);
tests.assert_float_close(3, math.cbrt(27), math.EPSILON);
tests.assert_float_close(6, math.ceil(5.3), math.EPSILON);
tests.assert_float_close(31, math.clz32(1), math.EPSILON);
tests.assert_float_close(5, math.floor(5.7), math.EPSILON);
tests.assert_float_close(5, math.max(3, 5), math.EPSILON);
tests.assert_float_close(3, math.min(3, 5), math.EPSILON);
tests.assert_float_close(-1, math.sign(-5), math.EPSILON);
tests.assert_float_close(5, math.trunc(5.7), math.EPSILON);

// trig
tests.assert_float_close(1.0471975511965976, math.acos(0.5), math.EPSILON);
tests.assert_float_close(1.3169578969248166, math.acosh(2), math.EPSILON);
tests.assert_float_close(0.5235987755982988, math.asin(0.5), math.EPSILON);
tests.assert_float_close(1.4436354751788103, math.asinh(2), math.EPSILON);
tests.assert_float_close(0.7853981633974483, math.atan(1), math.EPSILON);
tests.assert_float_close(0.7853981633974483, math.atan2(1, 1), math.EPSILON);
tests.assert_float_close(0.5493061443340549, math.atanh(0.5), math.EPSILON);
tests.assert_float_close(-1, math.cos(math.PI), math.EPSILON * 18); // Error 3.77475828372553223744e-15
tests.assert_float_close(1, math.sin(math.PI / 2), math.EPSILON * 4.5); // Error 9.99200722162640886381e-16

// powers
tests.assert_float_close(5, math.sqrt(25), math.EPSILON);
tests.assert_float_close(8, math.pow(2, 3), math.EPSILON);
tests.assert_float_close(2.718281828459045, math.exp(1), math.EPSILON * 2); // Error 4.44089209850062616169e-16
