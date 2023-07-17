/**
 * @author alteredq / http://alteredqualia.com/
 *
 * Based on Nvidia Cg tutorial
 */

THREE.DialuxShader = {

	uniforms: THREE.UniformsUtils.merge([

		//THREE.UniformsLib["common"]
		{

			"diffuse": { type: "c", value: new THREE.Color(0xff0000) },
			"envMap": { type: "t", value: null },
			"reflectivity": { type: "f", value: 1.0 },
			"refractionRatio": { type: "f", value: 0.98 },


			// not use
			"opacity": { type: "f", value: 1.0 },
			"map": { type: "t", value: null },
			"offsetRepeat": { type: "v4", value: new THREE.Vector4(0, 0, 1, 1) },
			"lightMap": { type: "t", value: null },
			"specularMap": { type: "t", value: null },
			"flipEnvMap": { type: "f", value: -1 },
			"useRefract": { type: "i", value: 0 },
			"combine": { type: "i", value: 0 },
			// -------

		},

		// THREE.UniformsLib["phong"],
		{
			"ambient": { type: "c", value: new THREE.Color(0x000000) },
			"emissive": { type: "c", value: new THREE.Color(0xff0000) },
			"specular": { type: "c", value: new THREE.Color(0x111111) },
			"shininess": { type: "f", value: 30 },
		},

		// THREE.UniformsLib["lights"],
		{

			"ambientLightColor": { type: "fv", value: [] },

			"directionalLightDirection": { type: "fv", value: [] },
			"directionalLightColor": { type: "fv", value: [] },

			"hemisphereLightDirection": { type: "fv", value: [] },
			"hemisphereLightSkyColor": { type: "fv", value: [] },
			"hemisphereLightGroundColor": { type: "fv", value: [] },

			"pointLightColor": { type: "fv", value: [] },
			"pointLightPosition": { type: "fv", value: [] },
			"pointLightDistance": { type: "fv1", value: [] },

			"spotLightColor": { type: "fv", value: [] },
			"spotLightPosition": { type: "fv", value: [] },
			"spotLightDirection": { type: "fv", value: [] },
			"spotLightDistance": { type: "fv1", value: [] },
			"spotLightAngleCos": { type: "fv1", value: [] },
			"spotLightExponent": { type: "fv1", value: [] }

		},



	]),

	vertexShader: [

		//"#define PHONG",

		"varying vec3 vViewPosition;",
		"varying vec3 vNormal;",

		"varying vec3 vReflect;",
		"uniform float refractionRatio;",
		// ----------------------------------------

		//custom start 
		"varying vec3 vRefract;",
		// custom end


		THREE.ShaderChunk["lights_phong_pars_vertex"],
		THREE.ShaderChunk["color_pars_vertex"],

		"void main() {",

		THREE.ShaderChunk["color_vertex"],
		THREE.ShaderChunk["defaultnormal_vertex"],

		"vNormal = normalize( transformedNormal );",

		THREE.ShaderChunk["default_vertex"],

		"vViewPosition = -mvPosition.xyz;",

		THREE.ShaderChunk["worldpos_vertex"],
		//THREE.ShaderChunk["envmap_vertex"],
		"vec3 worldNormal = mat3( modelMatrix[ 0 ].xyz, modelMatrix[ 1 ].xyz, modelMatrix[ 2 ].xyz ) * objectNormal;",
		"worldNormal = normalize( worldNormal );",

		"vec3 cameraToVertex = normalize( worldPosition.xyz - cameraPosition );",

		"vRefract = refract( cameraToVertex, worldNormal, refractionRatio );",
		"vReflect = reflect( cameraToVertex, worldNormal );",
		// ---------------------------------------------------------------


		THREE.ShaderChunk["lights_phong_vertex"],

		"}"

	].join("\n"),

	fragmentShader: [

		"uniform vec3 diffuse;",
		//"uniform float opacity;",

		"uniform vec3 ambient;",
		"uniform vec3 emissive;",
		"uniform vec3 specular;",
		"uniform float shininess;",


		//THREE.ShaderChunk["envmap_pars_fragment"], 
		"uniform float reflectivity;",
		"uniform samplerCube envMap;",
		//"uniform float flipEnvMap;",
		//"uniform int combine;",
		"varying vec3 vReflect;",
		// ----------------------------------------------------------------

		//custom start
		"uniform float transmission;",
		"varying vec3 vRefract;",
		//custom end

		THREE.ShaderChunk["lights_phong_pars_fragment"],




		"void main() {",

		"gl_FragColor = vec4( vec3 ( 1.0 ), 1.0 );", //opacity


		"float specularStrength = reflectivity;", //1.0




		//	THREE.ShaderChunk["envmap_fragment"],
		"vec4 reflectedColor = textureCube( envMap, vec3( - vReflect.x, vReflect.yz ) );",
		"vec4 refractedColor = textureCube( envMap, vec3( - vRefract.x, vRefract.yz ) );",
		"vec4 cubeColor = mix( refractedColor, reflectedColor, clamp( transmission, 0.0, 1.0 ) );",
		// --------------------------------------------------------------------------------------------------



		// THREE.ShaderChunk["lights_phong_fragment "],
		"vec3 normal = normalize( vNormal );",
		"vec3 viewPosition = normalize( vViewPosition );",


		"#if MAX_POINT_LIGHTS > 0",

		"vec3 pointDiffuse  = vec3( 0.0 );",
		"vec3 pointSpecular = vec3( 0.0 );",

		"for ( int i = 0; i < MAX_POINT_LIGHTS; i ++ ) {",

		"vec4 lPosition = viewMatrix * vec4( pointLightPosition[ i ], 1.0 );",
		"vec3 lVector = lPosition.xyz + vViewPosition.xyz;",

		"float lDistance = 1.0;",
		"if ( pointLightDistance[ i ] > 0.0 )",
		"lDistance = 1.0 - min( ( length( lVector ) / pointLightDistance[ i ] ), 1.0 );",

		"lVector = normalize( lVector );",

		// diffuse

		"float dotProduct = dot( normal, lVector );",

		"float pointDiffuseWeight = max( dotProduct, 0.0 );",

		"pointDiffuse  += diffuse * pointLightColor[ i ] * pointDiffuseWeight * lDistance;",

		// specular

		"vec3 pointHalfVector = normalize( lVector + viewPosition );",
		"float pointDotNormalHalf = max( dot( normal, pointHalfVector ), 0.0 );",
		"float pointSpecularWeight = specularStrength * max( pow( pointDotNormalHalf, shininess ), 0.0 );",

		"pointSpecular += specular * pointLightColor[ i ] * pointSpecularWeight * pointDiffuseWeight * lDistance;",

		"}",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0",

		"vec3 spotDiffuse  = vec3( 0.0 );",
		"vec3 spotSpecular = vec3( 0.0 );",

		"for ( int i = 0; i < MAX_SPOT_LIGHTS; i ++ ) {",

		"vec4 lPosition = viewMatrix * vec4( spotLightPosition[ i ], 1.0 );",
		"vec3 lVector = lPosition.xyz + vViewPosition.xyz;",

		"float lDistance = 1.0;",
		"if ( spotLightDistance[ i ] > 0.0 )",
		"lDistance = 1.0 - min( ( length( lVector ) / spotLightDistance[ i ] ), 1.0 );",

		"lVector = normalize( lVector );",

		"float spotEffect = dot( spotLightDirection[ i ], normalize( spotLightPosition[ i ] - vWorldPosition ) );",

		"if ( spotEffect > spotLightAngleCos[ i ] ) {",

		"spotEffect = max( pow( spotEffect, spotLightExponent[ i ] ), 0.0 );",

		// diffuse

		"float dotProduct = dot( normal, lVector );",

		"float spotDiffuseWeight = max( dotProduct, 0.0 );",
		"spotDiffuse += diffuse * spotLightColor[ i ] * spotDiffuseWeight * lDistance * spotEffect;",

		// specular

		"vec3 spotHalfVector = normalize( lVector + viewPosition );",
		"float spotDotNormalHalf = max( dot( normal, spotHalfVector ), 0.0 );",
		"float spotSpecularWeight = specularStrength * max( pow( spotDotNormalHalf, shininess ), 0.0 );",

		"spotSpecular += specular * spotLightColor[ i ] * spotSpecularWeight * spotDiffuseWeight * lDistance * spotEffect;",


		"}",

		"}",

		"#endif",

		"#if MAX_DIR_LIGHTS > 0",

		"vec3 dirDiffuse  = vec3( 0.0 );",
		"vec3 dirSpecular = vec3( 0.0 );",

		"for( int i = 0; i < MAX_DIR_LIGHTS; i ++ ) {",

		"vec4 lDirection = viewMatrix * vec4( directionalLightDirection[ i ], 0.0 );",
		"vec3 dirVector = normalize( lDirection.xyz );",

		// diffuse

		"float dotProduct = dot( normal, dirVector );",


		"float dirDiffuseWeight = max( dotProduct, 0.0 );",

		// "#endif",

		"dirDiffuse  += diffuse * directionalLightColor[ i ] * dirDiffuseWeight;",

		// specular

		"vec3 dirHalfVector = normalize( dirVector + viewPosition );",
		"float dirDotNormalHalf = max( dot( normal, dirHalfVector ), 0.0 );",
		"float dirSpecularWeight = specularStrength * max( pow( dirDotNormalHalf, shininess ), 0.0 );",


		"dirSpecular += specular * directionalLightColor[ i ] * dirSpecularWeight * dirDiffuseWeight;",

		"}",

		"#endif",

		"#if MAX_HEMI_LIGHTS > 0",

		"vec3 hemiDiffuse  = vec3( 0.0 );",
		"vec3 hemiSpecular = vec3( 0.0 );",

		"for( int i = 0; i < MAX_HEMI_LIGHTS; i ++ ) {",

		"vec4 lDirection = viewMatrix * vec4( hemisphereLightDirection[ i ], 0.0 );",
		"vec3 lVector = normalize( lDirection.xyz );",

		// diffuse

		"float dotProduct = dot( normal, lVector );",
		"float hemiDiffuseWeight = 0.5 * dotProduct + 0.5;",

		"vec3 hemiColor = mix( hemisphereLightGroundColor[ i ], hemisphereLightSkyColor[ i ], hemiDiffuseWeight );",

		"hemiDiffuse += diffuse * hemiColor;",

		// specular (sky light)

		"vec3 hemiHalfVectorSky = normalize( lVector + viewPosition );",
		"float hemiDotNormalHalfSky = 0.5 * dot( normal, hemiHalfVectorSky ) + 0.5;",
		"float hemiSpecularWeightSky = specularStrength * max( pow( hemiDotNormalHalfSky, shininess ), 0.0 );",

		// specular (ground light)

		"vec3 lVectorGround = -lVector;",

		"vec3 hemiHalfVectorGround = normalize( lVectorGround + viewPosition );",
		"float hemiDotNormalHalfGround = 0.5 * dot( normal, hemiHalfVectorGround ) + 0.5;",
		"float hemiSpecularWeightGround = specularStrength * max( pow( hemiDotNormalHalfGround, shininess ), 0.0 );",

		"hemiSpecular += specular * hemiColor * ( hemiSpecularWeightSky + hemiSpecularWeightGround ) * hemiDiffuseWeight;",

		"}",

		"#endif",

		"vec3 totalDiffuse = vec3( 0.0 );",
		"vec3 totalSpecular = vec3( 0.0 );",

		"#if MAX_DIR_LIGHTS > 0",

		"totalDiffuse += dirDiffuse;",
		"totalSpecular += dirSpecular;",

		"#endif",

		"#if MAX_HEMI_LIGHTS > 0",

		"totalDiffuse += hemiDiffuse;",
		"totalSpecular += hemiSpecular;",

		"#endif",

		"#if MAX_POINT_LIGHTS > 0",

		"totalDiffuse += pointDiffuse;",
		"totalSpecular += pointSpecular;",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0",

		"totalDiffuse += spotDiffuse;",
		"totalSpecular += spotSpecular;",

		"#endif",



		"gl_FragColor.xyz = cubeColor.xyz * ambient.xyz * reflectivity;",
		"gl_FragColor.xyz += ( totalDiffuse.xyz * (1.0 - reflectivity)) ;",
		"gl_FragColor.xyz += ( totalSpecular.xyz * reflectivity) ;",


		"}"

	].join("\n")

}


class DialuxMaterial extends THREE.MeshPhongMaterial {

	constructor(parameters) {
		super(parameters);

		var transmission = (parameters.transmission === undefined) ? 1.0 : parameters.transmission;

		this.uniforms = {
			...THREE.DialuxShader.uniforms, transmission: { type: "f", value: transmission },

		};
		this.vertexShader = THREE.DialuxShader.vertexShader;
		this.fragmentShader = THREE.DialuxShader.fragmentShader;
	}

}



