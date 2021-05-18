#include <chrono>
#include <k4a/k4a.hpp>
#include <knn.h>
#include <opencv2/core.hpp>
#include <searcher.h>
#include <thread>
#include <utility>

#include "calibration.h"
#include "cast.h"
#include "dbscan.h"
#include "intact.h"
#include "io.h"
#include "kinect.h"
#include "knn.h"
#include "logger.h"
#include "object.h"
#include "ply.h"
#include "region.h"
#include "searcher.h"
#include "timer.h"
#include "viewer.h"
#include "yolov5.h"

#define LOG_TRACE 0
#if LOG_TRACE == 1
#define log(string) LOG(INFO) << string
#else
#define log
#endif

int Intact::getNumPoints()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_numPoints;
}

void Intact::setDepthImgHeight(const int& height)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_depthHeight = height;
}

void Intact::setDepthImgWidth(const int& width)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_depthWidth = width;
}

int Intact::getDepthImgWidth()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_depthWidth;
}

int Intact::getDepthImgHeight()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_depthHeight;
}

void Intact::setSegmentBoundary(std::pair<Point, Point>& boundary)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_segmentBoundary = boundary;
}

std::pair<Point, Point> Intact::getSegmentBoundary()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_segmentBoundary;
}

void Intact::setTabletopBoundary(std::pair<Point, Point>& boundary)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    m_tabletopBoundary = boundary;
}

std::pair<Point, Point> Intact::getTabletopBoundary()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_tabletopBoundary;
}

void Intact::setSegPclBuf(
    int16_t* ptr_segPclBuf, int16_t* ptr_pclBuf, const int& pclSize)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    std::memcpy(ptr_segPclBuf, ptr_pclBuf, sizeof(int16_t) * pclSize);
    sptr_segPclBuf = std::make_shared<int16_t*>(ptr_segPclBuf);
}

std::shared_ptr<int16_t*> Intact::getSegPclBuf()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_segPclBuf;
}

void Intact::setSegImgBuf(
    uint8_t* ptr_segImgBuf, uint8_t* ptr_imgBuf, const int& imgSize)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    std::memcpy(ptr_segImgBuf, ptr_imgBuf, sizeof(uint8_t) * imgSize);
    sptr_segImgBuf = std::make_shared<uint8_t*>(ptr_segImgBuf);
}

std::shared_ptr<uint8_t*> Intact::getSegImgBuf()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_segImgBuf;
}

void Intact::setTtpImgBuf(
    uint8_t* ptr_ttpImgBuf, uint8_t* ptr_imgBuf, const int& imgSize)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    std::memcpy(ptr_ttpImgBuf, ptr_imgBuf, sizeof(uint8_t) * imgSize);
    sptr_ttpImgBuf = std::make_shared<uint8_t*>(ptr_ttpImgBuf);
}

std::shared_ptr<uint8_t*> Intact::getTtpImgBuf()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_ttpImgBuf;
}

void Intact::setRawPcl(const std::vector<float>& pcl)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_rawPcl = pcl;
}

std::shared_ptr<std::vector<float>> Intact::getRawPcl()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_rawPcl;
}

std::shared_ptr<std::vector<uint8_t>> Intact::getRawImg()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_rawImg;
}

void Intact::setRawImg(const std::vector<uint8_t>& img)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_rawImg = img;
}

void Intact::setRawPts(const std::vector<Point>& points) // todo what points?
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_rawPts = points;
}

std::shared_ptr<std::vector<float>> Intact::getSegPcl() // todo set?
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_segPcl;
}

std::shared_ptr<std::vector<uint8_t>> Intact::getSegImg() // todo set?
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_segImg;
}

void Intact::setSegFrame(cv::Mat& imgData) // todo check me please
{
    std::lock_guard<std::mutex> lck(m_mutex);
    sptr_segFrame = std::make_shared<cv::Mat>(imgData);
}

void Intact::setTtpFrame(cv::Mat& imgData)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    sptr_ttpFrame = std::make_shared<cv::Mat>(imgData);
}

std::shared_ptr<cv::Mat> Intact::getTtpFrame()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_ttpFrame;
}

