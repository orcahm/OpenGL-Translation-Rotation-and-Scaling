#version 150

in  vec4 vPosition;
in  vec4 vColor;
out vec4 color;

uniform vec3 theta;
uniform vec3 orbit;
uniform int time;
uniform mat4 smallScale;
uniform mat4 smalltraslation;

void main() 
{
    // Compute the sines and cosines of theta for each of
    //   the three axes in one computation.
    vec3 angles = radians( theta );
    vec3 c = cos( angles );
    vec3 s = sin( angles );

    // Remeber: thse matrices are column-major
    mat4 rx = mat4( 1.0,  0.0,  0.0, 0.0,
					0.0,  c.x,  s.x, 0.0,
					 0.0, -s.x,  c.x, 0.0,
					 0.0,  0.0,  0.0, 1.0 );

    mat4 ry = mat4( c.y, 0.0, -s.y, 0.0,
					0.0, 1.0,  0.0, 0.0,
					s.y, 0.0,  c.y, 0.0,
					0.0, 0.0,  0.0, 1.0 );

    // Workaround for bug in ATI driver
    ry[1][0] = 0.0;
    ry[1][1] = 1.0;

    mat4 rz = mat4( c.z, -s.z, 0.0, 0.0,
					s.z,  c.z, 0.0, 0.0,
					0.0,  0.0, 1.0, 0.0,
					0.0,  0.0, 0.0, 1.0 );

    // Workaround for bug in ATI driver
    rz[2][2] = 1.0;
	
	vec3 anglesOrbit = radians( orbit );
    vec3 corbit = cos( orbit );
    vec3 sorbit = sin( orbit );

	 mat4 rzOrbit = mat4( corbit.z, -sorbit.z, 0.0, 0.0,
						sorbit.z,  corbit.z, 0.0, 0.0,
						0.0,  0.0, 1.0, 0.0,
						0.0,  0.0, 0.0, 1.0 );

    // Workaround for bug in ATI driver
    rzOrbit[2][2] = 1.0;

    color = vColor;
    gl_Position =   rzOrbit * smalltraslation * rz * ry * rx * smallScale * vPosition;
} 
