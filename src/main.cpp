#include "MyApplication.hpp"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 800;

int main() {
  spdlog::set_level(spdlog::level::info);
    MyApplication app{SCR_WIDTH, SCR_HEIGHT};
  app.start();
  return 0;
}
