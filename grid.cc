#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>


namespace {
	const std::string ENVIRONMENT = "hospital_section.pnm";
	const std::string OUTPUT_FILEPATH = "output_wavefront.pnm";
	const int DIRECTIONS = 8;
	const int ADJ = 3;
	const int COLOR_FACTOR = 200;
};


// simple geometric data structures
struct Point2D {
	Point2D(double x, double y) : 
		x_(x), y_(y) 
	{}
	double x_, y_;
};


struct PointXY {
	PointXY() :
		x_(0),y_(0)
	{}
	PointXY(int x, int y) :
		x_(x),y_(y)
	{}
	int x_,y_;
};

typedef std::vector<PointXY>::iterator pIter;


class Map {

	
public:
	Map(const std::string& filepath);
	~Map();
	
	int wavefront(
		Point2D& pos,
		Point2D& destination);
	
	PointXY* propagate_wave(
		PointXY*& perimeter,
		int& size,
		int gradient);
	void swap_waves(
		PointXY*& perimeter,
		PointXY*& frontier,
		int size);
		
	bool goal_reached(
		PointXY*& perimeter,
		int size,
		PointXY& goal);
	bool goal_unreachable(int perimeter_size);
	
	PointXY cast_point(Point2D& point);
	void clear_perimeter(PointXY*& perimeter);
	
	void read(const std::string& filepath);
	void read_txt(const std::string& filepath);
	void output_wavefront(
		const std::string& filepath,
		const std::string& output_path,
		int gradient);
	
	
private:
	std::vector<std::vector<double> > map_;
	std::vector<std::vector<int> > wave_;
	
	
};


Map::Map(const std::string& filepath) {
	this->read(filepath);
}


Map::~Map() {
	
}


void Map::read(const std::string& filepath) {
	std::string line, header;
	std::ifstream in_file(filepath);
	std::getline(in_file,header);
	int width,height,max_val;
	in_file >> width >> height >> max_val;
	std::cout << header << std::endl;
	while (std::getline(in_file,line)) {
		map_= std::vector<std::vector<double> >(
			height,std::vector<double>(width));
		wave_ = std::vector<std::vector<int> >(
			height,std::vector<int>(width,0));
		for (size_t i = 0; i < line.size(); ++i)
			map_[i/width][i%width] = static_cast<double>(line[i]+1);
	}
}


void Map::output_wavefront(
	const std::string& filepath,
	const std::string& output_path,
	int gradient)
{
	std::string line, header;
	std::ifstream in_file(filepath);
	std::getline(in_file,header);
	int width,height,max_val;
	in_file >> width >> height >> max_val;

	float max_grd = static_cast<float>(gradient);
	std::ofstream out_file(output_path);
	out_file << header << std::endl;
	out_file << width << " " << height << " " << max_val << std::endl;
	for (size_t r = 0; r < wave_.size(); ++r)
		for (size_t c = 0; c < wave_[r].size(); ++c)
			out_file << (map_[r][c] == 1 || wave_[r][c] == 0 ? 
				static_cast<char>(map_[r][c]-1) : 
				static_cast<char>(wave_[r][c]*COLOR_FACTOR/max_grd));
}


void Map::read_txt(const std::string& filepath) {
	std::ifstream in_file(filepath); 
	std::string line;
	while (std::getline(in_file,line)) {
		map_.push_back(std::vector<double>(line.size()));
		wave_.push_back(std::vector<int>(line.size(),0));
		for (size_t i = 0; i < map_.back().size(); ++i) {
			map_.back()[i] = line[i]-'0';
		}
	}
}


PointXY* Map::propagate_wave(
	PointXY*& perimeter,
	int& size,
	int gradient)
{
	int iter = 0;
	PointXY* frontier = new PointXY[DIRECTIONS*size];
	for (int p = 0; p < size; ++p) {
		for (int i = 0; i < ADJ*ADJ; ++i) {
			int r = (perimeter[p].y_-1)+i/ADJ,
				c = (perimeter[p].x_-1)+i%ADJ;
			if (map_[r][c] == 0 && wave_[r][c] == 0) {
				frontier[iter++] = PointXY(c,r);
				wave_[r][c] = gradient;
			}
		}
	}
	size = iter;
	return frontier;
}


void Map::swap_waves(PointXY*& perimeter, PointXY*& frontier, int size) {
	this->clear_perimeter(perimeter);
	perimeter = new PointXY[size];
	for (int i = 0; i < size; ++i)
		perimeter[i] = frontier[i];
	this->clear_perimeter(frontier);
}



bool Map::goal_reached(PointXY*& perimeter, int size, PointXY& goal) {
	for (int i = 0; i < size; ++i)
		if (perimeter[i].x_ == goal.x_ && perimeter[i].y_ == goal.y_)
			return true;
	return false;
}


bool Map::goal_unreachable(int perimeter_size) {
	if (perimeter_size == 0)
		return true;
	return false;
}


int Map::wavefront(Point2D& player, Point2D& destination) {
	int gradient = 1, size = 1;
	PointXY dest = this->cast_point(destination),
			plyr = this->cast_point(player);
	this->wave_[dest.y_][dest.x_] = gradient;
	PointXY* perim = new PointXY[size];
	perim[size-1] = PointXY(dest.x_,dest.y_);
	while (!this->goal_reached(perim,size,plyr)) {
		int perim_s = size;
		PointXY* frontier = this->propagate_wave(
			perim,size,++gradient);
		if (this->goal_unreachable(perim_s))
			return -1;
		this->swap_waves(perim,frontier,size);
	}
	return gradient;
}


PointXY Map::cast_point(Point2D& point) {
	return PointXY(
		static_cast<int>(point.x_),
		static_cast<int>(point.y_));
}


void Map::clear_perimeter(PointXY*& perimeter) {
	if (perimeter != nullptr) {
		delete [] perimeter;
		perimeter = nullptr;
	}
}


int main(int argc, char* argv[]) {	
	Point2D dest = Point2D(700.0,400.0);
	Point2D player = Point2D(50.0,50.0);
	Map map;
	int gradient = map.wavefront(player,dest);
	map.output_wavefront(ENVIRONMENT,OUTPUT_FILEPATH,gradient);
	return 0;
}



