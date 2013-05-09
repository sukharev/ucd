varying vec3  textureCoords;

void main(void)
{
	// normal opengl transforms
    
	// pass on the position
  	gl_Position = ftransform();
        
	// texture coords are the ones from glTexCoord0 multiplied by the 0th texture matrix
	textureCoords = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xyz;    
}