std::shared_ptr<std::vector<Point>> Intact::getTtpPoints()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_ttpPts;
}

std::shared_ptr<std::vector<Point>> Intact::getSegPts()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_segPts;
}

void Intact::setSegPcl(const std::vector<float>& seg)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_segPcl = seg;
}

void Intact::setSegImg(const std::vector<uint8_t>& segment)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_segImg = segment;
}

void Intact::setSegPts(const std::vector<Point>& points)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_segPts = points;
}

void Intact::setCluPcl(const std::vector<float>& points)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_cluPcl = points;
}

std::shared_ptr<std::vector<float>> Intact::getCluPcl()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_cluPcl;
}

void Intact::setCluImg(const std::vector<uint8_t>& img)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_cluImg = img;
}

std::shared_ptr<std::vector<uint8_t>> Intact::getCluImg()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_cluImg;
}

void Intact::setCluPts(const std::vector<Point>& points)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_cluPts = points;
}

std::shared_ptr<std::vector<Point>> Intact::getCluPts()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_cluPts;
}

void Intact::setTtpPcl(const std::vector<float>& points)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_ttpPcl = points;
}

std::shared_ptr<std::vector<float>> Intact::getTtpPcl()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_ttpPcl;
}
void Intact::setTtpImg(const std::vector<uint8_t>& img)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_ttpImg = img;
}

std::shared_ptr<std::vector<uint8_t>> Intact::getTtpImg()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return sptr_ttpImg;
}

void Intact::setTtpPts(const std::vector<Point>& points)
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_ttpPts = points;
}

// thread-safe semaphores
//
void Intact::raiseSegmentedFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isContextSegmented = true;
}

void Intact::raiseChromakeyedFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isChromakeyed = true;
}

void Intact::raiseClusteredFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isContextClustered = true;
}

void Intact::raiseEpsilonFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isEpsilonComputed = true;
}

void Intact::raiseRunFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_run = true;
}

void Intact::raiseStopFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_stop = true;
}

bool Intact::isRun()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_run;
}

bool Intact::isStop()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_stop;
}

void Intact::raiseKinectReadyFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isKinectReady = true;
}

bool Intact::isChromakeyed()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isChromakeyed;
}

bool Intact::isKinectReady()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isKinectReady;
}

bool Intact::isSegmented()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isContextSegmented;
}

bool Intact::isClustered()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isContextClustered;
}

bool Intact::isEpsilonComputed()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isEpsilonComputed;
}

bool Intact::isCalibrated()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    return *sptr_isCalibrated;
}

void Intact::raiseCalibratedFlag()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_isCalibrated = true;
}

void Intact::stop()
{
    std::lock_guard<std::mutex> lck(m_mutex);
    *sptr_run = false;
}

