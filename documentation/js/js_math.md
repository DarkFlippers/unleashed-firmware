# js_math {#js_math}

# Math module
```js
let math = require("math");
```
# Constants

## PI
The number π = 3.14159265358979323846264338327950288.

## E
The number e (Euler's number) = 2.71828182845904523536028747135266250.

## EPSILON
The smallest number that satisfies the condition: 1.0 + EPSILON != 1.0.
EPSILON = 2.2204460492503131e-16.

# Methods

## abs
Return the absolute value of a number.

### Parameters
- x: A number

### Returns
The absolute value of `x`. If `x` is negative (including -0), returns `-x`. Otherwise, returns `x`. The result is therefore always a positive number or 0.

### Example
```js
math.abs(-5); // 5
```

## acos
Return the inverse cosine (in radians) of a number.

### Parameters
- x: A number between -1 and 1, inclusive, representing the angle's cosine value

### Returns
The inverse cosine (angle in radians between 0 and π, inclusive) of `x`. If `x` is less than -1 or greater than 1, returns `NaN`.

### Example
```js
math.acos(-1); // 3.141592653589793
```

## acosh
Return the inverse hyperbolic cosine of a number.

### Parameters
- x: A number greater than or equal to 1

### Returns
The inverse hyperbolic cosine of `x`.

### Example
```js
math.acosh(1); // 0
```

## asin
Return the inverse sine (in radians) of a number.

### Parameters
- x: A number between -1 and 1, inclusive, representing the angle's sine value

### Returns
The inverse sine (angle in radians between -𝜋/2 and 𝜋/2, inclusive) of `x`.

### Example
```js
math.asin(0.5); // 0.5235987755982989
```

## asinh
Return the inverse hyperbolic sine of a number.

### Parameters
- x: A number

### Returns
The inverse hyperbolic sine of `x`.

### Example
```js
math.asinh(1); // 0.881373587019543
```

## atan
Return the inverse tangent (in radians) of a number.

### Parameters
- x: A number

### Returns
The inverse tangent (angle in radians between -𝜋/2 and 𝜋/2, inclusive) of `x`.

### Example
```js
math.atan(1); // 0.7853981633974483
```

## atan2
Return the angle in the plane (in radians) between the positive x-axis and the ray from (0, 0) to the point (x, y), for math.atan2(y, x).

### Parameters
- y: The y coordinate of the point
- x: The x coordinate of the point

### Returns
The angle in radians (between -π and π, inclusive) between the positive x-axis and the ray from (0, 0) to the point (x, y).

### Example
```js
math.atan2(90, 15); // 1.4056476493802699
```

## atanh
The method returns the inverse hyperbolic tangent of a number.

### Parameters
- x: A number between -1 and 1, inclusive

### Returns
The inverse hyperbolic tangent of `x`.

### Example
```js
math.atanh(0.5); // 0.5493061443340548
```

## cbrt
Return the cube root of a number.

### Parameters
- x: A number

### Returns
The cube root of `x`.

### Example
```js
math.cbrt(2); // 1.2599210498948732
```

## ceil
Round up and return the smallest integer greater than or equal to a given number.

### Parameters
- x: A number

### Returns
The smallest integer greater than or equal to `x`. It's the same value as `-math.floor(-x)`.

### Example
```js
math.ceil(-7.004); // -7
math.ceil(7.004);  // 8
```

## clz32
Return the number of leading zero bits in the 32-bit binary representation of a number.

### Parameters
- x: A number

### Returns
The number of leading zero bits in the 32-bit binary representation of `x`.

### Example
```js
math.clz32(1);    // 31
math.clz32(1000); // 22
```

## cos
Return the cosine of a number in radians.

### Parameters
- x: A number representing an angle in radians

### Returns
The cosine of `x`, between -1 and 1, inclusive.

### Example
```js
math.cos(math.PI); // -1
```

## exp
Return e raised to the power of a number.

### Parameters
- x: A number

### Returns
A nonnegative number representing `e^x`, where `e` is the base of the natural logarithm.

### Example
```js
math.exp(0); // 1
math.exp(1); // 2.718281828459045
```

## floor
Round down and return the largest integer less than or equal to a given number.

### Parameters
- x: A number

### Returns
The largest integer smaller than or equal to `x`. It's the same value as `-math.ceil(-x)`.

### Example
```js
math.floor(-45.95); // -46
math.floor(-45.05); // -46
math.floor(-0); // -0
math.floor(0); // 0
math.floor(45.05); // 45
math.floor(45.95); // 45
```

## is_equal
Return true if the difference between numbers `a` and `b` is less than the specified parameter `e`.

### Parameters
- a: A number a
- b: A number b
- e: An epsilon parameter

### Returns
True if the difference between numbers `a` and `b` is less than the specified parameter `e`. Otherwise, false.

### Example
```js
math.is_equal(1.4, 1.6, 0.2);      // false
math.is_equal(3.556, 3.555, 0.01); // true
```

## max
Return the largest of two numbers given as input parameters.

### Parameters
- a: A number a
- b: A number b

### Returns
The largest of the given numbers.

### Example
```js
math.max(10, 20);   // 20
math.max(-10, -20); // -10
```

## min
Return the smallest of two numbers given as input parameters.

### Parameters
- a: A number a
- b: A number b

### Returns
The smallest of the given numbers.

### Example
```js
math.min(10, 20);   // 10
math.min(-10, -20); // -20
```

## pow
Return the value of a base raised to a power.

### Parameters
- base: The base number
- exponent: The exponent number

### Returns
A number representing base taken to the power of exponent.

### Example
```js
math.pow(7, 2);  // 49
math.pow(7, 3);  // 343
math.pow(2, 10); // 1024
```

## random
Return a floating-point, pseudo-random number that's greater than or equal to 0 and less than 1, with approximately uniform distribution over that range - which you can then scale to your desired range.

### Returns
A floating-point, pseudo-random number between 0 (inclusive) and 1 (exclusive).

### Example
```js
let num = math.random();
```

## sign
Return 1 or -1, indicating the sign of the number passed as argument.

### Parameters
- x: A number

### Returns
-1 if the number is less than 0, and 1 otherwise.

### Example
```js
math.sign(3);  // 1
math.sign(0);  // 1
math.sign(-3); // -1
```

## sin
Return the sine of a number in radians.

### Parameters
- x: A number representing an angle in radians

### Returns
The sine of `x`, between -1 and 1, inclusive.

### Example
```js
math.sin(math.PI / 2); // 1
```

## sqrt
Return the square root of a number.

### Parameters
- x: A number greater than or equal to 0

### Returns
The square root of `x`, a nonnegative number. If `x` < 0, script will fail with an error.

### Example
```js
math.sqrt(25); // 5
```

## trunc
Return the integer part of a number by removing any fractional digits.

### Parameters
- x: A number

### Returns
The integer part of `x`.

### Example
```js
math.trunc(-1.123); // -1
math.trunc(0.123);  // 0
math.trunc(13.37);  // 13
math.trunc(42.84);  // 42
```
