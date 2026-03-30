#ifdef USE_MORPHNORMALS

	#ifdef MORPHTARGETS_TEXTURE

		objectNormal *= morphTargetBaseInfluence;

		for ( int i = 0; i < MORPHTARGETS_COUNT; i ++ ) {

			if ( morphTargetInfluences[ i ] != 0.0 ) objectNormal += getMorph( gl_VertexID, i, 1 ).xyz * morphTargetInfluences[ i ];

		}

	#else

		objectNormal += ( morphNormal0 - normal ) * morphTargetInfluences[ 0 ];
		objectNormal += ( morphNormal1 - normal ) * morphTargetInfluences[ 1 ];
		objectNormal += ( morphNormal2 - normal ) * morphTargetInfluences[ 2 ];
		objectNormal += ( morphNormal3 - normal ) * morphTargetInfluences[ 3 ];

	#endif

#endif
