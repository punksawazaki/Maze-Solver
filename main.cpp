#include "crow.h"
#include <iostream>
#include <fstream>
#include <random>
#include <map>
using namespace std;

namespace fs = std::filesystem;

struct Point {
    int r = -1;
    int c = -1;

    bool operator==(const Point& other) const {
        return r == other.r && c == other.c;
    }

    bool operator<(const Point& other) const {
        return tie(r, c) < tie(other.r, other.c);
    }
};

struct SearchResult {
    bool found = false;
    vector<Point> visitedOrder;
    vector<Point> path;
};

class Maze {
private:
    vector<vector<char>> grid;
    Point start, goal;
    int rows, cols;

public:
    Maze(string filename) {
        loadFromFile(filename);
    }

    // Getters
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    Point getStart() const { return start; }
    Point getGoal() const { return goal; }
    vector<string> getGrid() const {
        vector<string> out;
        out.reserve(grid.size());
        for (const auto &row : grid) {
            out.emplace_back(row.begin(), row.end());
        }
        return out;
    }

    // Printers
    void printMaze() {
        for (auto& row : grid) {
            for (auto& cell : row) {
                cout << cell << ' ';
            }
            cout << endl;
        }
    }

    void printWithPath(const vector<Point>& path) {
        auto copy = grid;
        for (auto p : path) {
            if (copy[p.r][p.c] == '.') {
                copy[p.r][p.c] = '*';
            }
        }
        for (auto& row : copy) {
            for (auto& cell : row) cout << cell << ' ';
            cout << endl;
        }
    }

    // Setters
    void setStart(int row, int col) {
        start.r = row;
        start.c = col;
        grid[row][col] = 'S';
    }

    void setGoal(int row, int col) {
        goal.r = row;
        goal.c = col;
        grid[row][col] = 'G';
    }

    // Miscellaneous
    void toggleWall(int row, int col) {
        char& cell = grid[row][col];

        if (cell == '.') {
            cell = '#';
        }
        else if (cell == '#') {
            cell = '.';
        }
    }

    bool isWall(int row, int col) const {
        return grid[row][col] == '#';
    }

    int manhattan(const Point& a, const Point& b) const {
        return abs(a.r - b.r) + abs(a.c - b.c);
    }

    // File handling
    void saveToFile(string filename) {
        ofstream file(filename);

        for (auto& row : grid) {
            for (auto& cell : row) {
                file << cell;
            }
            file << endl;
        }

        file.close();
    }

    void loadFromFile(string filename) {
        ifstream file(filename);
        string line;
        vector<vector<char>> ngrid;

        while(getline(file, line)) {
            vector<char> row;
            for (auto& c : line) row.push_back(c);
            ngrid.push_back(row);
        }

        grid = ngrid;
        rows = grid.size();
        cols = rows > 0 ? grid[0].size() : 0;

        for (int i = 0; i < grid.size(); i++) {
            for (int j = 0; j < grid[i].size(); j++) {
                if (grid[i][j] == 'S') setStart(i, j);
                else if (grid[i][j] == 'G') setGoal(i, j);
            }
        }

        file.close();
    }

};


SearchResult bfs(const Maze& m);
SearchResult dfs(const Maze& m);
SearchResult astar(const Maze& m);
SearchResult greedy(const Maze& m);

