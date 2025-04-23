from ortools.linear_solver import pywraplp

#input

n,k = [int(u) for u in input().split()]

work_time = [0]+[int(u) for u in input().split()]

A=[]
for i in range(n+1):
    A.append([int(u) for u in input().split()])

#setup
# solver = pywraplp.Solver('SCIP')
solver = pywraplp.Solver(name='factory-mlip',problem_type=pywraplp.Solver.CBC_MIXED_INTEGER_PROGRAMMING)
inf = solver.infinity()

pass_ = [[[None for j in range(n+1)] for i in range(n+1)]for K in range(k)]
for K in range(k):
    for i in range(n+1):
        for j in range(n+1):
            pass_[K][i][j] = solver.IntVar(lb=0, ub=1 if i!=j else 0, name=f'x_{K}_{i}_{j}')

work_at = [[None for j in range(n+1)] for K in range(k)]
for K in range(k):
    for j in range(n+1):
        work_at[K][j] = solver.IntVar(lb=0, 
                                    #   ub=solver.Sum(pass_[K][i][j] for i in range(n+1)), #already set constraint for work_at[K][j]
                                      ub=1,
                                      name=f'work_at_{K}_{j}')

#constraint

# for j in range(1,n+1):
#     solver.Add(solver.Sum(solver.Sum(pass_[K][i][j] for i in range(0,n+1)) for K in range(k))==1)
# for i in range(1,n+1):
#     solver.Add(solver.Sum(solver.Sum(pass_[K][i][j] for j in range(0,n+1)) for K in range(k))==1)
for K in range(k):
    solver.Add(solver.Sum(pass_[K][0][j] for j in range(1,n+1))==1) #xuất phát ở 0
for K in range(k):
    solver.Add(solver.Sum(pass_[K][i][0] for i in range(1,n+1))==1) #kết thúc ở 0
# for K in range(k):
#     for i_fix in range(0,n+1):
#         for j_fix in range(0,n+1):
#             solver.Add(solver.Sum(pass_[K][i][j_fix] for i in range(n+1))==solver.Sum(pass_[K][i_fix][j] for j in range(n+1))) #một đường vào - 1 đường ra
for K in range(k):
        for j_fix in range(0,n+1):
            solver.Add(solver.Sum(pass_[K][i][j_fix] for i in range(n+1))==solver.Sum(pass_[K][j_fix][j] for j in range(n+1))) #một đường vào - 1 đường ra

for K in range(k):
    for j in range(n+1):
        solver.Add(work_at[K][j] <= solver.Sum(pass_[K][i][j] for i in range(n+1))) #already set constraint for work_at[K][j]; K chỉ có thể work at j nếu K có đi vào j 
for j in range(1,n+1):
    solver.Add(solver.Sum(work_at[K][j] for K in range(k))==1) # chỉ có 1 người work j

#objective
z = solver.NumVar(0, inf, 'z')

#intermediate value
total_time_worker = [None for K in range(k)]

for K in range(k):
    # for i in range(n+1):
        # for j in range
    total_time_worker[K]=solver.Sum((solver.Sum(pass_[K][i][j]*A[i][j] for i in range(n+1))+work_at[K][j]*work_time[j]) for j in range(n+1))
    solver.Add(z>=total_time_worker[K])

solver.Minimize(z)


#check 
#so what is a cycle and a valid cycle
def check_by_K(K): #check riêng với mỗi worker K # tìm all cycles 
    route = [[pass_[K][i][j].solution_value() for j in range(n+1)] for i in range(n+1)]
    mark = [0 for i in range(n+1)]

    cycles=[]
    def find_cyc(u):
        print(f"find cycle {u}")
        st=[u]
        while mark[u]==0:
            print(f"st:{st}")
            mark[u]=1
            for j in range(n+1):
                if route[u][j]==1:
                    # print(f"{u}->{5}:mark:{mark[5]}")
                    st.append(j)
                    # print(f"{u}->{j}:mark:{mark[j]}")
                    if mark[j]==1:
                        return st
                    u=j
                    break
        

    for u in range(n+1):
        if mark[u]==0:
            tmp =find_cyc(u)
            if tmp:
                cycles.append(tmp)
    return cycles


def check(): #duyệt qua all worker
    cycles, filtered_cycles = [],[]
    for K in range(k):
        cycles_K = check_by_K(K)
        if cycles_K==[]:continue
        if len(cycles_K)==1:filtered_cycles.append([K,cycles_K])
        else: cycles.append([K,cycles_K])
    print(f"cycles{cycles}, filtered_cycles {filtered_cycles}")
    return cycles, filtered_cycles
        

#solve
status = solver.Solve()
if status != pywraplp.Solver.OPTIMAL:
    print("not optimal")
    exit(0)
def prn():
    for K in range(k):
        print(f"k:{K}: total time {total_time_worker[K].solution_value()}")
        for i in range(n+1):
            for j in range(n+1):
                print(pass_[K][i][j].solution_value(),end=" ")
            print()
        # if total_time_worker[K] ==290: exit(0)
        print()

cycles, filtered_cycles = check()
while len(cycles) > 0:
    prn()
    for u in cycles:
        print(u)
        print(cycles)
        K,cycles_K=u         
        for cyc in cycles_K:
            if cyc[0]!=0:
                solver.Add(solver.Sum(pass_[K][cyc[i]][cyc[i+1]] for i in range(len(cyc)-1)) <= len(cyc)-2)
    status = solver.Solve()
    cycles,filtered_cycles = check()
print(filtered_cycles)
prn()

obj = solver.Objective().Value()

        