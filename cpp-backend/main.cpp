#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/octree/octree_search.h>
#include <pcl/common/centroid.h>
#include <httplib.h>
#include <json.hpp>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <stdlib.h>

#define RESET "\033[0m"
#define BOLD "\033[1m"   /* Bold */
#define RED "\033[31m"   /* Red */
#define GREEN "\033[32m" /* Green */

const bool DEBUG = true;

struct Voxel
{
    pcl::PointXYZ center;
    bool occupied;
    unsigned char depth;
};

class Octree
{
private:
    pcl::PointXYZ center;
    double halfSize;
    unsigned char depth;
    Octree *children[8];
    pcl::PointXYZ *point;

public:
    Octree() {}

    Octree(pcl::PointXYZ _center, double _halfSize, int _depth) : center(_center), halfSize(_halfSize), depth(_depth)
    {
        for (int i = 0; i < 8; ++i)
            children[i] = NULL;
        point = NULL;
    }

    ~Octree()
    {
        for (int i = 0; i < 8; ++i)
            delete children[i];
    }

    int getOctantContainingPoint(const pcl::PointXYZ &point) const
    {
        int oct = 0;
        if (point.x >= this->center.x)
            oct |= 4;
        if (point.y >= this->center.y)
            oct |= 2;
        if (point.z >= this->center.z)
            oct |= 1;
        return oct;
    }

    int getHalfSize() const
    {
        return this->halfSize;
    }

    bool isLeaf() const
    {
        for (int i = 0; i < 8; i++)
        {
            if (this->children[i] != NULL)
            {
                return false;
            }
        }
        return true;
    }

    Octree *createChild(const int &octant)
    {
        double quarter = this->halfSize / 2.0f;
        pcl::PointXYZ childCenter;
        childCenter.x = this->center.x + (octant & 4 ? quarter : -quarter);
        childCenter.y = this->center.y + (octant & 2 ? quarter : -quarter);
        childCenter.z = this->center.z + (octant & 1 ? quarter : -quarter);
        return new Octree(childCenter, quarter, this->depth + 1);
    }

    void insert(pcl::PointXYZ *point, const double &resolution)
    {
        // Abort early if the stopping criterion is met
        if (this->halfSize < resolution)
        {
            if (this->point == NULL)
            {
                this->point = point;
            }
            return;
        }

        if (this->isLeaf())
        {
            // Leaf node; store the point if it's empty, subdivide if it's occupied
            if (this->point == NULL)
            {
                this->point = point;
                return;
            }

            pcl::PointXYZ *oldPoint = this->point;
            this->point = NULL;
            double quarter = this->halfSize / 2.0f;
            const int &oldOctant = this->getOctantContainingPoint(*oldPoint);
            const int &newOctant = this->getOctantContainingPoint(*point);

            for (int i = 0; i < 8; i++)
            {
                if ((i == oldOctant || i == newOctant) && this->children[i] == NULL)
                    this->children[i] = createChild(i);
            }

            this->children[oldOctant]->insert(oldPoint, resolution);
            this->children[newOctant]->insert(point, resolution);
        }
        else
        {
            // Internal node; insert the point into the appropriate child octant
            int octant = this->getOctantContainingPoint(*point);
            if (this->children[octant] == NULL)
            {
                this->children[octant] = createChild(octant);
            }
            this->children[octant]->insert(point, resolution);
        }
    }

    void addVoxel(const pcl::PointXYZ &center, const bool &occupied, const double &size, const int &depth, std::vector<Voxel> &voxels)
    {
        Voxel voxel;
        voxel.center = center;
        voxel.occupied = occupied;
        voxel.depth = depth;
        voxels.push_back(voxel);
    }