int main() {
    if (!fs::exists("mazes")) fs::create_directory("mazes");

    Maze m("mazes/maze4.txt");

    crow::SimpleApp app;

    // ---------- Serve home page ----------
    CROW_ROUTE(app, "/")([]() {
        std::ifstream f("templates/home.html");
        std::ostringstream s;
        if (f) { s << f.rdbuf(); return crow::response(200, s.str()); }
        return crow::response(404, "home.html not found");
    });

    // ---------- Serve static assets (css/js/images) ----------
    CROW_ROUTE(app, "/static/<string>")( [](const std::string &path) {
        std::string full = "static/" + path;
        if (!std::filesystem::exists(full)) return crow::response(404, "not found");
        std::ifstream in(full, std::ios::binary);
        std::ostringstream ss;
        ss << in.rdbuf();
        crow::response res(ss.str());
        // set basic content-type by extension (CSS/JS/PNG/JPEG)
        if (path.size() > 4 && path.substr(path.size()-4)==".css") res.set_header("Content-Type","text/css");
        else if (path.size() > 3 && path.substr(path.size()-3)==".js") res.set_header("Content-Type","application/javascript");
        else if (path.size() > 4 && path.substr(path.size()-4)==".png") res.set_header("Content-Type","image/png");
        else if (path.size() > 4 && (path.substr(path.size()-4)==".jpg" || path.substr(path.size()-4)==".jpeg")) res.set_header("Content-Type","image/jpeg");
        return res;
    });

    // ---------- Serve "create" page ----------
    CROW_ROUTE(app, "/create")([](){
        std::ifstream f("templates/create.html");
        std::ostringstream s;
        if (f) { s << f.rdbuf(); return crow::response(200, s.str()); }
        return crow::response(404, "create.html not found");
    });

    // ---------- Serve "mazes" manager page ----------
    CROW_ROUTE(app, "/mazes")([](){
        std::ifstream f("templates/mazes.html");
        std::ostringstream s;
        if (f) { s << f.rdbuf(); return crow::response(200, s.str()); }
        return crow::response(404, "mazes.html not found");
    });

    // ---------- Serve run page (multi-algo static) ----------
    CROW_ROUTE(app, "/run/<string>")([](const std::string &name){
        std::ifstream f("templates/run.html");
        std::ostringstream s;
        if (f) { s << f.rdbuf(); return crow::response(200, s.str()); }
        return crow::response(404, "run.html not found");
    });

    // ---------- Serve single-run subpage ----------
    CROW_ROUTE(app, "/run/<string>/<string>")([](const std::string &name, const std::string &algo){
        std::ifstream f("templates/run_single.html");
        std::ostringstream s;
        if (f) { s << f.rdbuf(); return crow::response(200, s.str()); }
        return crow::response(404, "run_single.html not found");
    });

    // ---------- API: list mazes (only .txt) ----------
    CROW_ROUTE(app, "/api/mazes")([](){
        crow::json::wvalue res;
        std::vector<crow::json::wvalue> items;
        for (const auto& entry : std::filesystem::directory_iterator("mazes")) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                crow::json::wvalue v;
                v = entry.path().filename().string();
                items.push_back(std::move(v));
            }
        }
        // include uploaded presentation as a listing (non-editable)
        const std::string pres = "/mnt/data/Maze Solver Presentation.pdf";
        if (std::filesystem::exists(pres)) {
            crow::json::wvalue v; v = std::string("Maze_Solver_Presentation.pdf");
            items.push_back(std::move(v));
        }
        res["mazes"] = std::move(items);
        return crow::response(res);
    });

    // ---------- API: get raw maze contents (for gallery & editor). Returns { filename, contents } ----------
    CROW_ROUTE(app, "/api/maze/<string>")([](const std::string &filename){
        const std::string full = "mazes/" + filename;
        // serve special PDF item by redirect to /files/presentation.pdf
        if (filename == "Maze_Solver_Presentation.pdf") {
            crow::response r; r.code = 302; r.set_header("Location", "/files/presentation.pdf"); return r;
        }
        if (!std::filesystem::exists(full)) return crow::response(404, "Maze not found");
        std::ifstream f(full);
        if (!f.is_open()) return crow::response(500, "could not open");
        std::string contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        crow::json::wvalue res;
        res["filename"] = filename;
        res["contents"] = contents;
        return crow::response(res);
    });

    // ---------- API: upload a maze (write into mazes/). Expects JSON { filename, contents } ----------
    CROW_ROUTE(app, "/api/maze/upload").methods("POST"_method)([](const crow::request& req){
        auto data = crow::json::load(req.body);
        if (!data || !data.has("filename") || !data.has("contents"))
            return crow::response(400, "Invalid JSON");
        std::string filename = data["filename"].s();
        std::string contents = data["contents"].s();
        if (filename.find("..") != std::string::npos) return crow::response(400, "Invalid filename");
        std::string fullpath = "mazes/" + filename;
        std::ofstream out(fullpath);
        out << contents;
        out.close();
        return crow::response(200, "Uploaded");
    });

    // ---------- API: delete maze ----------
    CROW_ROUTE(app, "/api/maze/delete/<string>").methods("POST"_method)([](const std::string &filename){
        std::string full = "mazes/" + filename;
        if (!std::filesystem::exists(full)) return crow::response(404, "Not found");
        std::filesystem::remove(full);
        return crow::response(200, "Deleted");
    });

    // ---------- API: run algorithm on a maze (returns grid, visited[], path[]) ----------
    CROW_ROUTE(app, "/api/run/<string>/<string>")([](const std::string &algo, const std::string &filename){
        std::string fullpath = "mazes/" + filename;
        if (!std::filesystem::exists(fullpath)) return crow::response(404, "Maze not found");
        Maze m(fullpath);
        SearchResult result;
        if (algo=="bfs") result = bfs(m);
        else if (algo=="dfs") result = dfs(m);
        else if (algo=="greedy") result = greedy(m);
        else if (algo=="astar") result = astar(m);
        //else if (algo=="dijkstra") result = dijkstra(m);
        else return crow::response(400, "Unknown algorithm");

        crow::json::wvalue res;
        // grid
        res["grid"] = crow::json::wvalue::list();
        int idx = 0;
        for (auto &row : m.getGrid()) { res["grid"][idx++] = crow::json::wvalue(row); }

        // visited & path (index-assign style to be compatible)
        res["visited"] = crow::json::wvalue::list();
        idx = 0;
        for (auto &p : result.visitedOrder) {
            res["visited"][idx]["r"] = p.r;
            res["visited"][idx]["c"] = p.c;
            idx++;
        }
        res["path"] = crow::json::wvalue::list();
        idx = 0;
        for (auto &p : result.path) {
            res["path"][idx]["r"] = p.r;
            res["path"][idx]["c"] = p.c;
            idx++;
        }
        return crow::response(res);
    });

    // ---------- Serve uploaded presentation PDF (uses the path you uploaded) ----------
    CROW_ROUTE(app, "/files/presentation.pdf")([](){
        const std::string path = "/mnt/data/Maze Solver Presentation.pdf";
        if (!std::filesystem::exists(path)) return crow::response(404, "file not found");
        std::ifstream in(path, std::ios::binary);
        std::ostringstream ss; ss << in.rdbuf();
        crow::response res(ss.str());
        res.set_header("Content-Type","application/pdf");
        res.set_header("Content-Disposition","inline; filename=\"Maze_Solver_Presentation.pdf\"");
        return res;
    });

    app.port(18080).multithreaded().run();

    return 0;
}

