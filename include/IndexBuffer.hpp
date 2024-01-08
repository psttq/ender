#include <spdlog/spdlog.h>

namespace ENDER {

class IndexBuffer {
  unsigned int m_id;
  unsigned int m_count;

public:
  IndexBuffer(unsigned int *indices, unsigned int count);
  ~IndexBuffer();

  void bind();
  void unbind();
  unsigned int getIndex(){
    return m_id;
  }
};
}; // namespace ENDER
