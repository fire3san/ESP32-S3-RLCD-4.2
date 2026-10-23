// 验证设置页五枚单色图标的尺寸、行填充和有效像素预算。
#include "settings_naive_icons.h"
#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    using namespace settings_naive_icons;
    static_assert(kSize == 20 && kBytesPerRow == 3);
    static_assert(sizeof(k_wifi) + sizeof(k_volume) + sizeof(k_monitor) +
                      sizeof(k_info) + sizeof(k_settings) == 300);
    const std::array<const uint8_t *, 5> icons = {
        k_wifi, k_volume, k_monitor, k_info, k_settings,
    };
    for (const uint8_t *icon : icons) {
        unsigned ink = 0;
        for (int y = 0; y < kSize; ++y) {
            assert((icon[y * kBytesPerRow + 2] & 0x0fU) == 0);
            for (int x = 0; x < kSize; ++x) {
                ink += (icon[y * kBytesPerRow + x / 8] >> (7 - x % 8)) & 1U;
            }
        }
        assert(ink >= 40 && ink <= 200);
    }
}
