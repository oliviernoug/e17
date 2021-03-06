"#ifdef GL_ES\n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
"precision highp float;\n"
"#else\n"
"precision mediump float;\n"
"#endif\n"
"#endif\n"
"uniform sampler2D tex;\n"
"uniform sampler1D gaussian;\n"
"varying vec4 col;\n"
"varying vec2 tex_c;\n"
"varying weight;\n"
"uniform radius;\n"
"void main()\n"
"{\n"
"	int i;\n"
"	vec4 fc = vec4(0,0,0,0);\n"
"	\n"
"	for (i = 0 ; i < radius ; i ++){\n"
"		fc += texture2D(tex, tex_c.xy).rgba *\n"
"			texture1D(gaussian,i/radius).aaaa;\n"
"	}\n"
"	gl_FragColor = fc / 4 * col;\n"
"}\n"
