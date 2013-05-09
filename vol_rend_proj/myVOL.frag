varying vec3  textureCoords;

uniform sampler3D volumeTex;
uniform sampler1D tfTex;

uniform sampler1D gridTexX;
uniform sampler1D gridTexY;
uniform sampler1D gridTexZ;

uniform float sampleSpacing;
uniform vec3  centralDifferenceSpacing;
uniform vec4  lightPar;	
uniform vec3  eyePos;
uniform vec3  eyeDir;
uniform int   bOrtho;  
uniform int   drawSlice;


struct Light
{
    float ambient_diffuse;
    float specular;
};

vec3 getNormal(vec3 texPosition)
{
    vec3 gradient;
    
    gradient.x=texture3D(volumeTex,texPosition.xyz+vec3(centralDifferenceSpacing.r,0,0)).r
              -texture3D(volumeTex,texPosition.xyz+vec3(-centralDifferenceSpacing.r,0,0)).r;
    gradient.y=texture3D(volumeTex,texPosition.xyz+vec3(0,centralDifferenceSpacing.g,0)).r
              -texture3D(volumeTex,texPosition.xyz+vec3(0,-centralDifferenceSpacing.g,0)).r;
    gradient.z=texture3D(volumeTex,texPosition.xyz+vec3(0,0,centralDifferenceSpacing.b)).r
              -texture3D(volumeTex,texPosition.xyz+vec3(0,0,-centralDifferenceSpacing.b)).r;
    gradient=gradient * 10.0;
        
    if(length(gradient) > 0.0)
        gradient = normalize(gradient);
    
    return gradient;
}


vec4 getColor(vec3 texPosition)
{
    float x = texture1D(gridTexX, texPosition.x).r;
    float y = texture1D(gridTexY, texPosition.y).r;
    float z = texture1D(gridTexZ, texPosition.z).r;
     
    //float scalar = texture3D(volumeTex, texPosition.xyz).r;  
    float scalar = texture3D(volumeTex, vec3(x,y,z)).r;  
    
    return texture1D(tfTex, scalar);    
}

/* two side lighting*/
Light getLight(vec3 normal, vec3 lightDir, vec4 lightPar, vec3 rayDir)
{
    Light light;
    
    float ambient = lightPar.r;
        
    float diffuse = lightPar.g * max(dot(lightDir, normal), dot(lightDir, -normal));
        
    vec3 H = normalize(-rayDir + lightDir);    
        
    float DotHV = max(dot(H, normal), dot(H, -normal));
        
//     H = normalize(-rayDir - lightDir);
//     DotHV = max(DotHV, max(dot(H, normal), dot(H, -normal)));
        
    float specular = 0.0;
    
    if ( DotHV > 0.0 )
        specular = lightPar.b * pow(DotHV, lightPar.a);
    
    
    light.ambient_diffuse = ambient + diffuse;
    light.specular = specular;
    
    return light;
}

void DrawSlice()
{
    gl_FragColor = getColor(textureCoords);
    gl_FragColor /= gl_FragColor.a;   
}

void DrawVolume()
{
    
    vec3 rayStart = textureCoords;
        
    vec3 rayDir;
    
    if (bOrtho==1)
        rayDir = eyeDir;
    else
        rayDir = normalize(rayStart - eyePos);
    
    vec3 samplePos = vec3(0.0);
    float sampleLen = sampleSpacing;
    
    vec4 tfColor;
    
    vec4 sampleColor;
    
    vec4 accumulatedColor = vec4(0);
    
    vec3 normal;
    
    vec3 lightDir[2];
    
    lightDir[0] = normalize(vec3(1.0, 1.0, 1.0));
    //lightDir[0] = normalize(rayDir * -1.0);
    lightDir[1] = normalize(vec3(1.0, 0.0, 1.0));
    Light oneLight;
    
    float ambient_diffuse = 0.0;
    float specular = 0.0;
    float dis = 0.0;    
    
    while(samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 && 
          samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 && 
          accumulatedColor.a < 1.0) {
    while(samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 && 
          samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 && 
          accumulatedColor.a < 1.0) {
          
            samplePos = rayStart + rayDir * sampleLen;
            
            sampleLen += sampleSpacing;
                     
            tfColor = getColor(samplePos);
            
            sampleColor = tfColor;
            
            normal = getNormal(samplePos);
                    
            ambient_diffuse = 0.0;
            specular = 0.0;
            
            //for ( int i = 0; i < 1; i ++) {
                oneLight = getLight(normal, lightDir[0], lightPar, rayDir);
                ambient_diffuse += oneLight.ambient_diffuse;
                specular += oneLight.specular;
            //}
                
            sampleColor.rgb = tfColor.rgb * ambient_diffuse + tfColor.a * specular;
            
            accumulatedColor.rgb = (1.0-accumulatedColor.a)*sampleColor.rgb+accumulatedColor.rgb;
                        
            accumulatedColor.a   = (1.0-accumulatedColor.a)*sampleColor.a + accumulatedColor.a;

        }
    }
    
    gl_FragColor = min(accumulatedColor, vec4(1.0));
}

void main(void)
{

	if (drawSlice == 1) 
		DrawSlice();
	else
		DrawVolume();	
	
}




