

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "basic_camera.h"
#include "camera.h"
#include "pointLight.h"
#include "sphere.h"
#include "cube.h"
#include "stb_image.h"
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>

using namespace std;

//function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawTableChair(unsigned int VAO, Shader lightingShaderWithTexture, glm::mat4 matrix, Cube c);
void drawFan(unsigned int VAO, Shader lightingShader, glm::mat4 matrix);
void drawCube(unsigned int& VAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color);
unsigned int loadTexture(char const* path, GLenum textureWrappingModeS, GLenum textureWrappingModeT, GLenum textureFilteringModeMin, GLenum textureFilteringModeMax);

//settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//screen
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float aspectRatio = 4.0f / 3.0f;

//camera
float eyeX = 2.0f, eyeY = 1.0f, eyeZ = 13.5f;
float lookAtX = 2.0f, lookAtY = 1.5f, lookAtZ = 4.75f;
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

//timing
float deltaTime = 0.0f;     // time between current frame and last frame
float lastFrame = 0.0f;

//fan
float r = 0.0f;
bool fanOn = false;

//bird's eye view
bool birdEyeView = false;
glm::vec3 birdEyePosition(2.0f, 3.5f, 13.5f);
glm::vec3 birdEyeTarget(2.0f, 0.0f, 7.5f);
float birdEyeSpeed = 1.0f;

//rotation around a point
float theta = 0.0f; // Angle around the Y-axis
float radius = 2.0f;

//directional light
bool directionLightOn = true;
bool directionalAmbient = true;
bool directionalDiffuse = true;
bool directionalSpecular = true;

//spot light
bool spotLightOn = true;

//point light
bool point1 = true;
bool point2 = true;

//custom projection matrix
float fov = glm::radians(camera.Zoom);
float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
float near = 0.1f;
float far = 100.0f;
float tanHalfFOV = tan(fov / 2.0f);

//doors and windows
bool openDoor = true;
float doorAngle = 90.0f;

//positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(-2.9f,  2.0f,  5.0f),
    glm::vec3(6.7f,  2.0f,  5.0f),
};

PointLight pointlight1(
    pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z,       // position
    0.2f, 0.2f, 0.2f,       //ambient
    0.8f, 0.8f, 0.8f,       //diffuse
    0.2f, 0.2f, 0.2f,       //specular
    1.0f,       //k_c
    0.09f,      //k_l
    0.032f,     //k_q
    1       //light number
);

PointLight pointlight2(
    pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z,
    0.2f, 0.2f, 0.2f,
    0.8f, 0.8f, 0.8f,
    0.2f, 0.2f, 0.2f,
    1.0f,
    0.09f,
    0.032f,
    2
);

