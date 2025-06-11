from ortools.linear_solver import pywraplp

def solve_vrp_ilp():
    import sys
    input = sys.stdin.readline
    N, K = map(int, input().split())
    d = list(map(int, input().split()))
    t = []
    for _ in range(N+1):
        t.append(list(map(int, input().split())))

    solver = pywraplp.Solver.CreateSolver('CBC')
    if not solver:
        print(-1)
        return

    x = {}
    for k in range(K):
        for i in range(N+1):
            for j in range(N+1):
                if i != j:
                    x[i, j, k] = solver.BoolVar(f'x_{i}_{j}_{k}')
    u = {}
    for k in range(K):
        for j in range(1, N+1):
            u[j, k] = solver.NumVar(1, N, f'u_{j}_{k}')
    Tmax = solver.NumVar(0, sum(d)+sum(sum(row) for row in t), 'Tmax')

    # 1. Mỗi khách phục vụ đúng 1 lần
    for j in range(1, N+1):
        solver.Add(sum(x[i, j, k] for k in range(K) for i in range(N+1) if i != j) == 1)
    # 2. Depot đầu/cuối mỗi tuyến
    for k in range(K):
        solver.Add(sum(x[0, j, k] for j in range(1, N+1)) == 1)
        solver.Add(sum(x[i, 0, k] for i in range(1, N+1)) == 1)
    # 3. Luồng bảo toàn
    for k in range(K):
        for j in range(1, N+1):
            solver.Add(sum(x[i, j, k] for i in range(N+1) if i != j) ==
                       sum(x[j, l, k] for l in range(N+1) if l != j))
    # 4. Subtour elimination (MTZ)
    for k in range(K):
        for i in range(1, N+1):
            for j in range(1, N+1):
                if i != j:
                    solver.Add(u[i, k] - u[j, k] + N * x[i, j, k] <= N-1)
    # 5. Makespan
    for k in range(K):
        route_time = []
        for i in range(N+1):
            for j in range(N+1):
                if i != j:
                    service = d[j-1] if j > 0 else 0
                    route_time.append(x[i, j, k] * (t[i][j] + service))
        solver.Add(solver.Sum(route_time) <= Tmax)

    solver.Minimize(Tmax)

    status = solver.Solve()
    if status != pywraplp.Solver.OPTIMAL and status != pywraplp.Solver.FEASIBLE:
        print(-1)
        return

    # Xuất đúng chuẩn đề
    print(K)
    for k in range(K):
        route = [0]
        current = 0
        visited = set([0])
        while True:
            found = False
            for j in range(N+1):
                if current != j and (current, j, k) in x and x[current, j, k].solution_value() > 0.5 and j not in visited:
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
    solve_vrp_ilp()