    void getVoxels(int &maxDepth, std::vector<Voxel> &voxels)
    {
        if (this->depth == maxDepth)
        {
            addVoxel(this->center, this->point != NULL || !this->isLeaf(), this->halfSize * 2, this->depth, voxels);
            return;
        }
        else
        {
            if (this->isLeaf())
            {
                addVoxel(this->center, this->point != NULL, this->halfSize * 2, this->depth, voxels);
            }
            else
            {
                for (int i = 0; i < 8; i++)
                {
                    if (this->children[i] != NULL)
                    {
                        this->children[i]->getVoxels(maxDepth, voxels);
                    }
                    else
                    {
                        // Create empty voxel
                        Octree *child = createChild(i);
                        addVoxel(child->center, false, child->halfSize * 2, child->depth, voxels);
                    }
                }
            }
        }
    }
};

std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::stringstream currentTime;
    currentTime << BOLD << "[" << std::put_time(std::localtime(&timer), "%Y-%m-%d %H:%M:%S") << "." << ms.count() << "]" << RESET;
    return currentTime.str();
}

void logMessage(const std::string &message)
{
    if (DEBUG)
    {
        const auto now = getCurrentTime();
        std::cout << now << GREEN << " INFO: " << message << RESET << std::endl;
    }
}

void logError(const std::string &message)
{
    if (DEBUG)
    {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = std::chrono::system_clock::to_time_t(now);
        std::cerr << std::put_time(std::localtime(&timer), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count() << " " << message << std::endl;
    }
}

std::string trimExtension(const std::string &filename)
{
    size_t lastDotPos = filename.find_last_of('.');
    if (lastDotPos != std::string::npos)
    {
        return filename.substr(0, lastDotPos);
    }
    return filename;
}

std::tuple<int, Octree *> buildTree(std::string pcdFilename, double resolution)
{
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
    if (pcl::io::loadPCDFile<pcl::PointXYZ>("pcd-files/" + pcdFilename, *cloud) == -1)
    {
        logError("Couldn't read file " + pcdFilename);
        return std::tuple<int, Octree *>(-1, {});
    }
    logMessage("Loaded " + std::to_string(cloud->size()) + " data points from " + pcdFilename + ". Centering...");

    // Center mean the points in the cloud
    double sumX = 0, sumY = 0, sumZ = 0;
    for (const auto &point : cloud->points)
    {
        sumX += point.x;
        sumY += point.y;
        sumZ += point.z;
    }
    double meanX = sumX / cloud->points.size();
    double meanY = sumY / cloud->points.size();
    double meanZ = sumZ / cloud->points.size();

    pcl::PointXYZ center;
    center.x = 0.0f;
    center.y = 0.0f;
    center.z = 0.0f;
    double halfSize = 0;
    for (auto &point : cloud->points)
    {
        point.x -= meanX;
        point.y -= meanY;
        point.z -= meanZ;

        if (std::abs(point.x) > halfSize)
            halfSize = std::abs(point.x);
        if (std::abs(point.y) > halfSize)
            halfSize = std::abs(point.y);
        if (std::abs(point.z) > halfSize)
            halfSize = std::abs(point.z);
    }
    halfSize = std::pow(2, std::ceil(std::log2(halfSize)));

    Octree *tree = new Octree(center, halfSize, 0);
    logMessage("Octree root created, adding " + std::to_string(cloud->size()) + " points with resolution " + std::to_string(resolution) + "...");

    for (int i = 0; i < cloud->points.size(); i++)
    {
        pcl::PointXYZ *point = new pcl::PointXYZ(cloud->points[i]);
        tree->insert(point, resolution);
    }

    return std::tuple<int, Octree *>(0, tree);
}

bool folderExists(const std::string &folderPath)
{
    return std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath);
}

nlohmann::json getVoxels(Octree *tree, int maxDepth)
{
    std::vector<Voxel> voxels;
    tree->getVoxels(maxDepth, voxels);
    nlohmann::json j;
    j["rootSize"] = tree->getHalfSize() * 2;
    j["treeDepth"] = maxDepth;

    nlohmann::json voxelArray = nlohmann::json::array();
    for (int i = 0; i < voxels.size(); i++)
    {
        nlohmann::json voxel;
        voxel["c"]["x"] = voxels[i].center.x;
        voxel["c"]["y"] = voxels[i].center.y;
        voxel["c"]["z"] = voxels[i].center.z;
        voxel["o"] = voxels[i].occupied;
        voxel["d"] = voxels[i].depth;
        voxelArray.push_back(voxel);
    }
    j["voxels"] = voxelArray;

    return j;
}

