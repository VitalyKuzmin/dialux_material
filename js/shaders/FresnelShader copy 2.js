/**
 * @author alteredq / http://alteredqualia.com/
 *
 * Based on Nvidia Cg tutorial
 */

THREE.FresnelShader = {



	uniforms: THREE.UniformsUtils.merge([

		//THREE.UniformsLib["common"],
		{

			"diffuse": { type: "c", value: new THREE.Color(0xeeeeee) },
			"opacity": { type: "f", value: 1.0 },

			"map": { type: "t", value: null },
			"offsetRepeat": { type: "v4", value: new THREE.Vector4(0, 0, 1, 1) },

			"lightMap": { type: "t", value: null },
			"specularMap": { type: "t", value: null },

			"envMap": { type: "t", value: null },
			"flipEnvMap": { type: "f", value: -1 },
			"useRefract": { type: "i", value: 0 },
			"reflectivity": { type: "f", value: 1.0 },
			"refractionRatio": { type: "f", value: 0.98 },
			"combine": { type: "i", value: 0 },

			"morphTargetInfluences": { type: "f", value: 0 }

		},
		THREE.UniformsLib["bump"],
		THREE.UniformsLib["normalmap"],
		THREE.UniformsLib["fog"],
		THREE.UniformsLib["lights"],
		THREE.UniformsLib["shadowmap"],

		{
			"ambient": { type: "c", value: new THREE.Color(0xffffff) },
			"emissive": { type: "c", value: new THREE.Color(0x000000) },
			"specular": { type: "c", value: new THREE.Color(0x111111) },
			"shininess": { type: "f", value: 30 },
			"wrapRGB": { type: "v3", value: new THREE.Vector3(1, 1, 1) }
		},

		{
			"mRefractionRatio": { type: "f", value: 1.02 },
			"mReflectionFactor": { type: "f", value: 1.0 },
			"tCube": { type: "t", value: null },
		}

	]),


	vertexShader: [

		"#define PHONG",

		"varying vec3 vViewPosition;",
		"varying vec3 vNormal;",


		THREE.ShaderChunk["map_pars_vertex"],
		THREE.ShaderChunk["lightmap_pars_vertex"],
		THREE.ShaderChunk["envmap_pars_vertex"],
		THREE.ShaderChunk["lights_phong_pars_vertex"],
		THREE.ShaderChunk["color_pars_vertex"],
		THREE.ShaderChunk["morphtarget_pars_vertex"],
		THREE.ShaderChunk["skinning_pars_vertex"],
		THREE.ShaderChunk["shadowmap_pars_vertex"],

		//custom start 
		"uniform float mRefractionRatio;",
		"varying vec3 vReflect;",
		"varying vec3 vRefract;",
		//custom end		

		"void main() {",

		THREE.ShaderChunk["map_vertex"],
		THREE.ShaderChunk["lightmap_vertex"],
		THREE.ShaderChunk["color_vertex"],

		THREE.ShaderChunk["morphnormal_vertex"],
		THREE.ShaderChunk["skinbase_vertex"],
		THREE.ShaderChunk["skinnormal_vertex"],
		THREE.ShaderChunk["defaultnormal_vertex"],

		"vNormal = normalize( transformedNormal );",

		THREE.ShaderChunk["morphtarget_vertex"],
		THREE.ShaderChunk["skinning_vertex"],
		THREE.ShaderChunk["default_vertex"],

		"vViewPosition = -mvPosition.xyz;",

		THREE.ShaderChunk["worldpos_vertex"],
		THREE.ShaderChunk["envmap_vertex"],
		THREE.ShaderChunk["lights_phong_vertex"],
		THREE.ShaderChunk["shadowmap_vertex"],


		//custom start 
		//"mvPosition = modelViewMatrix * vec4( position, 1.0 );",
		//"worldPosition = modelMatrix * vec4( position, 1.0 );",

		// "vec3 worldNormal = normalize( mat3( modelMatrix[0].xyz, modelMatrix[1].xyz, modelMatrix[2].xyz ) * normal );",

		// "vec3 I = worldPosition.xyz - cameraPosition;",

		// "vReflect = reflect( I, worldNormal );",
		// "vRefract = refract( normalize( I ), worldNormal, mRefractionRatio );",

		// "gl_Position = projectionMatrix * mvPosition;",
		//custom end

		"}"

	].join("\n"),


	fragmentShader: [

		"uniform vec3 diffuse;",
		"uniform float opacity;",

		"uniform vec3 ambient;",
		"uniform vec3 emissive;",
		"uniform vec3 specular;",
		"uniform float shininess;",


		THREE.ShaderChunk["color_pars_fragment"],
		THREE.ShaderChunk["map_pars_fragment"],
		THREE.ShaderChunk["lightmap_pars_fragment"],
		THREE.ShaderChunk["envmap_pars_fragment"],
		THREE.ShaderChunk["fog_pars_fragment"],
		THREE.ShaderChunk["lights_phong_pars_fragment"],
		THREE.ShaderChunk["shadowmap_pars_fragment"],
		THREE.ShaderChunk["bumpmap_pars_fragment"],
		THREE.ShaderChunk["normalmap_pars_fragment"],
		THREE.ShaderChunk["specularmap_pars_fragment"],

		//custom start 
		"uniform samplerCube tCube;",
		"varying vec3 vReflect;",
		"varying vec3 vRefract;",
		"uniform float mReflectionFactor;",
		//custom end

		"void main() {",

		"gl_FragColor = vec4( vec3 ( 1.0 ), opacity );",

		THREE.ShaderChunk["map_fragment"],
		THREE.ShaderChunk["alphatest_fragment"],
		THREE.ShaderChunk["specularmap_fragment"],

		THREE.ShaderChunk["lights_phong_fragment"],

		THREE.ShaderChunk["lightmap_fragment"],
		THREE.ShaderChunk["color_fragment"],
		THREE.ShaderChunk["envmap_fragment"],
		THREE.ShaderChunk["shadowmap_fragment"],

		THREE.ShaderChunk["linear_to_gamma_fragment"],

		THREE.ShaderChunk["fog_fragment"],

		//custom start 
		// "vec4 reflectedColor = textureCube( tCube, vec3( -vReflect.x, vReflect.yz ) );",
		// "vec4 refractedColor = textureCube( tCube, vec3( -vRefract.x, vRefract.yz ) ) ;",
		// "vec4 ReflColor = mix( refractedColor, reflectedColor, clamp( mReflectionFactor, 0.0, 1.0 ) );",

		// "gl_FragColor = mix(gl_FragColor, ReflColor, 0.5);",
		//custom end

		"}"

	].join("\n")

};