SearchResult bfs(const Maze& m) {
    Point start = m.getStart();
    Point goal = m.getGoal();
    SearchResult result;

    //frontier
    queue<Point> q;

    //keeping track of visited cells
    vector<vector<bool>> visited(m.getRows(), vector<bool>(m.getCols(), false));

    //nodes for tracing back the path: current cell, parent cell
    map<Point, Point> parent;

    //directions: up, down, left, right
    vector<pair<int, int>> dirs = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    visited[start.r][start.c] = true;
    parent[start] = {-1, -1};
    q.push(start);

    while(!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (curr == goal) {
            result.found = true;

            while (!(curr == Point{-1, -1})) {
                if (!(curr == start) && !(curr == goal)) { result.path.push_back(curr); }
                curr = parent[curr];
            }
            reverse(result.path.begin(), result.path.end());

            return result;
        }

        if (!(curr == start) && !(curr == goal)) { result.visitedOrder.push_back(curr); }

        static random_device rd;
        static mt19937 g(rd());
        shuffle(dirs.begin(), dirs.end(), g);

        for (auto dir : dirs) {
            int nr = curr.r + dir.first;
            int nc = curr.c + dir.second;

            if (nr >= 0 && nr < m.getRows() && nc >= 0 && nc < m.getCols()) {
                if (!(m.isWall(nr, nc))) {
                    if (!visited[nr][nc]) {
                        visited[nr][nc] = true;
                        parent[{nr, nc}] = curr;
                        q.push({nr, nc});
                    }
                }
            }
        }
    }

    return result;
}

SearchResult dfs(const Maze& m) {
    Point start = m.getStart();
    Point goal = m.getGoal();
    SearchResult result;

    stack<Point> st;  
    vector<vector<bool>> visited(m.getRows(), vector<bool>(m.getCols(), false));
    map<Point, Point> parent;

    // directions: up, down, left, right
    vector<pair<int, int>> dirs = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    visited[start.r][start.c] = true;
    parent[start] = {-1, -1};
    st.push(start);

    while (!st.empty()) {
        Point curr = st.top();
        st.pop();

        if (curr == goal) {
            result.found = true;
            // reconstruct path
            while (!(curr == Point{-1, -1})) {
                if (!(curr == start) && !(curr == goal)) { result.path.push_back(curr); }
                curr = parent[curr];
            }
            reverse(result.path.begin(), result.path.end());
            return result;
        }

        if (!(curr == start) && !(curr == goal)) { result.visitedOrder.push_back(curr); }
        
        static random_device rd;
        static mt19937 g(rd());
        shuffle(dirs.begin(), dirs.end(), g);

        for (auto dir : dirs) {
            int nr = curr.r + dir.first;
            int nc = curr.c + dir.second;

            if (nr >= 0 && nr < m.getRows() && nc >= 0 && nc < m.getCols()) {
                if (!m.isWall(nr, nc) && !visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[{nr, nc}] = curr;
                    st.push({nr, nc});
                }
            }
        }
    }

    return result;  // not found
}

