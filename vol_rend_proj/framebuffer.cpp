#include "framebuffer.h"
#include "GL/glew.h"
#include "GL/gl.h"

/// <summary> Gets or Sets the Width of the Framebuffer</summary>
int FrameBuffer::GetWidth()
{
	return mywidth; 
}
	
void FrameBuffer::SetWidth(int value)
{
	mywidth = value;
	Initialize(mywidth, myheight);
}


/// <summary> Gets or Sets the Height of the Framebuffer</summary>
int FrameBuffer::GetHeight()
{
	return myheight; 
}

void FrameBuffer::SetHeight(int value)
{
	myheight = value;
	Initialize(mywidth, myheight);
}


/// <summary>
/// The number of bits used to store each color channel.
/// Note that the number of bits used to store each color is this
/// value times four.
/// </summary>
int FrameBuffer::GetChannelBitDepth()
{
	return bitDepth; 
}


void FrameBuffer::SetChannelBitDepth(int value)
{
	bitDepth = value;
	while (bitDepth >= 0) {
		try {
			Reset(mywidth, myheight);
			return;
		} catch(...) {
			bitDepth--;
		}
	}
	//throw new Exception("FrameBufferObject could not initialize. The video card or the currect display mode may not support it.");
}

/// <summary> The identification number of the corresponding texture </summary>
/// <remarks> Maybe Bind first? </remarks>
int FrameBuffer::GetTextureID()
{
	return img;
}
//#endregion

/// <summary> Initializes all of the GL stuff and allocates any necessary memory.
/// This method would probably be called in GL Initialization. </summary>
/// <param name="width">The width of the new Framebuffer</param>
/// <param name="height">The height of the new Framebuffer</param>
void FrameBuffer::Initialize(int width, int height)
{
	Initialize(width, height, bitDepth);
}
/// <summary> Initializes all of the GL stuff and allocates any necessary memory.
/// This method would probably be called in GL Initialization. </summary>
/// <param name="width">The width of the new Framebuffer</param>
/// <param name="height">The height of the new Framebuffer</param>
/// <param name="bitDepth">The number of bits per color channel. (8 = GL_RGBA8, 12 = GL_RGBA12, 16 = GL_RGBA16, 32 = GL_RGBA32F_ARB)</param>
void FrameBuffer::Initialize(int width, int height, int new_bitDepth)
{
	// set the appropriate GL enum values
	int GLBitDepth, GLValueType;
	switch (new_bitDepth) {
		case 8:
			GLBitDepth = GL_RGBA8; GLValueType = GL_UNSIGNED_BYTE;
			break;
		case 12:
			GLBitDepth = GL_RGBA12; GLValueType = GL_FLOAT;
			break;
		case 16:
			GLBitDepth = GL_RGBA16; GLValueType = GL_FLOAT;
			break;
		case 32:
			GLBitDepth = GL_RGBA32F_ARB; GLValueType = GL_FLOAT;
			break;
		default:
			;//throw new Exception("The bit depth of " + new_bitDepth + " is unknown. Try either 8, 12, 16, or 32.");
	}

	glPushAttrib(GL_LIGHTING_BIT | GL_LINE_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

	// Setup our FBO
	//int[] temp = new int[1];
	GLuint temp[1]; // = new GLuint[1];   //C#
	glGenFramebuffersEXT(1, temp);
	fbo = temp[0];
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	// Create the render buffer for depth
	glGenRenderbuffersEXT(1, temp);
	depthBuffer = temp[0];
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);

	// Now setup a texture to render to
	glGenTextures(1, temp);
	img = temp[0];
	glBindTexture(GL_TEXTURE_2D, img);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GLBitDepth, width, height, 0, GL_RGBA, GLValueType, NULL);

	//  The following 3 lines enable mipmap filtering and generate the mipmap data so rendering works
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmapEXT(GL_TEXTURE_2D);

	// And attach it to the FBO so we can render to it
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, img, 0);

	// Attach the depth render buffer to the FBO as it's depth attachment
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);


	int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT){
		return;
		//throw new Exception("The card may not be compatable with Framebuffers. Try another bit depth.");
	}
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);    // Unbind the FBO for now

	glPopAttrib();
	mywidth = width;
	myheight = height;
	this->bitDepth = new_bitDepth;
}

/// <summary> Start rendering to this framebuffer. </summary>
void FrameBuffer::Activate()
{
	// First we bind the FBO so we can render to it
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	// Save the view port and set it to the size of the texture
	//glPushAttrib(GL_VIEWPORT_BIT);
	//glViewport(0, 0, GlControl.Width, GlControl.Height);
}

/// <summary> Stop rendering to this framebuffer, and render to the screen. </summary>
void FrameBuffer::Deactivate()
{
	//glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/// <summary> Set this framebuffer as the current texture. </summary>
void FrameBuffer::Bind()
{
	glBindTexture(GL_TEXTURE_2D, img);
	//    If you enabled the mipmap filtering on setup earlier then you'll need to uncomment the line
	//    below so OpenGL can generate all the mipmap data for the new main image each frame
	//glGenerateMipmapEXT(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
}

/// <summary> Destroy this framebuffer and free any allocated resources. </summary>
void FrameBuffer::Destroy()
{
	GLuint gl_fbo = fbo;
	GLuint gl_depthBuffer = depthBuffer;
	GLuint gl_img = img;
	glDeleteFramebuffersEXT(1, &gl_fbo);
	glDeleteRenderbuffersEXT(1, &gl_depthBuffer);
	glDeleteTextures(1, &gl_img);
	fbo = -1;
	depthBuffer = -1;
	img = -1;
}

/// <summary> Reinitialize the framebuffer with a new size. </summary>
/// <remarks> Setting the <c>Width</c> or <c>Height</c> properties calls this method. </remarks>
/// <param name="width">The new width of the Framebuffer</param>
/// <param name="height">The new height of the Framebuffer</param>
void FrameBuffer::Reset(int width, int height)
{
	if (fbo > -1)
	    Destroy();
	Initialize(width, height, bitDepth);
}
