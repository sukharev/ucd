#ifndef FRAMEBUFFER_H_
#define FRAMEBUFFER_H_

    /// <summary> This class encapsulate an OpenGL FrameBuffer </summary>
    /// <remarks> General guidelines for use:
    /// <list type="number">
    ///    <item>Create a new instance of a <c>FrameBuffer</c> object.</item>
    ///    <item>Call <c>Initialize</c> in the initialization function.</item>
    ///    <item>Call <c>Activate</c> to draw to the framebuffer.</item>
    ///    <item>Call <c>Deactivate</c> to stop drawing to the framebuffer.</item>
    ///    <item>Call <c>Bind</c> to use the framebuffer as a texture.</item>
    /// </list>
    /// Make sure to check your card's compatability with high bit depths.
    /// Try 8 bits to significantly improve performance
    /// </remarks>
    class FrameBuffer
    {
    	private:
        //#region Fields
        // the Framebuffer index
        int fbo; // = -1;
        // the depth buffer index
        int depthBuffer; // = -1;
        // the index for the texture
        int img; // = -1;
        // store the bitdepth in case of reset
        int bitDepth; // = 8;
        // the width of the render buffer
        int mywidth;
        // the height of the render buffer
        int myheight;
        //#endregion

	public:
        //#region Properties
        /// <summary> Gets or Sets the Width of the Framebuffer</summary>
        int GetWidth();
            
        void SetWidth(int value);

        /// <summary> Gets or Sets the Height of the Framebuffer</summary>
        int GetHeight();
        
	void SetHeight(int value);
        

        /// <summary>
        /// The number of bits used to store each color channel.
        /// Note that the number of bits used to store each color is this
        /// value times four.
        /// </summary>
        int GetChannelBitDepth();
        
        
        void SetChannelBitDepth(int value);

        /// <summary> The identification number of the corresponding texture </summary>
        /// <remarks> Maybe Bind first? </remarks>
        int GetTextureID();

        /// <summary> Initializes all of the GL stuff and allocates any necessary memory.
        /// This method would probably be called in GL Initialization. </summary>
        /// <param name="width">The width of the new Framebuffer</param>
        /// <param name="height">The height of the new Framebuffer</param>
        void Initialize(int width, int height);
        /// <summary> Initializes all of the GL stuff and allocates any necessary memory.
        /// This method would probably be called in GL Initialization. </summary>
        /// <param name="width">The width of the new Framebuffer</param>
        /// <param name="height">The height of the new Framebuffer</param>
        /// <param name="bitDepth">The number of bits per color channel. (8 = Gl.GL_RGBA8, 12 = Gl.GL_RGBA12, 16 = Gl.GL_RGBA16, 32 = Gl.GL_RGBA32F_ARB)</param>
        void Initialize(int width, int height, int bitDepth);

        /// <summary> Start rendering to this framebuffer. </summary>
        void Activate();

        /// <summary> Stop rendering to this framebuffer, and render to the screen. </summary>
        void Deactivate();

        /// <summary> Set this framebuffer as the current texture. </summary>
        void Bind();

        /// <summary> Destroy this framebuffer and free any allocated resources. </summary>
        void Destroy();

        /// <summary> Reinitialize the framebuffer with a new size. </summary>
        /// <remarks> Setting the <c>Width</c> or <c>Height</c> properties calls this method. </remarks>
        /// <param name="width">The new width of the Framebuffer</param>
        /// <param name="height">The new height of the Framebuffer</param>
        void Reset(int width, int height);
    };


#endif //FRAMEBUFFER_H_