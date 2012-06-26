#version 120
//
// simple.frag
//
varying float u, v;
varying vec3 n0, n1, n2;

void main(void)
{
  float w = 1.0 - u - v;
  if (any(lessThan(vec3(u, v, w), vec3(0.0)))) discard;
  
  vec3 n = normalize(w * n0 + u * n1 + v * n2);
  gl_FragColor = vec4(n.z, n.z, n.z, 1.0);
}
