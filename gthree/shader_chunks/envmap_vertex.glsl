#if defined( USE_ENVMAP ) && ! defined( USE_BUMPMAP ) && ! defined( USE_NORMALMAP )

	vec3 worldNormal = transformDirection( objectNormal, modelMatrix );

	vec3 cameraToVertex = normalize( worldPosition.xyz - cameraPosition );

	if ( useRefract ) {

		vReflect = refract( cameraToVertex, worldNormal, refractionRatio );

	} else {

		vReflect = reflect( cameraToVertex, worldNormal );

	}

#endif
