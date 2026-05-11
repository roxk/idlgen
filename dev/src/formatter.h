#include <winrt/base.h>

consteval char hex_digit(unsigned v)
{
    return "0123456789ABCDEF"[v & 0xF];
}

consteval std::array<char, 37> guid_to_string(winrt::guid const& g)
{
    std::array<char, 37> out{};
    int i = 0;

    auto emit = [&](unsigned v, int n)
    {
        for (int k = n - 1; k >= 0; --k)
            out[i++] = hex_digit(v >> (k * 4));
    };

    emit(g.Data1, 8);
    out[i++] = '-';
    emit(g.Data2, 4);
    out[i++] = '-';
    emit(g.Data3, 4);
    out[i++] = '-';
    emit(g.Data4[0], 2);
    emit(g.Data4[1], 2);
    out[i++] = '-';
    for (int j = 2; j < 8; ++j)
        emit(g.Data4[j], 2);

    out[36] = '\0';
    return out;
}
