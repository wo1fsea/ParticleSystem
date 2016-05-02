uniform sampler2D tex;

void main()
{
//gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
vec4 color = texture2D(tex, gl_TexCoord[0].st);
gl_FragColor = vec4(color.b, color.r, color.g, color.a);
}