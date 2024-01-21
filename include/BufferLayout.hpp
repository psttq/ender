#pragma once

#include "spdlog/spdlog.h"
#include <string>
#include <vector>
#include <glad/glad.h>

namespace ENDER
{

  enum class LayoutObjectType
  {
    Float1,
    Float2,
    Float3,
    Float4
  };

  struct LayoutObject
  {
    LayoutObjectType type;
    unsigned int size = 0;
    unsigned int stride = 0;
    size_t offset = 0;

    LayoutObject(const LayoutObjectType &obj_type) { type = obj_type; }
  };

  class BufferLayout
  {
    std::vector<LayoutObject> _layout;

    void calculateLayoutProperties()
    {
      unsigned int offset = 0;
      for (auto &el : _layout)
      {
        el.size = convertTypeToSize(el.type);
        el.offset = offset;

        offset += el.size;
      }

      for (auto &el : _layout)
      {
        el.stride = offset;
      }
    }

  public:
    BufferLayout(const std::initializer_list<LayoutObject> &objects)
        : _layout(objects)
    {
      calculateLayoutProperties();
      spdlog::info("Created new buffer layout");
      debugPrint();
    }

    void addObject(const LayoutObject &object) { _layout.push_back(object); }

    void debugPrint() const
    {
      spdlog::debug("LayoutDebugPrint:");
      auto i = 0;
      for (auto &el : _layout)
      {
        spdlog::debug("Element {}: Type: {}, Size: {}, Stride: {}, Offset: {}", i,
                      convertTypeToString(el.type), el.size, el.stride,
                      el.offset);
      }
    }

    static unsigned int convertTypeToSize(LayoutObjectType type)
    {
      switch (type)
      {
      case LayoutObjectType::Float1:
      case LayoutObjectType::Float2:
      case LayoutObjectType::Float3:
      case LayoutObjectType::Float4:
        return convertTypeToNumberOfElements(type) * sizeof(float);
      default:
        spdlog::error("Unknown LayoutObjectType");
        throw;
      }
    }
    static unsigned int convertTypeToGLType(const LayoutObjectType &type)
    {
      switch (type)
      {
      case LayoutObjectType::Float1:
      case LayoutObjectType::Float2:
      case LayoutObjectType::Float3:
      case LayoutObjectType::Float4:
        return GL_FLOAT;
      default:
        spdlog::error("Unknown LayoutObjectType");
        throw;
      }
    }
    static unsigned int convertTypeToNumberOfElements(const LayoutObjectType &type)
    {
      switch (type)
      {
      case LayoutObjectType::Float1:
        return 1;
      case LayoutObjectType::Float2:
        return 2;
      case LayoutObjectType::Float3:
        return 3;
      case LayoutObjectType::Float4:
        return 4;
      default:
        spdlog::error("Unknown LayoutObjectType");
        throw;
      }
    }
    static std::string convertTypeToString(const LayoutObjectType &type)
    {
      switch (type)
      {
      case LayoutObjectType::Float1:
        return "Float1";
      case LayoutObjectType::Float2:
        return "Float2";
      case LayoutObjectType::Float3:
        return "Float3";
      case LayoutObjectType::Float4:
        return "Float4";
      default:
        spdlog::error("Unknown LayoutObjectType");
        throw;
      }
    }

    auto begin() { return _layout.begin(); }

    auto end() { return _layout.end(); }

    auto begin() const { return _layout.begin(); }

    auto end() const { return _layout.end(); }

    unsigned int getStride()
    {
      unsigned int stride = 0;
      for (const auto &el : _layout)
      {
        stride += el.size;
      }
      return stride;
    }
  };
} // namespace ENDER
