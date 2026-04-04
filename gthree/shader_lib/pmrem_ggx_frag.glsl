varying vec3 vOutputDirection;

uniform sampler2D envMap;
uniform float roughness;
uniform float mipInt;

#define cubeUV_minMipLevel 4.0
#define cubeUV_minTileSize 16.0

float getFace( vec3 direction ) {
	vec3 absDirection = abs( direction );
	float face = - 1.0;

	if ( absDirection.x > absDirection.z ) {
		if ( absDirection.x > absDirection.y )
			face = direction.x > 0.0 ? 0.0 : 3.0;
		else
			face = direction.y > 0.0 ? 1.0 : 4.0;
	} else {
		if ( absDirection.z > absDirection.y )
			face = direction.z > 0.0 ? 2.0 : 5.0;
		else
			face = direction.y > 0.0 ? 1.0 : 4.0;
	}

	return face;
}

vec2 getUV( vec3 direction, float face ) {
	vec2 uv;

	if ( face == 0.0 ) {
		uv = vec2( direction.z, direction.y ) / abs( direction.x );
	} else if ( face == 1.0 ) {
		uv = vec2( - direction.x, - direction.z ) / abs( direction.y );
	} else if ( face == 2.0 ) {
		uv = vec2( - direction.x, direction.y ) / abs( direction.z );
	} else if ( face == 3.0 ) {
		uv = vec2( - direction.z, direction.y ) / abs( direction.x );
	} else if ( face == 4.0 ) {
		uv = vec2( - direction.x, direction.z ) / abs( direction.y );
	} else {
		uv = vec2( direction.x, direction.y ) / abs( direction.z );
	}

	return 0.5 * ( uv + 1.0 );
}

vec3 bilinearCubeUV( sampler2D envMap, vec3 direction, float mipInt ) {
	float face = getFace( direction );

	float filterInt = max( cubeUV_minMipLevel - mipInt, 0.0 );
	mipInt = max( mipInt, cubeUV_minMipLevel );
	float faceSize = exp2( mipInt );

	vec2 uv = getUV( direction, face ) * ( faceSize - 2.0 ) + 1.0;

	if ( face > 2.0 ) {
		uv.y += faceSize;
		face -= 3.0;
	}

	uv.x += face * faceSize;
	uv.x += filterInt * 3.0 * cubeUV_minTileSize;
	uv.y += 4.0 * ( exp2( CUBEUV_MAX_MIP ) - faceSize );

	uv.x *= CUBEUV_TEXEL_WIDTH;
	uv.y *= CUBEUV_TEXEL_HEIGHT;

	return texture2D( envMap, uv ).rgb;
}

#define PI 3.14159265359

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint N) {
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

vec3 importanceSampleGGX_VNDF(vec2 Xi, vec3 V, float roughness) {
	float alpha = roughness * roughness;

	vec3 T1 = vec3(1.0, 0.0, 0.0);
	vec3 T2 = cross(V, T1);

	float r = sqrt(Xi.x);
	float phi = 2.0 * PI * Xi.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + V.z);
	t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

	vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * V;

	return normalize(vec3(alpha * Nh.x, alpha * Nh.y, max(0.0, Nh.z)));
}

void main() {
	vec3 N = normalize(vOutputDirection);
	vec3 V = N;

	vec3 prefilteredColor = vec3(0.0);
	float totalWeight = 0.0;

	if (roughness < 0.001) {
		gl_FragColor = vec4(bilinearCubeUV(envMap, N, mipInt), 1.0);
		return;
	}

	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	for(uint i = 0u; i < uint(GGX_SAMPLES); i++) {
		vec2 Xi = hammersley(i, uint(GGX_SAMPLES));

		vec3 H_tangent = importanceSampleGGX_VNDF(Xi, vec3(0.0, 0.0, 1.0), roughness);

		vec3 H = normalize(tangent * H_tangent.x + bitangent * H_tangent.y + N * H_tangent.z);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);

		if(NdotL > 0.0) {
			vec3 sampleColor = bilinearCubeUV(envMap, L, mipInt);
			prefilteredColor += sampleColor * NdotL;
			totalWeight += NdotL;
		}
	}

	if (totalWeight > 0.0) {
		prefilteredColor = prefilteredColor / totalWeight;
	}

	gl_FragColor = vec4(prefilteredColor, 1.0);
}
