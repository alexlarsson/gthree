#ifdef USE_SKINNING

	uniform mat4 bindMatrix;
	uniform mat4 bindMatrixInverse;

	#ifdef BONE_TEXTURE

		uniform highp sampler2D boneTexture;

		mat4 getBoneMatrix( const in float i ) {

			int size = textureSize( boneTexture, 0 ).x;
			int j = int( i ) * 4;
			int x = j % size;
			int y = j / size;
			vec4 v1 = texelFetch( boneTexture, ivec2( x, y ), 0 );
			vec4 v2 = texelFetch( boneTexture, ivec2( x + 1, y ), 0 );
			vec4 v3 = texelFetch( boneTexture, ivec2( x + 2, y ), 0 );
			vec4 v4 = texelFetch( boneTexture, ivec2( x + 3, y ), 0 );

			return mat4( v1, v2, v3, v4 );

		}

	#else

		uniform mat4 boneMatrices[ MAX_BONES ];

		mat4 getBoneMatrix( const in float i ) {

			return boneMatrices[ int(i) ];

		}

	#endif

#endif
