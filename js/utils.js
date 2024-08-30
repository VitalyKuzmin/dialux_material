
class color3f {

    r = 0.0;
    g = 0.0;
    b = 0.0;

    // constructor() {
    //     this.r = 0.0;
    //     this.g = 0.0;
    //     this.b = 0.0;
    // }

    constructor(c, c2, c3) {
        if (c != undefined && c2 != undefined && c3 != undefined) {
            this.r = c;
            this.g = c2;
            this.b = c3;
        } else if (Array.isArray(c)) {
            this.r = c[0];
            this.g = c[1];
            this.b = c[2];
        } else if (typeof c == "object") {
            this.r = c.r;
            this.g = c.g;
            this.b = c.b;
        } else if (typeof c == "number") {
            this.r = c;
            this.g = c;
            this.b = c;
        } else if (c == undefined) {
            this.r = 0.0;
            this.g = 0.0;
            this.b = 0.0;
        }
    }

    sum() {
        return this.r + this.g + this.b;
    }

    getNorm() {
        var sum = this.sum();
        var res = new color3f(this);
        if (sum > 0) {
            res.r /= sum;
            res.g /= sum;
            res.b /= sum;
        }
        return res;
    }

    plus(c) {
        var res = new color3f(this);
        if (typeof c == "number") {
            res.r += c;
            res.g += c;
            res.b += c;
        } else {
            res.r += c.r;
            res.g += c.g;
            res.b += c.b;
        }
        return res;
    }

    minus(c) {
        var res = new color3f(this);
        if (typeof c == "number") {
            res.r -= c;
            res.g -= c;
            res.b -= c;
        } else {
            res.r -= c.r;
            res.g -= c.g;
            res.b -= c.b;
        }
        return res;
    }

    func(f) {
        var res = new color3f(this);
        res.r = f(res.r);
        res.g = f(res.g);
        res.b = f(res.b);
        return res;
    }

    multiply(c) {
        var res = new color3f(this);
        if (typeof c == "number") {
            res.r *= c;
            res.g *= c;
            res.b *= c;
        } else {
            res.r *= c.r;
            res.g *= c.g;
            res.b *= c.b;
        }
        return res;

    }

    divide(c) {
        var res = new color3f(this);
        if (typeof c == "number") {
            res.r /= c;
            res.g /= c;
            res.b /= c;
        } else {
            res.r /= c.r;
            res.g /= c.g;
            res.b /= c.b;
        }
        return res;
    }


}


export { color3f }