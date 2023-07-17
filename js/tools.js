function hexToRgb(hex) {
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16) / 255,
        g: parseInt(result[2], 16) / 255,
        b: parseInt(result[3], 16) / 255
    } : null;
}

function componentToHex(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(r, g, b) {
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function rgbMax(color) {
    var max;
    if (!color.hasOwnProperty("r")) {
        max = color[0];
        max = color[1] > max ? color[1] : max;
        max = color[2] > max ? color[2] : max;
    } else {
        max = color.r;
        max = color.g > max ? color.g : max;
        max = color.b > max ? color.b : max;
    }
    return max;
}


function rgbSetY(color, Y) {
    var rgb = rgbTo1(color);
    var k = Y / rgbMax(rgb);
    rgb[0] *= k;
    rgb[1] *= k;
    rgb[2] *= k;
    return rgb;
}



function rgbTo1(rgb) {
    var  color = {...rgb};
    color[0] /= 255;
    color[1] /= 255;
    color[2] /= 255;
    return color;
}

function rgbMix() {
    var color = { ...arguments[0] };
    for (var i = 1; i < arguments.length; i++) {
        var c = arguments[i];
        color[0] += c[0],
            color[1] += c[1],
            color[2] += c[2]
    }
    return color;
}

function rgbSum(color) {
    var sum;
    if (Array.isArray())
        sum = color[0] + color[1] + color[2];
    else
        color.r + color.g + color.b
    return sum;;
}



const RGBToHSL = (rgb) => {
    var r = rgb[0];
    var g = rgb[1];
    var b = rgb[2];
    r /= 255;
    g /= 255;
    b /= 255;
    const l = Math.max(r, g, b);
    const s = l - Math.min(r, g, b);
    const h = s
        ? l === r
            ? (g - b) / s
            : l === g
                ? 2 + (b - r) / s
                : 4 + (r - g) / s
        : 0;

    return {
        h: (1 / 360) * (60 * h < 0 ? 60 * h + 360 : 60 * h),
        s: 0.01 * (100 * (s ? (l <= 0.5 ? s / (2 * l - s) : s / (2 - (2 * l - s))) : 0)),
        l: 0.01 * ((100 * (2 * l - s)) / 2),
    };
};

function RGBfromArray(arr) {
    return { r: arr[0], g: arr[1], b: arr[2] }
}




function elem_vis(elem, flag) {
    var str = flag ? "" : "none";
    elem.domElement.parentElement.parentElement.style.display = str;
}