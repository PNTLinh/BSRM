#include <bits/stdc++.h>
using namespace std;
using ll = long long;

ll route_time(const vector<int> &route,
               const vector<int> &d_list,
               const vector<vector<int>> &t_matrix) {
    if (route.empty()) return 0;
    ll total_time = 0;
    total_time += t_matrix[0][route[0]];
    total_time += d_list[route[0] - 1];
    for (size_t i = 0; i + 1 < route.size(); i++) {
        total_time += t_matrix[route[i]][route[i + 1]];
        total_time += d_list[route[i + 1] - 1];
    }
    total_time += t_matrix[route.back()][0];
    return total_time;
}

vector<vector<int>> decode_solution(const pair<vector<int>, vector<int>> &solution, int N, int K) {
    const vector<int> &perm = solution.first;
    const vector<int> &cuts = solution.second;
    vector<vector<int>> routes;
    routes.reserve(K);
    int start = 0;
    for (int i = 0; i < K - 1; i++) {
        int end = cuts[i];
        routes.emplace_back(perm.begin() + start, perm.begin() + end);
        start = end;
    }
    routes.emplace_back(perm.begin() + start, perm.end());
    return routes;
}

ll compute_fitness(const pair<vector<int>, vector<int>> &solution,
                   int N, int K,
                   const vector<int> &d_list,
                   const vector<vector<int>> &t_matrix) {
    vector<vector<int>> routes = decode_solution(solution, N, K);
    ll makespan = 0;
    for (auto &route : routes) {
        ll time_r = route_time(route, d_list, t_matrix);
        if (time_r > makespan) makespan = time_r;
    }
    return makespan;
}

pair<pair<vector<int>, vector<int>>, ll> hill_climbing(
    pair<vector<int>, vector<int>> solution,
    int N, int K,
    const vector<int> &d_list,
    const vector<vector<int>> &t_matrix,
    int max_iterations = 50,
    int max_no_improve = 20) {
    vector<int> perm = solution.first;
    vector<int> cuts = solution.second;
    ll current_fitness = compute_fitness(solution, N, K, d_list, t_matrix);
    int no_improve_count = 0;
    std::mt19937 rng(std::random_device{}());
    for (int iter = 0; iter < max_iterations; iter++) {
        bool improved = false;
        for (int trial = 0; trial < 3; trial++) {
            uniform_int_distribution<int> dist(0, (int)perm.size() - 1);
            int i = dist(rng);
            int j = dist(rng);
            if (i == j) continue;
            auto new_perm = perm;
            swap(new_perm[i], new_perm[j]);
            pair<vector<int>, vector<int>> new_solution = {new_perm, cuts};
            ll new_fitness = compute_fitness(new_solution, N, K, d_list, t_matrix);
            if (new_fitness < current_fitness) {
                perm = move(new_perm);
                current_fitness = new_fitness;
                improved = true;
                break;
            }
        }
        if (!improved && K > 1) {
            for (int trial = 0; trial < 3; trial++) {
                uniform_int_distribution<int> dist_cut(0, K - 2);
                int idx = dist_cut(rng);
                vector<bool> used(N + 1, false);
                for (int c : cuts) {
                    used[c] = true;
                }
                vector<int> candidates;
                candidates.reserve(N);
                for (int pos = 1; pos < N; pos++) {
                    if (!used[pos]) candidates.push_back(pos);
                }
                if (candidates.empty()) break;
                uniform_int_distribution<int> choose(0, (int)candidates.size() - 1);
                int new_cut = candidates[choose(rng)];
                auto new_cuts = cuts;
                new_cuts[idx] = new_cut;
                sort(new_cuts.begin(), new_cuts.end());
                pair<vector<int>, vector<int>> new_solution = {perm, new_cuts};
                ll new_fitness = compute_fitness(new_solution, N, K, d_list, t_matrix);
                if (new_fitness < current_fitness) {
                    cuts = move(new_cuts);
                    current_fitness = new_fitness;
                    improved = true;
                    break;
                }
            }
        }
        if (!improved) {
            no_improve_count++;
            if (no_improve_count >= max_no_improve) break;
        } else {
            no_improve_count = 0;
        }
    }
    return {{perm, cuts}, current_fitness};
}

pair<vector<int>, vector<int>> create_heuristic_solution(
    int N, int K,
    const vector<int> &d_list,
    const vector<vector<int>> &t_matrix) {
    vector<int> unvisited(N);
    iota(unvisited.begin(), unvisited.end(), 1);
    vector<int> perm;
    perm.reserve(N);
    int current = 0;
    for (int step = 0; step < N; step++) {
        int best = unvisited[0];
        int best_dist = t_matrix[current][best];
        for (int c : unvisited) {
            if (t_matrix[current][c] < best_dist) {
                best = c;
                best_dist = t_matrix[current][c];
            }
        }
        perm.push_back(best);
        unvisited.erase(find(unvisited.begin(), unvisited.end(), best));
        current = best;
    }
    vector<int> cuts;
    cuts.reserve(K - 1);
    int idx = 0;
    for (int i = 0; i < K - 1; i++) {
        int seg_len = N / K + (i < (N % K) ? 1 : 0);
        idx += seg_len;
        cuts.push_back(idx);
    }
    return {perm, cuts};
}

