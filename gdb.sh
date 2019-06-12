#########################################################################
# File Name: gdb.sh
# Author: Yibo Lin
# mail: yibolin@utexas.edu
# Created Time: Sat 23 May 2015 05:22:02 PM CDT
#########################################################################
#!/bin/bash

color_num=3
simplify_level=3
thread_num=1
algo=LP # BACKTRACK or ILP or LP or SDP or MIS
#benchmark="output_4x4-flat.gds"
#benchmark="Via2_local_precolor.gds"
#benchmark="via2_local_precolor.gds"
#benchmark="via2.gds"
benchmark="sim_s2.gds"
#benchmark="mpl_sim_s3_c${color_num}_algo1.gds" # output from mpl 
#benchmark="Via1_clip_300x300.gds"

if [[ $benchmark == output_* ]]; then 
	benchmark_dir="/home/local/eda03/shared_benchmarks/imec_7nm/dpt_array"
elif [[ $benchmark == Via2_local_precolor* ]]; then
	benchmark_dir="/home/local/eda03/shared_benchmarks/imec_7nm"
elif [[ $benchmark == via2_local_precolor* ]]; then
	benchmark_dir="./bin/bench"
elif [[ $benchmark == via2.gds ]]; then
    benchmark_dir="/home/local/eda03/shared_benchmarks"
elif [[ $benchmark == sim_* ]]; then
    benchmark_dir="${BENCHMARKS_DIR}/ISCAS_sim"
elif [[ $benchmark == total_* ]]; then
    benchmark_dir="${BENCHMARKS_DIR}/ISCAS_total"
elif [[ $benchmark == mpl_* ]]; then 
    benchmark_dir="${BENCHMARKS_DIR}/mpl_output/ISCAS_sim"
elif [[ $benchmark == Via1* ]]; then
    benchmark_dir="/home/local/eda03/shared_benchmarks"
fi

#output="${benchmark%.*}-c${color_num}-out.gds"
output=""

if [[ $benchmark == output_* ]]; then 

# this parameter works for output_1x1.gds 
gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args ./bin/SimpleMPL \
    -shape "RECTANGLE" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 208 \
	-uncolor_layer 209 \
	-uncolor_layer 210 \
	-uncolor_layer 211 \
	-uncolor_layer 216 \
	-path_layer 207 \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} #\
#	-verbose

elif [[ $benchmark == "via2.gds" ]]; then 

gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args ./bin/SimpleMPL \
    -shape "RECTANGLE" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 208 \
	-uncolor_layer 209 \
	-uncolor_layer 210 \
	-uncolor_layer 211 \
	-uncolor_layer 216 \
	-path_layer 207 \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} #\
#	-verbose

elif [[ $benchmark == Via2_local_precolor* ]]; then

# this parameter works for Via2_local_precolor.gds
gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args ./bin/SimpleMPL \
    -shape "RECTANGLE" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 100 \
	-precolor_layer 201 \
	-precolor_layer 202 \
	-precolor_layer 203 \
	-coloring_distance 130 \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} \
	-verbose

elif [[ $benchmark == via2_local_precolor* ]]; then

# this parameter works for via2_local_precolor.gds
gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args ./bin/SimpleMPL \
    -shape "RECTANGLE" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 100 \
	-precolor_layer 201 \
	-precolor_layer 202 \
	-precolor_layer 203 \
	-coloring_distance 300 \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} \
	-verbose

elif [[ $benchmark == sim_* ]]; then

# this parameter works for sim_c1
    if [[ $color_num == 3 ]]; then
        coloring_distance=120
    else
        coloring_distance=160
    fi
gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args \
    ./bin/SimpleMPL \
    -shape "POLYGON" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 1 \
    -uncolor_layer 101 \
	-coloring_distance ${coloring_distance} \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} \
    -dbg_comp_id 130 
    #-verbose 

elif [[ $benchmark == mpl_* ]]; then 

# this parameter works for mpl_sim_c9
if [[ $color_num == 3 ]]; then 
    coloring_distance=120
elif [[ $color_num == 4 ]]; then 
    coloring_distance=160
fi

gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args \
    ./bin/SimpleMPL \
    -shape "POLYGON" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
    -precolor_layer 3 \
    -precolor_layer 4 \
    -precolor_layer 5 \
    -precolor_layer 6 \
    -uncolor_layer 0 \
	-coloring_distance ${coloring_distance} \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} \
    -dbg_comp_id 1686000 \
    -verbose 

elif [[ $benchmark == Via1* ]]; then 

# this parameter works for mpl_sim_c9
if [[ $color_num == 3 ]]; then 
    coloring_distance=64
elif [[ $color_num == 4 ]]; then 
    coloring_distance=90
fi

gdb \
	-ex "source ${LIBRARIES_DIR}/gdb_container.sh" \
	--args ./bin/SimpleMPL \
    -shape "RECTANGLE" \
	-in "${benchmark_dir}/${benchmark}" \
	-out "${output}" \
	-uncolor_layer 71 \
	-uncolor_layer 72 \
	-uncolor_layer 73 \
	-coloring_distance ${coloring_distance} \
	-color_num ${color_num} \
	-simplify_level ${simplify_level} \
	-thread_num ${thread_num} \
	-algo ${algo} #\
#	-verbose

fi

