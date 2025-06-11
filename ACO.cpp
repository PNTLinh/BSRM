#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using vi = vector<int>;
using vvi = vector<vi>;
using vd = vector<double>;
using vvd = vector<vd>;
using namespace std::chrono;

const int    NUM_ANTS    = 20;       
const int    MAX_ITER    = 300;      
const double TAU_0       = 0.01;     
const double RHO         = 0.1;      
const int    BETA        = 2;        
const double Q0_MIN      = 0.5;      
const double Q0_MAX      = 0.9;      
const double Q0_STEP     = (Q0_MAX - Q0_MIN) / 200.0;  
const int    MAX_TIME    = 60;      
const bool   USE_LOCAL_SEARCH = true;  
int calculate_route_cost(const vi &route, const vvi &travel_time, const vi &service_time) {
    int cost = 0;
    int m = (int)route.size();
    for (int i = 0; i < m - 1; i++) {
        int u = route[i], v = route[i + 1];
        cost += travel_time[u][v];
        if (u != 0) cost += service_time[u];
    }
    return cost;
}

vvd compute_eta(const vvi &travel_time, const vi &service_time) {
    int n = (int)travel_time.size() - 1;
    vvd eta(n + 1, vd(n + 1, 0.0));
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == j) continue;
            int denom = travel_time[i][j] + (j == 0 ? 0 : service_time[j]);
            if (denom > 0) eta[i][j] = 1.0 / denom;
        }
    }
    return eta;
}

vvd compute_eta_ret(const vvi &travel_time, const vi &service_time) {
    int n = (int)travel_time.size() - 1;
    vvd eta(n + 1, vd(n + 1, 0.0));
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == j) continue;
            int denom = travel_time[i][j];
            if (j != 0) denom += service_time[j] + travel_time[j][0];
            if (denom > 0) eta[i][j] = 1.0 / denom;
        }
    }
    return eta;
}

vi two_opt(const vi &route, const vvi &travel_time, const vi &service_time) {
    vi best = route;
    int best_cost = calculate_route_cost(best, travel_time, service_time);
    int n = (int)best.size();
    bool improved = true;
    while (improved) {
        improved = false;
        for (int a = 1; a < n - 2 && !improved; a++) {
            for (int b = a + 1; b < n - 1 && !improved; b++) {
                vi candidate = best;
                reverse(candidate.begin() + a, candidate.begin() + b + 1);
                int c_cost = calculate_route_cost(candidate, travel_time, service_time);
                if (c_cost < best_cost) {
                    best = move(candidate);
                    best_cost = c_cost;
                    improved = true;
                }
            }
        }
        n = (int)best.size();
    }
    return best;
}

vector<vi> local_search(const vector<vi> &solution, const vvi &travel_time, const vi &service_time) {
    int k_routes = (int)solution.size();
    vector<vi> best_solution = solution;
    vector<int> route_costs(k_routes);
    for (int i = 0; i < k_routes; i++) {
        route_costs[i] = calculate_route_cost(best_solution[i], travel_time, service_time);
    }
    int best_max_cost = *max_element(route_costs.begin(), route_costs.end());

    for (int i = 0; i < k_routes; i++) {
        vi improved = two_opt(best_solution[i], travel_time, service_time);
        best_solution[i] = move(improved);
        route_costs[i] = calculate_route_cost(best_solution[i], travel_time, service_time);
    }
    best_max_cost = *max_element(route_costs.begin(), route_costs.end());

    int idx_long = int(max_element(route_costs.begin(), route_costs.end()) - route_costs.begin());
    int idx_short = int(min_element(route_costs.begin(), route_costs.end()) - route_costs.begin());
    vi &route_i = best_solution[idx_long];
    vi &route_j = best_solution[idx_short];

    if ((int)route_i.size() <= 2) {
        return best_solution;
    }

    vi customers_i(route_i.begin() + 1, route_i.end() - 1);
    if (customers_i.empty()) return best_solution;

    int c_to_move = customers_i[0];
    int max_srv = service_time[c_to_move];
    for (int c : customers_i) {
        if (service_time[c] > max_srv) {
            max_srv = service_time[c];
            c_to_move = c;
        }
    }

    vi new_route_i;
    new_route_i.reserve(route_i.size());
    for (int x : route_i) {
        if (x != c_to_move) new_route_i.push_back(x);
    }
    int cost_new_i = calculate_route_cost(new_route_i, travel_time, service_time);

    int best_move_cost = best_max_cost;
    vi best_new_i, best_new_j;

    for (int pos = 1; pos < (int)route_j.size(); pos++) {
        vi temp_j = route_j;
        temp_j.insert(temp_j.begin() + pos, c_to_move);
        int cost_new_j = calculate_route_cost(temp_j, travel_time, service_time);

        vector<int> new_costs = route_costs;
        new_costs[idx_long] = cost_new_i;
        new_costs[idx_short] = cost_new_j;
        int new_max = *max_element(new_costs.begin(), new_costs.end());
        if (new_max < best_move_cost) {
            best_move_cost = new_max;
            best_new_i = new_route_i;
            best_new_j = move(temp_j);
        }
    }

    if (!best_new_i.empty()) {
        best_solution[idx_long]  = move(best_new_i);
        best_solution[idx_short] = move(best_new_j);
    }

    return best_solution;
}