int main()
{
    //glfw initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1907031: Assignment 3", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //glad load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //configure global opengl state
    glEnable(GL_DEPTH_TEST);

    //build and compile our shader program
    Shader lightingShader("vertexShaderForPhongShading.vs", "fragmentShaderForPhongShading.fs");
    Shader lightingShaderWithTexture("vertexShaderForPhongShadingWithTexture.vs", "fragmentShaderForPhongShadingWithTexture.fs");
    //Shader lightingShader("vertexShaderForGouraudShading.vs", "fragmentShaderForGouraudShading.fs");
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");
    //Shader constantShader("vertexShader.vs", "fragmentShaderV2.fs");

    //set up vertex data (and buffer(s)) and configure vertex attributes
    float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f
    };
    
    unsigned int cube_indices[] = {
        0, 3, 2,
        2, 1, 0,

        4, 5, 7,
        7, 6, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    //position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);

    //second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //ourShader.use();
    //constantShader.use();
    /**/
    //floor texture
    string diffuseMapPath = "classroom_floor.png";
    string specularMapPath = "classroom_floor.png";
    unsigned int diffMap = loadTexture(diffuseMapPath.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap = loadTexture(specularMapPath.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube = Cube(diffMap, specMap, 32.0f, 0.0f, 0.0f, 1.25f, 2.25f);

    //roof texture
    string diffuseMapPath2 = "classroom_roof.jpg";
    string specularMapPath2 = "classroom_roof.jpg";
    unsigned int diffMap2 = loadTexture(diffuseMapPath2.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap2 = loadTexture(specularMapPath2.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube2 = Cube(diffMap2, specMap2, 32.0f, 0.0f, 0.0f, 2.5f, 4.5f);

    //wall texture
    string diffuseMapPath3 = "classroom_wall.jpg";
    string specularMapPath3 = "classroom_wall.jpg";
    unsigned int diffMap3 = loadTexture(diffuseMapPath3.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap3 = loadTexture(specularMapPath3.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube3 = Cube(diffMap3, specMap3, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    //wood texture
    string diffuseMapPath4 = "wood.jpg";
    string specularMapPath4 = "wood.jpg";
    unsigned int diffMap4 = loadTexture(diffuseMapPath4.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap4 = loadTexture(specularMapPath4.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube4 = Cube(diffMap4, specMap4, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    
    //wood texture 2
    string diffuseMapPath5 = "wood2.jpg";
    string specularMapPath5 = "wood2.jpg";
    unsigned int diffMap5 = loadTexture(diffuseMapPath5.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap5 = loadTexture(specularMapPath5.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube5 = Cube(diffMap5, specMap5, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    //door and window texture
    string diffuseMapPath6 = "door.jpg";
    string specularMapPath6 = "door.jpg";
    unsigned int diffMap6 = loadTexture(diffuseMapPath6.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap6 = loadTexture(specularMapPath6.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube6 = Cube(diffMap6, specMap6, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    //whiteboard texture
    string diffuseMapPath7 = "whiteboard.jpg";
    string specularMapPath7 = "whiteboard.jpg";
    unsigned int diffMap7 = loadTexture(diffuseMapPath7.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    unsigned int specMap7 = loadTexture(specularMapPath7.c_str(), GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    Cube cube7 = Cube(diffMap7, specMap7, 32.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    r = 0.0f;

    //render loop
    while (!glfwWindowShouldClose(window))
    {
        //per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //input
        processInput(window);

        //render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //ourShader.use();
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        //point lights set up
        pointlight1.setUpPointLight(lightingShader);
        pointlight2.setUpPointLight(lightingShader);

        //directional light set up
        lightingShader.setVec3("directionalLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("directionalLight.ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("directionalLight.diffuse", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("directionalLight.specular", 0.0f, 0.0f, 0.0f);
        lightingShader.setBool("directionLightOn", directionLightOn);

        //spot light set up
        lightingShader.setVec3("spotLight.position", 2.0f, 2.0f, 2.0f);
        lightingShader.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("spotLight.ambient", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("spotLight.specular", 0.2f, 0.2f, 0.2f);
        lightingShader.setFloat("spotLight.k_c", 1.0f);
        lightingShader.setFloat("spotLight.k_l", 0.09);
        lightingShader.setFloat("spotLight.k_q", 0.032);
        //lightingShader.setFloat("spotLight.cos_theta", glm::cos(glm::radians(60.0f)));
        lightingShader.setFloat("spotLight.inner_circle", glm::cos(glm::radians(7.5f)));
        lightingShader.setFloat("spotLight.outer_circle", glm::cos(glm::radians(15.0f)));
        lightingShader.setBool("spotLightOn", spotLightOn);

        //handle for changes in directional light directly from shedder
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("ambientLight", !directionalAmbient);
                directionalAmbient = !directionalAmbient;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("diffuseLight", !directionalDiffuse);
                directionalDiffuse = !directionalDiffuse;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("specularLight", !directionalSpecular);
                directionalSpecular = !directionalSpecular;
            }
        }

        glm::mat4 projection(0.0f);
        projection[0][0] = 1.0f / (aspect * tanHalfFOV);
        projection[1][1] = 1.0f / tanHalfFOV;
        projection[2][2] = -(far + near) / (far - near);
        projection[2][3] = -1.0f;
        projection[3][2] = -(2.0f * far * near) / (far - near);
        //pass projection matrix to shader (note that in this case it could change every frame)
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
        //ourShader.setMat4("projection", projection);
        //constantShader.setMat4("projection", projection);
        lightingShader.setMat4("projection", projection);

        //camera view transformation
        //constantShader.setMat4("view", view);
        //ourShader.setMat4("view", view);

        glm::mat4 view;

        //check for bird's eye view and normal view
        if (birdEyeView) {
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(birdEyePosition, birdEyeTarget, up);
        }
        else {
            view = camera.GetViewMatrix();
        }
        
        lightingShader.setMat4("view", view);

        //define matrices and vectors needed
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
        glm::vec3 color;
        
        //initialize all elements as non-emissive
        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
        
        float z = 0.0f;
        
        if (openDoor) {
            if (doorAngle < 90.0f) {
                doorAngle += 0.25;
            }
        }

        if (!openDoor) {
            if (doorAngle > 0.0f) {
                doorAngle -= 0.25;
            }
        }

        //draw fans with rotations
        z = 0.0f;
        for (int i = 0; i < 3; i++) {
            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z));
            drawFan(VAO, lightingShader, translateMatrix);

            translateMatrix = glm::translate(identityMatrix, glm::vec3(3.33f, 0.0f, z));
            drawFan(VAO, lightingShader, translateMatrix);

            z += 3.5;
        }

        //draw the lamp object(s)
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);

        for (unsigned int i = 0; i < 2; i++)
        {
            translateMatrix = glm::translate(identityMatrix, pointLightPositions[i]);
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.2f, 0.2f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        lightingShaderWithTexture.use();
        lightingShaderWithTexture.setMat4("projection", projection);
        lightingShaderWithTexture.setMat4("view", view);

        //point lights set up
        pointlight1.setUpPointLight(lightingShaderWithTexture);
        pointlight2.setUpPointLight(lightingShaderWithTexture);

        //directional light set up
        lightingShaderWithTexture.setVec3("directionalLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShaderWithTexture.setVec3("directionalLight.ambient", 0.1f, 0.1f, 0.1f);
        lightingShaderWithTexture.setVec3("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShaderWithTexture.setVec3("directionalLight.specular", 0.0f, 0.0f, 0.0f);
        lightingShaderWithTexture.setBool("directionLightOn", directionLightOn);

        //spot light set up
        lightingShaderWithTexture.setVec3("spotLight.position", 2.0f, 2.0f, 2.0f);
        lightingShaderWithTexture.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShaderWithTexture.setVec3("spotLight.ambient", 0.5f, 0.5f, 0.5f);
        lightingShaderWithTexture.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShaderWithTexture.setVec3("spotLight.specular", 0.2f, 0.2f, 0.2f);
        lightingShaderWithTexture.setFloat("spotLight.k_c", 1.0f);
        lightingShaderWithTexture.setFloat("spotLight.k_l", 0.09);
        lightingShaderWithTexture.setFloat("spotLight.k_q", 0.032);
        //lightingShaderWithTexture.setFloat("spotLight.cos_theta", glm::cos(glm::radians(30.0f)));
        lightingShaderWithTexture.setFloat("spotLight.inner_circle", glm::cos(glm::radians(7.5f)));
        lightingShaderWithTexture.setFloat("spotLight.outer_circle", glm::cos(glm::radians(15.0f)));
        lightingShaderWithTexture.setBool("spotLightOn", spotLightOn);

        //floor
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.0f, -0.1f, 18.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.494f, 0.514f, 0.541f);
        //drawCube(VAO, lightingShader, model, color);
        cube.drawCubeWithTexture(lightingShaderWithTexture, model);

        //roof
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 3.5f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.0f, 0.1f, 18.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.494f, 0.514f, 0.541f);
        //drawCube(VAO, lightingShader, model, color);
        cube2.drawCubeWithTexture(lightingShaderWithTexture, model);

        //front wall
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.0f, 4.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 4.5f, 4.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 1 down section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, 0.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 1.5f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 1 up section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 2.5f, 0.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 1.0f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, 3.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 4.5f, 4.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 2 down section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, 7.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 1.5f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 2 up section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 2.5f, 7.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 1.0f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall section 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, 10.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 4.5f, 4.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, -1.0f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 4.5f, 6.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall window down section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, -1.0f, 2.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 1.5f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall window up section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, 2.5f, 2.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 1.0f, 2.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, -1.0f, 4.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 4.5f, 6.5f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall door up section
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, 2.0f, 11.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 1.5f, 2.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall section 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, -1.0f, 13.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 4.5f, 1.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);

        //back wall
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, -1.0f, 14.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.0f, 4.5f, -0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.659f, 0.820f, 0.843f);
        //drawCube(VAO, lightingShader, model, color);
        cube3.drawCubeWithTexture(lightingShaderWithTexture, model);
        
        //left wall window 1 1
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 0.5f, 0.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.0f, 1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 1 2
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(-doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 0.5f, 3.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.0f, -1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 2 1
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 0.5f, 7.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.0f, 1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //left wall window 2 2
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(-doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-3.0f, 0.5f, 10.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.0f, -1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall window 1
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(-doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, 0.5f, 2.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 2.0f, 1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //right wall window 2
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, 0.5f, 4.50f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 2.0f, -1.25f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        //door
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(doorAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.0f, -1.0f, 13.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 3.0f, -2.0f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        //color = glm::vec3(0.404f, 0.353f, 0.325f);
        //drawCube(VAO, lightingShader, model, color);
        cube6.drawCubeWithTexture(lightingShaderWithTexture, model);

        z = 0.0f;
        //draw tables and chairs
        for (int j = 0; j < 5; j++) {
            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z));
            drawTableChair(VAO, lightingShaderWithTexture, translateMatrix, cube4);

            for (int i = 0; i < 2; i++) {
                translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(-1.25f, 0.0f, 0.0f));
                drawTableChair(VAO, lightingShaderWithTexture, translateMatrix, cube4);
            }

            translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 0.0f, z));
            drawTableChair(VAO, lightingShaderWithTexture, translateMatrix, cube4);

            for (int i = 0; i < 2; i++) {
                translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(1.25f, 0.0f, 0.0f));
                drawTableChair(VAO, lightingShaderWithTexture, translateMatrix, cube4);
            }

            z += 2.25;
        }

        //faculty table
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.5f, 0.0f, -0.75f));
        drawTableChair(VAO, lightingShaderWithTexture, translateMatrix * rotateYMatrix, cube4);

        //standing structure top
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-1.0f, -0.5f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(6.0f, 0.1f, 3.0f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.624f, 0.416f, 0.310f);
        //drawCube(VAO, lightingShader, model, color);
        cube5.drawCubeWithTexture(lightingShaderWithTexture, model);

        //standing structure back left leg
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-1.0f, -0.5f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.455f, 0.235f, 0.102f);
        //drawCube(VAO, lightingShader, model, color);
        cube5.drawCubeWithTexture(lightingShaderWithTexture, model);

        //standing structure front left leg
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-1.0f, -0.5f, -1.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.455f, 0.235f, 0.102f);
        //drawCube(VAO, lightingShader, model, color);
        cube5.drawCubeWithTexture(lightingShaderWithTexture, model);

        //standing structure back right leg
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.9f, -0.5f, -4.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.455f, 0.235f, 0.102f);
        //drawCube(VAO, lightingShader, model, color);
        cube5.drawCubeWithTexture(lightingShaderWithTexture, model);

        //standing structure front right leg
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.9f, -0.5f, -1.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(0.455f, 0.235f, 0.102f);
        //drawCube(VAO, lightingShader, model, color);
        cube5.drawCubeWithTexture(lightingShaderWithTexture, model);

        //whiteboard
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-0.5f, 0.5f, -3.9f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(5.0f, 2.0f, 0.1f));
        model = translateMatrix * scaleMatrix;
        //color = glm::vec3(1.0f, 1.0f, 1.0f);
        //drawCube(VAO, lightingShader, model, color);
        cube7.drawCubeWithTexture(lightingShaderWithTexture, model);

        //glfw swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //optional: de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //glfw terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
}

void drawFan(unsigned int VAO, Shader lightingShader, glm::mat4 matrix) {
    //define matrices and vectors needed
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;
    glm::vec3 color;

    //when fan is on
    if (fanOn) {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.28f, 3.5f, 1.61f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.75f, 0.1f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(0.0f, 0.0f, 0.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan middle
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.13f, 2.75f, 1.46f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.1f, 0.4f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        color = glm::vec3(0.0f, 0.0f, 0.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.13f, 2.75f, 1.56f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, -0.1f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, 0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.75f, -0.1f, 0.2f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.53f, 2.75f, 1.56f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, -0.1f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, 0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.75f, -0.1f, 0.2f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.23f, 2.75f, 1.46f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.1f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.1f, -0.75f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.23f, 2.75f, 1.86f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 0.0f, 0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.1f, 0.0f, -0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.1f, 0.75f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        r += 1.0f;
    }

    //when fan is off
    else {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.28f, 3.5f, 1.61f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.75f, 0.1f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(0.0f, 0.0f, 0.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan middle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.13f, 2.75f, 1.46f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.1f, 0.4f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(0.0f, 0.0f, 0.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.13f, 2.75f, 1.56f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.75f, -0.1f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.53f, 2.75f, 1.56f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.75f, -0.1f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.23f, 2.75f, 1.46f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.1f, -0.75f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.23f, 2.75f, 1.86f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.1f, 0.75f));
        model = matrix * translateMatrix * scaleMatrix;
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        drawCube(VAO, lightingShader, model, color);
    }
}

void drawTableChair(unsigned int VAO, Shader lightingShaderWithTexture, glm::mat4 matrix, Cube c) {
    //define matrices and vectors needed
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    glm::vec3 color;

    //table top
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.1f, 1.0f));
    model = matrix * scaleMatrix;
    color = glm::vec3(0.882f, 0.710f, 0.604f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //table left leg back
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -1.0f, 0.1f));
    model = matrix * scaleMatrix;
    //color = glm::vec3(0.647f, 0.408f, 0.294f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //table right leg back
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.9f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -1.0f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.647f, 0.408f, 0.294f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //table leg left front
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.9f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -1.0f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.647f, 0.408f, 0.294f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //table leg right front
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.9f, 0.0f, 0.9f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -1.0f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.647f, 0.408f, 0.294f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair mid section
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.25f, -0.5f, 1.15f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 0.1f, 0.5f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.455f, 0.235f, 0.102f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair leg back left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.25f, -0.5f, 1.15f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.329f, 0.173f, 0.110f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair leg front left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.25f, -0.5f, 1.55f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.329f, 0.173f, 0.110f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair leg front right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.65f, -0.5f, 1.55f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.329f, 0.173f, 0.110f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair leg back right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.65f, -0.5f, 1.15f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, -0.5f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.329f, 0.173f, 0.110f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);

    //chair upper piller left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.25f, -0.4f, 1.55f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 0.65f, 0.1f));
    model = matrix * translateMatrix * scaleMatrix;
    //color = glm::vec3(0.329f, 0.173f, 0.110f);
    //drawCube(VAO, lightingShader, model, color);
    c.drawCubeWithTexture(lightingShaderWithTexture, model);
}

void drawCube(unsigned int& VAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color)
{
    //use the shadder
    lightingShader.use();

    //define lighting properties
    lightingShader.setVec3("material.ambient", color);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", color);
    lightingShader.setFloat("material.shininess", 32.0f);

    lightingShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

//process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if(!birdEyeView)
            camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(P_UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(P_DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(Y_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(Y_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(R_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!birdEyeView)
            camera.ProcessKeyboard(R_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        fanOn = !fanOn;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        birdEyeView = !birdEyeView;
    }

    if (birdEyeView) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            birdEyePosition.z -= birdEyeSpeed * deltaTime; // Move forward along Z
            birdEyeTarget.z -= birdEyeSpeed * deltaTime;
            if (birdEyePosition.z <= 2.0) {
                birdEyePosition.z = 2.0;
            }
            if (birdEyeTarget.z <= -4.0) {
                birdEyeTarget.z = -4.0;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            birdEyePosition.z += birdEyeSpeed * deltaTime; // Move backward along Z
            birdEyeTarget.z += birdEyeSpeed * deltaTime;
            if (birdEyePosition.z >= 13.5) {
                birdEyePosition.z = 13.5;
            }
            if (birdEyeTarget.z >= 7.5) {
                birdEyeTarget.z = 7.5;
            }
        }
    }

    /*
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {   
        if (!birdEyeView) {
            theta += 0.01f;
            camera.Position.x = lookAtX + radius * sin(theta);
            camera.Position.y = lookAtY;
            camera.Position.z = lookAtZ + radius * cos(theta);
        }
    }
    */

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        if (pointlight1.ambientOn > 0 && pointlight1.diffuseOn > 0 && pointlight1.specularOn > 0) {
            pointlight1.turnOff();
            point1 = false;
        }    
        else {
            pointlight1.turnOn();
            point1 = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        if (pointlight2.ambientOn > 0 && pointlight2.diffuseOn > 0 && pointlight2.specularOn > 0) {
            pointlight2.turnOff();
            point2 = false;
        }            
        else {
            pointlight2.turnOn();
            point2 = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        directionLightOn = !directionLightOn;
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        spotLightOn = !spotLightOn;
    }

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (pointlight1.ambientOn > 0 || pointlight2.ambientOn > 0) {
            if(point1)
                pointlight1.turnAmbientOff();
            if(point2)
                pointlight2.turnAmbientOff();
        }
        else {
            if(point1)
                pointlight1.turnAmbientOn();
            if(point2)
                pointlight2.turnAmbientOn();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (pointlight1.diffuseOn > 0 || pointlight2.diffuseOn > 0) {
            if (point1)
                pointlight1.turnDiffuseOff();
            if (point2)
                pointlight2.turnDiffuseOff();
        }
        else {
            if (point1)
                pointlight1.turnDiffuseOn();
            if (point2)
                pointlight2.turnDiffuseOn();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (pointlight1.specularOn > 0 || pointlight2.specularOn > 0) {
            if (point1)
                pointlight1.turnSpecularOff();
            if (point2)
                pointlight2.turnSpecularOff();
        }
        else {
            if (point1)
                pointlight1.turnSpecularOn();
            if (point2)
                pointlight2.turnSpecularOn();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        openDoor = true;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        openDoor = false;
    }
}

//glfw whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //make sure the viewport matches the new window dimensions; note that width and height will be significantly larger than specified on retina displays.
    float aspectRatio = 4.0f / 3.0f;
    int viewWidth, viewHeight;

    if (width / (float)height > aspectRatio) {
        //Window is too wide, fit height and adjust width
        viewHeight = height;
        viewWidth = (int)(height * aspectRatio);
    }
    else {
        //Window is too tall, fit width and adjust height
        viewWidth = width;
        viewHeight = (int)(width / aspectRatio);
    }

    //Center the viewport
    int xOffset = (width - viewWidth) / 2;
    int yOffset = (height - viewHeight) / 2;

    glViewport(xOffset, yOffset, viewWidth, viewHeight);
    //glViewport(0, 0, width, height);
}

//glfw whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;       //reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

//glfw whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path, GLenum textureWrappingModeS, GLenum textureWrappingModeT, GLenum textureFilteringModeMin, GLenum textureFilteringModeMax)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrappingModeS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrappingModeT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilteringModeMin);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilteringModeMax);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
