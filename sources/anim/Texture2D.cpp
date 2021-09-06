#include "Texture2D.h"
#include <glad/glad.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2DPtr createTexture(int32_t width, int32_t height, void* data)
{
    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA;

    GLuint instance = 0;
    glGenTextures(1, &instance);
    glActiveTexture(GL_TEXTURE0 + 16 - 1);
    glBindTexture(GL_TEXTURE_2D, instance);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_shared<Texture2D>(instance, width, height);
}

Texture2DPtr createTexture(const std::string& path, bool vflip)
{
    stbi_set_flip_vertically_on_load(vflip);

    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
        return nullptr;

    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    std::vector<char> storage(length);;
    length = (long)fread(storage.data(), 1, length, fp);
    fclose(fp);

    GLenum target = GL_TEXTURE_2D;
    GLenum type = GL_UNSIGNED_BYTE;
    int width = 0, height = 0, nrComponents = 0;
    stbi_uc* imagedata = stbi_load_from_memory(
        (stbi_uc*)storage.data(),
        (int)length, &width, &height, &nrComponents, 4);

    if (!imagedata)
        return nullptr;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLuint id = 0;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0 + 16 - 1);
    glBindTexture(target, id);
    glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_RGBA, type, imagedata);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, 16);
    glGenerateMipmap(target);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(imagedata);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    return std::make_shared<Texture2D>(id, width, height);
}

void bindTexture(const Texture2DPtr& texture, uint8_t slot) {
    assert(texture->_id != 0);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->_id);
};
