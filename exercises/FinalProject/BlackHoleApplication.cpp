#include "BlackHoleApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/shader/Material.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <imgui.h>
#include <iostream>
#include <stb_image.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

#include "ituGL/asset/TextureCubemapLoader.h"
#include "ituGL/scene/Transform.h"

BlackHoleApplication::BlackHoleApplication()
    : Application(1024, 1024, "Ray-marching demo")
    , m_renderer(GetDevice())
{
}

void BlackHoleApplication::Initialize()
{
    Application::Initialize();

    // Initialize DearImGUI
    m_imGui.Initialize(GetMainWindow());

    InitializeCamera();
    InitializeTextures();
    InitializeMaterial();
    InitializeRenderer();
}

void BlackHoleApplication::Update()
{
    Application::Update();

    // Update camera controller
    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    // Set renderer camera
    const Camera& camera = *m_cameraController.GetCamera()->GetCamera();
    m_renderer.SetCurrentCamera(camera);


    glm::vec3 rotation = m_cameraController.GetCamera()->GetCamera()->ExtractRotation();

    std::cout << rotation.x <<std::endl;
    // Update the material properties
    m_material->SetUniformValue("ProjMatrix", camera.GetProjectionMatrix());

    m_material->SetUniformValue("InvProjMatrix", glm::inverse(camera.GetProjectionMatrix()));
    m_material->SetUniformValue("ViewMatrix", glm::inverse(camera.GetViewMatrix()));
    m_material->SetUniformValue("Time", GetCurrentTime());
    m_material->SetUniformValue("CameraRotX", rotation.x);
}

void BlackHoleApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    // Render the scene
    m_renderer.Render();

    // Render the debug user interface
    RenderGUI();
}

void BlackHoleApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void BlackHoleApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void BlackHoleApplication::InitializeMaterial()
{
    m_skyboxTexture = TextureCubemapLoader::LoadTextureShared("models/skybox/starCubemap.png", TextureObject::FormatRGB, TextureObject::InternalFormatSRGB8);
    TextureCubemapObject::Unbind();

    m_skyboxTexture->Bind();
    float maxLod;
    m_skyboxTexture->GetParameter(TextureObject::ParameterFloat::MaxLod, maxLod);
    
    m_material = CreateRaymarchingMaterial("shaders/blackhole.glsl");

    m_material->SetUniformValue("CubeMap", m_skyboxTexture);
    //Textures
    m_material->SetUniformValue("GroundTexture", m_groundTexture);
    m_material->SetUniformValue("BlackHoleTexture", m_blackHoleTexture);
    m_material->SetUniformValue("BlackHoleParticlesTexture", m_blackHoleParticlesTexture);
    m_material->SetUniformValue("BackgroundTexture", m_backgroundTexture);
    
    // Initialize material uniforms
    m_material->SetUniformValue("GroundNormal", glm::vec3(0, 1, 0));
    m_material->SetUniformValue("GroundColor", glm::vec3(1, 1, 1));
    
    m_material->SetUniformValue("GroundSpeed", 3.0f);
    
    m_material->SetUniformValue("BlackHoleRadius", 3.0f);
    m_material->SetUniformValue("BlackHoleColor", glm::vec3(1, 0, 0));
    m_material->SetUniformValue("BlackHoleInfluenceBounds", glm::vec2(-3, 3));
    m_material->SetUniformValue("BlackHoleInfluence", 5.0f);
   
    m_material->SetUniformValue("BlackHoleParticlesSmoothness", 0.3f);
    m_material->SetUniformValue("Smoothness", .2f);
    m_material->SetUniformValue("BendStrength", -1.4f);
    m_material->SetUniformValue("BendDistanceFactor", .1f);
    m_material->SetUniformValue("BendStartOffset", -37.0f);
    m_material->SetUniformValue("FresnelPower", -13.0f);
    m_material->SetUniformValue("FresnelStrength", 50.0f);
    m_material->SetUniformValue("FresnelColor", glm::vec3(1, 0.3, 1));
}

void BlackHoleApplication::InitializeTextures()
{
    m_groundTexture = LoadTexture("textures/ground.png");
    m_blackHoleTexture = LoadTexture("textures/stars.png");
    m_blackHoleParticlesTexture = LoadTexture("textures/dirt.png");
    m_blackHoleParticlesTexture = LoadTexture("textures/background.jpg");
    
}

void BlackHoleApplication::InitializeRenderer()
{
    m_renderer.AddRenderPass(std::make_unique<PostFXRenderPass>(m_material));
}

std::shared_ptr<Material> BlackHoleApplication::CreateRaymarchingMaterial(const char* fragmentShaderPath)
{
    // We could keep this vertex shader and reuse it, but it looks simpler this way
    std::vector<const char*> vertexShaderPaths;
    vertexShaderPaths.push_back("shaders/version330.glsl");
    vertexShaderPaths.push_back("shaders/renderer/fullscreen.vert");
    Shader vertexShader = ShaderLoader(Shader::VertexShader).Load(vertexShaderPaths);

    std::vector<const char*> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/version330.glsl");
    fragmentShaderPaths.push_back("shaders/utils.glsl");
    fragmentShaderPaths.push_back("shaders/sdflibrary.glsl");
    fragmentShaderPaths.push_back("shaders/raymarcher.glsl");
    fragmentShaderPaths.push_back(fragmentShaderPath);
    fragmentShaderPaths.push_back("shaders/raymarching.frag");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->Build(vertexShader, fragmentShader);

    // Create material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgramPtr);
    
    return material;
}