void setResponse(httplib::Response &res, const std::string &message, int statusCode = 200)
{
    nlohmann::json j;
    j["message"] = message;
    res.status = statusCode;
    res.set_content(j.dump(), "application/json");
}

int main(void)
{
    httplib::Server svr;

    Octree *currentTree;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
            { nlohmann::json j;
                j["data"] = "Hello World!";
                res.set_content(j.dump(), "application/json"); });

    svr.Get("/octree", [&currentTree](const httplib::Request &req, httplib::Response &res)
            {
        try {
        int height = 5;
        if (req.has_param("height")) {
            height = std::stoi(req.get_param_value("height"));
        }

        if (currentTree == nullptr) {
            setResponse(res, "No tree is currently loaded", 400);
            return;
        }

        logMessage("Getting voxels with depth " + std::to_string(height) + "...");
        auto start = std::chrono::high_resolution_clock::now();
        nlohmann::json j = getVoxels(currentTree, height);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        logMessage("Finished getting " + std::to_string(j["voxels"].size()) + " voxels! Took " + std::to_string(elapsed.count()) + " seconds");
        res.set_content(j.dump(), "application/json");

    } catch (std::invalid_argument& e) {
        setResponse(res, "Height must be an integer", 400);
    } catch (nlohmann::json::parse_error &e) {
        setResponse(res, "Invalid JSON", 400);
    } });

    svr.Post("/new-pcd", [&currentTree](const httplib::Request &req, httplib::Response &res)
             {
                 // Initial filtering
                if (!req.has_file("file"))
                {
                    setResponse(res, "No pcd file uploaded", 400);
                }
                if (!req.has_param("resolution"))
                {
                    setResponse(res, "No resolution was provided", 400);
                }

                auto file = req.get_file_value("file");
                std::string filename = file.filename;
                std::string content = file.content;
                // Get resolution and convert to centimeters
                double resolution = std::stof(req.get_param_value("resolution")) * 0.01;

                // Create folder if it doesn't exist
                logMessage("Creating pcd-files folder...");
                struct stat st = {0};
                if (stat("./pcd-files", &st) == -1)
                {
                    mkdir("./pcd-files", 0700);
                }

                // Save PCD file
                logMessage("Saving PCD file...");
                std::ofstream out("./pcd-files/" + filename, std::ios::binary);
                if (!out)
                {
                    res.status = 500;
                    res.set_content("Failed to save file", "text/plain");
                }
                out.write(content.c_str(), content.size());
                out.close();

                // Build octree
                logMessage("Creating octree...");
                auto start = std::chrono::high_resolution_clock::now();
                std::tuple<int, Octree *> result = buildTree(filename, resolution);
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                logMessage("Octree finished building! Took " + std::to_string(elapsed.count()) + " seconds");
                if (std::get<0>(result) != 0)
                {
                    logError("Failed to build tree");
                    setResponse(res, "Failed to build tree", 500);
                }
                currentTree = std::get<1>(result);

                // Get voxels
                logMessage("Getting voxels with depth 5...");
                start = std::chrono::high_resolution_clock::now();
                nlohmann::json j = getVoxels(currentTree, 5);
                end = std::chrono::high_resolution_clock::now();
                elapsed = end - start;
                logMessage("Finished getting " + std::to_string(j["voxels"].size()) + " voxels! Took " + std::to_string(elapsed.count()) + " seconds");
                res.set_content(j.dump(), "application/json"); });

    std::cout << "Server is running on http://localhost:3000" << std::endl;
    svr.listen("localhost", 3000);

    return 0;
}