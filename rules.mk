
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

perf_sizes = 8 16 32 64 128 # 256 512 # 1024 2048 4096 # 8192 16284

all: bench bench2 bench3

generate_data: $(perf_sizes:%=data_%.h)

$(bld)/data_%.h: generate.sh | $(mkdir)
	bash $< $* > $@

build_bench = c++ -c $< -DDATA_FILE=\"bld/data_$*.h\" -ftemplate-depth-8192 $(CXXFLAGS) -o $@.o

define bench_pattern
$(bld)/$1_bench_%.result: $1.cpp $(bld)/data_%.h | $(mkdir)
	/usr/bin/time -l $$(build_bench) > $$@ 2>&1
$1_bench: $(perf_sizes:%=$(bld)/$1_bench_%.result)
bench: $1_bench
endef

$(foreach i,\
    perf_test map_sort_perf_test skew_sort_perf qsort_perf_test flat_merge2_perf_test,\
    $(eval $(call bench_pattern,$i)))
