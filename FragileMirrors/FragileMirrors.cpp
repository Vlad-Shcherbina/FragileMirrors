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
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>

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
    float weight;
    int x() const;
    int y() const;
    bool is_outside() const;
};

const int STRIDE = 128;

CellInfo board[STRIDE*STRIDE];
int col_pop[100];
int row_pop[100];
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
        p->weight = 0;
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
    col_pop[p->x()]--; row_pop[p->y()]--;
    *output_iterator++ = p;
    if (p->is_right) goto down;
    goto up;
left:
    do { p = p->left; } while (p->broken);
    if (p->x() < 0) return output_iterator;
    p->broken = true;
    col_pop[p->x()]--; row_pop[p->y()]--;
    *output_iterator++ = p;
    if (p->is_right) goto up;
    goto down;
down:
    do { p = p->down; } while (p->broken);
    if (p->y() >= n) return output_iterator;
    p->broken = true;
    col_pop[p->x()]--; row_pop[p->y()]--;
    *output_iterator++ = p;
    if (p->is_right) goto right;
    goto left;
up:
    do { p = p->up; } while (p->broken);
    if (p->y() < 0) return output_iterator;
    p->broken = true;
    col_pop[p->x()]--; row_pop[p->y()]--;
    *output_iterator++ = p;
    if (p->is_right) goto left;
    goto right;
}

template<typename Iterator>
void undo_path(Iterator begin, Iterator end) {
    for (Iterator i = begin; i != end; ++i) {
        assert((*i)->broken);
        (*i)->broken = false;
        col_pop[(*i)->x()]++; row_pop[(*i)->y()]++;
    }
}

void undo_path(const vector<Point> &path) {
    undo_path(path.begin(), path.end());
}

void parse_board(vector<string> rows) {
    n = rows.size();
    for (int i = 0; i < n; i++) {
        col_pop[i] = row_pop[i] = n;
        for (int j = 0; j < n; j++) {
            Point p = from_coords(j, i);
            p->broken = false;
            assert(rows[i][j] == 'L' || rows[i][j] == 'R');
            p->is_right = (rows[i][j] == 'R');
            p->weight = 1.0;
        }
    }
}

// Notice unbalanced bracket!
#define FOREACH_POINT_IN_ROW(y, pt) \
    for (Point pt = from_coords(-1, y)->right; pt->x() < n; pt = pt->right) { \
        if (pt->broken) continue;
#define FOREACH_POINT_IN_COLUMN(x, pt) \
    for (Point pt = from_coords(x, -1)->down; pt->y() < n; pt = pt->down) { \
        if (pt->broken) continue;


struct Subset {
    vector<int> ys;
    static Subset full() {
        Subset result;
        for (int i = 0; i < n; i++)
            result.ys.push_back(i);
        return result;
    }
    Subset clip() const {
        Subset result;
        for (int i = 0; i < ys.size(); i++) {
            FOREACH_POINT_IN_ROW(ys[i], pt)
                result.ys.push_back(ys[i]);
                break;
            }
        }
        return result;
    }
    int mirror_count() const {
        int result = 0;
        for (int i = 0; i < ys.size(); i++)
            result += row_pop[ys[i]];
        return result;
    }
    vector<int> get_xs() const {
        set<int> xs;
        for (int i = 0; i < ys.size(); i++) {
            FOREACH_POINT_IN_ROW(ys[i], pt)
                xs.insert(pt->x());
            }
            if (xs.size() == n) break;
        }
        return vector<int>(xs.begin(), xs.end());
    }
    vector<Point> all_enters() const {
        vector<Point> result;
        for (int i = 0; i < ys.size(); i++) {
            result.push_back(from_coords(-1, ys[i]));
            result.push_back(from_coords(n, ys[i]));
        }
        vector<int> xs = get_xs();
        for (int i = 0; i < xs.size(); ++i) {
            result.push_back(from_coords(xs[i], -1));
            result.push_back(from_coords(xs[i], n));
        }
        return result;
    }
    vector<Subset> connected_components() const {
        int component_number = 0;

        int visited_rows[100] = {0};
        int visited_columns[100] = {0};
        for (int i = 0; i < ys.size(); i++) {
            int y0 = ys[i];
            if (visited_rows[y0]) continue;
            component_number++;
            vector<int> tasks;
            tasks.push_back(y0);
            visited_rows[y0] = component_number;
            while (!tasks.empty()) {
                int task = tasks.back();
                tasks.pop_back();
                if (task < 1000) {
                    FOREACH_POINT_IN_ROW(task, pt)
                        if (visited_columns[pt->x()]) continue;
                        visited_columns[pt->x()] = component_number;
                        tasks.push_back(pt->x() + 1000);
                    }
                }
                else {
                    FOREACH_POINT_IN_COLUMN(task - 1000, pt)
                        if (visited_rows[pt->y()]) continue;
                        visited_rows[pt->y()] = component_number;
                        tasks.push_back(pt->y());
                    }
                }
            }
        }

        vector<Subset> components(component_number);
        for (int i = 0; i < ys.size(); i++) {
            int y = ys[i];
            components[visited_rows[y]-1].ys.push_back(y);
        }
        return components;
    }

    struct SizeComparer {
        bool operator()(const Subset &s1, const Subset &s2) {
            return s1.mirror_count() < s2.mirror_count();
        }
    };

    string compute_features() const {
        ostringstream out;
        out << "dict(";
        out << "n=" << n << ", ";
        out << "mc=" << mirror_count() << ", ";
        out << "rows=" << ys.size() << ", ";
        out << "cols=" << get_xs().size() << ", ";
        out << ")";
        return out.str();
    }
};

