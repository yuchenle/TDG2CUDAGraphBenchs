/**
 * Author:  Florian Stock, Technische Universität Darmstadt,
 * Embedded Systems & Applications Group 2018
 * Author:  Lukas Sommer, Technische Universität Darmstadt,
 * Embedded Systems & Applications Group 2018
 * Author:  Sebastian Müller, Techinsche Universität Darmstadt,
 * Embedded Systems & Applications Group 2018 - 2019
 * License: Apache 2.0 (see attachached File)
 *
 * Kernel extracted from Autoware suite.
 * Dependencies on the PCL (PointCloudLib) and CV (OpenCV) libs are removed.
 * For their licenses see license folder.
 *
 * Kernel uses 2500 invocations of the pointcloud2_to_image function from the
 * points2image-package/node
 * (see Autoware/ros/src/sensing/fusion/packages/points2image/lib/points_image/points_image.cpp)
 *
 * Computed results are compared with the Autoware computed result.
 */
#include "benchmark.h"
#include "datatypes.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <omp.h>
#include "timer.h"
//#include "../include/meassurement_AverageOnly.h"


// Defining Pointers to be used in heterogeneous graph here
// so that mem copy and host pointers they operate over constant addresses
// For fairness, we declared them here for teams distribute and
// target tasks
float cloud[64000*32];
PointsImage msg;
double total_time = 0;
double distanceArr[64000];
Point2d imagePointArr[64000];
#define MAX_EPS 0.001

class points2image : public kernel {
private:
	int deviceId = 0;
private:
	// the number of testcases read
	int read_testcases = 0;
	// testcase and reference data streams
	std::ifstream input_file, output_file;
	// whether critical deviation from the reference data has been detected
	bool error_so_far = false;
	// deviation from the reference data
	double max_delta = 0.0;
	// the point clouds to process in one iteration
	PointCloud2* pointcloud2 = nullptr;
	// the associated camera extrinsic matrices
	Mat44* cameraExtrinsicMat = nullptr;
	// the associated camera intrinsic matrices
	Mat33* cameraMat = nullptr;
	// distance coefficients for the current iteration
	Vec5* distCoeff = nullptr;
	// image sizes for the current iteration
	ImageSize* imageSize = nullptr;
	// Algorithm results for the current iteration
	PointsImage* results = nullptr;
public:
	/*
	 * Initializes the kernel. Must be called before run().
	 */
	virtual void init();
	/**
	 * Performs the kernel operations on all input and output data.
	 * p: number of testcases to process in one step
	 */
	virtual void run(int p = 1);
	/**
	 * Finally checks whether all input data has been processed successfully.
	 */
	virtual bool check_output();

protected:
	/**
	* Reads the next test cases.
	* count: the number of testcases to read
	* returns: the number of testcases actually read
	*/
	virtual int read_next_testcases(int count);
	/**
	 * Compares the results from the algorithm with the reference data.
	 * count: the number of testcases processed
	 */
	virtual void check_next_outputs(int count);
	/**
	 * Reads the number of testcases in the data set.
	 */
	int read_number_testcases(std::ifstream& input_file);
};

/**
 * Parses the next point cloud from the input stream.
 */
void  parsePointCloud(std::ifstream& input_file, PointCloud2* pointcloud2) {
	try {
		input_file.read((char*)&(pointcloud2->height), sizeof(int32_t));
		input_file.read((char*)&(pointcloud2->width), sizeof(int32_t));
		input_file.read((char*)&(pointcloud2->point_step), sizeof(uint32_t));
		pointcloud2->data = new float[pointcloud2->height * pointcloud2->width * pointcloud2->point_step];
		input_file.read((char*)pointcloud2->data, pointcloud2->height * pointcloud2->width * pointcloud2->point_step);
	}  catch (std::ifstream::failure) {
		throw std::ios_base::failure("Error reading the next point cloud.");
	}
}
/**
 * Parses the next camera extrinsic matrix.
 */
