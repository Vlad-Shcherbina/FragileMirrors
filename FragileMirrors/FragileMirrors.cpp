#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <iterator>
#include <string>
#include <ctime>
#include <map>
#include <utility>
#include <set>
#include <algorithm>
#include <fstream>

struct CellInfo;
typedef CellInfo *Point;

#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
using __gnu_cxx::hash_map;
namespace __gnu_cxx
{
    template<> struct hash<Point> {
        size_t operator()(Point p) const {
            return reinterpret_cast<size_t>(p);
        }
    };
}
#endif

using namespace std;

struct CellInfo {
    bool is_right;
    bool broken;
    Point up, down, left, right;
    int x() const;
    int y() const;
    bool is_outside() const;
};

const int STRIDE = 128;

CellInfo board[STRIDE*STRIDE];
int n;

inline int CellInfo::x() const {
    int offset = this - board;
    return offset % STRIDE - 1;
};

inline int CellInfo::y() const {
    int offset = this - board;
    return offset / STRIDE - 1;
};

inline bool CellInfo::is_outside() const {
    int offset = this - board;
    int x = offset % STRIDE;
    if (x == 0 || x == n+1)
        return true;
    int y = offset / STRIDE;
    if (y == 0 || y == n+1)
        return true;
    return false;
}

inline Point from_coords(int x, int y) {
    return &board[(y+1)*STRIDE+x+1];
}

void init_board() {
    for (int i = 0; i < STRIDE*STRIDE; i++) {
        Point p = &board[i];
        p->broken = false;
        p->up = p - STRIDE;
        p->down = p + STRIDE;
        p->left = p - 1;
        p->right = p + 1;
    }
}

void random_board() {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            Point p = from_coords(j, i);
            assert(!p->is_outside());
            p->is_right = rand()&1;
            p->broken = false;
        }
}

void print_board(ostream &out) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            Point p = from_coords(j, i);
            assert(!p->is_outside());
            if (p->broken)
                out << "  ";
            else if (p->is_right)
                out << "\\ ";
            else out << "/ ";
        }
        out << endl;
    }
}

ostream& operator<<(ostream &out, Point p) {
    if (p)
        out << '(' << p->x() << ", " << p->y() << ')';
    else
        out << "NullPoint";
    return out;
}

template<typename T>
ostream& operator<<(ostream &out, vector<T> v) {
    out << '[';
    for (int i = 0; i < v.size(); i++) {
        if (i) out << ", ";
        out << v[i];
    }
    out << ']';
    return out;
}

/*template<typename K, typename V>
ostream& operator<<(ostream &out, const map<K, V> &m) {
    out << '{';
    for (map<K, V>::const_iterator i = m.begin(); i != m.end(); ++i) {
        if (i != m.begin()) out << ", ";
        out << i->first << ": " << i->second;
    }
    out << '}';
    return out;
}*/

template<typename OutputIterator>
OutputIterator trace_path(Point enter, OutputIterator output_iterator) {
    Point p = enter;
    assert(
        p->x() >= 0 && p->x() < n ||
        p->y() >= 0 && p->y() < n);
    if (p->x() == -1) goto right;
    if (p->x() == n) goto left;
    if (p->y() == -1) goto down;
    if (p->y() == n) goto up;
    assert(false);

right:
    do { p = p->right; } while (p->broken);
    if (p->x() >= n) return output_iterator;
    p->broken = true;
    *output_iterator++ = p;
    if (p->is_right) goto down;
    goto up;
left:
    do { p = p->left; } while (p->broken);
    if (p->x() < 0) return output_iterator;
    p->broken = true;
    *output_iterator++ = p;
    if (p->is_right) goto up;
    goto down;
down:
    do { p = p->down; } while (p->broken);
    if (p->y() >= n) return output_iterator;
    p->broken = true;
    *output_iterator++ = p;
    if (p->is_right) goto right;
    goto left;
up:
    do { p = p->up; } while (p->broken);
    if (p->y() < 0) return output_iterator;
    p->broken = true;
    *output_iterator++ = p;
    if (p->is_right) goto left;
    goto right;
}

template<typename Iterator>
void undo_path(Iterator begin, Iterator end) {
    for (Iterator i = begin; i != end; ++i) {
        assert((*i)->broken);
        (*i)->broken = false;
    }
}

void undo_path(const vector<Point> &path) {
    undo_path(path.begin(), path.end());
}

void parse_board(vector<string> rows) {
    n = rows.size();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            Point p = from_coords(j, i);
            p->broken = false;
            assert(rows[i][j] == 'L' || rows[i][j] == 'R');
            p->is_right = (rows[i][j] == 'R');
        }
}

// Notice unbalanced bracket!
#define FOREACH_POINT_IN_ROW(y, pt) \
    for (Point pt = from_coords(-1, y)->right; pt->x() < n; pt = pt->right) { \
        if (pt->broken) continue;

int mirror_count() {
    int result = 0;
    for (int y = 0; y < n; y++)
        FOREACH_POINT_IN_ROW(y, pt)
            result++;
        }
    return result;
}

