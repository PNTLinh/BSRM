import heapq

def bsrm_ga(n, k, d, t):
    staff_routes = [[0] for _ in range(k)]
    staff_workload = [0] * k

    customers = sorted(range(1, n + 1), key=lambda i: d[i - 1], reverse=True)
    pq = [(0, i) for i in range(k)]
    heapq.heapify(pq)

    for customer in customers:
        workload, staff = heapq.heappop(pq)
        last_pos = staff_routes[staff][-1]

        # cập nhật workload và route
        staff_workload[staff] += d[customer - 1] + t[last_pos][customer]
        staff_routes[staff].append(customer)

        heapq.heappush(pq, (staff_workload[staff], staff))

    # quay về 0 sau khi phân công xong
    for i in range(k):
        last_pos = staff_routes[i][-1]
        staff_workload[i] += t[last_pos][0]
        staff_routes[i].append(0)

    # ghi file output
    with open('Output/output1.txt', 'w') as file:
        file.write(f"{k}\n")
        for i in range(k):
            route = staff_routes[i]
            file.write(f"{len(route)}\n")
            file.write(" ".join(map(str, route)) + "\n")

if __name__ == "__main__":
    with open('Input/case1.txt', 'r') as file:
        n, k = map(int, file.readline().split())
        d = list(map(int, file.readline().split()))
        t = [list(map(int, file.readline().split())) for _ in range(n + 1)]
        
    bsrm_ga(n,k,d,t)