void  parseCameraExtrinsicMat(std::ifstream& input_file, Mat44* cameraExtrinsicMat) {
	try {
		for (int h = 0; h < 4; h++)
			for (int w = 0; w < 4; w++)
				input_file.read((char*)&(cameraExtrinsicMat->data[h][w]),sizeof(double));
	} catch (std::ifstream::failure) {
		throw std::ios_base::failure("Error reading the next extrinsic matrix.");
	}
}
/**
 * Parses the next camera matrix.
 */
void parseCameraMat(std::ifstream& input_file, Mat33* cameraMat ) {
	try {
		for (int h = 0; h < 3; h++)
			for (int w = 0; w < 3; w++)
				input_file.read((char*)&(cameraMat->data[h][w]), sizeof(double));
	} catch (std::ifstream::failure) {
		throw std::ios_base::failure("Error reading the next camera matrix.");
	}
}
/**
 * Parses the next distance coefficients.
 */
void  parseDistCoeff(std::ifstream& input_file, Vec5* distCoeff) {
	try {
		for (int w = 0; w < 5; w++)
			input_file.read((char*)&(distCoeff->data[w]), sizeof(double));
	} catch (std::ifstream::failure) {
		throw std::ios_base::failure("Error reading the next set of distance coefficients.");
	}
}
/**
 * Parses the next image sizes.
 */
void  parseImageSize(std::ifstream& input_file, ImageSize* imageSize) {
        try {
                input_file.read((char*)&(imageSize->width), sizeof(int32_t));
                input_file.read((char*)&(imageSize->height), sizeof(int32_t));
        } catch (std::ifstream::failure) {
                throw std::ios_base::failure("Error reading the next image size.");
        }
}
/**
 * Parses the next reference image.
 */
void parsePointsImage(std::ifstream& output_file, PointsImage* goldenResult) {
	try {
		// read data of static size
		output_file.read((char*)&(goldenResult->image_width), sizeof(int32_t));
		output_file.read((char*)&(goldenResult->image_height), sizeof(int32_t));
		output_file.read((char*)&(goldenResult->max_y), sizeof(int32_t));
		output_file.read((char*)&(goldenResult->min_y), sizeof(int32_t));
		int pos = 0;
		int elements = goldenResult->image_height * goldenResult->image_width;
		goldenResult->intensity = new float[elements];
		goldenResult->distance = new float[elements];
		goldenResult->min_height = new float[elements];
		goldenResult->max_height = new float[elements];
		// read data of variable size
		for (int h = 0; h < goldenResult->image_height; h++)
			for (int w = 0; w < goldenResult->image_width; w++)
			{
				output_file.read((char*)&(goldenResult->intensity[pos]), sizeof(float));
				output_file.read((char*)&(goldenResult->distance[pos]), sizeof(float));
				output_file.read((char*)&(goldenResult->min_height[pos]), sizeof(float));
				output_file.read((char*)&(goldenResult->max_height[pos]), sizeof(float));
				pos++;
			}
	} catch (std::ios_base::failure) {
		throw std::ios_base::failure("Error reading the next reference image.");
	}
}

int points2image::read_next_testcases(int count)
{
	// free the memory that has been allocated in the previous iteration
	// and allocate new for the currently required data sizes
	if (pointcloud2)
		for (int m = 0; m < count; ++m)
			delete [] pointcloud2[m].data;
	delete [] pointcloud2;
	pointcloud2 = new PointCloud2[count];
	delete [] cameraExtrinsicMat;
	cameraExtrinsicMat = new Mat44[count];
	delete [] cameraMat;
	cameraMat = new Mat33[count];
	delete [] distCoeff;
	distCoeff = new Vec5[count];
	delete [] imageSize;
	imageSize = new ImageSize[count];
	if (results)
	for (int m = 0; m < count; ++m)
	{
		delete [] results[m].intensity;
		delete [] results[m].distance;
		delete [] results[m].min_height;
		delete [] results[m].max_height;
	}
	delete [] results;
	results = new PointsImage[count];

	// iteratively read the data for the test cases
	int i;
	for (i = 0; (i < count) && (read_testcases < testcases); i++,read_testcases++)
	{
		try {
			parsePointCloud(input_file, pointcloud2 + i);
			parseCameraExtrinsicMat(input_file, cameraExtrinsicMat + i);
			parseCameraMat(input_file, cameraMat + i);
			parseDistCoeff(input_file, distCoeff + i);
			parseImageSize(input_file, imageSize + i);
		} catch (std::ios_base::failure& e) {
			std::cerr << e.what() << std::endl;
			exit(-3);
		}
	}
	return i;
}
int points2image::read_number_testcases(std::ifstream& input_file)
{
	// reads the number of testcases in the data stream
	int32_t number;
	try {
		input_file.read((char*)&(number), sizeof(int32_t));
	} catch (std::ifstream::failure) {
		throw std::ios_base::failure("Error reading the number of testcases.");
	}

	return number;
}

