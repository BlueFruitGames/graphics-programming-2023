#include "TerrainApplication.h"
#include "ituGL/geometry/VertexAttribute.h"


// (todo) 01.1: Include the libraries you need
#include <vector>
#define STB_PERLIN_IMPLEMENTATION 
#include "stb_perlin.h"

#include <cmath>
#include <iostream>

// Helper structures. Declared here only for this exercise
struct Vector2
{
    Vector2() : Vector2(0.f, 0.f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct Vector3
{
    Vector3() : Vector3(0.f,0.f,0.f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;

    Vector3 Normalize() const
    {
        float length = std::sqrt(1 + x * x + y * y);
        return Vector3(x / length, y / length, z / length);
    }
};

// (todo) 01.8: Declare an struct with the vertex format
struct Vertex {
    Vector3 position;
    Vector2 textureCoordinate;
    Vector3 color;
};


TerrainApplication::TerrainApplication()
    : Application(1024, 1024, "Terrain demo"), m_gridX(16), m_gridY(16), m_shaderProgram(0)
{
}

Vector3 GetColor(float z) {
    if (z < -0.025) {
        return Vector3(0, 0, 1);
    }
    else if (z < 0.05) {
        return Vector3(0, 1, 0);
    }
    else {
        return Vector3(1, 1, 1);
    }
}

void TerrainApplication::Initialize()
{
    Application::Initialize();

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);
    // Build shaders and store in m_shaderProgram
    BuildShaders();

    // (todo) 01.1: Create containers for the vertex position
    std::vector<Vertex> vertexAttributes;
    std::vector<unsigned int> indices; 


    // (todo) 01.1: Fill in vertex data
    float lacunarity = 5;
    float gain = 1;
    int octaves = 2;
    float zInfluence = .15;

    float spaceBetweenVerticesH = 1.0 / float(m_gridX);
    float spaceBetweenVerticesV = 1.0 / float(m_gridY);

    for (int row = 0; row < m_gridY; ++row) {
        for (int column = 0; column < m_gridX; ++column) {

            float xLeft = float(column) * spaceBetweenVerticesH - 0.5;
            float xRight = float(column + 1) * spaceBetweenVerticesH - 0.5;

            float yUp = float(row) * spaceBetweenVerticesV - 0.5;
            float yDown = float(row + 1) * spaceBetweenVerticesV - 0.5;

            float z01 = stb_perlin_fbm_noise3(xLeft, yUp, 0, lacunarity, gain, octaves) * zInfluence;
            float z02 = stb_perlin_fbm_noise3(xRight, yUp, 0, lacunarity, gain, octaves) * zInfluence;
            float z03 = stb_perlin_fbm_noise3(xRight, yDown, 0, lacunarity, gain, octaves) * zInfluence;
            float z04 = stb_perlin_fbm_noise3(xLeft, yDown, 0, lacunarity, gain, octaves) * zInfluence;

            Vertex vertex01;
            vertex01.position = Vector3(xLeft, yUp, z01);
            vertex01.textureCoordinate = Vector2(0, 0);
            vertex01.color = GetColor(z01);

            Vertex vertex02;
            vertex02.position = Vector3(xRight, yUp, z02);
            vertex02.textureCoordinate = Vector2(1, 0);
            vertex02.color = GetColor(z02);

            Vertex vertex03;
            vertex03.position = Vector3(xRight, yDown, z03);
            vertex03.textureCoordinate = Vector2(1, 1);
            vertex03.color = GetColor(z03);

            Vertex vertex04;
            vertex04.position = Vector3(xLeft, yDown, z04);
            vertex04.textureCoordinate = Vector2(0, 1);
            vertex04.color = GetColor(z04);

            vertexAttributes.push_back(vertex01);
            vertexAttributes.push_back(vertex02);
            vertexAttributes.push_back(vertex03);
            vertexAttributes.push_back(vertex04);

            indices.push_back(row * m_gridY * 4 + column * 4);
            indices.push_back(row * m_gridY * 4 + column * 4 + 1);
            indices.push_back(row * m_gridY * 4 + column * 4 + 3);

            indices.push_back(row * m_gridY * 4 + column * 4 + 1);
            indices.push_back(row * m_gridY * 4 + column * 4 + 2);
            indices.push_back(row * m_gridY * 4 + column * 4 + 3);
        }
    }

    // (todo) 01.1: Initialize VAO, and VBO
    m_VAO.Bind();

    int vertexCount = m_gridX * m_gridY * 4;
    m_VBO.Bind();
    m_VBO.AllocateData(std::span(vertexAttributes));

    VertexAttribute position(Data::Type::Float, 3);
    VertexAttribute textureCoordinate(Data::Type::Float, 2);
    VertexAttribute color(Data::Type::Float, 3);

    m_VAO.SetAttribute(0, position, 0, sizeof(Vertex));
    m_VAO.SetAttribute(1, textureCoordinate, sizeof(Vector3), sizeof(Vertex));
    m_VAO.SetAttribute(2, color, sizeof(Vector3) + sizeof(Vector2), sizeof(Vertex));

    // (todo) 01.5: Initialize EBO
    m_EBO.Bind();
    m_EBO.AllocateData<unsigned int>(std::span(indices));


    // (todo) 01.1: Unbind VAO, and VBO
    m_VAO.Unbind();
    m_VBO.Unbind();

    // (todo) 01.5: Unbind EBO
    m_EBO.Unbind();

}

void TerrainApplication::Update()
{
    Application::Update();

    UpdateOutputMode();
}

void TerrainApplication::Render()
{
    Application::Render();

    // Clear color and depth
    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    m_VAO.Bind();

    // Set shader to be used
    glUseProgram(m_shaderProgram);

    // (todo) 01.1: Draw the grid

    glDrawElements(GL_TRIANGLES, 2 * 3 * m_gridX * m_gridY, GL_UNSIGNED_INT, 0);

}

void TerrainApplication::Cleanup()
{
    Application::Cleanup();
}

void TerrainApplication::BuildShaders()
{
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aColor;\n"
        "layout (location = 3) in vec3 aNormal;\n"
        "uniform mat4 Matrix = mat4(1);\n"
        "out vec2 texCoord;\n"
        "out vec3 color;\n"
        "out vec3 normal;\n"
        "void main()\n"
        "{\n"
        "   texCoord = aTexCoord;\n"
        "   color = aColor;\n"
        "   normal = aNormal;\n"
        "   gl_Position = Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "uniform uint Mode = 0u;\n"
        "in vec2 texCoord;\n"
        "in vec3 color;\n"
        "in vec3 normal;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   switch (Mode)\n"
        "   {\n"
        "   default:\n"
        "   case 0:\n"
        "       FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "       break;\n"
        "   case 1:\n"
        "       FragColor = vec4(fract(texCoord), 0.0f, 1.0f);\n"
        "       break;\n"
        "   case 2:\n"
        "       FragColor = vec4(color, 1.0f);\n"
        "       break;\n"
        "   case 3:\n"
        "       FragColor = vec4(normalize(normal), 1.0f);\n"
        "       break;\n"
        "   case 4:\n"
        "       FragColor = vec4(color * max(dot(normalize(normal), normalize(vec3(1,0,1))), 0.2f), 1.0f);\n"
        "       break;\n"
        "   }\n"
        "}\n\0";

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    m_shaderProgram = shaderProgram;
}

void TerrainApplication::UpdateOutputMode()
{
    for (int i = 0; i <= 4; ++i)
    {
        if (GetMainWindow().IsKeyPressed(GLFW_KEY_0 + i))
        {
            int modeLocation = glGetUniformLocation(m_shaderProgram, "Mode");
            glUseProgram(m_shaderProgram);
            glUniform1ui(modeLocation, i);
            break;
        }
    }
    if (GetMainWindow().IsKeyPressed(GLFW_KEY_TAB))
    {
        const float projMatrix[16] = { 0, -1.294f, -0.721f, -0.707f, 1.83f, 0, 0, 0, 0, 1.294f, -0.721f, -0.707f, 0, 0, 1.24f, 1.414f };
        int matrixLocation = glGetUniformLocation(m_shaderProgram, "Matrix");
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(matrixLocation, 1, false, projMatrix);
    }
}
