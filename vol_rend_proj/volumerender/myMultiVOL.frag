varying vec3  textureCoords;

uniform int drawvol0;
uniform sampler1D tfTex0;
uniform sampler3D volumeTex0;
uniform vec4  lightPar0;

uniform int drawvol1;
uniform sampler3D volumeTex1;
uniform sampler1D tfTex1;
uniform vec4  lightPar1;


uniform float sampleSpacing;
uniform vec3  centralDifferenceSpacing;
uniform vec3  eyePos;

struct Light
{
    float ambient_diffuse;
    float specular;
};

vec3 getNormal(sampler3D volumeTex, vec3 texPosition)
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

vec4 getColor_isosurface(sampler3D volumeTex, sampler1D tfTex, vec3 texPosition)
{
    float scalar = texture3D(volumeTex, texPosition.xyz).r;     
    
    vec4 color = texture1D(tfTex, scalar);
    
    //color.rgb /= color.a;
    
    if (scalar > 0.18 && scalar < 0.22)
        color.a = 1.0;
    else
        color.a = 0.0;
        
    color.rgb *= color.a;
    
    return color;
}


vec4 getColor(sampler3D volumeTex, sampler1D tfTex, vec3 texPosition)
{
    float scalar = texture3D(volumeTex, texPosition.xyz).r;     
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

void DrawVolume()
{
    
    vec3 rayStart = textureCoords;
    
    vec3 rayDir = normalize(rayStart - eyePos);
    
    vec3 samplePos = vec3(0.0);
    float sampleLen = sampleSpacing;
    
    vec4 tfColor[2];
    tfColor[0] = vec4(0);
    tfColor[1] = vec4(0);
    
    vec4 sampleColor[2];
    sampleColor[0] = vec4(0);
    sampleColor[1] = vec4(0);
    
    vec3 normal[2];
    
    vec4 accumulatedColor = vec4(0);
    
    vec4 combineColor;
        
    vec3 lightDir[2];
    
    lightDir[0] = normalize(rayDir * -1.0);    
    
    Light oneLight[2];
    
    float ambient_diffuse[2];
    float specular[2];
    
    while(samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 && 
          samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 && 
          accumulatedColor.a < 1.0) {
    while(samplePos.x <= 1.0 &&  samplePos.y <= 1.0 && samplePos.z <= 1.0 && 
          samplePos.x >= 0.0 &&  samplePos.y >= 0.0 && samplePos.z >= 0.0 && 
          accumulatedColor.a < 1.0) {
          
            samplePos = rayStart + rayDir * sampleLen;
                     
            sampleLen += sampleSpacing;
                     
            if (drawvol0 == 1) {                                
                //tfColor[0] = getColor_isosurface(volumeTex0, tfTex0, samplePos);
                tfColor[0] = getColor(volumeTex0, tfTex0, samplePos);
                normal[0] = getNormal(volumeTex0, samplePos);                            
                oneLight[0] = getLight(normal[0], lightDir[0], lightPar0, rayDir);
                ambient_diffuse[0] = oneLight[0].ambient_diffuse;
                specular[0] = oneLight[0].specular;
                sampleColor[0].rgb = tfColor[0].rgb * ambient_diffuse[0] + tfColor[0].a * specular[0];
                
//                 if (abs(dot(normal[0], rayDir)) < 0.1)
//                     sampleColor[0].rgb = tfColor[0].a * vec3(0);
            }
            
            if (drawvol1 == 1) {            
                tfColor[1] = getColor(volumeTex1, tfTex1, samplePos);
                normal[1] = getNormal(volumeTex1, samplePos);
                oneLight[1] = getLight(normal[1], lightDir[0], lightPar1, rayDir);            
                ambient_diffuse[1] = oneLight[1].ambient_diffuse;
                specular[1] = oneLight[1].specular;
                sampleColor[1].rgb = tfColor[1].rgb * ambient_diffuse[1] + tfColor[1].a * specular[1];
            }
            
            if (tfColor[0].a == 1)
                combineColor.rgb = sampleColor[0].rgb;
            else if (tfColor[1].a == 1)
                combineColor.rgb = sampleColor[1].rgb;
            else
                combineColor.rgb = sampleColor[0].rgb + sampleColor[1].rgb;                               
                               
            combineColor.a = tfColor[0].a + tfColor[1].a;
            
            
            accumulatedColor.rgb = (1.0-accumulatedColor.a)*combineColor.rgb+accumulatedColor.rgb;
                        
            accumulatedColor.a   = (1.0-accumulatedColor.a)*combineColor.a + accumulatedColor.a;

        }
    }
    
    gl_FragColor = min(accumulatedColor, vec4(1.0));
}

void main(void)
{
    DrawVolume();	
    
}


