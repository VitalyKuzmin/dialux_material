
import { GUI } from './libs/lil-gui.module.min.js';

// MAIN
var container, scene, camera, renderer, controls;
var material, SphereCamera;


init();
animate();

function init() {

    //////////// Init scene //////////

    // RENDERER
    renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(window.innerWidth, window.innerHeight);
    container = document.getElementById('ThreeJS');
    container.appendChild(renderer.domElement);

    // SCENE
    scene = new THREE.Scene();

    // CAMERA
    camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 2000);
    scene.add(camera);
    camera.position.set(0, 150, 400);
    camera.lookAt(scene.position);

    // CONTROLS
    controls = new THREE.OrbitControls(camera, renderer.domElement);

    // EVENTS
    window.addEventListener('resize', onWindowResize);

    // FLOOR
    var floorTexture = new THREE.ImageUtils.loadTexture('images/checkerboard.jpg');
    floorTexture.wrapS = floorTexture.wrapT = THREE.RepeatWrapping;
    floorTexture.repeat.set(10, 10);
    var floorMaterial = new THREE.MeshBasicMaterial({ map: floorTexture, side: THREE.DoubleSide });
    var floorGeometry = new THREE.PlaneGeometry(1000, 1000, 10, 10);
    var floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.position.y = -100;
    floor.rotation.x = Math.PI / 2;
    scene.add(floor);

    // POINT LIGHT
    var light = new THREE.PointLight(0xffffff);
    light.position.set(-150, 180, 200);
    scene.add(light);
    // AMBIENT LIGHT
    var light2 = new THREE.AmbientLight(0x333333);
    light2.position.set(light.position);
    scene.add(light2);

    ////////////// CUSTOM //////////////

    // Sphere
    var geometry = new THREE.SphereGeometry(100, 50, 50);
    material = new DialuxMaterial();
    var mesh = new THREE.Mesh(geometry, material);
    mesh.position.set(0, 30, 0);
    scene.add(mesh);


    // SphereCamera
    SphereCamera = new THREE.CubeCamera(0.1, 1000, 1024);
    SphereCamera.renderTarget.mapping = new THREE.CubeRefractionMapping(); //SphericalReflectionMapping

    SphereCamera.position = mesh.position;
    scene.add(SphereCamera);
    material.envMap = SphereCamera.renderTarget;


    // Init data ---------------------------------------------------------------
    var gui = new GUI();




    var prm =
    {
        diff: [0.5, 0.3, 0.3],
        spec: [0.5, 0.5, 0.5],
        amb: [0.2, 0.1, 0.1],
        opacity: 0.2,
        Shin: 40,
        N: 1.02,
    };

    initMaterialFromUrl(prm);


    var elem;
    // Colors
    elem = gui.addColor(prm, 'diff').name('Diffuse').listen();
    elem.onChange(c => material.color.setRGB(c[0], c[1], c[2]));

    elem = gui.addColor(prm, 'spec').name('Specular').listen();
    elem.onChange(c => material.specular.setRGB(c[0], c[1], c[2]));

    elem = gui.addColor(prm, 'amb').name('Ambient').listen();
    elem.onChange(c => material.ambient.setRGB(c[0], c[1], c[2]));

    // Params
    elem = gui.add(prm, 'Shin').min(0).max(128).step(1).name('Shininess').listen();
    elem.onChange(value => material.shininess = value);

    elem = gui.add(prm, 'opacity').min(0).max(1).step(0.01).name('Opacity').listen();
    elem.onChange(value => material.opacity = value);

    elem = gui.add(prm, 'N').min(1).max(2).step(0.01).name('Refraction ratio').listen();
    elem.onChange(value => material.refractionRatio = value);

    SetMaterial(prm.diff, prm.spec, prm.amb, prm.opacity, prm.Shin, prm.N);



}


function SetMaterial(diff, spec, amb, opacity, Shin, N) {
    material.color.setRGB(diff[0], diff[1], diff[2]);      // диффузная часть
    material.specular.setRGB(spec[0], spec[1], spec[2]);   // зеркальный блик
    material.ambient.setRGB(amb[0], amb[1], amb[2]);       // окружение (зеркальный + прозрачный)

    material.opacity = opacity;           // отношение между 0 - зеркальный, 1 - прозрачный
    material.Shin = Shin;                 // блеск (размер зеркального блика)
    material.refractionRatio = N;     // коэфиент преломления прозрачной части
}

//URL ----------------------------------------------------------------
function initMaterialFromUrl(p) {
    const queryString = window.location.search;
    if (queryString != "") {
        //var prms = '{"D":[0.000000,0.000000,0.000000],"S":[0.511817,0.511817,0.665696],"A":[0.511817,0.511817,0.665696],"Tr":0.000000,"Sh":40,"N":1.000000}';
        var prms = new URLSearchParams(queryString).get('params');
        prms = JSON.parse(prms);

        p.diff = prms.D;	 // диффузная часть
        p.spec = prms.S;	 // зеркальный блик
        p.amb = prms.A;		 // окружение (зеркальный + прозрачный)
        p.opacity = prms.Tr; // отношение между 0 - зеркальный, 1 - прозрачный

        p.Shin = prms.Sh;	 // блеск (размер зеркального блика)
        p.N = prms.N;		 // коэфиент преломления прозрачной части
    }
}

//RENDER ----------------------------------------------------------------
function animate() {
    requestAnimationFrame(animate);
    render();
    controls.update();
}

function render() {
    SphereCamera.updateCubeMap(renderer, scene); //update
    renderer.render(scene, camera);
}

function onWindowResize() {
    renderer.setSize(window.innerWidth, window.innerHeight);
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

}
