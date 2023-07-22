
import { GUI } from './libs/lil-gui.module.min.js';

/* Лакированный материал
----------------------------------------------------------------
    Система уравнений:
    Yspec = mProp.Kspec_refl * mProp.Yrefl = K
    Ydiff = (1 - Yspec) * Y
    Yrefl = Yspec + Ydiff
    Решение:
    Yrefl = K + ( 1 - K ) * Y
    Kspec_refl = K / Yrefl
*/

/* Прозрачный материал
----------------------------------------------------------------
    Система уравнений:
    Ytrans/Yrefl = mProp.Ytrans/mProp.Yrefl = K
    Yrefl + Ytrans = Y
    Решение:
    Yrefl = Y/(1+K)
    Ytrans = K*Yrefl
 */

// Constants (https://en.wikipedia.org/wiki/Relative_luminance)
const K_R = 0.2126;
const K_G = 0.7152;
const K_B = 0.0722;

// sRGB <-> lRGB (https://mina86.com/2019/srgb-xyz-conversion) 

function toLinear(value) {
    if (value <= 0.04045) {
        value = value / 12.92;
    } else {
        value = Math.pow((value + 0.055) / 1.055, 2.4);
    }
    return value;
}

function fromLinear(value) {
    if (value <= 0.0031308) {
        value = value * 12.92; // * 255
    } else {
        value = 1.055 * Math.pow(value, 1 / 2.4) - 0.055; //* 255
    }
    return value;
}

// Change rgb by Luminance ----------------------------------------------------------------

function changeY(color, Ynew) {
    const K = [K_R, K_G, K_B]

    // foe no division to null errors
    for (var j = 0; j < 3; j++) {
        if (!color[j])
            color[j] = 0.000000001;
    }

    var sum = K[0] * color[0] + K[1] * color[1] + K[2] * color[2];
    var coeff = Ynew / sum;
    var index = rgb_max_i(color);
    color[index] *= coeff;
    if (color[index] > 1) {
        color[index] = 1;
        sum = 0;
        for (var j = 0; j < 3; j++) {
            if (j == index) continue;
            sum += color[j] * K[j]
        }
        coeff = (Ynew - K[index]) / sum;
        var index2 = rgb_max_i(color, index);
        color[index2] *= coeff;
        if (color[index2] > 1) {
            color[index2] = 1;
            var index3;
            for (var j = 0; j < 3; j++) {
                if (j == index || j == index2) continue;
                sum += color[j] * K[j]
                index3 = j;
            }
            coeff = (Ynew - K[index] - K[index2]) / sum;

            color[index3] *= coeff;

            if (color[index3] > 1)
                color[index3] = 1;

        }
        else {
            for (var j = 0; j < 3; j++) {
                if (j == index || j == index2) continue;
                color[j] *= coeff;
            }
        }
    } else {
        for (var j = 0; j < 3; j++) {
            if (j == index) continue;
            color[j] *= coeff;
        }
    }

    return color;
}


function changeLuminance(rgb, Ynew) {
    var color = [...rgb];

    // sRGB to linear RGB
    color[0] = toLinear(color[0]);
    color[1] = toLinear(color[1]);
    color[2] = toLinear(color[2]);

    // Change luminance
    color = changeY(color, Ynew);

    // linear RGB to sRGB
    color[0] = fromLinear(color[0])
    color[1] = fromLinear(color[1])
    color[2] = fromLinear(color[2])

    return color;
}


// Tools ----------------------------------------------------------------

function check_value(val) {
    if (val.upd === false) {
        val.upd = true;
        return false;
    }
    return true;
}

