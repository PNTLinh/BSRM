from ortools.sat.python import cp_model

def solve_vrp_cp():
    import sys
    input = sys.stdin.readline
    N, K = map(int, input().split())
    d = list(map(int, input().split()))
    t = []
    for _ in range(N + 1):
        t.append(list(map(int, input().split())))

    model = cp_model.CpModel()
    x = {}
    for k in range(K):
        for i in range(N+1):
            for j in range(N+1):
                if i != j:
                    x[i, j, k] = model.NewBoolVar(f'x_{i}_{j}_{k}')
    Tmax = model.NewIntVar(0, sum(d) + sum(sum(row) for row in t), 'Tmax')

    for j in range(1, N+1):
        model.Add(sum(x[i, j, k] for k in range(K) for i in range(N+1) if i != j) == 1)
    for k in range(K):
        model.Add(sum(x[0, j, k] for j in range(1, N+1)) == 1)
        model.Add(sum(x[i, 0, k] for i in range(1, N+1)) == 1)
    for k in range(K):
        for j in range(1, N+1):
            model.Add(sum(x[i, j, k] for i in range(N+1) if i != j) ==
                      sum(x[j, l, k] for l in range(N+1) if l != j))
    u = {}
    for k in range(K):
        for j in range(1, N+1):
            u[j, k] = model.NewIntVar(1, N, f'u_{j}_{k}')
    for k in range(K):
        for i in range(1, N+1):
            for j in range(1, N+1):
                if i != j:
                    model.Add(u[i, k] - u[j, k] + N * x[i, j, k] <= N-1)
    for k in range(K):
        route_time = []
        for i in range(N+1):
            for j in range(N+1):
                if i != j:
                    service = d[j-1] if j > 0 else 0
                    route_time.append(x[i, j, k] * (t[i][j] + service))
        model.Add(sum(route_time) <= Tmax)
    model.Minimize(Tmax)

    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = 120.0  # timeout nếu cần
    status = solver.Solve(model)
    if status not in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        print(-1)
        return

    print(K)
    for k in range(K):
        route = [0]
        current = 0
        visited = set([0])
        while True:
            found = False
            for j in range(N+1):
                if current != j and (current, j, k) in x and solver.Value(x[current, j, k]) and j not in visited:
                    route.append(j)
                    visited.add(j)
                    current = j
                    found = True
                    break
            if not found or current == 0:
                break
        if len(route) == 1:
            route.append(0)
        print(len(route))
        print(' '.join(map(str, route)))

if __name__ == "__main__":
    solve_vrp_cp()
