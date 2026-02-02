#include "perlinNoise.h"

#include <random>

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);

    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);

    p.insert(p.end(), p.begin(), p.end());
}

PerlinNoise::~PerlinNoise() {}

float PerlinNoise::noise(float nx, float ny) {
    int xi = static_cast<int>(std::floor(nx)) & 255;
    int yi = static_cast<int>(std::floor(ny)) & 255;

    float xf = nx - std::floor(nx);
    float yf = ny - std::floor(ny);

    float u = fade(xf);
    float v = fade(yf);

    int aa = p[p[xi] + yi];
    int ab = p[p[xi] + yi + 1];
    int ba = p[p[xi + 1] + yi];
    int bb = p[p[xi + 1] + yi + 1];

    float x1 = lerp(grad(aa, xf, yf),
                    grad(ba, xf - 1, yf), u);
    float x2 = lerp(grad(ab, xf, yf - 1),
                    grad(bb, xf - 1, yf - 1), u);

    return (lerp(x1, x2, v) + 1.0f) / 2.0f;
}

float PerlinNoise::val(float x, float y) {
    float e = elevation(x, y, 8);

    if (e < 0.0f) e = 0.0f;

    float val = static_cast<float>(std::pow(e, 5));

    return val;
}

float PerlinNoise::elevation(float nx, float ny, int layers) {
    float e = 0.0f;
    float maxAmpl = 0.0f;

    for (int i = 0; i < layers; i++) {
        int fac = 1 << i;
        double ampl = pow(0.5, i);

        e += amplitude(nx, ny, fac, ampl);
        maxAmpl += ampl;
    }

    return e / maxAmpl;
}

float PerlinNoise::amplitude(float nx, float ny, int fac, double amplitude) {
    return amplitude * noise(fac * nx, fac * ny);
}

BiomeType PerlinNoise::get_biome(int wx, int wy) {
    double temp = noise(wx * 0.003, wy * 0.003) * 2.0 - 1.0;
    double moisture = noise(wx * 0.003 + 1000, wy * 0.003 + 1000) * 2.0 - 1.0;

    if (temp < -0.3) {
        return BiomeType::TUNDRA;
    } else if (temp > 0.3) {
        if (moisture < -0.2) {
            return BiomeType::MOUNTAIN;
        } else {
            return BiomeType::FOREST;
        }
    } else {
        if (moisture < -0.3) {
            return BiomeType::MOUNTAIN;
        } else if (moisture > 0.3) {
            return BiomeType::OCEAN;
        } else {
            return BiomeType::FOREST;
        }
    }
}
