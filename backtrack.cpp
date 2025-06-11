#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

int N, K;
vector<int> d;                // d[i]: thời gian bảo trì khách i (1-based)
vector<vector<int>> t;        // t[i][j]: thời gian di chuyển i->j (0..N)
int best_makespan = INT_MAX;
vector<vector<int>> best_assign;

int calc_route_time(const vector<int>& route) {
    if (route.empty()) return 0;
    int time = t[0][route[0]] + d[route[0] - 1];
    for (int i = 0; i + 1 < route.size(); i++)
        time += t[route[i]][route[i + 1]] + d[route[i + 1] - 1];
    time += t[route.back()][0];
    return time;
}

int estimate_lower_bound(const vector<vector<int>>& assign, int next_customer) {
    int max_cur = 0;
    vector<int> route_times;
    for (auto& r : assign) {
        int t_r = calc_route_time(r);
        route_times.push_back(t_r);
        max_cur = max(max_cur, t_r);
    }
    int remain = N - next_customer;
    if (remain == 0) return max_cur;
    sort(route_times.begin(), route_times.end());
    for (int i = 0; i < remain; ++i)
        route_times[i % K] += 1; 
    return *max_element(route_times.begin(), route_times.end());
}

void backtrack(int idx, vector<vector<int>>& assign) {
    if (idx == N) {
        int cur_makespan = 0;
        for (int k = 0; k < K; ++k)
            cur_makespan = max(cur_makespan, calc_route_time(assign[k]));
        if (cur_makespan < best_makespan) {
            best_makespan = cur_makespan;
            best_assign = assign;
        }
        return;
    }
    if (estimate_lower_bound(assign, idx) >= best_makespan) return;

    vector<pair<int, int>> order;
    for (int k = 0; k < K; ++k)
        order.push_back({calc_route_time(assign[k]), k});
    sort(order.begin(), order.end());

    for (int ord = 0; ord < K; ++ord) {
        int k = order[ord].second;
        int M = assign[k].size();
        assign[k].push_back(idx + 1);
        int cur_makespan = 0;
        for (int kk = 0; kk < K; ++kk)
            cur_makespan = max(cur_makespan, calc_route_time(assign[kk]));
        if (cur_makespan < best_makespan)
            backtrack(idx + 1, assign);
        assign[k].pop_back();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cin >> N >> K;
    d.resize(N);
    for (int i = 0; i < N; ++i) cin >> d[i];
    t.assign(N + 1, vector<int>(N + 1));
    for (int i = 0; i <= N; ++i)
        for (int j = 0; j <= N; ++j)
            cin >> t[i][j];
    vector<vector<int>> assign(K);
    backtrack(0, assign);
    cout << K << "\n";
    for (int k = 0; k < K; ++k) {
        vector<int> route = best_assign[k];
        cout << route.size() + 2 << "\n";
        cout << "0";
        for (int v : route) cout << " " << v;
        cout << " 0\n";
    }
    return 0;
}