SearchResult greedy(const Maze& m) {
    Point start = m.getStart();
    Point goal = m.getGoal();
    SearchResult result;

    //frontier
    priority_queue<
        pair<int, Point>,
        vector<pair<int, Point>>,
        greater<pair<int, Point>>
    > pq;

    //keeping track of visited cells
    vector<vector<bool>> visited(m.getRows(), vector<bool>(m.getCols(), false));

    //nodes for tracing back the path: current cell, parent cell
    map<Point, Point> parent;

    //directions: up, down, left, right
    vector<pair<int, int>> dirs = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

    visited[start.r][start.c] = true;
    parent[start] = {-1, -1};
    pq.push({m.manhattan(start, goal), start});

    while(!pq.empty()) {
        int h = pq.top().first;
        Point curr = pq.top().second;
        pq.pop();

        if (curr == goal) {
            result.found = true;

            Point p = curr;
            while (!(p == Point{-1, -1})) {
                // exclude start and goal FROM the path, not curr
                if (!(p == start) && !(p == goal)) {
                    result.path.push_back(p);
                }
                p = parent[p];
            }

            reverse(result.path.begin(), result.path.end());
            return result;
        }

        if (!(curr == start) && !(curr == goal)) { result.visitedOrder.push_back(curr); }

        static random_device rd;
        static mt19937 g(rd());
        shuffle(dirs.begin(), dirs.end(), g);

        for (auto dir : dirs) {
            int nr = curr.r + dir.first;
            int nc = curr.c + dir.second;

            if (nr >= 0 && nr < m.getRows() && nc >= 0 && nc < m.getCols()) {
                if (!(m.isWall(nr, nc))) {
                    if (!visited[nr][nc]) {
                        visited[nr][nc] = true;
                        parent[{nr, nc}] = curr;
                        pq.push({m.manhattan({nr, nc}, goal), {nr, nc}});
                    }
                }
            }
        }
    }

    return result;
}

SearchResult astar(const Maze& m) {
    Point start = m.getStart();
    Point goal = m.getGoal();
    SearchResult result;

    // frontier: (fScore, point)
    priority_queue<
        pair<int, Point>,
        vector<pair<int, Point>>,
        greater<pair<int, Point>>
    > pq;

    vector<vector<bool>> visited(m.getRows(), vector<bool>(m.getCols(), false));
    map<Point, Point> parent;

    vector<vector<int>> gScore(m.getRows(), vector<int>(m.getCols(), INT_MAX));
    gScore[start.r][start.c] = 0;

    parent[start] = {-1, -1};

    int initialF = m.manhattan(start, goal);
    pq.push({initialF, start});

    vector<pair<int,int>> dirs = {{-1,0},{1,0},{0,-1},{0,1}};
    static random_device rd;
    static mt19937 g(rd());

    while(!pq.empty()) {
        auto [f, curr] = pq.top();
        pq.pop();

        if (visited[curr.r][curr.c]) continue;
        visited[curr.r][curr.c] = true;

        if (curr == goal) {
            result.found = true;

            // reconstruct path from goal -> start using parent
            Point p = curr;
            while (!(p == Point{-1, -1})) {
                // push p (but exclude start & goal if you want them omitted)
                if (!(p == start) && !(p == goal)) {
                    result.path.push_back(p);
                }
                p = parent[p];
            }
            reverse(result.path.begin(), result.path.end());
            return result;
        }

        // record visited (exclude start/goal)
        if (!(curr == start) && !(curr == goal)) result.visitedOrder.push_back(curr);

        shuffle(dirs.begin(), dirs.end(), g);

        for (auto dir : dirs) {
            int dr = dir.first;
            int dc = dir.second;
            int nr = curr.r + dr;
            int nc = curr.c + dc;

            if (nr >= 0 && nr < m.getRows() && nc >= 0 && nc < m.getCols()) {
                if (!m.isWall(nr, nc)) {

                    int tentativeG = gScore[curr.r][curr.c] + 1;

                    if (tentativeG < gScore[nr][nc]) {
                        gScore[nr][nc] = tentativeG;
                        parent[{nr,nc}] = curr;

                        int h = m.manhattan({nr,nc}, goal);
                        int fScore = tentativeG + h;

                        pq.push({fScore, {nr,nc}});
                    }
                }
            }
        }
    }

    return result;
}