
import { GUI } from './libs/lil-gui.module.min.js';

import { color3f } from './utils.js';

/* Лакированный материал
----------------------------------------------------------------
    Система уравнений:
    Yspec = mProp.ReflC * mProp.ReflF = K
    Ydiff = (1 - Yspec) * Y
    YRefl = Yspec + Ydiff
    Решение:
    YRefl = K + ( 1 - K ) * Y
    ReflC = K / YRefl
*/

/* Прозрачный материал
----------------------------------------------------------------
    Система уравнений:
    Ytrans/YRefl = mProp.Ytrans/mProp.ReflF = K
    YRefl + Ytrans = Y
    Решение:
    YRefl = Y/(1+K)
    Ytrans = K*Yrefl
 */

// Constants -----------------------------------------------------------------------------------------------------
var K = new color3f(0.2126, 0.7152, 0.0722); //(https://en.wikipedia.org/wiki/Relative_luminance)
var epsilon = 0.000001;

// Tools ----------------------------------------------------------------------------------------------------------
// sRGB <-> lRGB (https://mina86.com/2019/srgb-xyz-conversion) 
function to_linear(value) {
    if (value <= 0.04045) {
        value = value / 12.92;
    } else {
        value = Math.pow((value + 0.055) / 1.055, 2.4);
    }
    return value;
}

function from_linear(value) {
    if (value <= 0.0031308) {
        value = value * 12.92; // * 255
    } else {
        value = 1.055 * Math.pow(value, 1 / 2.4) - 0.055; //* 255
    }
    return value;
}

function to_luminance(color) {
    var Yrgb = color.func(to_linear);
    Yrgb = Yrgb.multiply(K);
    return Yrgb;
}




// Change rgb by Luminance -----------------------------------------------------------------------

function clamp_color_zero(color) {

    if (color.r < epsilon && color.g < epsilon && color.b < epsilon)
        color = K;
    else {
        color.r = Math.max(epsilon, color.r);
        color.g = Math.max(epsilon, color.g);
        color.b = Math.max(epsilon, color.b);
    }
    return color;
}


function check_Yrgb(Yrgb) {

    var sum = Yrgb.sum();

    if (Yrgb.r > K.r) {
        Yrgb.r = K.r;
    }

    if (Yrgb.g > K.g) {
        Yrgb.g = K.g;
    }

    if (Yrgb.b > K.b) {
        Yrgb.b = K.b;
    }

    var diff = (sum - Yrgb.sum());

    return diff;
}

function changeY(color, Ynew) {
    var Yrgb = to_luminance(color);
    Yrgb = clamp_color_zero(Yrgb);
    Yrgb = Yrgb.getNormalize().multiply(Ynew);
    var diff = check_Yrgb(Yrgb);
    if (diff > 0) {
        var coeff = K.minus(Yrgb).getNormalize();
        Yrgb = Yrgb.plus(coeff.multiply(diff));
    }
    return Yrgb.divide(K);
}


// Tools ----------------------------------------------------------------

function elem_vis(elem, flag) {
    var str = flag ? "" : "none";
    elem.domElement.style.display = str;
}



// MaterialMaster ----------------------------------------------------------------

class MaterialMaster {

    MType = class {
        Metallic = "Metallic"
        Painted = "Painted"
        Transparent = "Transparent"

        getArr() { return [this.Metallic, this.Painted, this.Transparent]; }
    };

    Color = new color3f();
    Y = 0.0;
    K = 0.0;
    K_T = 0.0;
    Type = "";
    Props = {}
    material = {};
    upd = true;


    constructor() {
    }

    init(m) {
        this.material = m;
        this.init_gui();
        this.updateType();
    }

