/*** 
 * @Author: Matt.SHI
 * @Date: 2022-12-10 14:46:29
 * @LastEditTime: 2022-12-10 21:01:28
 * @LastEditors: Matt.SHI
 * @Description: 
 * @FilePath: /opengl_demo/features/gaussian_blur.cpp
 * @Copyright © 2022 Essilor. All rights reserved.
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tools/stb_image_write.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>

#include <features/framebuffer/FrameBuffer.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int g_save_frame = 0;
unsigned char g_save_base_path[] = "./frames/";

void saveFrameBuffer2PNG(const char* buf, 
    unsigned int w, unsigned int h, unsigned int c, 
    const char* filename)
{
    stbi_write_png(filename, w, h, c, buf, w * c);
}

void generateFilePath(char* filename, double index,const unsigned char* basePath)
{
    sprintf(filename, "%s_%.4f.png", basePath,index);
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("../resources/features_res/gaussain_bulr/gauss_blur.vs", 
        "../resources/features_res/gaussain_bulr/gauss_blur.fs");

    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture 
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT/*GL_REPEAT*/);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT/*GL_REPEAT*/);
    // set boarder color
    float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // load image, create texture and generate mipmaps
    int width, height, nrChannels,reqComp = 3;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load(FileSystem::getPath("resources/features_res/gaussain_bulr/test.png").c_str(), &width, &height, &nrChannels, reqComp);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }
    stbi_image_free(data);
    //texture end

    //for swap texture==========================
    /*
    GLuint textureForSwapId;  
    glGenTextures(1, &textureForSwapId);  
    glBindTexture(GL_TEXTURE_2D, textureForSwapId);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); //  automatic  mipmap  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
                width, 
                height, 0,  
                GL_RGBA, 
                GL_UNSIGNED_BYTE, 0);  
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a renderbuffer object to store depth info  
    GLuint rboId;  
    glGenRenderbuffers(1, &rboId);  
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);  
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,  
                            width, height);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0); 

    // create a framebuffer object
    GLuint fboId;
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER
                        GL_COLOR_ATTACHMENT0,  // 2. attachment point
                        GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                        textureForSwapId,             // 4. tex ID
                        0);                    // 5. mipmap level: 0(base)

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
                            GL_DEPTH_ATTACHMENT, // 2. attachment point
                            GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
                            rboId);              // 4. rbo ID

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool fboUsed = true;
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        fboUsed = false;
    }
    */
    FrameBuffer fbo;
    fbo.init(width, height);        // for single-sample FBO
    //============================
            
    // all upcoming GL_TEXTURE_2D operations now have effect on this texture object

    double startTime = 0.0;
    double endTime = 0.0;
    int frame = 0;
    bool bSave = false;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // render
        // ------
        //glBindFramebuffer(GL_FRAMEBUFFER, fboId);

        fbo.bind();
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);
        // render container
        ourShader.use();
        glBindVertexArray(VAO);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        fbo.update();
        fbo.unbind();

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GLuint texId = fbo.getColorId();        // texture object ID for render-to-texture
        const unsigned char* buffer = fbo.getColorBuffer();

        frame++;
        endTime = glfwGetTime();
        if(endTime - startTime > 1.0)
        {
            std::cout << "the fps is " <<  frame / (endTime - startTime) << std::endl;
            if(g_save_frame > 0)
            {
                g_save_frame = 0;
                char save_path[256] = {0};
                generateFilePath(save_path,endTime, g_save_base_path);
                saveFrameBuffer2PNG((const char*)(buffer), width, height, 4,save_path);
            }
            startTime = endTime; 
            frame = 0;
        }
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        
        //std::cout << "the cost time is " << end - start << " ms" << std::endl;
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_save_frame = 1;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
