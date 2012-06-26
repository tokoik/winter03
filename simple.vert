#version 120
//
// simple.vert
//
uniform vec2 scr;
uniform vec3 org, dir;
attribute vec3 p0, e1, e2;
attribute vec3 v0, v1, v2;
varying float u, v;
varying vec3 n0, n1, n2;

void main(void)
{
  vec3 q = cross(dir, e2);
  float s = dot(q, e1);
  float t = -2.0;
  
  if (s != 0.0) {
    vec3 e = org - p0; 
    vec3 r = cross(e, e1);
    
    t = dot(r, e2) / s;
    u = dot(q, e) / s;
    v = dot(r, dir) / s;

	  n0 = v0;
	  n1 = v1;
	  n2 = v2;
  }

  gl_Position = vec4(scr, t, 1.0);
}