function elem_vis(elem, flag) {
    var str = flag ? "" : "none";
    elem.domElement.style.display = str;
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


// MaterialMaster ----------------------------------------------------------------

class MaterialMaster {

    MType = class {
        Metallic = "Metallic"
        Painted = "Painted"
        Transparent = "Transparent"

        getArr() { return [this.Metallic, this.Painted, this.Transparent]; }
    };

    Color = [0, 0, 0];
    Type = "";
    Props = {}
    material = {};
    Koeff = 0;

    constructor() {
    }

    init(m) {
        this.material = m;
        this.init_gui();
        this.setType();
    }

    init_gui() {

        var def_props = {
            Color: [0, 0, 0],                 // Material Color
            Type: "Metallic",                 // Material type
            Shin: 40,                         // Shininess
            Refl: 0.0,                        // Reflection factor        [0,0.9]
            Kspec_refl: 0.0,                  // Reflective coating       [0,1]
            Trans: 0.0,                       // Degree of transmission   [0,1]
            n: 1.0                            // Refractive index         [1,2]
        }

        var Props = this.Props;
        var gui = new GUI({ name: "Material" });

        Props.Color = gui.addColor(def_props, 'Color').name('Color').listen();
        Props.Color.onChange(value => this.setColor());

        Props.Type = gui.add(def_props, 'Type', ["Metallic", "Painted", "Transparent"]).name('Material Type').listen();
        Props.Type.onChange(value => this.setType());

        Props.Refl = gui.add(def_props, 'Refl').min(0.00).max(0.90).step(0.01).name('Reflection factor').listen();
        Props.Refl.onChange(value => this.setRefl());

        Props.Kspec_refl = gui.add(def_props, 'Kspec_refl').min(0.00).max(1.00).step(0.01).name('Reflective coating').listen();
        Props.Kspec_refl.onChange(value => this.setKspec_refl());

        Props.Trans = gui.add(def_props, 'Trans').min(0.00).max(1.00).step(0.01).name('Degree of trans.').listen();
        Props.Trans.onChange(value => this.setTrans());

        Props.n = gui.add(def_props, 'n').min(1.00).max(2.00).step(0.01).name('Refractive index').listen();
        Props.n.onChange(value => this.setN());
    }

    // SET ----------------------------------------------------------------
    setType() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        if (type == "Metallic") {
            elem_vis(Props.Kspec_refl, true);
            elem_vis(Props.Trans, false);
            elem_vis(Props.n, false);
        }
        else if (type == "Painted") {
            elem_vis(Props.Kspec_refl, true);
            elem_vis(Props.Trans, false);
            elem_vis(Props.n, false);
        }
        else if (type == "Transparent") {
            elem_vis(Props.Kspec_refl, false);
            elem_vis(Props.Trans, true);
            elem_vis(Props.n, true);
        }
        this.Type = type;
        this.update(1);
    }

    setColor(Color) {
        var Props = this.Props;
        if (!check_value(Props.Color))
            return;

        if (Color !== undefined) {
            Props.Color.upd = false;
            Props.Color.setValue(Color);
            return
        }

        this.Koeff = undefined;
        this.Color = Props.Color.getValue();
        this.update(1);
    }

    setRefl(Refl) {
        var Props = this.Props;
        var type = Props.Type.getValue();

        if (!check_value(Props.Refl)) return;

        if (Refl === undefined)
            Refl = Props.Refl.getValue();
        else {
            Props.Refl.upd = false;
            Props.Refl.setValue(Refl);
            return
        }

        if (type == "Painted") {
            this.Koeff = Refl * Props.Kspec_refl.getValue();
            if (Refl == 0)
                this.setKspec_refl(0);
        }
        else if (type == "Transparent") {
            this.Koeff = 1.0;
            if (Refl > (1 - Props.Trans.getValue())) {
                this.setTrans(1 - Refl);
            }
        }
        this.update(2);
    }

    setKspec_refl(Kspec_refl) {
        var Props = this.Props;
        var type = Props.Type.getValue();
        if (!check_value(Props.Kspec_refl)) return;

        if (Kspec_refl !== undefined) {
            Props.Kspec_refl.upd = false;
            Props.Kspec_refl.setValue(Kspec_refl);
            return
        }

        Kspec_refl = Props.Kspec_refl.getValue();
        if (type == "Metallic") {
            this.Koeff = Kspec_refl;
            this.update();
        } else if (type == "Painted") {
            this.Koeff = Kspec_refl * Props.Refl.getValue();
            this.update(2);
        }

    }

    setTrans(Trans) {
        var Props = this.Props;
        if (!check_value(Props.Trans)) return;

        if (Trans === undefined)
            Trans = Props.Trans.getValue();
        else {
            Props.Trans.upd = false;
            Props.Trans.setValue(Trans);
            return
        }

        this.Koeff = 1.0;

        if (Trans > (1 - Props.Refl.getValue()))
            this.setRefl(1 - Trans);

        this.update(2);

    }

    setN() {
        this.update();
    }

    SetMaterial(diff, spec, amb, opacity, Shin, N) {
        this.material.color.setRGB(diff[0], diff[1], diff[2]);      // диффузная часть
        this.material.specular.setRGB(spec[0], spec[1], spec[2]);   // зеркальный блик 
        this.material.ambient.setRGB(amb[0], amb[1], amb[2]);       // окружение (зеркальный + прозрачный) 

        this.material.opacity = opacity;           // отношение между 0 - зеркальный, 1 - прозрачный
        this.material.Shin = Shin;                 // блеск (размер зеркального блика)
        this.material.refractionRatio = N;     // коэфиент преломления прозрачной части
    }

    // GetY ----------------------------------------------------------------
    getYfromRgb() {
        var color = this.Props.Color.getValue();//this.Color;
        // sRGB to Y
        var Yrgb = [0, 0, 0];
        Yrgb[0] = toLinear(color[0]);
        Yrgb[1] = toLinear(color[1]);
        Yrgb[2] = toLinear(color[2]);

        return 0.9 * (Yrgb[0] * K_R + Yrgb[1] * K_G + Yrgb[2] * K_B); // 10% идет на поглощение
    }

    getYfromProps() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var Refl = Props.Refl.getValue();
        var Trans = Props.Trans.getValue();
        var Kspec_refl = Props.Kspec_refl.getValue();
        var Y;
        switch (type) {
            case "Metallic":
                Y = Refl;
                break;
            case "Painted":
                var K = Kspec_refl == 1 ? 0 : Kspec_refl * Refl;
                Y = (Refl - K) / (1 - K);
                break;
            case "Transparent":
                if (Refl == 0) {
                    Y = Trans;
                } else {
                    var K = Trans / Refl;
                    Y = Refl * (1 + K);
                }
                break;
        }
        return Y;
    }

    //Update ----------------------------------------------------------------

    updateRGB() {
        var Y = this.getYfromProps();
        var color = changeLuminance(this.Color, Y);
        this.setColor(color);
    }

    updateProps() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var Y = this.getYfromRgb();
        var Refl = Props.Refl.getValue();
        var Kspec_refl = Props.Kspec_refl.getValue();
        var Trans = Props.Trans.getValue();
        var K = this.Koeff;
        switch (type) {
            case "Metallic":
                if (K !== undefined)
                    this.setKspec_refl(K);
                this.setRefl(Y);
                break;
            case "Painted":
                if (K === undefined)
                    K = Kspec_refl * Refl

                Refl = K + (1 - K) * Y;
                Kspec_refl = Refl ? K / Refl : 0;

                if (this.Koeff != undefined && Refl > 0.9)
                    Refl = 0.9;
                this.setRefl(Refl);
                this.setKspec_refl(Kspec_refl);
                break;
            case "Transparent":
                if (Refl == 0) {
                    Trans = Y;
                } else {
                    if (K === undefined)
                        K = Trans / Refl;
                    Refl = Y / (1 + K);
                    Trans = K * Refl;
                }

                this.setRefl(Refl);
                this.setTrans(Trans);
                break;
        }
    }

    update(flag = 0) {
        if (flag == 1)
            this.updateProps();
        else if (flag == 2)
            this.updateRGB();

        this.render();
    }

    // Render

    render() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var Kspec_refl = Props.Kspec_refl.getValue();
        var Refl = Props.Refl.getValue();
        var Trans = Props.Trans.getValue();

        var color = Props.Color.getValue();

        var diff, spec, amb;
        var Shin, N, opacity;
        if (type == "Metallic") {

            var Ys = Refl * Kspec_refl;
            var Yd = Refl * (1 - Kspec_refl);
            diff = changeLuminance(color, Yd);
            spec = changeLuminance(color, Ys);
            amb = spec;

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (type == "Painted") {

            var Ys = Refl * Kspec_refl;
            var Yd = Refl * (1 - Ys);
            diff = changeLuminance(color, Yd);
            spec = changeLuminance([0, 0, 0], Ys); // grayscale
            amb = spec;

            opacity = 0;
            Shin = 80; // примерно   (10 + 2 * Math.pow(Refl, 1.5) * 90);
            N = 1;

        }
        else if (type == "Transparent") {
            var Y = Refl + Trans;
            diff = [0, 0, 0];
            spec = changeLuminance(color, Refl);
            amb = changeLuminance(color, Y);

            opacity = Trans / Y;
            Shin = 40; // примерно
            N = Props.n.getValue();
        }


        this.SetMaterial(diff, spec, amb, opacity, Shin, N);
    }



}

export { MaterialMaster }