void points2image::init() {
	std::cout << "init\n";

	// open testcase and reference data streams
	input_file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	output_file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try {
		input_file.open("../../../data/p2i_input.dat", std::ios::binary);
	} catch (std::ifstream::failure) {
		std::cerr << "Error opening the input data file" << std::endl;
		exit(-2);
	}
	try {
		output_file.open("../../../data/p2i_output.dat", std::ios::binary);
	} catch (std::ifstream::failure) {
		std::cerr << "Error opening the output data file" << std::endl;
		exit(-2);
	}
	try {
	// consume the total number of testcases
		testcases = read_number_testcases(input_file);
	} catch (std::ios_base::failure& e) {
		std::cerr << e.what() << std::endl;
		exit(-3);
	}
	// device selection
	int deviceNo = omp_get_num_devices();
	deviceId = std::max(0, deviceNo -1);
	std::cout << "Selected device " << deviceId;
	std::cout << " out of " << deviceNo << std::endl;

	// prepare the first iteration
	error_so_far = false;
	max_delta = 0.0;
	pointcloud2 = nullptr;
	cameraExtrinsicMat = nullptr;
	cameraMat = nullptr;
	distCoeff = nullptr;
	imageSize = nullptr;
	results = nullptr;

	std::cout << "done\n" << std::endl;
}


/**
 * This code is extracted from Autoware, file:
 * ~/Autoware/ros/src/sensing/fusion/packages/points2image/lib/points_image/points_image.cpp
 * It uses the test data that has been read before and applies the linked algorithm.
 * pointcloud2: cloud of points to transform
 * cameraExtrinsicMat: camera matrix used for transformation
 * cameraMat: camera matrix used for transformation
 * distCoeff: distance coefficients for cloud transformation
 * imageSize: the size of the resulting image
 * returns: the two dimensional image of transformed points
 */