    init_gui() {

        var def_props = {
            Color: [0, 0, 0],                 // Material Color
            Type: "Metallic",                 // Material type
            Shin: 40,                         // Shininess
            ReflF: 0.0,                        // Reflection factor        [0,0.9]
            ReflC: 0.0,                       // Reflective coating       [0,1]
            Trans: 0.0,                       // Degree of transmission   [0,1]
            n: 1.0                            // Refractive index         [1,2]
        }

        var Props = this.Props;
        var gui = new GUI({ name: "Material" });

        Props.Color = gui.addColor(def_props, 'Color').name('Color').listen();
        Props.Color.onChange(value => this.updateColor());

        Props.Type = gui.add(def_props, 'Type', ["Metallic", "Painted", "Transparent"]).name('Material Type').listen();
        Props.Type.onChange(value => this.updateType());

        Props.ReflF = gui.add(def_props, 'ReflF').min(0.00).max(0.90).step(0.01).name('Reflection factor').listen();
        Props.ReflF.onChange(value => this.updateReflectionFactor());

        Props.ReflC = gui.add(def_props, 'ReflC').min(0.00).max(1.00).step(0.01).name('Reflective coating').listen();
        Props.ReflC.onChange(value => this.updateReflectingCoating());

        Props.Trans = gui.add(def_props, 'Trans').min(0.00).max(1.00).step(0.01).name('Degree of trans.').listen();
        Props.Trans.onChange(value => this.updateTransparency());

        Props.n = gui.add(def_props, 'n').min(1.00).max(2.00).step(0.01).name('Refractive index').listen();
        Props.n.onChange(value => this.updateN());
    }

    // SET ----------------------------------------------------------------


    // setColor(Color) {
    //     this.Props.Color.setValue(Color);
    // }


    setReflF(ReflF) {
        this.upd = false;
        this.Props.ReflF.setValue(ReflF);
    }


    setReflC(ReflC) {
        this.upd = false;
        this.Props.ReflC.setValue(ReflC);
    }


    setTrans(Trans) {
        this.upd = false;
        this.Props.Trans.setValue(Trans);
    }


    SetMaterial(diff, spec, amb, trans, opacity, Shin, N) {

        // convert to sRGB
        diff = diff.func(from_linear);
        spec = spec.func(from_linear);
        amb = amb.func(from_linear);
        trans = trans.func(from_linear);

        this.material.color.setRGB(diff.r, diff.g, diff.b);      // диффузная часть
        this.material.specular.setRGB(spec.r, spec.g, spec.b);   // зеркальный блик 
        this.material.ambient.setRGB(amb.r, amb.g, amb.b);       // окружение (зеркальный + прозрачный) 

        this.material.opacity = opacity;           // отношение между 0 - зеркальный, 1 - прозрачный
        this.material.Shin = Shin;                 // блеск (размер зеркального блика)
        this.material.refractionRatio = N;     // коэфиент преломления прозрачной части
    }

    // GetY ----------------------------------------------------------------
    getYfromRgb() {
        var color = this.Color;
        var Yrgb = to_luminance(color); // sRGB to Y
        return 0.9 * Yrgb.sum(); // 10% идет на поглощение
    }

