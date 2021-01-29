#include <iostream>
#include <cmath>
#include <cassert>

#include "l_terrain.hpp"


namespace vanquisher {

	// - TerrainChunk

	TerrainChunk::TerrainChunk(int cx, int cy, int seed, int size, int resolution, double base_height) : heights(size * size), seed(seed), resolution(resolution), width(size) {
		this->pos_x = cx;
		this->pos_y = cy;
		this->off_x = cx * size / resolution;
		this->off_y = cy * size / resolution;
	
		int area = width * width;

		for (int i = 0; i < area; i++) {
			this->heights[i] = base_height;
		}
	}

	double TerrainChunk::get_off_x() {
		return this->off_x;
	}

	double TerrainChunk::get_off_y() {
		return this->off_y;
	}

	int TerrainChunk::get_resolution() {
		return resolution;
	}

	double TerrainChunk::get(int x, int y) {
		assert(x < this->width);
		assert(y < this->width);
		assert(x >= 0);
		assert(y >= 0);

		return this->heights[y * this->width + x];
	}

	double TerrainChunk::get(int index) {
		return this->heights[index];
	}

	void TerrainChunk::set(int x, int y, double value) {
		this->heights[y * this->width + x] = value;
	}

	void TerrainChunk::set(int index, double value) {
		this->heights[index] = value;
	}

	void TerrainChunk::add(int x, int y, double amount) {
		this->heights[y * this->width + x] += amount;
	}

	void TerrainChunk::add(int index, double amount) {
		this->heights[index] += amount;
	}

	template<typename... Args> 
	void TerrainChunk::generate(TerrainGenerator &generator) {
		generator.seed(this->seed);

		double off_x = this->get_off_x();
		double off_y = this->get_off_y();
		
		generator.generate(*this, off_x, off_y);
	}

	TerrainChunkIterCursor TerrainChunk::iter() {
		return TerrainChunkIterCursor(*this);
	}

	// - TerrainChunkIter

	TerrainChunkIterCursor::TerrainChunkIterCursor(TerrainChunk &terrain) : terrain(terrain) {
		this->width = terrain.width;
		this->area = width * width;
		this->index = 0;
		this->cx = 0;
		this->cy = 0;
		this->px = 0;
		this->py = 0;
	}

	bool TerrainChunkIterCursor::next() {
		index++;

		if (index >= area) {
			return false;
		}
		
		cx = index % width;
		cy = index / width;

		int resolution = terrain.get_resolution();

		px = (double) cx / resolution;
		py = (double) cy / resolution;

		return true;
	}

	double TerrainChunkIterCursor::get() {
		return this->terrain.get(this->index);
	}

	void TerrainChunkIterCursor::set(double value) {
		return this->terrain.set(this->index, value);
	}

	void TerrainChunkIterCursor::add(double amount) {
		return this->terrain.add(this->index, amount);
	}

	void TerrainChunkIterCursor::seek(int index) {
		this->index = index;

		if (index < 0) this->index = 0;
		if (index >= this->area) this->index = this->area - 1;

		cx = this->index % this->width;
		cy = this->index / this->width;

		int resolution = terrain.get_resolution();
		
		px = (double) cx / resolution;
		py = (double) cy / resolution;
	}

	void TerrainChunkIterCursor::seek(int x, int y) {
		this->seek(y * this->width + x);
	}
	
	TerrainGenerator::TerrainGenerator() : rng() {
		set_default_parameters();
	}

	void TerrainGenerator::set_default_parameters() {}

	void TerrainGenerator::seed(long int seed) {
		this->rng.seed(seed);
	}

	void TerrainGenerator::set_parameter(std::string name, double value) {
		params[name] = value;
	}

	SineTerrainGenerator::SineTerrainGenerator(double amplitude, double offset, double x_scale, double y_scale, double roughness) {
		params["amplitude"] = amplitude;
		params["offset"] = offset;
		params["xscale"] = x_scale;
		params["yscale"] = y_scale;
		params["roughness"] = roughness;
	}

	void SineTerrainGenerator::set_default_parameters() {
		// values
		double amplitude = 18, offset = 30, x_scale = 32, y_scale = 42, roughness = 0.15;

		// set them
		params["amplitude"] = amplitude;
		params["offset"] = offset;
		params["xscale"] = x_scale;
		params["yscale"] = y_scale;
		params["roughness"] = roughness;
	}