vector<Point> all_enters() {
    vector<Point> result;
    for (int i = 0; i < n; i++) {
        result.push_back(from_coords(-1, i));
        result.push_back(from_coords(n, i));
        result.push_back(from_coords(i, -1));
        result.push_back(from_coords(i, n));
    }
    return result;
}

vector<Point> greedy() {
    vector<Point> enters = all_enters();

    int best_len = -1;
    Point best_enter;

    for (int i = 0; i < enters.size(); i++) {
        Point enter = enters[i];
        vector<Point> path;
        trace_path(enter, back_inserter(path));

        int len = path.size();
        if (len > best_len) {
            best_len = len;
            best_enter = enter;
        }

        undo_path(path);
    }
    assert(best_len > 0);
    return vector<Point>(1, best_enter);
}

vector<Point> greedy_depth_two() {
    vector<Point> enters = all_enters();

    float best_gain = -1;
    vector<Point> best_solution;

    typedef hash_map<Point, vector<Point> > PtoPs;
    PtoPs passes;
    PtoPs interacts;

    for (int i = 0; i < enters.size(); i++) {
        Point enter = enters[i];
        vector<Point> path;
        trace_path(enter, back_inserter(path));
        for (int j = 0; j < path.size(); j++) {
            vector<Point> &ps = passes[path[j]];
            for (int k = 0; k < ps.size(); k++) {
                interacts[enter].push_back(ps[k]);
                interacts[ps[k]].push_back(enter);
            }
            ps.push_back(enter);
        }
        undo_path(path);

        float gain = path.size();
        if (gain > best_gain) {
            best_gain = gain;
            best_solution.clear();
            best_solution.push_back(enter);
        }
    }

    float fill = 1.0 * mirror_count() / (n*n);

    for (PtoPs::iterator i = interacts.begin(); i != interacts.end(); ++i) {
        Point e1 = i->first;

        vector<Point> &es = i->second;

        vector<Point> path1;
        trace_path(e1, back_inserter(path1));
        for (int j = 0; j < es.size(); j++) {
            Point e2 = es[j];
            vector<Point> path2;
            trace_path(e2, back_inserter(path2));
            float gain = 0.5 * (path1.size() + path2.size());
            if (gain > best_gain) {
                best_gain = gain;
                best_solution.clear();
                best_solution.push_back(e1);
                best_solution.push_back(e2);
            }

            if (fill > 0.1 && gain < 0.5 * n) {
                undo_path(path2);
                continue;
            }

            set<Point> e3s;
            for (int k = 0; k < path2.size(); k++) {
                if (passes.count(path2[k]) == 0)
                    continue;
                vector<Point> &ps = passes[path2[k]];
                for (int l = 0; l < ps.size(); l++)
                    e3s.insert(ps[l]);
            }
            for (set<Point>::iterator k = e3s.begin(); k != e3s.end(); k++) {
                Point e3 = *k;
                Point path3[10000];
                Point *path3_end = path3;
                path3_end = trace_path(e3, path3_end);
                float gain = 0.333333 * (path1.size() + path2.size() + (path3_end-path3));
                if (gain > best_gain) {
                    best_gain = gain;
                    best_solution.clear();
                    best_solution.push_back(e1);
                    best_solution.push_back(e2);
                    best_solution.push_back(e3);
                }
                undo_path(path3, path3_end);
            }

            undo_path(path2);
        }
        undo_path(path1);
    }

    cerr << "best solution: " << best_gain << " " << best_solution << endl;
    assert(best_gain > 0);

    return best_solution;
}


void do_step(Point enter) {
    vector<Point> path;
    trace_path(enter, back_inserter(path));
    for (int i = 0; i < path.size(); i++) {
        Point p = path[i];
        p->up->down = p->down;
        p->down->up = p->up;
        p->left->right = p->right;
        p->right->left = p->left;
    }
}


class FragileMirrors {
public:
    vector<int> destroy(vector<string> rows) {
        clock_t start = clock();

        init_board();
        parse_board(rows);

        vector<int> solution;
        int step = 0;
        while (mirror_count() > 0) {
            cerr << "mirror count = " << mirror_count() << endl;
            vector<Point> es = greedy_depth_two();
            for (int i = 0; i < es.size(); i++) {
                Point e = es[i];
                do_step(es[i]);
                step++;
                solution.push_back(e->y());
                solution.push_back(e->x());
                cerr << e << endl;
            }
            cerr << "data point: (" << step << ", " << mirror_count() << ")" << endl;
        }

        cerr << "it took " << 1.0 * (clock() - start) / CLOCKS_PER_SEC << endl;

        return solution;
    }
};

#ifdef _WIN32
int main(int argc, char* argv[])
{
    //ifstream cin("..\\test_large.in");
    if (cin.fail()) {
        cout << "INPUT ERROR" << endl;
        return 1;
    }

    int n;
    vector<string> rows;
    cin >> n;
    for (int i = 0; i < n; i++) {
        string s;
        cin >> s;
        assert(s.size() == n);
        rows.push_back(s);
    }

    vector<int> solution = FragileMirrors().destroy(rows);

    cout << solution.size() << endl;
    for (int i = 0; i < solution.size(); i++)
        cout << solution[i] << endl;
    cout.flush();

    return 0;
}
#endif

