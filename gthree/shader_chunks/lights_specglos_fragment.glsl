PhysicalMaterial material;
material.diffuseColor = diffuseColor.rgb;
material.specularRoughness = clamp( 1.0 - glossinessFactor, 0.04, 1.0 );
material.specularColor = specularFactor.rgb;