#define SEGMENT 1
void Intact::segment(std::shared_ptr<Intact>& sptr_intact)
{
#if SEGMENT == 1
    bool init = true;
    while (!sptr_intact->isKinectReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    while (sptr_intact->isRun()) {
        /** cast point cloud to Point type definition for processing */
        std::vector<Point> points = cast::toPoint(*sptr_intact->getRawPcl(),
            *sptr_intact->getRawImg(), sptr_intact->getNumPoints());
        sptr_intact->setRawPts(points); // <- update raw points

        /** segment tabletop interaction context ~15ms */
        std::vector<Point> seg = region::segment(points);
        std::pair<Point, Point> boundary = region::queryBoundary(seg);
        // todo fix this !!
        setSegmentBoundary(boundary);
        sptr_intact->setSegPts(seg); // <- update segment points

        /** update flow control semaphores */
        if (init) {
            init = false;
            sptr_intact->raiseSegmentedFlag();
            log("-- context segmented"); /*NOLINT*/
        }
    }
#endif
}

void Intact::calibrate(std::shared_ptr<Intact>& sptr_intact)
{
    while (!sptr_intact->isKinectReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    cv::Mat distanceCoefficients;
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);

    // todo: move user specification to interface
    const float arucoSquareEdgeLength = 0.0565f;        // in meters
    const float calibrationSquareEdgeLength = 0.02500f; // in meters
    const std::string calibrationFile
        = "calibration.txt"; // external file for saving calibration

#define CALIBRATE 0
#if CALIBRATE == 1
    /** 1st calibrate the camera */
    calibration::startChessBoardCalibration(cameraMatrix, distanceCoefficients);
    // This operation will loop infinitely until calibration
    // images have been taken! take at least 20 images of the
    // chessboard. A criteria for good calibration images is variable
    // chessboard poses equally across all 6 degrees of freedom.
    //

    /** grace window for writing calibration file */
    std::this_thread::sleep_for(std::chrono::seconds(5));
#endif

#define FIND_ARUCO 0
#if FIND_ARUCO == 1
    /** load the calibration */
    calibration::importCalibration(
        "calibration.txt", cameraMatrix, distanceCoefficients);

    /** detect aruco markers */
    calibration::findArucoMarkers(cameraMatrix, distanceCoefficients);
#endif
    std::cout << "-- calibration done!!" << std::endl;
}

// Developer option:
// print knn results
//
#define PRINT_KNN 0
void printKnn(int& i, std::vector<std::pair<Point, float>>& nn)
{
#if PRINT_KNN == 1
    std::cout << "#" << i << ",\t"
              << "dist: " << std::sqrt(nn[i].second) << ",\t"
              << "point: (" << nn[i].first.m_xyz[0] << ", "
              << nn[i].first.m_xyz[1] << ", " << nn[i].first.m_xyz[2] << ")"
              << std::endl;
#endif
}

// Developer option:
// write knn results to file
//
#define WRITE_KNN 0
void writeKnn(std::vector<float>& knnQuery)
{
#if WRITE_KNN == 1
    std::sort(knnQuery.begin(), knnQuery.end(), std::greater<>());
    const std::string file = io::pwd() + "/knn.csv";
    std::cout << "writing the knn (k=4) of every point to: ";
    std::cout << file << std::endl;
    io::write(knnQuery, file);
#endif
}

#define COMPUTE_EPSILON 1
void Intact::estimateEpsilon(const int& K, std::shared_ptr<Intact>& sptr_intact)
{
#if COMPUTE_EPSILON
    /** wait for segmented context */
    while (!sptr_intact->isSegmented()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    log("-- evaluating k nearest neighbours"); /*NOLINT*/
    std::vector<Point> points = *sptr_intact->getSegPts();

    const int testVal = 3;
    // testVal used for arbitrary test for release, use
    // points.size()) (computationally expensive task)
    //

    /** evaluate k=4th nearest neighbours for every point */
    std::vector<float> knnQuery;
    for (int i = 0; i < testVal; i++) {
        int indexOfQueryPoint = i;
        std::vector<std::pair<Point, float>> nn
            = knn::compute(points, K, indexOfQueryPoint);
        knnQuery.push_back(std::sqrt(nn[i].second));
        printKnn(i, nn);
    }
    writeKnn(knnQuery);
    sptr_intact->raiseEpsilonFlag();
#endif
}

// Developer option:
// write segmented context to ply file
//
#define WRITE_PLY_FILE 0
#if WRITE_PLY_FILE == 1
#define WRITE_CLUSTERED_SEGMENT_TO_PLY_FILE(points) ply::write(points)
#else
#define WRITE_CLUSTERED_SEGMENT_TO_PLY_FILE(points)
#endif

#define CLUSTER 1
void Intact::cluster(
    const float& E, const int& N, std::shared_ptr<Intact>& sptr_intact)
{
#if CLUSTER
    {
        /** wait for epsilon value */
        while (!sptr_intact->isEpsilonComputed()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }

        bool init = true;
        /** n.b., clustering loop takes ~130ms per iteration */
        while (sptr_intact->isRun()) {

            /** cluster segmented context ~130ms/loop iteration */
            std::vector<std::vector<Point>> clusters
                = dbscan::cluster(*sptr_intact->getSegPts(), E, N);

            /** create object list using the density clusters */
            std::vector<Object> objects;
            for (auto& cluster : clusters) {
                Object object(cluster);
                objects.emplace_back(object);
            }

            /** cast region points to pcl format  for rendering */
            std::pair<std::vector<float>, std::vector<uint8_t>>
                spatialDensityClusters = cast::toClusteredPcl(
                    objects.back().m_points); // objects.back() = all clusters
            sptr_intact->setCluPcl(spatialDensityClusters.first);
            sptr_intact->setCluImg(spatialDensityClusters.second);
            sptr_intact->setCluPts(objects.back().m_points);

            /** cast object points to pcl format  for rendering */
            std::pair<std::vector<float>, std::vector<uint8_t>> object
                = cast::toClusteredPcl(
                    objects.front().m_points); // objects.front() = tabletop
            sptr_intact->setTtpPcl(object.first);
            sptr_intact->setTtpImg(object.second);
            sptr_intact->setTtpPts(objects.front().m_points);

            std::pair<Point, Point> boundary
                = region::queryBoundary(objects.front().m_points);
            sptr_intact->setTabletopBoundary(boundary);

            /** sequence cross thread semaphore */
            if (init) {
                init = false;
                WRITE_CLUSTERED_SEGMENT_TO_PLY_FILE(
                    *sptr_intact->getCluPts()); /*NOLINT*/
                sptr_intact->raiseClusteredFlag();
            }
        }
    }
#endif
}

#define RENDER 0
void Intact::render(std::shared_ptr<Intact>& sptr_intact)
{
#if RENDER == 1
    while (!sptr_intact->isKinectReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    viewer::draw(sptr_intact);
#endif
}

void chromaPixelData(const int& index, uint8_t* ptr_data)
{
    // 65,171,93 -> green
    ptr_data[4 * index + 0] = 93;  // blue
    ptr_data[4 * index + 1] = 171; // green
    ptr_data[4 * index + 2] = 65;  // red
    ptr_data[4 * index + 3] = 0;   // alpha
}

bool outsideBoundary(
    const int& index, const short* ptr_data, const Point& min, const Point& max)
{
    if (max.m_xyz[2] == __FLT_MAX__ || min.m_xyz[2] == __FLT_MIN__) {
        return true;
    }
    if ((float)ptr_data[3 * index + 0] > max.m_xyz[0]
        || (float)ptr_data[3 * index + 0] < min.m_xyz[0]
        || (float)ptr_data[3 * index + 1] > max.m_xyz[1]
        || (float)ptr_data[3 * index + 1] < min.m_xyz[1]
        || (float)ptr_data[3 * index + 2] > max.m_xyz[2]
        || (float)ptr_data[3 * index + 2] < min.m_xyz[2]) {
        return true;
    }
    return false;
}

#define CHROMAKEY 1
void Intact::chroma(std::shared_ptr<Intact>& sptr_intact)
{
    int imgSize = sptr_intact->getNumPoints() * 4; // r, g, b, a
    int pclSize = sptr_intact->getNumPoints() * 3; // x, y, z

    auto* ptr_imgData = (uint8_t*)malloc(sizeof(uint8_t) * imgSize);
    auto* ptr_TabletopImgData = (uint8_t*)malloc(sizeof(uint8_t) * imgSize);

    auto* ptr_pclData = (int16_t*)malloc(sizeof(int16_t*) * pclSize);
    // auto* ptr_TabletopPclData = (int16_t*)malloc(sizeof(int16_t*) * pclSize);

    bool init = true;
    while (!sptr_intact->isClustered()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    // todo: create helpers.hpp

#if CHROMAKEY == 1
    while (sptr_intact->isRun()) {
        std::memcpy(ptr_pclData, *sptr_intact->getSegPclBuf(),
            sizeof(int16_t) * pclSize);
        std::memcpy(ptr_imgData, *sptr_intact->getSegImgBuf(),
            sizeof(uint8_t) * imgSize);

        // todo:
        //   1. build pclVec
        //   2. build imgVec

        int numPoints = sptr_intact->getNumPoints();
        for (int i = 0; i < numPoints; i++) {
            if (ptr_imgData[4 * i + 0] == 0 && ptr_imgData[4 * i + 1] == 0
                && ptr_imgData[4 * i + 2] == 0 && ptr_imgData[4 * i + 3] == 0) {
                continue;
            }

            if (outsideBoundary(i, ptr_pclData,
                    sptr_intact->getTabletopBoundary().first,
                    sptr_intact->getTabletopBoundary().second)) {
                continue;
            }
            chromaPixelData(i, ptr_imgData);

            // float x = ptr_pclData[3 * i + 0];
            // float y = ptr_pclData[3 * i + 1];
            // float z = ptr_pclData[3 * i + 2];
            // Point queryPoint(x, y, z);
            // if (searcher::pointFound(*sptr_intact->getTabletopPoints(),
            // queryPoint)){
            //           chromaPixelData(i, ptr_imgData);
            // }
        }

        // sptr_intact->setTabletopPclData(
        //     ptr_TabletopPclData, ptr_pclData, pclSize);

        sptr_intact->setTtpImgBuf(ptr_TabletopImgData, ptr_imgData, imgSize);

        int height = sptr_intact->getDepthImgHeight();
        int width = sptr_intact->getDepthImgWidth();

        /** create image for segmented tabletop in cv::Mat format */
        cv::Mat frame = cv::Mat(height, width, CV_8UC4,
            (void*)*sptr_intact->getTtpImgBuf(), cv::Mat::AUTO_STEP)
                            .clone();

        sptr_intact->setTtpFrame(frame);

        /** update flow-control semaphore */
        if (init) {
            init = false;
            sptr_intact->raiseChromakeyedFlag();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
#endif
}

#define DETECT_OBJECTS 1
void Intact::detectObjects(std::vector<std::string>& classnames,
    torch::jit::script::Module& module, std::shared_ptr<Intact>& sptr_intact)
{
#if DETECT_OBJECTS == 1
    while (!sptr_intact->isChromakeyed()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    while (sptr_intact->isRun()) {
        clock_t start = clock();

        // cv::Mat frame = *sptr_intact->getSegmentedImgFrame();
        cv::Mat frame = *sptr_intact->getTtpFrame();

        cv::Mat img;
        /** prepare tensor input */
        cv::resize(frame, img, cv::Size(640, 384));
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        torch::Tensor imgTensor = torch::from_blob(
            img.data, { img.rows, img.cols, 3 }, torch::kByte);
        imgTensor = imgTensor.permute({ 2, 0, 1 });
        imgTensor = imgTensor.toType(torch::kFloat);
        imgTensor = imgTensor.div(255);
        imgTensor = imgTensor.unsqueeze(0);

        // preds: [?, 15120, 9]
        torch::Tensor preds
            = module.forward({ imgTensor }).toTuple()->elements()[0].toTensor();
        std::vector<torch::Tensor> dets
            = yolo::non_max_suppression(preds, 0.4, 0.5);

        if (!dets.empty()) {
            /** show result */
            for (size_t i = 0; i < dets[0].sizes()[0]; ++i) {
                float left
                    = dets[0][i][0].item().toFloat() * (float)frame.cols / 640;
                float top
                    = dets[0][i][1].item().toFloat() * (float)frame.rows / 384;
                float right
                    = dets[0][i][2].item().toFloat() * (float)frame.cols / 640;
                float bottom
                    = dets[0][i][3].item().toFloat() * (float)frame.rows / 384;
                float score = dets[0][i][4].item().toFloat();
                int classID = dets[0][i][5].item().toInt();

                cv::rectangle(frame,
                    cv::Rect(left, top, (right - left), (bottom - top)),
                    cv::Scalar(0, 255, 0), 2);

                cv::putText(frame,
                    classnames[classID] + ": " + cv::format("%.2f", score),
                    cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX,
                    (right - left) / 200, cv::Scalar(0, 255, 0), 2);
            }
        }

#define SHOW_DETECTED_OBJECTS 1
#if SHOW_DETECTED_OBJECTS == 1
        cv::putText(frame,
            "FPS: " + std::to_string(int(1e7 / (double)(clock() - start))),
            cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1,
            cv::Scalar(0, 255, 0), 2);
        cv::imshow("", frame);
#endif

        if (cv::waitKey(1) == 27) {
            sptr_intact->raiseStopFlag();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
#endif
}
