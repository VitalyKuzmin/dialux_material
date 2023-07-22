var DialuxShader = {

	uniforms: {

		// params ----------------------------------------------------------------
		"diffuse": { type: "c", value: new THREE.Color(0xff0000) },
		"ambient": { type: "c", value: new THREE.Color(0x000000) },
		"specular": { type: "c", value: new THREE.Color(0x111111) },
		"shininess": { type: "f", value: 30 },
		"refractionRatio": { type: "f", value: 1.00 },
		"opacity": { type: "f", value: 1.0 },
		"envMap": { type: "t", value: null },


		// lights ----------------------------------------------------------------
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
		"spotLightExponent": { type: "fv1", value: [] },


		// not use
		"emissive": { type: "c", value: new THREE.Color(0xff0000) },
		"reflectivity": { type: "f", value: 1.0 },
		"map": { type: "t", value: null },
		"offsetRepeat": { type: "v4", value: new THREE.Vector4(0, 0, 1, 1) },
		"lightMap": { type: "t", value: null },
		"specularMap": { type: "t", value: null },
		"flipEnvMap": { type: "f", value: -1 },
		"useRefract": { type: "i", value: 0 },
		"combine": { type: "i", value: 0 },

	}



	,

	vertexShader: [

		"uniform float refractionRatio;",

		"varying vec3 vViewPosition;",
		"varying vec3 vNormal;",
		"varying vec3 vReflect;",
		"varying vec3 vRefract;",


		// lights_phong_pars_vertex ----------------------------------------

		"#ifndef PHONG_PER_PIXEL",

		"#if MAX_POINT_LIGHTS > 0",

		"uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ];",
		"uniform float pointLightDistance[ MAX_POINT_LIGHTS ];",

		"varying vec4 vPointLight[ MAX_POINT_LIGHTS ];",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0",

		"uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ];",
		"uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];",

		"varying vec4 vSpotLight[ MAX_SPOT_LIGHTS ];",

		"#endif",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP )",

		"varying vec3 vWorldPosition;",

		"#endif",
		// ----------------------------------------------------------------


		"void main() {",


		"vec3 objectNormal = normal;",
		"vec3 transformedNormal = normalMatrix * objectNormal;",

		"vNormal = normalize( transformedNormal );",

		"vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );",
		"gl_Position = projectionMatrix * mvPosition;",

		"vViewPosition = -mvPosition.xyz;",

		// envMap ----------------------------------------------------------------
		"vec4 worldPosition = modelMatrix * vec4( position, 1.0 );",
		"vec3 worldNormal = mat3( modelMatrix[ 0 ].xyz, modelMatrix[ 1 ].xyz, modelMatrix[ 2 ].xyz ) * objectNormal;",
		"worldNormal = normalize( worldNormal );",

		//	"float length = sqrt(pow(cameraToVertex.x,2.0)+pow(cameraToVertex.y,2.0)+pow(cameraToVertex.z,2.0));",
		//	"float K = (length - 50.0)/length;",
		"vec3 cameraToVertex = normalize( worldPosition.xyz - cameraPosition );",
		"cameraToVertex = normalize(cameraToVertex);",
		"vRefract = refract( cameraToVertex, worldNormal, 1.0/refractionRatio );",
		"vReflect = reflect( cameraToVertex, worldNormal );",

		// 	lights_phong_vertex ---------------------------------------------------------------

		"#ifndef PHONG_PER_PIXEL",

		"#if MAX_POINT_LIGHTS > 0",

		"for( int i = 0; i < MAX_POINT_LIGHTS; i ++ ) {",

		"vec4 lPosition = viewMatrix * vec4( pointLightPosition[ i ], 1.0 );",
		"vec3 lVector = lPosition.xyz - mvPosition.xyz;",

		"float lDistance = 1.0;",
		"if ( pointLightDistance[ i ] > 0.0 )",
		"lDistance = 1.0 - min( ( length( lVector ) / pointLightDistance[ i ] ), 1.0 );",

		"vPointLight[ i ] = vec4( lVector, lDistance );",

		"}",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0",

		"for( int i = 0; i < MAX_SPOT_LIGHTS; i ++ ) {",

		"vec4 lPosition = viewMatrix * vec4( spotLightPosition[ i ], 1.0 );",
		"vec3 lVector = lPosition.xyz - mvPosition.xyz;",

		"float lDistance = 1.0;",
		"if ( spotLightDistance[ i ] > 0.0 )",
		"lDistance = 1.0 - min( ( length( lVector ) / spotLightDistance[ i ] ), 1.0 );",

		"vSpotLight[ i ] = vec4( lVector, lDistance );",

		"}",

		"#endif",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP )",

		"vWorldPosition = worldPosition.xyz;",

		"#endif",

		// ----------------------------------------------------------------


		"}"

	].join("\n"),

	fragmentShader: [

		"uniform vec3 diffuse;",
		"uniform vec3 ambient;",
		"uniform vec3 specular;",
		"uniform float shininess;",
		"uniform float opacity;",
		"uniform samplerCube envMap;",

		"varying vec3 vReflect;",
		"varying vec3 vRefract;",

		// lights_phong_pars_fragment ----------------------------------------------------------------

		"uniform vec3 ambientLightColor;",

		"#if MAX_DIR_LIGHTS > 0",

		"uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ];",
		"uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];",

		"#endif",

		"#if MAX_HEMI_LIGHTS > 0",

		"uniform vec3 hemisphereLightSkyColor[ MAX_HEMI_LIGHTS ];",
		"uniform vec3 hemisphereLightGroundColor[ MAX_HEMI_LIGHTS ];",
		"uniform vec3 hemisphereLightDirection[ MAX_HEMI_LIGHTS ];",

		"#endif",

		"#if MAX_POINT_LIGHTS > 0",

		"uniform vec3 pointLightColor[ MAX_POINT_LIGHTS ];",

		"#ifdef PHONG_PER_PIXEL",

		"uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ];",
		"uniform float pointLightDistance[ MAX_POINT_LIGHTS ];",

		"#else",

		"varying vec4 vPointLight[ MAX_POINT_LIGHTS ];",

		"#endif",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0",

		"uniform vec3 spotLightColor[ MAX_SPOT_LIGHTS ];",
		"uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ];",
		"uniform vec3 spotLightDirection[ MAX_SPOT_LIGHTS ];",
		"uniform float spotLightAngleCos[ MAX_SPOT_LIGHTS ];",
		"uniform float spotLightExponent[ MAX_SPOT_LIGHTS ];",

		"#ifdef PHONG_PER_PIXEL",

		"uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];",

		"#else",

		"varying vec4 vSpotLight[ MAX_SPOT_LIGHTS ];",

		"#endif",

		"#endif",

		"#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP )",

		"varying vec3 vWorldPosition;",

		"#endif",

		"#ifdef WRAP_AROUND",

		"uniform vec3 wrapRGB;",

		"#endif",

		"varying vec3 vViewPosition;",
		"varying vec3 vNormal;",


		"void main() {",

		"gl_FragColor = vec4( vec3 ( 1.0 ), 1.0 );",
		"float specularStrength = 1.0;",


		//	Envmap mix ----------------------------------------------------
		// Смешивается карта отражения и карта пропускания с соотношением в зависимости от параметра opacity:
		"vec4 reflectedColor = textureCube( envMap, vec3( - vReflect.x, vReflect.yz ) );",
		"vec4 refractedColor = textureCube( envMap, vec3(  - vRefract.x, vRefract.yz ) );",
		"vec4 cubeColor = mix( reflectedColor, refractedColor , clamp( opacity, 0.0, 1.0 ) );",

		// lights_phong_fragment  ----------------------------------------------------
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


		// SUM ------------------------------------------------------------------

		"gl_FragColor.xyz = cubeColor.xyz * ambient.xyz;", //Добавляется составляющая окружения (зеркальная или прозрачная)
		"gl_FragColor.xyz += totalDiffuse.xyz ;",		 // Добавляется  диффузная составляющая	
		"gl_FragColor.xyz += totalSpecular.xyz ;",		// Добавляется  блик	

		"}"

	].join("\n")

}

class DialuxMaterial extends THREE.MeshPhongMaterial {

	constructor(parameters) {
		super(parameters);

		this.uniforms = DialuxShader.uniforms;
		this.vertexShader = DialuxShader.vertexShader;
		this.fragmentShader = DialuxShader.fragmentShader;
	}

}

// export { DialuxMaterial }

