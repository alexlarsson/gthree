// This is different from specularmap_fragment and used by specglos material

vec3 specularFactor = specular;
#ifdef USE_SPECULARMAP
	vec4 texelSpecular = texture2D( specularMap, vUv );
	texelSpecular = sRGBToLinear( texelSpecular );
	// reads channel RGB, compatible with a glTF Specular-Glossiness (RGBA) texture
	specularFactor *= texelSpecular.rgb;
#endif
