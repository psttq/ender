#pragma once

namespace ENDER {
    class Light {
    public:
        enum class LightType {
            PointLight,
            DirectionalLight,
            None
        };
        Light() {
            type = LightType::None;
        }

        virtual ~Light() = default;

        LightType type;
    };
}