std::shared_ptr<Texture2DObject> BlackHoleApplication::LoadTexture(const char* path)
{
    std::shared_ptr<Texture2DObject> texture = std::make_shared<Texture2DObject>();

    int width = 0;
    int height = 0;
    int components = 0;
    
    // Load the texture data here
    unsigned char* data = stbi_load(path, &width, &height, &components, 4);

    texture->Bind();
    texture->SetImage(0, width, height, TextureObject::FormatRGBA, TextureObject::InternalFormatRGBA, std::span<const unsigned char>(data, width * height * 4));

    // Generate mipmaps
    texture->GenerateMipmap();

    // Release texture data
    stbi_image_free(data);

    return texture;
}

void BlackHoleApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    //m_cameraController.DrawGUI(m_imGui);

    glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();
    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        // Get the camera view matrix and transform the sphere center and the box matrix

        if (ImGui::TreeNodeEx("Ground", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 normal(0, 1, 0);
            static float speed = 3.0f;
            static float offset = -11.f;
            static glm::vec2 textureScale(.2f, .2f);
            
            ImGui::DragFloat3("Normal", &normal[0], 0.1f);
            m_material->SetUniformValue("GroundNormal", glm::vec3(viewMatrix * glm::vec4(normal,0.0f)));
            ImGui::DragFloat("Speed", &speed, 0.1f);
            m_material->SetUniformValue("AnimationSpeed", speed);
            ImGui::DragFloat("Offset", &offset, 0.1f);
            m_material->SetUniformValue("GroundOffset", offset);
            //ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("GroundColor"));
            ImGui::DragFloat2("Texture Scale", &textureScale[0], 0.1f);
            m_material->SetUniformValue("GroundTextureScale", textureScale);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("BlackHole", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            static glm::vec3 blackHoleStartPosition(0, -7, -45);
            static glm::vec2 blackHoleInfluenceBounds(-8, 6);
            static float influence = -8.f;
            static glm::vec2 textureScale(1, 1);

            ImGui::DragFloat3("StartPosition", &blackHoleStartPosition[0], 0.1f);
            m_material->SetUniformValue("BlackHoleStartPosition",  glm::vec3(viewMatrix * glm::vec4(blackHoleStartPosition, 1.0f)));
            ImGui::DragFloat2("Influence Bounds", &blackHoleInfluenceBounds[0], 0.1f);
            m_material->SetUniformValue("BlackHoleInfluenceBounds", blackHoleInfluenceBounds);
            
            
            ImGui::DragFloat("Influence", &influence, 0.1f);
            m_material->SetUniformValue("BlackHoleInfluence", influence);
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("BlackHoleRadius"), 0.1f);
            //ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("BlackHoleColor"));
            ImGui::DragFloat2("Texture Scale", &textureScale[0], 0.1f);
            m_material->SetUniformValue("BlackHoleTextureScale", textureScale);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("BlackHoleParticles", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            static float pullSpeed = .05f;
            static float rotationSpeed = .2f;
            static int amount = 7;
            static float spawnDistance = 16.f;
            static float radius = .2f;
            static float rotationOffset = 0.04f;
            static int layers = 6;
            static glm::vec2 textureScale(.2f, .2f);
            
            ImGui::DragFloat("Pull Speed", &pullSpeed, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesPullSpeed", pullSpeed);
            ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesRotationSpeed", rotationSpeed);
            ImGui::DragInt("Amount", &amount, 1, 1,10);
            m_material->SetUniformValue("BlackHoleParticlesAmount", amount);
            ImGui::DragFloat("Spawn Distance", &spawnDistance, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesSpawnDistance", spawnDistance);
            ImGui::DragFloat("Radius", &radius, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesRadius", radius);
            ImGui::DragInt("Layers", &layers, 1, 1, 20);
            m_material->SetUniformValue("BlackHoleParticlesLayers", layers);
            ImGui::DragFloat("rotationOffset", &rotationOffset, 0.01, 0, 1);
            m_material->SetUniformValue("BlackHoleParticlesRotationOffset", rotationOffset);
            ImGui::DragFloat("Smoothness", m_material->GetDataUniformPointer<float>("BlackHoleParticlesSmoothness"), 0.1f);
            
            ImGui::DragFloat2("Texture Scale", &textureScale[0], 0.1f);
            m_material->SetUniformValue("BlackHoleTextureParticlesScale", textureScale);
            ImGui::TreePop();
        }
        ImGui::DragFloat("Smoothness", m_material->GetDataUniformPointer<float>("Smoothness"), 0.1f);
        ImGui::DragFloat("BendStrength", m_material->GetDataUniformPointer<float>("BendStrength"), 0.1f);
        ImGui::DragFloat("BendDistanceFactor", m_material->GetDataUniformPointer<float>("BendDistanceFactor"), 0.1f);
        ImGui::DragFloat("BendStartOffset", m_material->GetDataUniformPointer<float>("BendStartOffset"), 0.1f);
        ImGui::DragFloat("FresnelPower", m_material->GetDataUniformPointer<float>("FresnelPower"), 0.1f);
        ImGui::DragFloat("FresnelStrength", m_material->GetDataUniformPointer<float>("FresnelStrength"), 0.1f);
        ImGui::ColorEdit3("FresnelColor", m_material->GetDataUniformPointer<float>("FresnelColor"));
    }

    m_imGui.EndFrame();
}
