#include "glm/gtc/type_ptr.hpp"
#include <Material.hpp>
#include <imgui.h>

namespace ENDER {
void Material::drawImguiEdit() {
  ImGui::ColorEdit3("Ambient", glm::value_ptr(ambient));
  ImGui::ColorEdit3("Diffuse", glm::value_ptr(diffuse));
  ImGui::ColorEdit3("Specular", glm::value_ptr(specular));
  ImGui::InputFloat("Shininess", &shininess);
}
} // namespace ENDER
