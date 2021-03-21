/*
Copyright (c) 2021 Oscar Riveros. all rights reserved. oscar.riveros@peqnp.science

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <sstream>

struct cell {
    int id;
    int dem;
    double x, y;
};

int cap;

struct cell depot;
std::vector<std::vector<struct cell>> clusters;

void inv(int i, int j, std::vector<struct cell> &cells) {
    while (i < j) {
        std::swap(cells[i], cells[j]);
        i++;
        j--;
    }
}

double delta(std::vector<struct cell> &cells) {
    auto loc = 0.0;
    for (auto k = 0; k < cells.size(); k++) {
        loc += std::sqrt(std::pow(cells[k].x - cells[(k + 1) % cells.size()].x, 2) + std::pow(cells[k].y - cells[(k + 1) % cells.size()].y, 2));
    }
    return loc;
}

double h_algorithm(std::vector<struct cell> &cells) {
    cells.emplace_back(depot);
    auto top = delta(cells);
    for (auto k{0}; k < cells.size(); k++) {
        auto glb = std::numeric_limits<double>::max();
        o:
        for (auto i{0}; i < cells.size() - 1; i++) {
            for (auto j{i + 1}; j < cells.size(); j++) {
                oo:
                inv(i, j, cells);
                auto loc = delta(cells);
                if (loc < glb) {
                    glb = loc;
                    if (glb < top) {
                        top = glb;
                        goto o;
                    }
                    goto oo;
                } else if (loc > glb) {
                    goto oo;
                }
            }
        }
    }
    return top;
}

double oracle(std::vector<struct cell> &cells, cell &depot) {
    auto lod = 0;
    auto loc = 0.0;
    clusters.clear();
    std::vector<struct cell> cluster;
    for (auto k{0}; k < cells.size(); k++) {
        lod += cells[k].dem;
        cluster.emplace_back(cells[k]);
        if (lod > cap) {
            lod = cells[k].dem;
            cluster.pop_back();
            loc += h_algorithm(cluster);
            clusters.emplace_back(cluster);
            cluster.clear();
            cluster.emplace_back(cells[k]);
        }
    }
    if (!cluster.empty()) {
        loc += h_algorithm(cluster);
        clusters.emplace_back(cluster);
    }
    return loc;
}

double hess(std::vector<struct cell> &cells) {
    std::cout.precision(std::numeric_limits<double>::max_digits10 + 1);
    depot = cells.front();
    cells.erase(cells.begin());
    auto top = oracle(cells, depot);
    for (auto k{0}; k < cells.size(); k++) {
        auto glb = std::numeric_limits<double>::max();
        o:
        for (auto i{0}; i < cells.size() - 1; i++) {
            for (auto j{i + 1}; j < cells.size(); j++) {
                oo:
                inv(i, j, cells);
                auto loc = oracle(cells, depot);
                if (loc < glb) {
                    glb = loc;
                    if (glb < top) {
                        top = glb;
                        std::cout << top << std::endl;
                        goto o;
                    }
                    goto oo;
                } else if (loc > glb) {
                    goto oo;
                }
            }
        }
    }
    cells.emplace_back(depot);
    std::rotate(cells.rbegin(), cells.rbegin() + 1, cells.rend());
    return top;
}

int main(int, char *argv[]) {

    int n;
    std::string dummy;
    std::vector<cell> cells;
    std::ifstream ifs(argv[1]);

    std::string line;
    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        if (line.substr(0, 4) == "NAME") {
            // std::cout << line << std::endl;
        }
        if (line.substr(0, 7) == "COMMENT") {
            // std::cout << line << std::endl;
        }
        if (line.substr(0, 4) == "TYPE") {
            // std::cout << line << std::endl;
        }
        if (line.substr(0, 9) == "DIMENSION") {
            // std::cout << line << std::endl;
            std::stringstream ss(line);
            ss >> dummy >> dummy >> n;
        }
        if (line.substr(0, 16) == "EDGE_WEIGHT_TYPE") {
            // std::cout << line << std::endl;
        }
        if (line.substr(0, 8) == "CAPACITY") {
            // std::cout << line << std::endl;
            std::stringstream ss(line);
            ss >> dummy >> dummy >> cap;
        }
        if (line.substr(0, 18) == "NODE_COORD_SECTION") {
            for (auto i{0}; i < n; i++) {
                struct cell cell;
                std::getline(ifs, line);
                std::stringstream ss(line);
                ss >> cell.id >> cell.x  >> cell.y;
                cells.emplace_back(cell);
            }
        }
        if (line.substr(0, 14) == "DEMAND_SECTION") {
            for (auto i{0}; i < n; i++) {
                struct cell cell;
                std::getline(ifs, line);
                std::stringstream ss(line);
                ss >> cell.id >> cells[i].dem;
            }
        }
    }
    ifs.close();

    auto glb = std::numeric_limits<double>::max();
    //for (;;) {
        auto loc = hess(cells);

        std::ofstream data_file("data.py");
        data_file << "data = [";

        int route = 0;
        for (auto &cluster : clusters) {
            std::cout << "Route #" << ++route << ": ";
            data_file << "[";
            for (auto &node : cluster) {
                //if (node.id == 1) {
                //    continue;
                //}
                data_file << "(" << node.id << ", " << node.x << ", " << node.y << ", " << node.dem << "), ";
                std::cout << node.id << " ";
            }
            data_file << "], ";
            std::cout << std::endl;
        }
        data_file << "]" << std::endl;
        data_file.close();
        if (loc < glb) {
            glb = loc;
            std::cout << "Cost " << glb << std::endl;
        }
    //}

    return EXIT_SUCCESS;
}