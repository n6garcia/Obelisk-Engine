#include "SkeletalModel.h"

SkeletalModel::~SkeletalModel() {
    std::unordered_set<GLuint> texIDs;
    std::vector<Texture> texWrangled;
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        auto& texs = meshes[i].textures;
        for (unsigned int i = 0; i < texs.size(); i++)
        {
            Texture tex = texs[i];
            if (!(texIDs.find(tex.ID) != texIDs.end())) {
                texWrangled.push_back(tex);
                texIDs.insert(tex.ID);
            }
        }
    }
    for (unsigned int i = 0; i < texWrangled.size(); i++)
    {
        texWrangled[i].Delete();
    }
    meshes.clear();
}

#ifndef NOASS
void SkeletalModel::loadModel(std::string path)
{
    Assimp::Importer import;
    path = "models/" + path;
    //const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_LimitBoneWeights);// | aiProcess_RemoveRedundantMaterials | aiProcess_OptimizeMeshes | aiProcess_FindInvalidData);

    /* some models need aiProcess_FlipUVs and some don't.
       note: gltf files and dae files converted from gltf files don't require FlipUVs since STB already flips images
       so any models that come from gltf file format don't require FlipUVs
    */

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::string directory = path.substr(0, path.find_last_of('/'));
    //fileType = path.substr(path.find_last_of('.'), path.back());

    aiMatrix4x4t<float> identity = aiMatrix4x4t<float>();
    std::vector<Texture> textures_loaded;
    processNode(scene->mRootNode, scene, identity, directory, textures_loaded);
}

void SkeletalModel::processNode(aiNode* node, const aiScene* scene, aiMatrix4x4t<float> pTransform, std::string directory, std::vector<Texture>& textures_loaded)
{

    aiMatrix4x4t<float> transform = pTransform * node->mTransformation;

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(std::move(processMesh(mesh, scene, transform, directory, textures_loaded)));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, transform, directory, textures_loaded);
    }
}
#endif

void SetVertexBoneDataToDefault(SkeletalVertex& vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void SetVertexBoneData(SkeletalVertex& vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.m_BoneIDs[i] < 0)
        {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}

void NormalizeBoneData(std::vector<SkeletalVertex>& vertices)
{
    for (int k = 0; k < vertices.size(); k++) {
        //std::cerr << "vertex[" << k << "]" << std::endl;
        float totalWeight = 0;
        int i;
        for (i = 0; i < MAX_BONE_INFLUENCE; ++i) // for loop until i == (first index with nil value)
        {
            if (vertices[k].m_BoneIDs[i] != -1)
            {
                totalWeight += vertices[k].m_Weights[i];
            }
            else break;
        }

        float nTerm = 1 / totalWeight;

        for (int j = 0; j < i; ++j) // for loop through all weights to be normalized
        {
            vertices[k].m_Weights[j] = vertices[k].m_Weights[j] * nTerm;
            //std::cerr << vertices[k].m_Weights[j] << std::endl;
        }
    }
}

void testBoneData(std::vector<SkeletalVertex>& vertices)
{
    uint64_t ct = 0;
    for (int k = 0; k < vertices.size(); k++) {
        //std::cerr << "vertex[" << k << "]" << std::endl;
        float totalWeight = 0;
        int i;
        for (i = 0; i < MAX_BONE_INFLUENCE; ++i) // for loop until i == (first index with nil value)
        {
            if (vertices[k].m_BoneIDs[i] != -1)
            {
                totalWeight += vertices[k].m_Weights[i];
            }
            else break;
        }

        if (totalWeight < .99f || totalWeight > 1.01f) {
            ct++;
        }
    }
    if (ct != 0) std::cerr << "WARNING: " << ct << " vertices have malformed/null bone weights" << std::endl;
}

#ifndef NOASS
void SkeletalModel::ExtractBoneWeightForVertices(std::vector<SkeletalVertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
    auto& boneInfoMap = m_BoneInfoMap;
    int& boneCount = m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = aiMat4toGLM(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}

SkeletalMesh SkeletalModel::processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4t<float>& transformation, std::string directory, std::vector<Texture>& textures_loaded)
{
    std::vector<SkeletalVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    glm::mat4 transform = aiMat4toGLM(transformation);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        SkeletalVertex vertex;
        SetVertexBoneDataToDefault(vertex);

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texUV = vec;
            //std::cout << vec.x << " " << vec.y << std::endl;
        }
        else
            vertex.texUV = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    // process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }


    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        int slotInc = 0;

        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, scene,
            aiTextureType_DIFFUSE, "diffuse", slotInc, directory, textures_loaded);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        slotInc = slotInc + diffuseMaps.size();
        //std::cerr << "test " << slotInc <<  std::endl;

        std::vector<Texture> specularMaps = loadMaterialTextures(material, scene,
            aiTextureType_SPECULAR, "specular", slotInc, directory, textures_loaded);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }// hmmm

    ExtractBoneWeightForVertices(vertices, mesh, scene);
    testBoneData(vertices);

    return std::move(SkeletalMesh(vertices, indices, textures, transform));
}
#endif

void SkeletalModel::Draw(Shader& shader, Camera& camera,
    glm::mat4& transform, glm::mat4& ntransform) {
    mat.linkShader(shader);
    for (unsigned int i = 0; i < meshes.size(); i++) {
        //std::cerr << "mesh ptr: " << meshes[i] << std::endl;
        //if (i == 0) continue;
        meshes[i].Draw(shader, camera, transform, ntransform);
    }
}


void SkeletalModel::DrawShadow(Shader& shader, glm::mat4& transform){
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].DrawShadow(shader, transform);
    }
}

// POSSIBLY CHANGE type in Texture from string to aiTextureType?

#ifndef NOASS
// BUG: .type member becomes corrupted when leaving loadMaterialTexture
// not really sure why adding int slotInc(rement) makes this work perfectly, was having issue where specular sampler was overriding diffuse sampler
std::vector<Texture> SkeletalModel::loadMaterialTextures(aiMaterial* mat, const aiScene* scene, aiTextureType type, std::string typeName, int slotInc, std::string directory, std::vector<Texture>& textures_loaded)
{
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        const char* c_str = str.C_Str();
        std::string path = directory + "/" + c_str;

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), path.c_str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture = Texture(path.c_str(), typeName, i + slotInc);
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

glm::mat4 SkeletalModel::aiMat4toGLM(aiMatrix4x4t<float>& matrix) {

    glm::mat4 m = glm::mat4(0.0f);
    m[0][0] = matrix.a1;
    m[0][1] = matrix.b1;
    m[0][2] = matrix.c1;
    m[0][3] = matrix.d1;

    m[1][0] = matrix.a2;
    m[1][1] = matrix.b2;
    m[1][2] = matrix.c2;
    m[1][3] = matrix.d2;

    m[2][0] = matrix.a3;
    m[2][1] = matrix.b3;
    m[2][2] = matrix.c3;
    m[2][3] = matrix.d3;

    m[3][0] = matrix.a4;
    m[3][1] = matrix.b4;
    m[3][2] = matrix.c4;
    m[3][3] = matrix.d4;

    return m;
}

#endif
