
class color3f {

    r = 0.0;
    g = 0.0;
    b = 0.0;

    constructor(r, g, b) {
        this.r = r;
        this.g = g;
        this.b = b;
    }

    sum() {
        return this.r + this.g + this.b;
    }

    getNorm() {
        var sum = this.sum();
        if (norm > 0) {
            this.r /= sum;
            this.g /= sum;
            this.b /= sum;
        }
        return this;
    }

    plus(c) {
        if (typeof c == "number") {
            this.r += c;
            this.g += c;
            this.b += c;
        } else {
            this.r += c.r;
            this.g += c.g;
            this.b += c.b;
        }
    }

    multiply(c) {
        if (typeof c == "number") {
            this.r *= c;
            this.g *= c;
            this.b *= c;
        } else {
            this.r *= c.r;
            this.g *= c.g;
            this.b *= c.b;
        }

    }

    divide(с) {
        if (typeof c == "number") {
            this.r /= c;
            this.g /= c;
            this.b /= c;
        } else {
            this.r /= c.r;
            this.g /= c.g;
            this.b /= c.b;
        }
    }


}


export { color3f }