************************************************************************
file with basedata            : c1522_.bas
initial value random generator: 140490209
************************************************************************
projects                      :  1
jobs (incl. supersource/sink ):  18
horizon                       :  135
RESOURCES
  - renewable                 :  2   R
  - nonrenewable              :  2   N
  - doubly constrained        :  0   D
************************************************************************
PROJECT INFORMATION:
pronr.  #jobs rel.date duedate tardcost  MPM-Time
    1     16      0       22       10       22
************************************************************************
PRECEDENCE RELATIONS:
jobnr.    #modes  #successors   successors
   1        1          3           2   3   4
   2        3          3           5   7   8
   3        3          2           6  17
   4        3          3           5   8  16
   5        3          2           9  10
   6        3          1          10
   7        3          1          16
   8        3          1          13
   9        3          3          11  12  17
  10        3          1          12
  11        3          1          15
  12        3          1          14
  13        3          1          15
  14        3          1          15
  15        3          1          18
  16        3          1          18
  17        3          1          18
  18        1          0        
************************************************************************
REQUESTS/DURATIONS:
jobnr. mode duration  R 1  R 2  N 1  N 2
------------------------------------------------------------------------
  1      1     0       0    0    0    0
  2      1     1       9    9    0    6
         2     4       9    6    0    3
         3    10       8    6    0    1
  3      1     1       9    2    8    0
         2     2       8    2    0    9
         3     7       8    1    0    9
  4      1     3       2    9    3    0
         2     8       1    7    2    0
         3     9       1    6    0    8
  5      1     9       6    3    6    0
         2     9       6    3    0    7
         3    10       6    2    5    0
  6      1     1       4    9    0    4
         2     3       4    6    0    1
         3     7       3    3    8    0
  7      1     2       9    3    2    0
         2     6       9    3    0    3
         3     7       8    2    2    0
  8      1     5       3    7    7    0
         2     9       3    7    0   10
         3    10       3    5    1    0
  9      1     1       8    9    7    0
         2     3       5    8    7    0
         3     7       4    8    0    8
 10      1     1       8    6    8    0
         2     3       7    6    6    0
         3     7       7    1    0    8
 11      1     8       6    5    4    0
         2     8       7    5    0    7
         3     9       6    4    0    5
 12      1     3       5   10    0    3
         2     4       5    7    0    2
         3     6       5    3    2    0
 13      1     5       3    2    0    4
         2     6       3    1    4    0
         3     9       3    1    3    0
 14      1     4       5    4    0    6
         2     4       6    3    5    0
         3     9       3    1    0    7
 15      1     1       9    8    6    0
         2     1       9    7    0    3
         3    10       6    3    0    2
 16      1     6       9    8    0    5
         2     6      10    7    6    0
         3     8       8    7    0    5
 17      1     4       5   10    8    0
         2     6       5    9    5    0
         3    10       4    8    2    0
 18      1     0       0    0    0    0
************************************************************************
RESOURCEAVAILABILITIES:
  R 1  R 2  N 1  N 2
   14   17   64   69
************************************************************************
