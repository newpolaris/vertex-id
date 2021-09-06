#pragma once

#include <memory>
#include <vector>
#include <string>

enum class ShaderDataType
{
    None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

uint32_t ShaderDataTypeSize(ShaderDataType type);
uint32_t GetComponentCount(ShaderDataType type);

struct BufferLayoutDesc;

typedef std::shared_ptr<class BufferLayout> BufferLayoutPtr;

struct BufferElement
{
    std::string Name;
    ShaderDataType Type;
    size_t Offset;
    size_t Size;
    size_t Count;

    BufferElement(ShaderDataType type, const std::string& Name);
};

struct BufferLayoutDesc
{
    BufferLayoutDesc() :
        Size(0) {
    }

    BufferLayoutDesc(const std::initializer_list<BufferElement>& elements) :
        Elements(elements), Size(0) {
        update();
    }

    void update() {
        size_t size = 0;
        for (auto& e : Elements) {
            e.Offset = size;
            size += e.Size;
        }
        Size = size;
    }

    std::vector<BufferElement> Elements;
    size_t Size = 0;
};

class BufferLayout
{
public:

    BufferLayout(const BufferLayoutDesc& desc) :
        _elements(desc.Elements),
        _size(desc.Size)
    {
    }

    const std::vector<BufferElement>& getElements() const { return _elements; }
    size_t getSize() const { return _size; }

    std::vector<BufferElement> _elements;
    size_t _size;
};