pair<vector<int>, vector<int>> create_random_solution(
    int N, int K,
    const vector<int> &d_list,
    const vector<vector<int>> &t_matrix,
    mt19937 &rng) {
    uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(rng) < 0.5) {
        return create_heuristic_solution(N, K, d_list, t_matrix);
    } else {
        vector<int> perm(N);
        iota(perm.begin(), perm.end(), 1);
        shuffle(perm.begin(), perm.end(), rng);
        vector<int> cuts;
        cuts.reserve(K - 1);
        if (K > 1) {
            vector<int> positions(N - 1);
            iota(positions.begin(), positions.end(), 1);
            shuffle(positions.begin(), positions.end(), rng);
            positions.resize(K - 1);
            sort(positions.begin(), positions.end());
            cuts = positions;
        }
        return {perm, cuts};
    }
}


int selection(const vector<pair<vector<int>, vector<int>>> &population,
              const vector<ll> &fitnesses,
              mt19937 &rng,
              int tournament_size = 2) {
    uniform_int_distribution<int> dist(0, (int)population.size() - 1);
    int best_idx = dist(rng);
    for (int i = 1; i < tournament_size; i++) {
        int idx = dist(rng);
        if (fitnesses[idx] < fitnesses[best_idx]) best_idx = idx;
    }
    return best_idx;
}

vector<int> pmx_crossover(const vector<int> &p1, const vector<int> &p2, mt19937 &rng) {
    int size = (int)p1.size();
    uniform_int_distribution<int> dist(0, size - 1);
    int cx1 = dist(rng);
    int cx2 = dist(rng);
    if (cx1 > cx2) swap(cx1, cx2);
    vector<int> child(size, -1);
    for (int i = cx1; i <= cx2; i++) child[i] = p1[i];
    for (int i = cx1; i <= cx2; i++) {
        if (find(child.begin() + cx1, child.begin() + cx2 + 1, p2[i]) == child.begin() + cx2 + 1) {
            int val = p2[i];
            int pos = i;
            while (true) {
                int mapped = p1[pos];
                pos = (int)(find(p2.begin(), p2.end(), mapped) - p2.begin());
                if (child[pos] < 0) {
                    child[pos] = p2[i];
                    break;
                }
            }
        }
    }
    for (int i = 0; i < size; i++) {
        if (child[i] < 0) child[i] = p2[i];
    }
    return child;
}

pair<vector<int>, vector<int>> crossover(
    const pair<vector<int>, vector<int>> &p1,
    const pair<vector<int>, vector<int>> &p2,
    int N, int K,
    mt19937 &rng) {
    const auto &perm1 = p1.first;
    const auto &cuts1 = p1.second;
    const auto &perm2 = p2.first;
    const auto &cuts2 = p2.second;
    vector<int> child_perm = pmx_crossover(perm1, perm2, rng);
    vector<int> child_cuts;
    uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(rng) < 0.5) child_cuts = cuts1;
    else child_cuts = cuts2;
    sort(child_cuts.begin(), child_cuts.end());
    return {child_perm, child_cuts};
}

void mutation(pair<vector<int>, vector<int>> &sol,
              int N, int K,
              double mutation_rate,
              mt19937 &rng) {
    auto &perm = sol.first;
    auto &cuts = sol.second;
    uniform_real_distribution<double> prob(0.0, 1.0);
    if (prob(rng) < mutation_rate) {
        uniform_int_distribution<int> dist(0, (int)perm.size() - 1);
        int i = dist(rng);
        int j = dist(rng);
        swap(perm[i], perm[j]);
    }
    if (K > 1 && prob(rng) < mutation_rate) {
        uniform_int_distribution<int> dist_cut(0, K - 2);
        int idx = dist_cut(rng);
        vector<bool> used(N + 1, false);
        for (int c : cuts) used[c] = true;
        vector<int> candidates;
        for (int pos = 1; pos < N; pos++) {
            if (!used[pos]) candidates.push_back(pos);
        }
        if (!candidates.empty()) {
            uniform_int_distribution<int> choose(0, (int)candidates.size() - 1);
            int new_cut = candidates[choose(rng)];
            cuts[idx] = new_cut;
            sort(cuts.begin(), cuts.end());
        }
    }
}

