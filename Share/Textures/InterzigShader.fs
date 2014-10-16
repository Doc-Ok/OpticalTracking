// Fragment shader to interzig view zone images into autostereoscopic display

uniform sampler2D viewZonesTexture;
uniform sampler2D viewMapTexture;
uniform vec2 quadSize;

void main()
	{
	/* Get the view map pixel value: */
	vec3 viewMap=texture2D(viewMapTexture,gl_TexCoord[1].st+vec2(0.00048828125*2.0,0.0)).rgb;
	
	/* Convert color to view zone row/column: */
	vec3 index=viewMap*15.9375+0.5; // index=color*255/16+fuzz
	vec3 row=floor(index/3.0);
	vec3 col=floor(mod(index,3.0));
	
	/* Compute view zone texture coordinates for RGB components: */
	vec2 rTexCoord;
	rTexCoord.s=gl_TexCoord[0].s+col.x*quadSize.s;
	rTexCoord.t=gl_TexCoord[0].t+row.x*quadSize.t;
	vec2 gTexCoord;
	gTexCoord.s=gl_TexCoord[0].s+col.y*quadSize.s;
	gTexCoord.t=gl_TexCoord[0].t+row.y*quadSize.t;
	vec2 bTexCoord;
	bTexCoord.s=gl_TexCoord[0].s+col.z*quadSize.s;
	bTexCoord.t=gl_TexCoord[0].t+row.z*quadSize.t;
	
	/* Assemble the final pixel color from its components: */
	vec4 color;
	color.r=texture2D(viewZonesTexture,rTexCoord).r;
	color.g=texture2D(viewZonesTexture,gTexCoord).g;
	color.b=texture2D(viewZonesTexture,bTexCoord).b;
	
	/* Set the fragment color: */
	gl_FragColor=color;
	}
