/**
 * Vanquisher terrain.
 */

#ifndef H_VANQUISHER_LEVEL_TERRAIN
#define H_VANQUISHER_LEVEL_TERRAIN


#include <vector>
#include <random>
#include <map>
#include <unordered_map>
#include <string>


class _hash_pair {
	// Credits to https://stackoverflow.com/a/20602159/5129091 for original solution
public:
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U> &x) const
	{
		auto hash_1 = std::hash<T>()(x.first);
		return hash_1 << 1 ^ std::hash<U>()(x.second) ^ (hash_1 >> (sizeof(hash_1) * 8 - 1));
	}
};

namespace vanquisher {
	class TerrainChunkIterCursor;
	class TerrainGenerator;

	class TerrainChunk {
	private:
		std::vector<double> heights; 
		int seed;
		int pos_x, pos_y;
		double off_x;
		double off_y;
		int resolution;

	public:
		int width;

		TerrainChunk(int cx, int cy, int seed, int width, int resolution = 1, double base_height = 0.0);
		double get(int x, int y);
		double get(int index);
		double get_off_x();
		double get_off_y();
		int get_resolution();
		void set(int x, int y, double value);
		void set(int index, double value);
		void add(int x, int y, double amount);
		void add(int index, double amount);
		TerrainChunkIterCursor iter();

		template<typename... Args> 
		void generate(TerrainGenerator &generator);
	};

	class TerrainChunkIterCursor {
	private:
		TerrainChunk &terrain;
		int width, area;

	public:
		int index, cx, cy;
		double px, py;

		explicit TerrainChunkIterCursor(TerrainChunk &terrain);
		TerrainChunk &get_terrain();
		double get();
		void set(double value);
		void add(double amount);
		bool next();
		void seek(int x, int y);
		void seek(int index);
	};

	class TerrainGenerator {
	protected:
		std::mt19937 rng;
		std::map<std::string, double> params;
		virtual void set_default_parameters();

	public:	
		TerrainGenerator();
		void seed(long int seed);
		void set_parameter(std::string name, double value);
		virtual void generate(TerrainChunk &target, double off_x, double off_y) = 0;
	};

	class SineTerrainGenerator : public TerrainGenerator {
	public:
		SineTerrainGenerator(double amplitude = 18., double offset = 30., double x_scale = 32., double y_scale = 42., double roughness = .15);
	
		void generate(TerrainChunk &target, double off_x, double off_y) override;
		void set_default_parameters() override;
	};

	class Terrain {
	protected:
		std::unordered_map<std::pair<int, int>, TerrainChunk*, _hash_pair> chunks;
		std::vector<TerrainChunk> chunk_list;
		TerrainGenerator &generator;
		int chunk_width;
		int world_seed;
		int resolution;

	public:
		explicit Terrain(int world_seed, int chunk_width, TerrainGenerator &generator, int resolution = 1);
		TerrainChunk &make(int cx, int cy, int seed, double base_height = 0.0);
		void generate(int cx, int cy, int seed, double base_height = 0.0);
		int chunk_seed_for(int cx, int cy);
		TerrainChunk &fetch(int cx, int cy);
		double get_height(double px, double py);
	};
};


#endif
