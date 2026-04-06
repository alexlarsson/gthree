varying vec3 vWorldPosition;
varying vec3 vSunDirection;
varying vec3 vBetaR;
varying vec3 vBetaM;
varying float vSunE;

uniform float mieDirectionalG;
uniform vec3 up;
uniform float cloudScale;
uniform float cloudSpeed;
uniform float cloudCoverage;
uniform float cloudDensity;
uniform float cloudElevation;
uniform float time;

float hash( vec2 p ) {
	return fract( sin( dot( p, vec2( 127.1, 311.7 ) ) ) * 43758.5453123 );
}

float noise( vec2 p ) {
	vec2 i = floor( p );
	vec2 f = fract( p );
	f = f * f * ( 3.0 - 2.0 * f );
	float a = hash( i );
	float b = hash( i + vec2( 1.0, 0.0 ) );
	float c = hash( i + vec2( 0.0, 1.0 ) );
	float d = hash( i + vec2( 1.0, 1.0 ) );
	return mix( mix( a, b, f.x ), mix( c, d, f.x ), f.y );
}

float fbm( vec2 p ) {
	float value = 0.0;
	float amplitude = 0.5;
	for ( int i = 0; i < 5; i ++ ) {
		value += amplitude * noise( p );
		p *= 2.0;
		amplitude *= 0.5;
	}
	return value;
}

const float pi = 3.141592653589793238462643383279502884197169;

const float n = 1.0003;
const float N = 2.545E25;

const float rayleighZenithLength = 8.4E3;
const float mieZenithLength = 1.25E3;
const float sunAngularDiameterCos = 0.999956676946448443553574619906976478926848692873900859324;

const float THREE_OVER_SIXTEENPI = 0.05968310365946075;
const float ONE_OVER_FOURPI = 0.07957747154594767;

float rayleighPhase( float cosTheta ) {
	return THREE_OVER_SIXTEENPI * ( 1.0 + pow( cosTheta, 2.0 ) );
}

float hgPhase( float cosTheta, float g ) {
	float g2 = pow( g, 2.0 );
	float inverse = 1.0 / pow( 1.0 - 2.0 * g * cosTheta + g2, 1.5 );
	return ONE_OVER_FOURPI * ( ( 1.0 - g2 ) * inverse );
}

void main() {

	vec3 direction = normalize( vWorldPosition - cameraPosition );

	float zenithAngle = acos( max( 0.0, dot( up, direction ) ) );
	float inverse = 1.0 / ( cos( zenithAngle ) + 0.15 * pow( 93.885 - ( ( zenithAngle * 180.0 ) / pi ), -1.253 ) );
	float sR = rayleighZenithLength * inverse;
	float sM = mieZenithLength * inverse;

	vec3 Fex = exp( -( vBetaR * sR + vBetaM * sM ) );

	float cosTheta = dot( direction, vSunDirection );

	float rPhase = rayleighPhase( cosTheta * 0.5 + 0.5 );
	vec3 betaRTheta = vBetaR * rPhase;

	float mPhase = hgPhase( cosTheta, mieDirectionalG );
	vec3 betaMTheta = vBetaM * mPhase;

	vec3 Lin = pow( vSunE * ( ( betaRTheta + betaMTheta ) / ( vBetaR + vBetaM ) ) * ( 1.0 - Fex ), vec3( 1.5 ) );
	Lin *= mix( vec3( 1.0 ), pow( vSunE * ( ( betaRTheta + betaMTheta ) / ( vBetaR + vBetaM ) ) * Fex, vec3( 1.0 / 2.0 ) ), clamp( pow( 1.0 - dot( up, vSunDirection ), 5.0 ), 0.0, 1.0 ) );

	float theta = acos( direction.y );
	float phi = atan( direction.z, direction.x );
	vec2 uv = vec2( phi, theta ) / vec2( 2.0 * pi, pi ) + vec2( 0.5, 0.0 );
	vec3 L0 = vec3( 0.1 ) * Fex;

	float sundisk = smoothstep( sunAngularDiameterCos, sunAngularDiameterCos + 0.00002, cosTheta );
	L0 += ( vSunE * 19000.0 * Fex ) * sundisk;

	vec3 texColor = ( Lin + L0 ) * 0.04 + vec3( 0.0, 0.0003, 0.00075 );

	if ( direction.y > 0.0 && cloudCoverage > 0.0 ) {

		float elevation = mix( 1.0, 0.1, cloudElevation );
		vec2 cloudUV = direction.xz / ( direction.y * elevation );
		cloudUV *= cloudScale;
		cloudUV += time * cloudSpeed;

		float cloudNoise = fbm( cloudUV * 1000.0 );
		cloudNoise += 0.5 * fbm( cloudUV * 2000.0 + 3.7 );
		cloudNoise = cloudNoise * 0.5 + 0.5;

		float cloudMask = smoothstep( 1.0 - cloudCoverage, 1.0 - cloudCoverage + 0.3, cloudNoise );

		float horizonFade = smoothstep( 0.0, 0.1 + 0.2 * cloudElevation, direction.y );
		cloudMask *= horizonFade;

		float sunInfluence = dot( direction, vSunDirection ) * 0.5 + 0.5;
		float daylight = max( 0.0, vSunDirection.y * 2.0 );

		vec3 atmosphereColor = Lin * 0.04;
		vec3 cloudColor = mix( vec3( 0.3 ), vec3( 1.0 ), daylight );
		cloudColor = mix( cloudColor, atmosphereColor + vec3( 1.0 ), sunInfluence * 0.5 );
		cloudColor *= vSunE * 0.00002;

		texColor = mix( texColor, cloudColor, cloudMask * cloudDensity );

	}

	gl_FragColor = vec4( texColor, 1.0 );

	#include <tonemapping_fragment>
	#include <colorspace_fragment>

}
