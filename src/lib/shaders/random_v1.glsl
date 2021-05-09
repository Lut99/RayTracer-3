/* RANDOM V 1.glsl
 *   by Lut99
 *
 * Created:
 *   09/05/2021, 16:59:35
 * Last edited:
 *   09/05/2021, 17:14:41
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Library shader file that contains functions to generate random float
 *   numbers in the range 0 (inclusive) to 1 (exclusive).
**/

// #version 450

#ifndef RANDOM_GLSL
#define RANDOM_GLSL

/* Simple hash function that hashes a given uint. */
uint _random_hash(uint x) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}
/* Extension of the simple hash function that works on unsigned vectors of size 2. */
uint _random_hash(uvec2 v) { return _random_hash(v.x ^ _random_hash(v.y)); }
/* Extension of the simple hash function that works on unsigned vectors of size 3. */
uint _random_hash(uvec3 v) { return _random_hash(v.x ^ _random_hash(v.y) ^ _random_hash(v.z)); }
/* Extension of the simple hash function that works on unsigned vectors of size 4. */
uint _random_hash(uvec4 v) { return _random_hash(v.x ^ _random_hash(v.y) ^ _random_hash(v.z) ^ _random_hash(v.w)); }

/* Creates a float using the given uint as the fractional part (i.e., so it's below 1.0). */
float _random_float_construct(uint m) {
    // Bitmask for 32-bit IEEE 754 mantissa
    const uint ieeeMantissa = 0x007FFFFFu;
    // 1.0 in IEEE binary
    const uint ieeeOne      = 0x3F800000u;

    // Next, we only keep the mantissa bits
    m &= ieeeMantissa;
    // Add the fractional part to the 1.0
    m |= ieeeOne;

    // Get a float from this
    float f = uintBitsToFloat(m);
    // Convert from 1.0-2.0 to 0.0-1.0
    return f - 1.0;
}

/* Interface to the random function, that takes some value (i.e., some coordinates of the current shader instance and some time to randomize calls) and returns a random float in the half-open interval [0:1]. This overload takes only 1 float. */
float random(float x) { return _random_float_construct(_random_hash(floatBitsToUint(x))); }
/* Interface to the random function, that takes some value (i.e., some coordinates of the current shader instance and some time to randomize calls) and returns a random float in the half-open interval [0:1]. This overload takes a vector of size 2. */
float random(vec2 v) { return _random_float_construct(_random_hash(floatBitsToUint(v))); }
/* Interface to the random function, that takes some value (i.e., some coordinates of the current shader instance and some time to randomize calls) and returns a random float in the half-open interval [0:1]. This overload takes a vector of size 3. */
float random(vec3 v) { return _random_float_construct(_random_hash(floatBitsToUint(v))); }
/* Interface to the random function, that takes some value (i.e., some coordinates of the current shader instance and some time to randomize calls) and returns a random float in the half-open interval [0:1]. This overload takes a vector of size 4. */
float random(vec4 v) { return _random_float_construct(_random_hash(floatBitsToUint(v))); }

#endif
