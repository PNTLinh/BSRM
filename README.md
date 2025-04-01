# BSRM
Có N khách hàng 1, 2, …, N cần được bảo trì mạng internet. Khách hàng i ở địa điểm i (i = 1,…,N) • Việc bảo trì cho khách hàng i kéo dài d(i) đơn vị thời gian (s) • Có K nhân viên kỹ thuật ở trụ sở công ty (điểm 0) và có thời điểm bắt đầu là việc là t0 = 0. • Thời gian di chuyển từ điểm i đến điểm j là t(i,j) • Lập kế hoạch phân công nhân viên thực hiện bảo trì cho các khách hàng sao cho thời gian làm việc nhiều nhất (thời gian di chuyển công thời gian bảo trì) của một nhân viên nào đó là nhỏ nhất
A route of staff k is represented by a sequence of points r[0], r[1], r[2], . . ., r[Lk] in which r[0] = r[Lk] = 0 (the depot)
Input
Line 1: contains N and K (1 <= N <= 1000, 1 <= K <= 100)
Line 2: contains d(1), d(2), . . ., d(N)  (1 <= d(i) <= 1000)
 Line i + 3 (i = 0, 1, 2, . . ., N): contains the ith row of the matrix t
Output
Line 1: contains K
Line 2k (k = 1, . . ., K): contains a positive integer Lk
Line 2k+1 (k = 1, 2, . . ., K): contains  r[0], r[1], r[2], . . ., r[Lk]
