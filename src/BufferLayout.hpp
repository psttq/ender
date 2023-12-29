#pragma once

#include "spdlog/spdlog.h"
#include <string>
#include <vector>
namespace ENDER {

enum class LayoutObjectType { Float1, Float2, Float3, Float4 };

struct LayoutObject {
  LayoutObjectType type;
  unsigned int size;
  unsigned int stride;
  unsigned int offset;

  LayoutObject(const LayoutObjectType &obj_type) { type = obj_type; }
};

class BufferLayout {
  std::vector<LayoutObject> m_layout;

  unsigned int convertTypeToSize(LayoutObjectType type) {
    switch (type) {
    case LayoutObjectType::Float1:
      return sizeof(float);
    case LayoutObjectType::Float2:
      return sizeof(float) * 2;
    case LayoutObjectType::Float3:
      return sizeof(float) * 3;
    case LayoutObjectType::Float4:
      return sizeof(float) * 4;
    default:
      spdlog::error("Unknown LayoutObjectType");
      throw;
    }
  }
  std::string convertTypeToString(LayoutObjectType type) {
    switch (type) {
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
  void calculateLayoutProperties() {
    unsigned int offset = 0;
    for (auto &el : m_layout) {
      el.size = convertTypeToSize(el.type);
      el.offset = offset;

      offset += el.size;
    }

    for(auto &el : m_layout){
      el.stride = offset;
    }
  }


public:
  BufferLayout(std::initializer_list<LayoutObject> objects)
      : m_layout(objects) {
    calculateLayoutProperties();
    spdlog::info("Created new buffer layout");
    debugPrint();
  }

  void addObject(const LayoutObject &object) { m_layout.push_back(object); }

  void debugPrint(){
    spdlog::debug("LayoutDebugPrint:");
    auto i = 0;
    for(auto &el: m_layout){
      spdlog::debug("Element {}: Type: {}, Size: {}, Stride: {}, Offset: {}", i, convertTypeToString(el.type), el.size, el.stride, el.offset);
    }
  }


};
} // namespace ENDER
