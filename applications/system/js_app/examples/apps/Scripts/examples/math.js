let math = require("math");

print("math.abs(-5):", math.abs(-5));
print("math.acos(0.5):", math.acos(0.5));
print("math.acosh(2):", math.acosh(2));
print("math.asin(0.5):", math.asin(0.5));
print("math.asinh(2):", math.asinh(2));
print("math.atan(1):", math.atan(1));
print("math.atan2(1, 1):", math.atan2(1, 1));
print("math.atanh(0.5):", math.atanh(0.5));
print("math.cbrt(27):", math.cbrt(27));
print("math.ceil(5.3):", math.ceil(5.3));
print("math.clz32(1):", math.clz32(1));
print("math.cos(math.PI):", math.cos(math.PI));
print("math.exp(1):", math.exp(1));
print("math.floor(5.7):", math.floor(5.7));
print("math.max(3, 5):", math.max(3, 5));
print("math.min(3, 5):", math.min(3, 5));
print("math.pow(2, 3):", math.pow(2, 3));
print("math.random():", math.random());
print("math.sign(-5):", math.sign(-5));
print("math.sin(math.PI/2):", math.sin(math.PI / 2));
print("math.sqrt(25):", math.sqrt(25));
print("math.trunc(5.7):", math.trunc(5.7));

// Unit tests. Please add more if you have time and knowledge.
// math.EPSILON on Flipper Zero is 2.22044604925031308085e-16

let succeeded = 0;
let failed = 0;

function test(text, result, expected, epsilon) {
    let is_equal = math.is_equal(result, expected, epsilon);
    if (is_equal)  {
        succeeded += 1;
    } else {
        failed += 1;
        print(text, "expected", expected, "got", result);
    }
}

test("math.abs(5)", math.abs(-5), 5, math.EPSILON);
test("math.abs(0.5)", math.abs(-0.5), 0.5, math.EPSILON);
test("math.abs(5)", math.abs(5), 5, math.EPSILON);
test("math.abs(-0.5)", math.abs(0.5), 0.5, math.EPSILON);
test("math.acos(0.5)", math.acos(0.5), 1.0471975511965976, math.EPSILON);
test("math.acosh(2)", math.acosh(2), 1.3169578969248166, math.EPSILON);
test("math.asin(0.5)", math.asin(0.5), 0.5235987755982988, math.EPSILON);
test("math.asinh(2)", math.asinh(2), 1.4436354751788103, math.EPSILON);
test("math.atan(1)", math.atan(1), 0.7853981633974483, math.EPSILON);
test("math.atan2(1, 1)", math.atan2(1, 1), 0.7853981633974483, math.EPSILON);
test("math.atanh(0.5)", math.atanh(0.5), 0.5493061443340549, math.EPSILON);
test("math.cbrt(27)", math.cbrt(27), 3, math.EPSILON);
test("math.ceil(5.3)", math.ceil(5.3), 6, math.EPSILON);
test("math.clz32(1)", math.clz32(1), 31, math.EPSILON);
test("math.floor(5.7)", math.floor(5.7), 5, math.EPSILON);
test("math.max(3, 5)", math.max(3, 5), 5, math.EPSILON);
test("math.min(3, 5)", math.min(3, 5), 3, math.EPSILON);
test("math.pow(2, 3)", math.pow(2, 3), 8, math.EPSILON);
test("math.sign(-5)", math.sign(-5), -1, math.EPSILON);
test("math.sqrt(25)", math.sqrt(25), 5, math.EPSILON);
test("math.trunc(5.7)", math.trunc(5.7), 5, math.EPSILON);
test("math.cos(math.PI)", math.cos(math.PI), -1, math.EPSILON * 18); // Error 3.77475828372553223744e-15
test("math.exp(1)", math.exp(1), 2.718281828459045, math.EPSILON * 2); // Error 4.44089209850062616169e-16
test("math.sin(math.PI / 2)", math.sin(math.PI / 2), 1, math.EPSILON * 4.5); // Error 9.99200722162640886381e-16

if (failed > 0) {
    print("!!!", failed, "Unit tests failed !!!");
}