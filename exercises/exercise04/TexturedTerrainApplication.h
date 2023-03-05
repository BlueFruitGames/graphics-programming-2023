#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/asset/ShaderLoader.h>
#include <ituGL/geometry/Mesh.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/shader/Material.h>
#include <glm/mat4x4.hpp>
#include <vector>

class Texture2DObject;

class TexturedTerrainApplication : public Application
{
public:
    TexturedTerrainApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;

private:
    void InitializeTextures();
    void InitializeMaterials();
    void InitializeMeshes();

    void DrawObject(const Mesh& mesh, Material& material, const glm::mat4& worldMatrix);

    std::shared_ptr<Texture2DObject> CreateDefaultTexture();
    std::shared_ptr<Texture2DObject> CreateHeightMap(unsigned int width, unsigned int height, glm::ivec2 coords);
    std::shared_ptr<Texture2DObject> LoadTexture(const char* path);

    void CreateTerrainMesh(Mesh& mesh, unsigned int gridX, unsigned int gridY);

private:
    unsigned int m_gridX, m_gridY;

    Camera m_camera;

    ShaderLoader m_vertexShaderLoader;
    ShaderLoader m_fragmentShaderLoader;

    Mesh m_terrainPatch_00;
    Mesh m_terrainPatch_01;
    Mesh m_terrainPatch_02;
    Mesh m_terrainPatch_03;
    std::shared_ptr<Material> m_defaultMaterial;

    // (todo) 04.X: Add additional materials
    std::shared_ptr<Material> m_terrainMaterial_00;
    std::shared_ptr<Material> m_terrainMaterial_01;
    std::shared_ptr<Material> m_terrainMaterial_02;
    std::shared_ptr<Material> m_terrainMaterial_03;


    std::shared_ptr<Texture2DObject> m_defaultTexture;

    // (todo) 04.X: Add additional textures
    std::shared_ptr<Texture2DObject> m_heightTexture_00;
    std::shared_ptr<Texture2DObject> m_heightTexture_01;
    std::shared_ptr<Texture2DObject> m_heightTexture_02;
    std::shared_ptr<Texture2DObject> m_heightTexture_03;


    std::shared_ptr<Texture2DObject> m_dirt_00;
};
