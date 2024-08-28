// 1_4
function changeY1_4(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);
    color = toYrgb(color);
    color = array_normalize(color);
    color = arr_multiply(color, Ynew);
    var index = color_max_i(color);
    if (color[index] > K[index]) {
        var diff = Ynew - K[index];
        color[index] = 0;
        color = array_normalize(color);
        color = arr_multiply(color, diff);
        color[index] = K[index];
    }

    color = fromYrgb(color);
    console.log(YfromRGB(color))
    console.log(color);
    color = clamp_array(color);
    return color;
}



// 2
function changeY2(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);

    color = toYrgb(color);

    var diff = Ynew - arr_sum(color);

    var coeff = [...color];
    if (diff > 0) {
        coeff[0] = K[0] - color[0];
        coeff[1] = K[1] - color[1];
        coeff[2] = K[2] - color[2];
    }
    coeff = array_normalize(coeff);

    color[0] += diff * coeff[0];
    color[1] += diff * coeff[1];
    color[2] += diff * coeff[2];

    console.log(arr_sum(color));

    color = fromYrgb(color);
    console.log(color);
    color = clamp_array(color);
    return color;
}

// 3 (error)
function changeY3(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);

    var diff = Ynew - YfromRGB(color);
    diff *= 3;

    var coeff = [...color];
    if (diff > 0) {
        coeff[0] = 1.0 - color[0];
        coeff[1] = 1.0 - color[1];
        coeff[2] = 1.0 - color[2];
    }
    coeff = array_normalize(coeff);
    coeff = clamp_array(coeff);

    color[0] += diff * coeff[0];
    color[1] += diff * coeff[1];
    color[2] += diff * coeff[2];

    console.log(YfromRGB(color));

    console.log(color);
    color = clamp_array(color);
    return color;
}


// 4 YfromRGB
function changeY4(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);
    var coeff = Ynew / YfromRGB(color);
    color = arr_multiply(color, coeff);

    var index = rgb_max_i(color);

    if (color[index] > 1) {
        color[index] = 1;

        var index2 = rgb_max_i(color, index);

        if (color[index2] > 1) {
            color[index2] = 1;

            coeff = (Ynew - K[index] - K[index2]) / (YfromRGB(color) - K[index] - K[index2]);
            color = arr_multiply(color, coeff, [index, index2]);
        }
        else {
            coeff = (Ynew - K[index]) / (YfromRGB(color) - K[index]);

            color = arr_multiply(color, coeff, [index]);

            index2 = rgb_max_i(color, index);
            if (color[index2] > 1) {
                color[index2] = 1;

                coeff = (Ynew - K[index] - K[index2]) / (YfromRGB(color) - K[index] - K[index2]);
                color = arr_multiply(color, coeff, [index, index2]);
            }
        }
    }
    console.log(YfromRGB(color));
    console.log(color);
    color = clamp_array(color);
    return color;
}


// 5 arr_sum
function changeY5(color, Ynew) {

    // for no division to null errors --------------------------------
    color = clamp_arr_zero(color);

    color = toYrgb(color);

    var coeff = Ynew / arr_sum(color);

    color = arr_multiply(color, coeff);

    var index = check_Yrgb(color);

    if (index >= 0) {
        color[index] = K[index];

        coeff = (Ynew - K[index]) / (arr_sum(color) - K[index]);

        color = arr_multiply(color, coeff, [index])

        var index2 = check_Yrgb(color, index);
        if (index2 >= 0) {
            color[index2] = K[index2];

            coeff = (Ynew - K[index] - K[index2]) / (arr_sum(color) - K[index] - K[index2]);

            color = arr_multiply(color, coeff, [index, index2])

        }

    }

    console.log("Y= " + arr_sum(color));

    color = fromYrgb(color);
    console.log(color);
    color = clamp_array(color);
    return color;
}



// Tools ----------------------------------------------------------------



/**
 * A conversion matrix from linear sRGB colour space with coordinates normalised
 * to [0, 1] range into an XYZ space.
 */
const xyzFromRgbMatrix = [
    [0.4124108464885388, 0.3575845678529519, 0.18045380393360833],
    [0.21264934272065283, 0.7151691357059038, 0.07218152157344333],
    [0.019331758429150258, 0.11919485595098397, 0.9503900340503373]
];


/**
 * A conversion matrix from XYZ colour space to a linear sRGB space with
 * coordinates normalised to [0, 1] range.
 */
