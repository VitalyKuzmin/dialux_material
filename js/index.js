import { MaterialMaster } from './MaterialMaster.js';

// MAIN
var container, scene, camera, renderer, controls;
var mesh, CubeCamera;

init();
animate();

function init() {

    //----------- Init scene -----------

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
    addFloor();

    // POINT LIGHT
    var light = new THREE.PointLight(0xffffff);
    light.position.set(-150, 180, 200);
    scene.add(light);
    // AMBIENT LIGHT
    var light2 = new THREE.AmbientLight(0x333333);
    light2.position.set(light.position);
    scene.add(light2);


    //------------ CUSTOM ------------

    //mesh
    var geometry = new THREE.SphereGeometry(100, 50, 50);
    var material = new DialuxMaterial();
    mesh = new THREE.Mesh(geometry, material);
    mesh.position.set(0, 30, 0);
    scene.add(mesh);


    //CubeCamera
    CubeCamera = new THREE.CubeCamera(0.1, 1000, 1024);
    CubeCamera.position = mesh.position;
    scene.add(CubeCamera);
    mesh.material.envMap = CubeCamera.renderTarget;
    //CubeCamera.renderTarget.mapping = new THREE.SphericalRefractionMapping(); //CubeRefractionMapping

    // Init data ---------------------------------------------------------------
    var master = new MaterialMaster();
    master.init(mesh.material);

}

//TOOLS
function addFloor() {
    var floorTexture = new THREE.ImageUtils.loadTexture('images/checkerboard.jpg');
    floorTexture.wrapS = floorTexture.wrapT = THREE.RepeatWrapping;
    floorTexture.repeat.set(10, 10);
    var floorMaterial = new THREE.MeshBasicMaterial({ map: floorTexture, side: THREE.DoubleSide });
    var floorGeometry = new THREE.PlaneGeometry(1000, 1000, 10, 10);
    var floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.position.y = -100;
    floor.rotation.x = Math.PI / 2;
    scene.add(floor);

}

//RENDER
function animate() {
    requestAnimationFrame(animate);
    render();
    controls.update();
}

function render() {
    CubeCamera.updateCubeMap(renderer, scene); //update
    renderer.render(scene, camera);
}

function onWindowResize() {
    renderer.setSize(window.innerWidth, window.innerHeight);
    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

}

