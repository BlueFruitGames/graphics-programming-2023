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
    
    m_material->SetUniformValue("PlaneNormal", glm::vec3(0, 1, 0));
    m_material->SetUniformValue("PlanePosition", glm::vec3(0, 0, 0));
    m_material->SetUniformValue("PlaneOffset", -5.0f);
    m_material->SetUniformValue("PlaneColor", glm::vec3(1, 1, 1));
   
    //m_material->SetUniformValue("Smoothness", 0.25f);
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

        if (ImGui::TreeNodeEx("GroundPlane", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static glm::vec3 position(0, 0, 0);
            static glm::vec3 normal(0, 1, 0);
            
            // Add controls for sphere parameters
            ImGui::DragFloat3("Position", &position[0], 0.1f);
            m_material->SetUniformValue("PlanePosition", glm::vec3(viewMatrix * glm::vec4(position, 1.0f)));
            ImGui::DragFloat3("Normal", &normal[0], 0.1f);
            m_material->SetUniformValue("PlaneNormal", glm::vec3(viewMatrix * glm::vec4(normal, 1.0f)));
            ImGui::DragFloat("Offset", m_material->GetDataUniformPointer<float>("PlaneOffset"), 0.1f);
            ImGui::ColorEdit3("Color", m_material->GetDataUniformPointer<float>("PlaneColor"));
            
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Box", ImGuiTreeNodeFlags_DefaultOpen))
        {
            //static glm::vec3 translation(2, 0, -10);
            //static glm::vec3 rotation(0.0f);

            // Add controls for box parameters
            //m_material->SetUniformValue("BoxMatrix", viewMatrix * glm::translate(translation) * glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z));

            ImGui::TreePop();
        }

        //ImGui::DragFloat("Smoothness", m_material->GetDataUniformPointer<float>("Smoothness"), 0.1f);
    }

    m_imGui.EndFrame();
}
