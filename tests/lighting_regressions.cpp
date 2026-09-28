#include "../src/lib/matrix_lerp.cpp"
#include <cassert>
#include <kf/platform/prelude.hpp>

int main()
{
    MATRIX from{}, to{};
    KfRenderState view{};
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column) {
            from.m[row][column] = -1000 + 100 * row + column;
            to.m[row][column] = 2000 - 100 * row - column;
        }
    for (s32 blend : {0, 1024, 2048, 4096}) {
        auto &output = view.lighting.color_matrix;
        output.t[0] = 11;
        output.t[1] = 22;
        output.t[2] = 33;
        lighting_set_color_matrix(view, &from, &to, blend);
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 3; ++column)
                assert(output.m[row][column] ==
                       from.m[row][column] +
                           (((to.m[row][column] - from.m[row][column]) * blend) >> 12));
        assert(output.t[0] == 11 && output.t[1] == 22 && output.t[2] == 33);
        // Updating the active matrix in place must support aliased endpoints.
        lighting_set_color_matrix(view, &output, &to, 4096);
        assert(std::memcmp(output.m, to.m, sizeof output.m) == 0);
        lighting_set_color_matrix(view, &from, &output, 0);
        assert(std::memcmp(output.m, from.m, sizeof output.m) == 0);
        assert(output.t[0] == 11 && output.t[1] == 22 && output.t[2] == 33);
    }
}