const rgbFromXyzMatrix = [
    [3.240812398895283, -1.5373084456298136, -0.4985865229069666],
    [-0.9692430170086407, 1.8759663029085742, 0.04155503085668564],
    [0.055638398436112804, -0.20400746093241362, 1.0571295702861434]
];

/**
 * Multiplies a 3✕3 matrix by a 3✕1 column matrix.  The result is another 3✕1
 * column matrix.  The column matrices are represented as single-dimensional
 * 3-element array.  The matrix is represented as a two-dimensional array of
 * rows.
 */
function matrixMultiplication3x3x1(matrix, column) {
    return matrix.map((row) => (
        row[0] * column[0] + row[1] * column[1] + row[2] * column[2]
    ));
}


/**
 * Converts sRGB colour given as a triple of 8-bit integers into XYZ colour
 * space.
 */
function toXYZ(rgb) {
    return matrixMultiplication3x3x1(xyzFromRgbMatrix, rgb);
}


/**
 * Converts colour from XYZ space to sRGB colour represented as a triple of
 * 8-bit integers.
 */
function fromXYZ(xyz) {
    return matrixMultiplication3x3x1(rgbFromXyzMatrix, xyz);
}

function toHSL(rgb) {
    var hsl = { h: 0, s: 0, l: 0 };

    // h,s,l ranges are in 0.0 - 1.0

    var r = rgb[0], g = rgb[1], b = rgb[2];

    var max = Math.max(r, g, b);
    var min = Math.min(r, g, b);

    var hue, saturation;
    var lightness = (min + max) / 2.0;

    if (min === max) {

        hue = 0;
        saturation = 0;

    } else {

        var delta = max - min;

        saturation = lightness <= 0.5 ? delta / (max + min) : delta / (2 - max - min);

        switch (max) {

            case r: hue = (g - b) / delta + (g < b ? 6 : 0); break;
            case g: hue = (b - r) / delta + 2; break;
            case b: hue = (r - g) / delta + 4; break;

        }

        hue /= 6;

    }

    hsl.h = hue;
    hsl.s = saturation;
    hsl.l = lightness;
    return hsl;
}

function fromHSL(hsl) {

    var color = [0, 0, 0];
    var h = hsl.h, s = hsl.s, l = hsl.l;

    if (s === 0) {

        color[0] = color[1] = color[2] = l;

    } else {

        var hue2rgb = function (p, q, t) {

            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1 / 6) return p + (q - p) * 6 * t;
            if (t < 1 / 2) return q;
            if (t < 2 / 3) return p + (q - p) * 6 * (2 / 3 - t);
            return p;

        };

        var p = l <= 0.5 ? l * (1 + s) : l + s - (l * s);
        var q = (2 * l) - p;

        color[0] = hue2rgb(q, p, h + 1 / 3);
        color[1] = hue2rgb(q, p, h);
        color[2] = hue2rgb(q, p, h - 1 / 3);

    }

    return color;
}

// HSL  (error)
function changeY0(c, Ynew) {
    var color = [...c];
    var hsl = toHSL(color);
    hsl.l = Ynew;
    color = fromHSL(hsl);
    color = clamp_array(color);
    console.log(YfromRGB(RGBtoLinear(color)))
    console.log(color);
    return color;
}


// XYZ  (error)
function changeY01(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);
    var xyz = toXYZ(color);
    xyz[2] = Ynew;
    color = fromXYZ(xyz);
    color = clamp_array(color);
    console.log(YfromRGB(color))
    console.log(color);
    return color;
}


// 1 arr_sum
function changeY1(c, Ynew) {
    var color = [...c];
    color = toYrgb(color);
    var diff = Ynew - arr_sum(color);
    color[0] += diff * K_R;
    color[1] += diff * K_G;
    color[2] += diff * K_B;
    color = fromYrgb(color);
    console.log(color);
    color = clamp_array(color);
    return color;
}


// 1_2 
function changeY1_2(c, Ynew) {
    var color = [...c];
    color = clamp_arr_zero(color);
    //color = toYrgb(color);
    var coeff = Ynew / YfromRGB(color);
    color = arr_multiply(color, coeff);

    console.log(arr_sum(color))

    //color = fromYrgb(color);
    console.log(color);
    color = clamp_array(color);
    return color;
}


function check_value(val) {
    if (val.upd === false) {
        val.upd = true;
        return false;
    }
    return true;
}

function rgb_max_i(arr, no_index = -1) {
    var max = 0;
    var index = 0;

    for (var i = 0; i < arr.length; i++) {
        if (i == no_index) continue;
        if (arr[i] > max) {
            max = arr[i];
            index = i;
        }
    }
    return index;
}