PointsImage pointcloud2_to_image(
	const PointCloud2& pointcloud2,
	const Mat44& cameraExtrinsicMat,
	const Mat33& cameraMat, const Vec5& distCoeff,
	const ImageSize& imageSize, int iter)
{
        // initialize the resulting image data structure
	int w = imageSize.width;
	int h = imageSize.height;
	msg.intensity = new float[w*h];
	std::memset(msg.intensity, 0, sizeof(float)*w*h);
	msg.distance = new float[w*h];
	std::memset(msg.distance, 0, sizeof(float)*w*h);
	msg.min_height = new float[w*h];
	std::memset(msg.min_height, 0, sizeof(float)*w*h);
	msg.max_height = new float[w*h];
	std::memset(msg.max_height, 0, sizeof(float)*w*h);
	msg.max_y = -1;
	msg.min_y = h;
	msg.image_height = imageSize.height;
	msg.image_width = imageSize.width;
	int32_t max_y = -1;
	int32_t min_y = h;

	// float* cloud = (float *)pointcloud2.data;
	memcpy(cloud, (void *)pointcloud2.data, sizeof(float) * pointcloud2.height * pointcloud2.width * pointcloud2.point_step);

	// preprocess the given matrices
	// transposed 3x3 camera extrinsic matrix
	Mat33 invR;
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 3; col++)
			invR.data[row][col] = cameraExtrinsicMat.data[col][row];
	// translation vector: (transposed camera extrinsic matrix)*(fourth column of camera extrinsic matrix)
	Mat13 invT;
	for (int row = 0; row < 3; row++) {
		invT.data[row] = 0.0;
		for (int col = 0; col < 3; col++)
			invT.data[row] -= invR.data[row][col] * cameraExtrinsicMat.data[col][3];
	}
	// various data sizes in bytes
	int sizeMat = pointcloud2.width * pointcloud2.height;
	// setting as an upper bound so that we don't seg fault with taskgraph
	sizeMat = 64000;
	int sizeMaxCp = sizeMat * pointcloud2.point_step;
	// double* distanceArr = new double[sizeMat];
	// Point2d* imagePointArr = new Point2d[sizeMat];
	int cloudHeight = pointcloud2.height;
	int cloudWidth = pointcloud2.width;
	int cloudStepSize = pointcloud2.point_step;

    START_TIMER;
	// point transformation
	#pragma omp target enter data map(always, to:distanceArr[:sizeMat],imagePointArr[:sizeMat], cloud[:sizeMaxCp],distCoeff,\
	                                     cameraMat,invT,invR,cloudHeight,cloudWidth,cloudStepSize)
	#ifdef BLOCKING
	int num_blocks = 32;
	int block_size = sizeMat / num_blocks;
	// for (uint32_t k = 0; k < cloudWidth * cloudHeight; k += block_size) {
	for (uint32_t k = 0; k < sizeMat; k += block_size) {
		#pragma omp target nowait
		#pragma omp teams distribute parallel for
		for (uint32_t idx = k; idx < k + block_size; ++idx) {
			uint32_t x = idx / cloudHeight;
			uint32_t y = idx % cloudHeight;
			// potential overflow because of large stride (block size)
			if (x < cloudWidth) {

	#else
	#pragma omp target teams distribute parallel for collapse(2)
	for (uint32_t x = 0; x < cloudWidth; ++x) {
		for (uint32_t y = 0; y < cloudHeight; ++y) {
	#endif
			int iPoint =x + y * cloudWidth;
			float* fp = (float *)(((uintptr_t)cloud) + (x + y*cloudWidth) * cloudStepSize);

			double intensity = fp[4];

			Mat13 point, point2;
			point2.data[0] = double(fp[0]);
			point2.data[1] = double(fp[1]);
			point2.data[2] = double(fp[2]);
			// apply matrices
			for (int row = 0; row < 3; row++) {
				point.data[row] = invT.data[row];
				for (int col = 0; col < 3; col++)
					point.data[row] += point2.data[col] * invR.data[row][col];
			}
			distanceArr[iPoint] = point.data[2] * 100.0;
			// discard points that are too near
			if (point.data[2] <= 2.5) {
				Point2d imagepointError;
				imagepointError.x = -1;
				imagepointError.y = -1;
				imagePointArr[iPoint] = imagepointError;
				continue;
			}
			// determine image coordinates
			double tmpx = point.data[0] / point.data[2];
			double tmpy = point.data[1] / point.data[2];
			double r2 = tmpx * tmpx + tmpy * tmpy;
			double tmpdist = 1 + distCoeff.data[0] * r2
					+ distCoeff.data[1] * r2 * r2
					+ distCoeff.data[4] * r2 * r2 * r2;
			Point2d imagepoint;
			imagepoint.x = tmpx * tmpdist
				+ 2 * distCoeff.data[2] * tmpx * tmpy
				+ distCoeff.data[3] * (r2 + 2 * tmpx * tmpx);
			imagepoint.y = tmpy * tmpdist
				+ distCoeff.data[2] * (r2 + 2 * tmpy * tmpy)
				+ 2 * distCoeff.data[3] * tmpx * tmpy;
			imagepoint.x = cameraMat.data[0][0] * imagepoint.x + cameraMat.data[0][2];
			imagepoint.y = cameraMat.data[1][1] * imagepoint.y + cameraMat.data[1][2];
			imagePointArr[iPoint] = imagepoint;
		#ifdef BLOCKING
		}// if (x < cloudWidth && y < cloudHeight)
		#endif
		} // for y
	} // for x

	#ifdef BLOCKING
	#pragma omp taskwait
	#endif

	#pragma omp target exit data map(always,from:distanceArr[:sizeMat],imagePointArr[:sizeMat])
	#pragma omp taskwait


	// image formation
	for (uint32_t x = 0; x < cloudWidth; ++x) {
		for (uint32_t y = 0; y < cloudHeight; ++y) {
			int iPoint =x + y * cloudWidth;
			// restore values
			double distance = distanceArr[iPoint];
			// discard near points again
			if (distance <= (2.5 * 100.0)) {
				continue;
			}
			float* fp = (float *)(((uintptr_t)cloud) + (x + y*cloudWidth) * cloudStepSize);
			double intensity = fp[4];
			Point2d imagepoint = imagePointArr[iPoint];
			int px = int(imagepoint.x + 0.5);
			int py = int(imagepoint.y + 0.5);
			if(0 <= px && px < w && 0 <= py && py < h)
			{
				// write to image and update vertical extends
				int pid = py * w + px;
				if(msg.distance[pid] == 0 || msg.distance[pid] > distance)
				{
					msg.distance[pid] = float(distance); //msg is das problem beim paralelisieren
					msg.intensity[pid] = float(intensity);
					msg.max_y = py > msg.max_y ? py : msg.max_y;
					msg.min_y = py < msg.min_y ? py : msg.min_y;
				}
				msg.min_height[pid] = -1.25;
				msg.max_height[pid] = 0;
			}
		}
	}
	// delete distanceArr;
	// delete imagePointArr;
	END_TIMER;
	if (iter != 0)
		total_time += TIMER;
	return msg;
}



void points2image::run(int p) {
	#pragma omp parallel
	#pragma omp single
	{
		int iter = 0;
		pause_func();
		while (read_testcases < testcases)
		{
			int count = read_next_testcases(p);
			unpause_func();
			for (int i = 0; i < count; i++)
			{
				// actual kernel invocation
				results[i] = pointcloud2_to_image(pointcloud2[i],
				cameraExtrinsicMat[i],
				cameraMat[i], distCoeff[i],
				imageSize[i], iter++);
			}
			pause_func();
			check_next_outputs(count);
		}
		printf("total time: %f\n", total_time);
	}
}

void points2image::check_next_outputs(int count)
{
	PointsImage reference;

	for (int i = 0; i < count; i++)
	{
		parsePointsImage(output_file, &reference);
		if ((results[i].image_height != reference.image_height)
			|| (results[i].image_width != reference.image_width))
		{
			error_so_far = true;
		}
		if ((results[i].min_y != reference.min_y)
			|| (results[i].max_y != reference.max_y))
		{
			error_so_far = true;
		}

		int pos = 0;
		for (int h = 0; h < reference.image_height; h++)
			for (int w = 0; w < reference.image_width; w++)
			{
				if (fabs(reference.intensity[pos] - results[i].intensity[pos]) > max_delta)
					max_delta = fabs(reference.intensity[pos] - results[i].intensity[pos]);
				if (fabs(reference.distance[pos] - results[i].distance[pos]) > max_delta)
					max_delta = fabs(reference.distance[pos] - results[i].distance[pos]);
				if (fabs(reference.min_height[pos] - results[i].min_height[pos]) > max_delta)
					max_delta = fabs(reference.min_height[pos] - results[i].min_height[pos]);
				if (fabs(reference.max_height[pos] - results[i].max_height[pos]) > max_delta)
					max_delta = fabs(reference.max_height[pos] - results[i].max_height[pos]);
				pos++;
			}
		// complement to read_next_testcases()
		delete [] reference.intensity;
		delete [] reference.distance;
		delete [] reference.min_height;
		delete [] reference.max_height;
	}
}

bool points2image::check_output() {
	std::cout << "checking output \n";
	// complement to init()
	input_file.close();
	output_file.close();

	std::cout << "max delta: " << max_delta << "\n";
	if ((max_delta > MAX_EPS) || error_so_far)
			return false;
	return true;
}

points2image a = points2image();
kernel& myKernel = a;
