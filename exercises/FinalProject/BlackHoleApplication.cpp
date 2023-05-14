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
    
    
    glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();
    m_material->SetUniformValue("GroundNormal", glm::vec3(viewMatrix * glm::vec4(glm::vec3(0, 1, 0),0.0f)));
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
    m_material->SetUniformValue("BlackHoleParticlesTexture", m_blackHoleParticlesTexture);
    m_material->SetUniformValue("BackgroundTexture", m_backgroundTexture);
    
    // Initialize material uniforms
    //General
    m_material->SetUniformValue("Smoothness", .2f);
    
    //Ground
    m_material->SetUniformValue("GroundTexture", m_groundTexture);
    m_material->SetUniformValue("GroundOffset", -11.f);
    m_material->SetUniformValue("GroundTextureScale", glm::vec2(.2f, .2f));
    m_material->SetUniformValue("AnimationSpeed", 3.f);
    m_material->SetUniformValue("GroundSpeed", 3.0f);

    //Bending
    m_material->SetUniformValue("BendStrength", -1.4f);
    m_material->SetUniformValue("BendDistanceFactor", .1f);
    m_material->SetUniformValue("BendStartOffset", -37.0f);
    
    //Fresnel
    m_material->SetUniformValue("FresnelPower", -13.0f);
    m_material->SetUniformValue("FresnelStrength", 50.0f);
    m_material->SetUniformValue("FresnelColor", glm::vec3(1, 0.3, 1));

    //Blackhole
    
    m_material->SetUniformValue("BlackHoleTexture", m_blackHoleTexture);
    m_material->SetUniformValue("BlackHoleTextureScale", glm::vec2(1, 1));
    m_material->SetUniformValue("BlackHoleRadius", 3.0f);
    m_material->SetUniformValue("BlackHoleColor", glm::vec3(1, 0, 0));
    m_material->SetUniformValue("BlackHoleInfluenceBounds", glm::vec2(-3, 3));
    m_material->SetUniformValue("BlackHoleInfluence", 5.0f);


    //BlackholeParticleSmoothness    
    m_material->SetUniformValue("BlackHoleParticlesSpawnDistance", 16.f);
    m_material->SetUniformValue("BlackHoleParticlesRadius", .2f);
    m_material->SetUniformValue("BlackHoleParticlesRotationOffset",  0.04f);
    m_material->SetUniformValue("BlackHoleTextureParticlesScale", glm::vec2(.2f, .2f));
    m_material->SetUniformValue("BlackHoleParticlesSmoothness", 0.3f);

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
        static bool activeWorldBending = false;
        if (ImGui::TreeNodeEx("General", ImGuiTreeNodeFlags_DefaultOpen)){
        
            ImGui::Checkbox("ActivateWorldBending?", &activeWorldBending);
            m_material->SetUniformValue("isWorldBendingActive", static_cast<int>(activeWorldBending));
            
            static bool activateGroundAnimation = false;
            ImGui::Checkbox("AnimateGround?", &activateGroundAnimation);
            m_material->SetUniformValue("isGroundAnimationActive", static_cast<int>(activateGroundAnimation));

            static bool activateFresnel = true;
            ImGui::Checkbox("ActivateFresnel?", &activateFresnel);
            m_material->SetUniformValue("isFresnelActive", static_cast<int>(activateFresnel));

            
            ImGui::ColorEdit3("FresnelColor", m_material->GetDataUniformPointer<float>("FresnelColor"));
            
            ImGui::TreePop();
        }
        if (activeWorldBending && ImGui::TreeNodeEx("Bending", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePop();
            ImGui::DragFloat("BendStrength", m_material->GetDataUniformPointer<float>("BendStrength"), 0.1f);
            ImGui::DragFloat("BendStartOffset", m_material->GetDataUniformPointer<float>("BendStartOffset"), 0.1f);
        }
        if (ImGui::TreeNodeEx("BlackHole", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            static glm::vec3 blackHolePosition(0, -7, -45);
            static float influence = -8.f;

            ImGui::DragFloat3("Position", &blackHolePosition[0], 0.1f);
            m_material->SetUniformValue("BlackHolePosition",  glm::vec3(viewMatrix * glm::vec4(blackHolePosition, 1.0f)));

            static bool activateTextureAnimation = true;
            ImGui::Checkbox("AnimateTexture?", &activateTextureAnimation);
            m_material->SetUniformValue("isBlackHoleTextureMoving", static_cast<int>(activateTextureAnimation));
            
            
            ImGui::DragFloat("Influence", &influence, 0.1f);
            m_material->SetUniformValue("BlackHoleInfluence", influence);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("BlackHoleParticles", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            
            static float pullSpeed = .05f;
            ImGui::DragFloat("Pull Speed", &pullSpeed, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesPullSpeed", pullSpeed);
            
            static float rotationSpeed = .2f;
            ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.1f);
            m_material->SetUniformValue("BlackHoleParticlesRotationSpeed", rotationSpeed);
            
            static int columnCount = 7;
            ImGui::DragInt("ParticleColumnCount", &columnCount, 1, 1,10);
            m_material->SetUniformValue("BlackHoleParticlesColumnCount", columnCount);
            
            static int amountPerColumn = 6;
            ImGui::DragInt("ParticleAmountCountPerColumn", &amountPerColumn, 1, 1, 10);
            m_material->SetUniformValue("BlackHoleParticlesPerColumn", amountPerColumn);
            
            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
}
