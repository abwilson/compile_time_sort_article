
compile_time_sort_article.src = compile_time_sort_article.cpp

$(call exec,compile_time_sort_article)

map_sort.src = map_sort.cpp
$(call exec,map_sort)

flat_merge.src = flat_merge.cpp 
$(call exec,flat_merge)

flat_merge_godbolt.src = flat_merge_godbolt.cpp
$(call exec,flat_merge_godbolt)

flat_qsort.src = flat_qsort.cpp
$(call exec,flat_qsort)

perf_sizes = 8 16 32 64 # 128 256 512 1024 2048 # 4096 8192 16284

all: bench bench2 bench3

generate_data: $(perf_sizes:%=data_%.h)

$(bld)/data_%.h: generate.sh
	bash $< $* > $@

bench: $(perf_sizes:%=$(bld)/bench_%.result)

build_bench = c++ -c $< -DDATA_FILE=\"bld/data_$*.h\" -ftemplate-depth-8192 $(CXXFLAGS) -o $@.o

$(bld)/bench_%.result: perf_test.cpp $(bld)/data_%.h
	/usr/bin/time -l $(build_bench) > $@ 2>&1

bench2: $(perf_sizes:%=$(bld)/bench2_%.result)

$(bld)/bench2_%.result: map_sort_perf_test.cpp $(bld)/data_%.h
	/usr/bin/time -l $(build_bench) > $@ 2>&1

bench3: $(perf_sizes:%=$(bld)/bench3_%.result)

$(bld)/bench3_%.result: skew_sort_perf.cpp $(bld)/data_%.h
	/usr/bin/time -l $(build_bench) > $@ 2>&1