ostream& operator<<(ostream &out, const Subset &s) {
    vector<int> xs = s.get_xs();
    out << "  ";
    for (int i = 0; i < xs.size(); ++i)
        out << setw(2) << xs[i];
    out << endl;
    for (int i = 0; i < s.ys.size(); i++) {
        out << setw(2) << s.ys[i];
        for (int j = 0; j < xs.size(); ++j) {
            Point pt = from_coords(xs[j], s.ys[i]);
            if (pt->broken)
                out << "  ";
            else if (pt->is_right)
                out << " \\";
            else
                out << " /";
        }
        out << endl;
    }
    return out;
}

int precise_cost(const Subset &s) {
    if (s.ys.size() == 1 || s.get_xs().size() == 1)
        return (s.mirror_count() + 1) / 2;
    return 0;
}

struct Evaluator {
    virtual float operator()(const Subset &s) const = 0;
    virtual ~Evaluator() {}
};

struct SparseEvaluator : Evaluator {
    virtual float operator()(const Subset &s) const {
        int t = precise_cost(s);
        if (t)
            return t;

        int mc = s.mirror_count();
        return (mc+1)/2-0.1;
        /*int rows = s.ys.size();
        int cols = s.get_xs().size();

        return 0.01*mc + 0.5*(rows+cols);*/
    }
};

struct ComponentwiseEvaluator : Evaluator {
    const Evaluator &e;
    ComponentwiseEvaluator(const Evaluator &component_evaluator) : e(component_evaluator) {}
    virtual float operator()(const Subset &s) const {
        vector<Subset> components = s.connected_components();
        float result = 0;
        for (int i = 0; i < components.size(); i++) {
            result += e(components[i]);
        }
        return result;
    }
};

struct NaiveDenseEvaluator : Evaluator {
    virtual float operator()(const Subset &s) const {
        return s.mirror_count();
    }
};

struct WeightedDenseEvaluator : Evaluator {
    virtual float operator()(const Subset &s) const {
        float result = 0;
        for (int y = 0; y < n; y++)
            FOREACH_POINT_IN_ROW(y, pt)
                result += 1000.0/sqrt(pt->weight+5);
            }
        return result;
    }
};

vector<Point> greedy(Subset subset, const Evaluator &evaluator) {
    vector<Point> enters = subset.all_enters();

    float best_cost = 1e10;
    Point best_enter;

    for (int i = 0; i < enters.size(); i++) {
        Point enter = enters[i];
        vector<Point> path;
        trace_path(enter, back_inserter(path));

        float cost = 1 + evaluator(subset.clip());
        if (cost < best_cost) {
            best_cost = cost;
            best_enter = enter;
        }

        undo_path(path);
    }
    assert(best_cost < 1e10);
    return vector<Point>(1, best_enter);
}

vector<Point> greedy_depth_two(Subset subset, const Evaluator &evaluator) {
    vector<Point> enters = subset.all_enters();

    float best_gain = -1e10;
    float orig_score = evaluator(subset);
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

        float gain = orig_score - evaluator(subset);
        if (gain > best_gain) {
            best_gain = gain;
            best_solution.clear();
            best_solution.push_back(enter);
        }

        undo_path(path);
    }

    float fill = 1.0 * Subset::full().mirror_count() / (n*n);

    for (PtoPs::iterator i = interacts.begin(); i != interacts.end(); ++i) {
        Point e1 = i->first;

        vector<Point> &es = i->second;

        vector<Point> path1;
        trace_path(e1, back_inserter(path1));
        for (int j = 0; j < es.size(); j++) {
            Point e2 = es[j];
            vector<Point> path2;
            trace_path(e2, back_inserter(path2));

            float gain = 0.5 * (orig_score - evaluator(subset));
            if (gain > best_gain) {
                best_gain = gain;
                best_solution.clear();
                best_solution.push_back(e1);
                best_solution.push_back(e2);
            }

            /*if (fill > 0.1 && gain < 0.5 * n) {
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
            }*/

            undo_path(path2);
        }
        undo_path(path1);
    }

    cerr << "best solution: " << best_gain << " " << best_solution << endl;
    assert(best_gain > 0);

    return best_solution;
}