    getYfromProps() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var ReflF = Props.ReflF.getValue();
        var Trans = Props.Trans.getValue();
        var ReflC = Props.ReflC.getValue();
        var Y;
        switch (type) {
            case "Metallic":
                Y = ReflF;
                break;
            case "Painted":
                Y = ReflF * (1 - ReflC) / (1 - ReflC * ReflF);
                break;
            case "Transparent":
                Y = ReflF + Trans;
                break;
        }
        return Y;
    }

    //Update ----------------------------------------------------------------

    recalcGlobalYK() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var ReflC = Props.ReflC.getValue();
        var ReflF = Props.ReflF.getValue();
        var Trans = Props.Trans.getValue();
        if (type == "Metallic") {
            this.K = ReflC;
            this.Y = ReflF;
        }
        else if (type == "Painted") {
            this.K = ReflC * ReflF
            this.Y = (ReflF - this.K) / (1 - this.K);
        }
        else if (type == "Transparent") {
            this.K_T = Trans / ReflF;
            this.Y = ReflF + Trans;
            if (Trans > 0) {
                this.K = 1;
            }
        }
        this.upd = true;
    }

    updateType() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        if (type == "Metallic") {
            elem_vis(Props.ReflC, true);
            elem_vis(Props.Trans, false);
            elem_vis(Props.n, false);
        }
        else if (type == "Painted") {
            elem_vis(Props.ReflC, true);
            elem_vis(Props.Trans, false);
            elem_vis(Props.n, false);
        }
        else if (type == "Transparent") {
            elem_vis(Props.ReflC, false);
            elem_vis(Props.Trans, true);
            elem_vis(Props.n, true);
        }
        this.Type = type;
        this.recalcProps();
    }

    updateColor() {
        this.Color = new color3f(this.Props.Color.getValue());
        this.Y = this.getYfromRgb();
        this.recalcProps();
    }

    updateReflectionFactor() {
        if (!this.upd) return;
        var Props = this.Props;
        var type = Props.Type.getValue();

        var ReflF = Props.ReflF.getValue();
        if (type == "Painted") {
            if (ReflF == 0) this.setReflC(0);
        }
        else if (type == "Transparent") {
            if (ReflF > (1 - Props.Trans.getValue())) {
                this.setTrans(1 - ReflF);
            }
        }

        this.recalcGlobalYK();
        this.recalcMaterial();
    }


    updateReflectingCoating() {
        if (!this.upd) return;
        this.recalcGlobalYK();
        this.recalcMaterial();
    }

    updateTransparency() {
        if (!this.upd) return;
        var Props = this.Props;
        var Trans = Props.Trans.getValue();

        if (Trans > (1 - Props.ReflF.getValue()))
            this.setReflF(1 - Trans);

        this.recalcGlobalYK();
        this.recalcMaterial();
    }

    updateN() {
        this.recalcMaterial();
    }

    recalcProps() {
        var type = this.Props.Type.getValue();
        var Y = this.Y;
        var K = this.K;
        var K_T = this.K_T;
        switch (type) {
            case "Metallic":
                this.setReflC(K);
                this.setReflF(Y);
                break;
            case "Painted":
                var ReflF = K + (1 - K) * Y;
                var ReflC = ReflF ? K / ReflF : 0;
                if (ReflF > 0.9) ReflF = 0.9;

                this.setReflF(ReflF);
                this.setReflC(ReflC);
                break;
            case "Transparent":
                var ReflF = Y / (1 + K_T);
                var Trans = Y - ReflF;

                this.setReflF(ReflF);
                this.setTrans(Trans);
                break;
        }
        this.recalcGlobalYK();
        this.recalcMaterial();
    }

    // Render
    recalcMaterial() {
        var Props = this.Props;
        var type = Props.Type.getValue();
        var ReflC = Props.ReflC.getValue();
        var ReflF = Props.ReflF.getValue();
        var Trans = Props.Trans.getValue();

        var color = this.Color;

        var diff, spec, amb, trans = new color3f();
        var Shin, N, opacity;
        if (type == "Metallic") {

            var Ys = ReflF * ReflC;
            var Yd = ReflF * (1 - ReflC);

            var Ysum = ReflF;

            var Yrgb = changeY(color, Ysum);

            diff = Yrgb.multiply(Yd / Ysum);
            spec = Yrgb.multiply(Ys / Ysum);

            amb = spec;

            opacity = 0;
            Shin = 40; // примерно
            N = 1;
        }
        else if (type == "Painted") {

            var Ys = ReflF * ReflC;
            var Yd = ReflF * (1 - ReflC); // Ys

            var Ysum = ReflF;

            var Yrgb = changeY(color, Ysum);

            diff = Yrgb.multiply(Yd / Ysum);
            spec = new color3f(Ys, Ys, Ys);
            amb = spec;

            opacity = 0;
            Shin = 80; // примерно   (10 + 2 * Math.pow(Refl, 1.5) * 90);
            N = 1;

        }
        else if (type == "Transparent") {
            var Ysum = ReflF + Trans;


            var Yrgb = changeY(color, Ysum);

            diff = new color3f(0, 0, 0);
            spec = Yrgb.multiply(ReflF / Ysum);
            trans = Yrgb.multiply(Trans / Ysum);
            amb = new color3f(Yrgb);

            opacity = Trans / Ysum;
            Shin = 40; // примерно
            N = Props.n.getValue();
        }


        this.SetMaterial(diff, spec, amb, trans, opacity, Shin, N);
    }



}

export { MaterialMaster }
