attribute vec3 position;
attribute vec2 uv;
attribute float faceIndex;

varying vec3 vOutputDirection;

vec3 getDirection( vec2 uv, float face ) {
	uv = 2.0 * uv - 1.0;

	vec3 direction = vec3( uv, 1.0 );

	if ( face == 0.0 ) {
		direction = direction.zyx;
	} else if ( face == 1.0 ) {
		direction = direction.xzy;
		direction.xz *= -1.0;
	} else if ( face == 2.0 ) {
		direction.x *= -1.0;
	} else if ( face == 3.0 ) {
		direction = direction.zyx;
		direction.xz *= -1.0;
	} else if ( face == 4.0 ) {
		direction = direction.xzy;
		direction.xy *= -1.0;
	} else if ( face == 5.0 ) {
		direction.z *= -1.0;
	}

	return direction;
}

void main() {
	vOutputDirection = getDirection( uv, faceIndex );
	gl_Position = vec4( position, 1.0 );
}