pair<pair<vector<int>, vector<int>>, ll> genetic_algorithm(
    int N, int K,
    const vector<int> &d_list,
    const vector<vector<int>> &t_matrix,
    int pop_size = 50,
    int generations = 1000,
    double crossover_rate = 0.8,
    double mutation_rate = 0.1,
    bool use_hill_climbing = true,
    int hill_climbing_freq = 10,
    int time_limit = 300) {
    auto start_time = chrono::steady_clock::now();
    mt19937 rng(random_device{}());
    vector<pair<vector<int>, vector<int>>> population;
    population.reserve(pop_size);
    for (int i = 0; i < pop_size; i++) {
        population.push_back(create_random_solution(N, K, d_list, t_matrix, rng));
    }
    vector<ll> fitnesses(pop_size);
    ll best_fit = LLONG_MAX;
    pair<vector<int>, vector<int>> best_sol;
    for (int i = 0; i < pop_size; i++) {
        fitnesses[i] = compute_fitness(population[i], N, K, d_list, t_matrix);
        if (fitnesses[i] < best_fit) {
            best_fit = fitnesses[i];
            best_sol = population[i];
        }
    }
    vector<ll> best_fitness_history;
    vector<double> avg_fitness_history;
    for (int gen = 0; gen < generations; gen++) {
        auto now = chrono::steady_clock::now();
        double elapsed = chrono::duration<double>(now - start_time).count();
        if (elapsed > time_limit) {
            cerr << "Time limit reached at generation " << gen << "\n";
            break;
        }
        vector<pair<vector<int>, vector<int>>> new_population;
        new_population.reserve(pop_size);
        while ((int)new_population.size() < pop_size) {
            int idx1 = selection(population, fitnesses, rng);
            int idx2 = selection(population, fitnesses, rng);
            auto p1 = population[idx1];
            auto p2 = population[idx2];
            pair<vector<int>, vector<int>> child;
            uniform_real_distribution<double> prob(0.0, 1.0);
            if (prob(rng) < crossover_rate) {
                child = crossover(p1, p2, N, K, rng);
            } else {
                child = p1;
            }
            mutation(child, N, K, mutation_rate, rng);
            new_population.push_back(move(child));
        }
        population = move(new_population);
        for (int i = 0; i < pop_size; i++) {
            fitnesses[i] = compute_fitness(population[i], N, K, d_list, t_matrix);
        }
        ll gen_best = LLONG_MAX;
        ll gen_sum = 0;
        int gen_best_idx = 0;
        for (int i = 0; i < pop_size; i++) {
            gen_sum += fitnesses[i];
            if (fitnesses[i] < gen_best) {
                gen_best = fitnesses[i];
                gen_best_idx = i;
            }
        }
        best_fitness_history.push_back(gen_best);
        avg_fitness_history.push_back((double)gen_sum / pop_size);
        if (gen_best < best_fit) {
            best_fit = gen_best;
            best_sol = population[gen_best_idx];
        }
        if (use_hill_climbing && (gen + 1) % hill_climbing_freq == 0) {
            now = chrono::steady_clock::now();
            elapsed = chrono::duration<double>(now - start_time).count();
            if (elapsed > time_limit) break;
            ll improved_fit;
            pair<vector<int>, vector<int>> improved_sol;
            tie(improved_sol, improved_fit) = hill_climbing(best_sol, N, K, d_list, t_matrix);
            if (improved_fit < best_fit) {
                int worst_idx = max_element(fitnesses.begin(), fitnesses.end()) - fitnesses.begin();
                population[worst_idx] = improved_sol;
                fitnesses[worst_idx] = improved_fit;
                best_sol = improved_sol;
                best_fit = improved_fit;
            }
        }
    }
    return {best_sol, best_fit};
}

void print_solution(const pair<vector<int>, vector<int>> &solution, int N, int K) {
    auto routes = decode_solution(solution, N, K);
    cout << K << "\n";
    for (auto &route : routes) {
        vector<int> full_route;
        full_route.reserve(route.size() + 2);
        full_route.push_back(0);
        for (int x : route) full_route.push_back(x);
        full_route.push_back(0);
        cout << full_route.size() << "\n";
        for (size_t i = 0; i < full_route.size(); i++) {
            cout << full_route[i] << (i + 1 < full_route.size() ? ' ' : '\n');
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N, K;cin >> N >> K;
    vector<int> d_list(N);
    for (int i = 0; i < N; i++) cin >> d_list[i];
    vector<vector<int>> t_matrix(N + 1, vector<int>(N + 1));
    for (int i = 0; i <= N; i++) {
        for (int j = 0; j <= N; j++) cin >> t_matrix[i][j];
    }
    auto result = genetic_algorithm(N, K, d_list, t_matrix, 100,500,0.8,0.1,true,20,300);
    auto best_sol = result.first;
    print_solution(best_sol, N, K);
    return 0;
}