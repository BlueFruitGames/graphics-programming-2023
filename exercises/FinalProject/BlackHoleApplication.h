#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>

#include "ituGL/texture/Texture2DObject.h"
#include "ituGL/texture/TextureCubemapObject.h"

class Material;

class BlackHoleApplication : public Application
{
public:
    BlackHoleApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeMaterial();
    void InitializeTextures();
    void InitializeRenderer();

    std::shared_ptr<Material> CreateRaymarchingMaterial(const char* fragmentShaderPath);
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);
    
    void RenderGUI();

private:
    // Helper object for debug GUI
    DearImGui m_imGui;

    // Camera controller
    CameraController m_cameraController;

    // Renderer
    Renderer m_renderer;

    // Materials
    std::shared_ptr<Material> m_material;

    //textures
    std::shared_ptr<Texture2DObject> m_groundTexture;
    std::shared_ptr<Texture2DObject> m_blackHoleTexture;
    std::shared_ptr<Texture2DObject> m_blackHoleParticlesTexture;
    std::shared_ptr<Texture2DObject> m_backgroundTexture;

    // Skybox texture
    std::shared_ptr<TextureCubemapObject> m_skyboxTexture;
};