vector<vi> ant_construct_solution(int n, int k,
                                  const vvi &travel_time,
                                  const vi &service_time,
                                  vvd &pheromones,
                                  const vvd &eta,
                                  const vvd &eta_ret,
                                  mt19937 &rng,
                                  double q0) {
    vector<bool> visited(n+1, false);
    visited[0] = true;
    int unvisited_count = n;

    vector<vi> solution;
    solution.reserve(k);

    int route_count = 0;
    vi current_route;
    current_route.push_back(0);
    int current_customers = 0;

    double avg_customers = double(n) / double(k);
    double min_cut = max(1.0, avg_customers - avg_customers / 3.0);
    double max_cut = avg_customers + avg_customers / 3.0;

    while (route_count < k) {
        if (route_count == k - 1) {
            for (int c = 1; c <= n; c++) {
                if (!visited[c]) {
                    current_route.push_back(c);
                    visited[c] = true;
                }
            }
            current_route.push_back(0);
            solution.push_back(current_route);
            return solution;
        }

        int curr = current_route.back();
        if (current_customers >= (int)floor(max_cut)) {
            current_route.push_back(0);
            solution.push_back(current_route);
            route_count++;
            current_route.clear();
            current_route.push_back(0);
            current_customers = 0;
            continue;
        }

        vi candidates;
        candidates.reserve(unvisited_count + 1);
        for (int c = 1; c <= n; c++) {
            if (!visited[c]) candidates.push_back(c);
        }
        if (current_customers >= (int)floor(min_cut)) {
            candidates.push_back(0); 
        }

        if (candidates.empty()) {
            current_route.push_back(0);
            solution.push_back(current_route);
            route_count++;
            current_route.clear();
            current_route.push_back(0);
            current_customers = 0;
            continue;
        }

        int m = (int)candidates.size();
        vd tau_eta(m, 0.0);
        for (int i = 0; i < m; i++) {
            int next = candidates[i];
            double t = pheromones[curr][next];
            double h = (current_customers >= (int)floor(max_cut)) ? eta_ret[curr][next] : eta[curr][next];
            tau_eta[i] = t * pow(h, BETA);
            if (next == 0) {
                tau_eta[i] *= 1.5;
            }
        }
        double sum_tau = 0.0;
        for (double v : tau_eta) sum_tau += v;

        int next_node;
        if (sum_tau <= 0.0) {
            uniform_int_distribution<int> uni(0, m - 1);
            next_node = candidates[uni(rng)];
        } else {
            uniform_real_distribution<double> uni01(0.0, 1.0);
            double r = uni01(rng);
            if (r < q0) {
                int idx = int(max_element(tau_eta.begin(), tau_eta.end()) - tau_eta.begin());
                next_node = candidates[idx];
            } else {
                discrete_distribution<int> dist(tau_eta.begin(), tau_eta.end());
                next_node = candidates[dist(rng)];
            }
        }

        pheromones[curr][next_node] = (1.0 - RHO) * pheromones[curr][next_node] + RHO * TAU_0;

        if (next_node == 0) {
            current_route.push_back(0);
            solution.push_back(current_route);
            route_count++;
            current_route.clear();
            current_route.push_back(0);
            current_customers = 0;
        } else {
            current_route.push_back(next_node);
            visited[next_node] = true;
            unvisited_count--;
            current_customers++;
        }
    }

    return solution;
}
vector<vi> acs_vrp(int n, int k, const vvi &travel_time, const vi &service_time) {
    auto start = steady_clock::now();

    vvd pheromones(n + 1, vd(n + 1, TAU_0));
    vvd eta = compute_eta(travel_time, service_time);
    vvd eta_ret = compute_eta_ret(travel_time, service_time);

    vector<vi> best_solution;
    int best_max_cost = INT_MAX;
    int unchange_counter = 0;
    double q0 = Q0_MIN;

    random_device rd;
    mt19937 rng(rd());

    for (int iter = 0; iter < MAX_ITER; iter++) {
        auto now = steady_clock::now();
        double elapsed = duration<double>(now - start).count();
        if (elapsed > MAX_TIME) break;

        vector< vector<vi> > ant_sols;
        vector<int> ant_costs;
        ant_sols.reserve(NUM_ANTS);
        ant_costs.reserve(NUM_ANTS);

        for (int a = 0; a < NUM_ANTS; a++) {
            vector<vi> sol = ant_construct_solution(n, k, travel_time, service_time,
                                                    pheromones, eta, eta_ret, rng, q0);
            if (USE_LOCAL_SEARCH) {
                sol = local_search(sol, travel_time, service_time);
            }
            ant_sols.push_back(sol);
            int mcost = 0;
            for (auto &r : sol) {
                int c = calculate_route_cost(r, travel_time, service_time);
                mcost = max(mcost, c);
            }
            ant_costs.push_back(mcost);
        }

        int idx_iter_best = int(min_element(ant_costs.begin(), ant_costs.end()) - ant_costs.begin());
        int iter_best_cost = ant_costs[idx_iter_best];
        vector<vi> iter_best_sol = ant_sols[idx_iter_best];

        if (iter_best_cost < best_max_cost) {
            best_max_cost = iter_best_cost;
            best_solution = iter_best_sol;
            unchange_counter = 0;
        } else {
            unchange_counter++;
        }

        if (unchange_counter > 20) {
            q0 = max(q0 - Q0_STEP, Q0_MIN);
        } else {
            q0 = min(q0 + Q0_STEP, Q0_MAX);
        }

        for (auto &r : best_solution) {
            int len = (int)r.size();
            for (int i = 0; i < len - 1; i++) {
                int u = r[i], v = r[i + 1];
                pheromones[u][v] = (1.0 - RHO) * pheromones[u][v] + RHO * (1.0 / best_max_cost);
            }
        }
    }

    if (best_solution.empty()) {
        best_solution.resize(k, vi{0, 0});
    }
    while ((int)best_solution.size() < k) {
        best_solution.push_back(vi{0, 0});
    }
    return best_solution;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N, K; cin >> N >> K;
    vi service_time(N+1, 0);
    for (int i = 1; i <= N; i++) cin >> service_time[i];
    vvi travel_time(N+1, vi(N+1, 0));
    for (int i = 0; i <= N; i++) {
        for (int j = 0; j <= N; j++) cin >> travel_time[i][j];
    }
    vector<vi> solution = acs_vrp(N, K, travel_time, service_time);
    cout << K << "\n";
    for (auto &r : solution) {
        cout << r.size() << "\n";
        for (int i = 0; i < (int)r.size(); i++) {
            cout << r[i] << (i + 1 < (int)r.size() ? ' ' : '\n');
        }
    }
    return 0;
}
