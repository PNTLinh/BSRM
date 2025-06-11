#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll route_time(const vector<int> &route, const vector<int> &d_list, const vector<vector<int>> &t_matrix) {
    if (route.empty()) return 0;
    ll total = t_matrix[0][route[0]] + d_list[route[0]-1];
    for (size_t i = 0; i+1 < route.size(); i++)
        total += t_matrix[route[i]][route[i+1]] + d_list[route[i+1]-1];
    total += t_matrix[route.back()][0];
    return total;
}

ll compute_makespan(const vector<vector<int>> &routes, const vector<int> &d_list, const vector<vector<int>> &t_matrix) {
    ll max_t = 0;
    for (const auto &route : routes) {
        max_t = max(max_t, route_time(route, d_list, t_matrix));
    }
    return max_t;
}

vector<vector<int>> create_initial_solution(int N, int K) {
    vector<vector<int>> routes(K);
    for (int i = 1; i <= N; i++) {
        routes[(i-1)%K].push_back(i);
    }
    return routes;
}

vector<vector<int>> neighbor(const vector<vector<int>> &routes, mt19937 &rng) {
    vector<vector<int>> new_routes = routes;
    int K = routes.size();
    vector<pair<int,int>> pos; 
    for (int i = 0; i < K; i++)
        for (int j = 0; j < (int)new_routes[i].size(); j++)
            pos.emplace_back(i, j);
    uniform_int_distribution<int> d(0, (int)pos.size()-1);
    int x = d(rng), y = d(rng);
    if (x == y) y = (y+1)%pos.size();
    auto [rx, ix] = pos[x];
    auto [ry, iy] = pos[y];
    swap(new_routes[rx][ix], new_routes[ry][iy]);
    return new_routes;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K; cin >> N >> K;
    vector<int> d_list(N);
    for (int i = 0; i < N; i++) cin >> d_list[i];
    vector<vector<int>> t_matrix(N+1, vector<int>(N+1));
    for (int i = 0; i <= N; i++)
        for (int j = 0; j <= N; j++)
            cin >> t_matrix[i][j];

    mt19937 rng(random_device{}());
    double T = 10000, Tmin = 1e-3, alpha = 0.995;
    int max_iter = 50000;

    auto curr_sol = create_initial_solution(N, K);
    ll curr_fit = compute_makespan(curr_sol, d_list, t_matrix);
    auto best_sol = curr_sol;
    ll best_fit = curr_fit;

    for (int iter = 0; iter < max_iter && T > Tmin; iter++) {
        auto next_sol = neighbor(curr_sol, rng);
        ll next_fit = compute_makespan(next_sol, d_list, t_matrix);
        if (next_fit < curr_fit || exp((curr_fit-next_fit)/T) > uniform_real_distribution<double>(0,1)(rng)) {
            curr_sol = next_sol;
            curr_fit = next_fit;
            if (curr_fit < best_fit) {
                best_fit = curr_fit;
                best_sol = curr_sol;
            }
        }
        T *= alpha;
    }

    cout << K << "\n";
    for (auto &route : best_sol) {
        vector<int> full_route; full_route.push_back(0);
        for (int x : route) full_route.push_back(x);
        full_route.push_back(0);
        cout << full_route.size() << "\n";
        for (size_t i = 0; i < full_route.size(); i++)
            cout << full_route[i] << (i+1<full_route.size() ? ' ':'\n');
    }
    return 0;
}
