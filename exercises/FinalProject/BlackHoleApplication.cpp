#include "BlackHoleApplication.h"

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/lighting/DirectionalLight.h>
#include <ituGL/shader/Material.h>
#include <ituGL/renderer/PostFXRenderPass.h>
#include <ituGL/scene/RendererSceneVisitor.h>
#include <imgui.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

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

    // Update the material properties
    m_material->SetUniformValue("ProjMatrix", camera.GetProjectionMatrix());
    m_material->SetUniformValue("InvProjMatrix", glm::inverse(camera.GetProjectionMatrix()));
    m_material->SetUniformValue("Time", GetCurrentTime());
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
    m_material = CreateRaymarchingMaterial("shaders/blackhole.glsl");

    // Initialize material uniforms
    
    m_material->SetUniformValue("GroundNormal", glm::vec3(0, 1, 0));
    m_material->SetUniformValue("GroundOffset", -35.f);
    m_material->SetUniformValue("GroundColor", glm::vec3(1, 1, 1));
    m_material->SetUniformValue("BendDistanceBounds", glm::vec2(-1, 1));
    
    m_material->SetUniformValue("PlaneBendStrength", 0.01f);
    m_material->SetUniformValue("AnimationSpeed", 3.0f);
    
    m_material->SetUniformValue("BlackHoleRadius", 7.0f);
    m_material->SetUniformValue("BlackHoleColor", glm::vec3(1, 0, 0));
    m_material->SetUniformValue("BlackHoleInfluenceBounds", glm::vec2(-3, 3));
    m_material->SetUniformValue("BlackHoleInfluence", 5.0f);
   
    m_material->SetUniformValue("Smoothness", 1.75f);
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

void BlackHoleApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    // Draw GUI for camera controller
    //m_cameraController.DrawGUI(m_imGui);

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        // Get the camera view matrix and transform the sphere center and the box matrix
        glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Ground", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 bendOrigin(0, 0, -50);
            static glm::vec2 bendDistanceBounds(0, 100);
            static glm::vec3 normal(0, 1, 0);
            static float speed = 3.0f;
            
            // Add controls for sphere parameters
            ImGui::DragFloat3("Bend Origin", &bendOrigin[0], 0.1f);
            m_material->SetUniformValue("BendOrigin", bendOrigin);
            ImGui::DragFloat2("Bend Distance Bounds", &bendDistanceBounds[0], 0.1f);
            m_material->SetUniformValue("BendDistanceBounds", bendDistanceBounds);
            ImGui::DragFloat3("Normal", &normal[0], 0.1f);
            m_material->SetUniformValue("GroundNormal", normal);
            ImGui::DragFloat("Offset", m_material->GetDataUniformPointer<float>("GroundOffset"), 0.1f);
            ImGui::DragFloat("Speed", &speed, 0.1f);
            m_material->SetUniformValue("AnimationSpeed", speed);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("GroundColor"));
            
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("BlackHole", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            static glm::vec3 blackHoleStartPosition(0, 0, 8);
            static glm::vec2 blackHoleInfluenceBounds(-3, 5);
            static float influence = -8.f;

            ImGui::DragFloat3("StartPosition", &blackHoleStartPosition[0], 0.1f);
            m_material->SetUniformValue("BlackHoleStartPosition", blackHoleStartPosition);
            ImGui::DragFloat2("Influence Bounds", &blackHoleInfluenceBounds[0], 0.1f);
            m_material->SetUniformValue("BlackHoleInfluenceBounds", blackHoleInfluenceBounds);
            
            
            ImGui::DragFloat("Influence", &influence, 0.1f);
            m_material->SetUniformValue("BlackHoleInfluence", influence);
            ImGui::DragFloat("Radius", m_material->GetDataUniformPointer<float>("BlackHoleRadius"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("BlackHoleColor"));
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("BlackHoleParticles", ImGuiTreeNodeFlags_DefaultOpen))
        {            
            static float pullSpeed = .75f;
            static float rotationSpeed = 4.f;
            static int amount = 5;
            static float spawnDistance = 30.f;
            static float radius = .6f;
            
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
            ImGui::TreePop();
        }
        ImGui::DragFloat("Smoothness", m_material->GetDataUniformPointer<float>("Smoothness"), 0.1f);
    }

    m_imGui.EndFrame();
}
