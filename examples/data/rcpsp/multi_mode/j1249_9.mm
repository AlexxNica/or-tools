************************************************************************
file with basedata            : md113_.bas
initial value random generator: 1943377927
************************************************************************
projects                      :  1
jobs (incl. supersource/sink ):  14
horizon                       :  98
RESOURCES
  - renewable                 :  2   R
  - nonrenewable              :  2   N
  - doubly constrained        :  0   D
************************************************************************
PROJECT INFORMATION:
pronr.  #jobs rel.date duedate tardcost  MPM-Time
    1     12      0       16        8       16
************************************************************************
PRECEDENCE RELATIONS:
jobnr.    #modes  #successors   successors
   1        1          3           2   3   4
   2        3          3           5   9  10
   3        3          3           7  10  11
   4        3          3           6   7  13
   5        3          3           7   8  13
   6        3          3           8   9  10
   7        3          1          12
   8        3          1          11
   9        3          2          11  12
  10        3          1          12
  11        3          1          14
  12        3          1          14
  13        3          1          14
  14        1          0        
************************************************************************
REQUESTS/DURATIONS:
jobnr. mode duration  R 1  R 2  N 1  N 2
------------------------------------------------------------------------
  1      1     0       0    0    0    0
  2      1     2       0    3    7    4
         2     5       0    3    3    2
         3    10       0    2    2    2
  3      1     4       0    7    5    4
         2     5       0    4    5    3
         3     8       4    0    4    3
  4      1     2       0    3    9   10
         2     7       5    0    9    8
         3     8       0    2    8    8
  5      1     1       0    4    5   10
         2     6       8    0    5    8
         3    10       0    4    4    7
  6      1     4       5    0    5    7
         2     5       0    1    5    6
         3     6       3    0    5    6
  7      1     4       0    8    4    7
         2     8       0    8    2    7
         3     9       1    0    2    7
  8      1     3       0    9    9    7
         2     4       0    7    9    5
         3     8       7    0    8    4
  9      1     2       7    0    8    6
         2     7       7    0    7    5
         3    10       0    8    7    2
 10      1     4       7    0    8    6
         2     4       0    9    8    5
         3     6       0    5    6    3
 11      1     3       2    0    8    8
         2     6       0    7    8    7
         3     9       0    2    7    6
 12      1     6       2    0    5    7
         2     9       1    0    1    4
         3     9       0    5    1    6
 13      1     1       7    0    7    6
         2     2       0    6    4    5
         3     5       2    0    3    4
 14      1     0       0    0    0    0
************************************************************************
RESOURCEAVAILABILITIES:
  R 1  R 2  N 1  N 2
    6   10   74   76
************************************************************************