	void SineTerrainGenerator::generate(TerrainChunk &target, double off_x, double off_y) {
		// parameters

		double amplitude	= params["amplitude"];
		double offset		= params["offset"];
		double roughness	= params["roughness"];
		double x_scale		= params["xscale"];
		double y_scale		= params["yscale"];

		// setup
	
		double half_amplitude = amplitude / 2.0;
			
		auto cursor = target.iter();
		std::uniform_real_distribution<double> dist;

		if (roughness) {
			dist = std::uniform_real_distribution<double>(-roughness * amplitude, roughness * amplitude);
		}

		// generate

		x_scale = M_PI * 2 / x_scale;
		y_scale = M_PI * 2 / y_scale;

		while (cursor.next()) {
			double rough = 0.0;

			if (roughness) {
				rough += dist(this->rng);
			}

			double val = offset + rough + half_amplitude * (
				sin((off_x + cursor.px) * x_scale) +
				sin((off_y + cursor.py) * y_scale)
			);

			cursor.add(val);
		}
	}

	// - Terrain

	Terrain::Terrain(int world_seed, int chunk_width, TerrainGenerator &generator, int resolution) : chunks(), chunk_list(), generator(generator), chunk_width(chunk_width), world_seed(world_seed), resolution(resolution) {}

	std::pair<size_t, TerrainChunk &> Terrain::make(int cx, int cy, int seed, double base_height) {
		TerrainChunk item(cx, cy, seed, this->chunk_width, resolution, base_height);

		this->chunk_list.push_back(item);

		TerrainChunk &res = this->chunk_list.back();
		res.generate(this->generator);

		return { this->chunk_list.size() - 1, res };
	}

	TerrainChunk &Terrain::generate(int cx, int cy, int seed, double base_height) {
		auto new_chunk = this->make(cx, cy, seed, base_height);
		
		size_t index = new_chunk.first;
		TerrainChunk &res = new_chunk.second;
	
		this->chunks.insert({{cx, cy}, index});

		return res;
	}

	int Terrain::chunk_seed_for(int cx, int cy) {
		// Gets the seed for a specific chunk X and chunk Y.

		return (world_seed << 4) ^ (0xAAAA ^ cx ^ 2 * cy);
	}

	TerrainChunk &Terrain::fetch(int cx, int cy) {
		if (!chunks.count({cx, cy})) {
			generate(cx, cy, chunk_seed_for(cx, cy));
		}
	
		return chunk_list[chunks[std::make_pair(cx, cy)]];
	}

	double Terrain::get_height(double px, double py) {
		// Compute some coords

		double tile_x = px * resolution;
		double tile_y = py * resolution;
		double tile_x1 = floor(tile_x);
		double tile_y1 = floor(tile_y);
		double tile_x2 = tile_x1 + 1;
		double tile_y2 = tile_y1 + 1;

		double cx1 = floor(tile_x1 / chunk_width);
		double cx2 = floor(tile_x2 / chunk_width);
		double cy1 = floor(tile_y1 / chunk_width);
		double cy2 = floor(tile_y2 / chunk_width);

		double localtile_x1 = tile_x1 - cx1 * chunk_width;
		double localtile_x2 = tile_x2 - cx2 * chunk_width;
		double localtile_y1 = tile_y1 - cy1 * chunk_width;
		double localtile_y2 = tile_y2 - cy2 * chunk_width;

		// Fetch heights from chunks

		auto chunk_a = fetch(cx1, cy1);
		auto chunk_b = fetch(cx2, cy1);
		auto chunk_c = fetch(cx1, cy2);
		auto chunk_d = fetch(cx2, cy2);

		double val_a = chunk_a.get(localtile_x1, localtile_y1);
		double val_b = chunk_b.get(localtile_x2, localtile_y1);
		double val_c = chunk_c.get(localtile_x1, localtile_y2);
		double val_d = chunk_d.get(localtile_x2, localtile_y2);

		// Interpolate and return

		double weight_a = (tile_x2 - tile_x) * (tile_y2 - tile_y);
		double weight_b = (tile_x2 - tile_x) * (tile_y - tile_y1);
		double weight_c = (tile_x - tile_x1) * (tile_y2 - tile_y);
		double weight_d = (tile_x - tile_x1) * (tile_y - tile_y1);

		return (
			val_a * weight_a +
			val_b * weight_b +
			val_c * weight_c +
			val_d * weight_d
		);
	}
}