void print_weight(ostream &out) {
    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            Point pt = from_coords(x, y);
            if (pt->broken)
                out << "------";
            else
                out << pt->weight;
            out << " ";
        }
        out << endl;
    }
}

#define zzz(dir) \
    p = pt->dir; \
    while (p->broken) p = p->dir; \
    s += p->weight;

float update_weights() {
    float error = 0.0;
    for (int y = 0; y < n; y++)
        FOREACH_POINT_IN_ROW(y, pt)
            float s = 0.0;
            Point p;
            zzz(up);
            zzz(down);
            zzz(left);
            zzz(right);
            float new_weight = 0.25*s + 1;
            error = max(error, abs(new_weight - pt->weight) / new_weight);
            pt->weight = new_weight;
        }
    return error;
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


vector<Point> perfect_solve(Subset subset) {
    int mc = subset.mirror_count();
    assert(mc > 0);
    vector<Point> enters = subset.all_enters();

    vector<Point> best_solution(mc + 1);

    for (int i = 0; i < enters.size(); i++) {
        if (mc <= 2 && best_solution.size() == 1)
            break;
        vector<Point> path;
        trace_path(enters[i], back_inserter(path));
        vector<Point> solution;
        solution.push_back(enters[i]);

        vector<Subset> components = subset.clip().connected_components();
        for (int j = 0; j < components.size(); j++) {
            vector<Point> sub = perfect_solve(components[j]);
            copy(sub.begin(), sub.end(), back_inserter(solution));
        }

        if (solution.size() < best_solution.size())
            best_solution = solution;

        undo_path(path);
    }
    assert(best_solution.size() <= mc);
    return best_solution;
}

class FragileMirrors {
    vector<Point> solution;

    void solve_for_subset(Subset subset) {
        int mc = subset.mirror_count();
        for (int i = 0; i < 10*n; i++) {
            float err = update_weights();
            if (err < 1e-2) {
                cerr << i << ":" << err << " ";
                break;
            }
        }
        cerr << endl;
        //print_weight(cerr);
        //cerr << subset;
        assert(mc > 0);
        cerr << "mirror count = " << mc << endl;

        string features = subset.compute_features();
        int precise_cost = ::precise_cost(subset);
        int start_steps = solution.size();

        vector<Point> es;
        /*if (mc <= 4) {
            cerr << subset;
            es = perfect_solve(subset);
            cerr << "perfect solution: " << es << endl;
        }
        else */
        if (mc <= n) {
            es = greedy_depth_two(subset, ComponentwiseEvaluator(SparseEvaluator()));
        }
        else {
            //es = greedy_depth_two(subset, NaiveDenseEvaluator());
            es = greedy_depth_two(subset, WeightedDenseEvaluator());
        }
        for (int i = 0; i < es.size(); i++) {
            Point e = es[i];
            do_step(es[i]);
            solution.push_back(e);
            cerr << e << endl;
        }

        cerr << "data point: (" << solution.size() << ", " << Subset::full().mirror_count() << ")" << endl;

        subset = subset.clip();

        vector<Subset> components = subset.connected_components();
        stable_sort(components.begin(), components.end(), Subset::SizeComparer());
        for (int i = 0; i < components.size(); i++)
            solve_for_subset(components[i]);

        if (precise_cost) {
            assert(solution.size() - start_steps == precise_cost);
            cerr << "precise: " << features << endl;
        } else {
            cerr << "subtask: (" << solution.size() - start_steps << ", " << features << ")" << endl;
        }
    }

public:
    vector<int> destroy(vector<string> rows) {
        clock_t start = clock();

        init_board();
        parse_board(rows);

        solution.clear();

        solve_for_subset(Subset::full());

        vector<int> result;
        for (int i = 0; i < solution.size(); i++) {
            result.push_back(solution[i]->y());
            result.push_back(solution[i]->x());
        }
        cerr << "it took " << 1.0 * (clock() - start) / CLOCKS_PER_SEC << endl;

        return result;
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

