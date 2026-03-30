#ifdef USE_MORPHTARGETS

	#ifdef MORPHTARGETS_TEXTURE

		transformed *= morphTargetBaseInfluence;

		for ( int i = 0; i < MORPHTARGETS_COUNT; i ++ ) {

			if ( morphTargetInfluences[ i ] != 0.0 ) transformed += getMorph( gl_VertexID, i, 0 ).xyz * morphTargetInfluences[ i ];

		}

	#else

		transformed += ( morphTarget0 - position ) * morphTargetInfluences[ 0 ];
		transformed += ( morphTarget1 - position ) * morphTargetInfluences[ 1 ];
		transformed += ( morphTarget2 - position ) * morphTargetInfluences[ 2 ];
		transformed += ( morphTarget3 - position ) * morphTargetInfluences[ 3 ];

		#ifndef USE_MORPHNORMALS

			transformed += ( morphTarget4 - position ) * morphTargetInfluences[ 4 ];
			transformed += ( morphTarget5 - position ) * morphTargetInfluences[ 5 ];
			transformed += ( morphTarget6 - position ) * morphTargetInfluences[ 6 ];
			transformed += ( morphTarget7 - position ) * morphTargetInfluences[ 7 ];

		#endif

	#endif

#endif
