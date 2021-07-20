bsub -b -I -J arraySum -q q_sw_expr -n 1 -cgsp 64 -host_stack 1024 -share_size 6500 -perf ./build/arraySum
