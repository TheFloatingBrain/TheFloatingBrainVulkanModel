#version 450

layout( location = 0 ) out vec4 fragmentColor;

vec3 positions[ 5 ] = vec3[]( 
		vec3( -.5, -.5, 1.0 ), 
		vec3( -.5, -.5, 0.0 ), 
		vec3( 0.5, -.5, 0.0 ), 
		vec3( 0.5, -.5, 1.0 ), 
		vec3( 0.0, 0.5, 0.0 )
);

vec4 colors[ 5 ] = vec4[]( 
		vec4( 1.0, 0.0, 0.0, 1.0 ), 
		vec4( 0.0, 1.0, 0.0, 1.0 ), 
		vec4( 0.0, 0.0, 1.0, 1.0 ), 
		vec4( 1.0, 1.0, 0.0, 1.0 ), 
		vec4( 1.0, 0.0, 1.0, 1.0 )
);


void main() {
	gl_Position = vec4( positions[ gl_VertexIndex ], 1.0 );
	fragmentColor = colors[ gl_VertexIndex ];
}
