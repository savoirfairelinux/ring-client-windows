#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D Ytex, Utex, Vtex;
varying vec2 vTextureCoord;

void main(void) {
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture2D(Ytex, vTextureCoord).r;
    yuv.y = texture2D(Utex, vTextureCoord).r - 0.5;
    yuv.z = texture2D(Vtex, vTextureCoord).r - 0.5;

    rgb = mat3(1,1,1,
               0,-0.39465,2.03211,
               1.13983, -0.58060,  0) * yuv;
    gl_FragColor = vec4(rgb, 1);
}