#pragma once
# include "fastNoise/FastNoiseLite.h"

struct NosieSettings
{
	float lacunarity;
	float persistance;

	float noiseScale;
	float baseFrequency;
	float octaves;

	float maxHeight;
	bool  ridge;
	int	  ridgePower;
};

class NoiseGenerator
{
public:
	inline float GetHeight(FastNoiseLite& noise, int x, int y, NosieSettings& set)
	{
		float frequency = set.baseFrequency;
		float amplitude;
		float ampSum = 0.0f;

		float noiseHeight = 0.0f;
		noise.SetFrequency(frequency);
		for (int octave = 0; octave < set.octaves; octave++)
		{
			frequency = (std::powf(set.lacunarity, octave));
			amplitude = (std::powf(set.persistance, octave));

			float sampleX = x / set.noiseScale * frequency;
			float sampleY = y / set.noiseScale * frequency;

			float perlinValue = noise.GetNoise(sampleX, sampleY);

			if (set.ridge)
			{
				perlinValue = 1.0 - fabs(perlinValue);
				perlinValue = pow(perlinValue, set.ridgePower);
			}

			noiseHeight += perlinValue * amplitude;
			ampSum += amplitude;
		}
		noiseHeight /= ampSum;

		noiseHeight = (noiseHeight + 1.0f) * 0.65f - 0.5f;
		noiseHeight *= set.maxHeight;

		return noiseHeight;
	}
};