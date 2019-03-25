//
// Created by Mark Plagge on 2019-03-16.
//

#include <gtest/gtest.h>
#include <gtest/gtest.h>
#include "../src/include/globals.h"
#include <cmath>

template <typename STAT>
bool as_stat_ad(BF_Event_Status status, STAT stats){

    return bool(as_integer(status & stats));
}

template <typename S1, typename ... STAT>
bool as_stat_ad(BF_Event_Status status, S1 stat, STAT ... stats){
    //EXPECT_PRED3(in_the,status, stat, t);
    //return in_the(status, stat) && as_stat_ad(status,t,stats...);
    //ASSERT_EQ(as_integer(status & stat),1);
    return bool(as_integer(status & stat) && as_stat_ad(status, stats...));

}

#define BF BF_Event_Status
//print out enum function - might move this to globals.h
#define BNM(BF_TP) "BF_Event_Status::#BF_TP\n"
std::ostream& operator << (std::ostream & os, const BF_Event_Status & obj){
#define idf(BFNAME) if(as_integer(obj & BFNAME)){out_s = out_s + #BFNAME"\n";}
    std::string out_s = "";
    idf(BF::Heartbeat_Sent);
    idf(BF::Spike_Sent);
    idf(BF::Output_Spike_Sent);
    idf(BF::Heartbeat_Rec);
    idf(BF::Spike_Rec);
    idf(BF::NS_Tick_Update);
    idf(BF::Leak_Update);
    idf(BF::FR_Update);
    return os << out_s;
//    if(in_the(BF::Heartbeat_Sent    , obj){s = s + "Heartbeat_Sent\n"}
//    if(in_the(BF::Spike_Sent        , obj){s = s + "Spike_Sent\n"}
//    if(in_the(BF::Output_Spike_Sent , obj){s = s + "Output_SPike_Sent\n"}
//    if(in_the(BF::Heartbeat_Rec     , obj){s = s + "Heartbeat_Rec\n";}
//    if(in_the(BF::Spike_Rec         , obj){s = s + "Spike_Rec\n";}
//    if(in_the(BF::NS_Tick_Update    , obj){s = s + "NS_Tick_Update\n";}
//    if(in_the(BF::Leak_Update       , obj){s = s + "Leak_Update\n";}
//    if(in_the(BF::FR_Update         , obj){s = s + "FR_Update\n";}

}
#undef idf


TEST(enums,add_enum_state) {
    //op_state:
    BF_Event_Status status = BF::Leak_Update;
    //add one new element:
    status = add_evt_status(status, BF::Heartbeat_Rec);
    //check 2:
    ASSERT_PRED3((as_stat_ad<BF,BF>),status,BF::Leak_Update, BF::Heartbeat_Rec);
    //check 3:
    status = add_evt_status(status, BF::Output_Spike_Sent);
    ASSERT_PRED4((as_stat_ad<BF,BF,BF>),status,BF::Leak_Update, BF::Heartbeat_Rec,BF::Output_Spike_Sent);

    //test N
    BF_Event_Status statuses[] = {
            BF::Heartbeat_Sent    ,
            BF::Spike_Sent        ,
            BF::Output_Spike_Sent ,
            BF::Heartbeat_Rec     ,
            BF::Spike_Rec         ,
            BF::NS_Tick_Update    ,
            BF::Leak_Update       ,
            BF::FR_Update         };

    for(int i = 0; i < 512; i ++){
        int max = 7;
        int min = 1;

        auto num_to_add = (rand() % (max + 1 - min)) + min;

        std::vector<int> add_these;
        std::vector<BF> check_these;

        for(int j = 0; j < num_to_add; j ++){
            auto n2 = (rand() % (max + 1 - min)) + min;
            add_these.push_back(n2);
            check_these.push_back(statuses[n2]);
        }
        //create the status.
        status = statuses[add_these.back()];

        while(add_these.size() > 1){
            add_these.pop_back();
            status = add_evt_status(status, statuses[add_these.back()]);
        }
        //check the elements:
        for(auto check_status : check_these){
            ASSERT_PRED3((as_stat_ad<BF,BF>),status,check_status,check_status);
        }
    }

}
template<class BASE, typename DTYPE>
struct FMatrix : public crtp<BASE>{


    DTYPE get(int x, int y) {
        return this->underlying().get_impl(x,y);
    }
    void set(int x, int y, DTYPE v){
        this->underlying().set_impl(x,y,v);
    }

    size_t index(int x, int y ){
        return x + this->underlying().m_width * y;
    }

};
template<typename DTYPE>
struct SMatrix: public FMatrix<SMatrix<DTYPE>,DTYPE>{
    unsigned int m_width;
    unsigned int m_height;
    std::vector<DTYPE> data;

    SMatrix(unsigned int mWidth, unsigned int mHeight) : m_width(mWidth), m_height(mHeight) {
        data = std::vector<DTYPE>(m_width * m_height);
    }

    //size_t index( int x, int y) const {return  x + m_width * y;}
    DTYPE get_impl(int x, int y) {
        auto idx = this->index(x,y);
        return data[idx];
    }
    void set_impl(int x, int y, DTYPE v) { data[this->index(x,y)] = v;};

};
template<typename DTYPE>
struct DYMatrix : public FMatrix<DYMatrix<DTYPE>,DTYPE>{
    unsigned int m_height;
    unsigned int m_width;
    DTYPE * data;
    DYMatrix(unsigned int mWidth, unsigned int mHeight) : m_width(mWidth), m_height(mHeight){
        data = (DTYPE * ) malloc(sizeof(DTYPE) * mWidth * mHeight);
    }
    //size_t index(int x, int y)
    DTYPE get_impl(int x, int y) {return data[this->index(x,y)]; }
    void set_impl(int x, int y, DTYPE v) {
        data[this->index(x,y)] = v;}

};    unsigned int m_width;

size_t index(int x, int y, int width){
    return x + width * y;

}


TEST(matrix, mat_acc){
    int nv = 64;
    int test_arr[nv][nv];
    auto smat = SMatrix<int>(nv,nv);
    for(int i = 0; i < nv; i ++){
        for(int j = 0; j < nv; j ++){
            auto val = rand();
            test_arr[i][j] = val;
            smat.set(i,j,val);
        }
    }
    for(int i = 0; i < nv; i ++) {
        for (int j = 0; j < nv; j++) {
            ASSERT_EQ(test_arr[i][j], smat.get(i,j));
        }
    }
}

/*************** MATRIX TESTING ***********;
 *
 */

#include <chrono>
#include <random>
using namespace std::chrono;
const int test_runs = 1;
#define arr_size  512






time_point<high_resolution_clock> math_starts[test_runs];
time_point<high_resolution_clock> math_ends [test_runs];
time_point<high_resolution_clock> init_starts[test_runs];
time_point<high_resolution_clock> init_ends[test_runs];
time_point<high_resolution_clock> run_starts[test_runs];
time_point<high_resolution_clock> run_ends[test_runs];
time_point<high_resolution_clock> random_starts[test_runs];
time_point<high_resolution_clock> random_ends[test_runs];
std::string run_types [4] = {"Full Run", "Math Op", "Init op", "Random op"};
std::string run_vars [3] = {"Heap Array", "Custom Matrix","Cust Heap Matrix"};



auto max = arr_size -1;
auto min = 0;
std::random_device seed;
std::mt19937 rng(seed());
std::uniform_int_distribution<int> gen(min, max); // uniform, unbiased

template <typename T>
auto test_math(T v1, T v2){
    auto ret_val = (T) (v1 / (1.0L / (sqrt(v2) + .001L)));
    return ret_val;
}
#pragma omp declare target
template <typename T>
auto test_math_v(T v1, T v2){
    auto ret_val = (T) (v1 / (1.0L / (sqrt(v2) + .001L)));
    return ret_val;
}
#pragma omp end declare target

void test_multi_math(float ** arr, int n){
    while(n --){
        int ii = gen(rng);
        int jj = gen(rng);
        int ix = gen(rng);
        int jx = gen(rng);
        int i = gen(rng);
        int j = gen(rng);
        arr[i][j] = test_math(arr[ii][jj], arr[ix][jx]);
        auto m = 4;
        auto r = 1;
        auto e = 4;
#define b  arr[i][j]
        arr[i][j] = (int)arr[i][j] % m;
        while (e > 0){
            if (e % 2 == 1){
                r = (int)(r * b) % m;
            }
            e = e >> 1;
            b = (int) pow(b,2) % m;
        }
        arr[ix][jx] = b;
    }
#undef b
    //sequental:
    for(int i = 0; i < arr_size; i ++){
        for(int j = 0; j < arr_size; j ++){
            for(int ii = arr_size - 1 ; ii >= 0; ii --){
                for(int jj = arr_size -1; jj >= 0; jj--){
                    arr[i][j] = test_math(arr[ii][jj],arr[ii][jj]);
                }
            }
        }
    }
}
template<typename T>
void test_multi_math(T mat, int n){
    while(n --){
        int ii = gen(rng);
        int jj = gen(rng);
        int ix = gen(rng);
        int jx = gen(rng);
        int i = gen(rng);
        int j = gen(rng);
        mat->set(i,j, test_math(mat->get(ii,jj), mat->get(ii,jj)));
        //arr[i][j] = test_math(arr[ii][jj], arr[ix][jx]);
        auto m = 4;
        auto r = 1;
        auto e = 4;
#define b  mat->set(i,j,
        mat->set(i,j, (int)mat->get(i,j) % (int)m);
        while (e > 0){
            if (e % 2 == 1){
                r = (int)(r * mat->get(i,j)) % m;
            }
            e = e >> 1;
            b (int) pow(mat->get(i,j),2) % m);
        }
        mat->set(ix,jx, (mat->get(i,j)));
    }
#undef b
    for(int i = 0; i < arr_size; i ++) {
        for (int j = 0; j < arr_size; j++) {
            for (int ii = arr_size - 1; ii >= 0; ii--) {
                for (int jj = arr_size - 1; jj >= 0; jj--) {
                    mat->set(i, j, test_math(mat->get(ii, jj), mat->get(ii, jj)));
                }
            }
        }
    }
}

int nteams = 32;
int block_threads = arr_size/nteams;


template <typename T>
float test_simd(T  *mat){
    float result = 0;
    auto mat_data = mat->data;
    auto width = arr_size;

//#pragma omp target map(from: result) map(to: mat)
//#pragma omp teams num_teams(nteams)
//#pragma omp distribute parallel for dist_schedule(static, block_threads)
//#pragma omp parallel for simd collapse(2)

#pragma parallel omp for simd collapse(2) //parallel for map(from: result) map(to: mat[0:arr_size])
        for (int ii = arr_size - 1; ii >= 0; ii--) {
            for (int jj = arr_size - 1; jj >= 0; jj--) {
                //result +=  1/sqrt(mat->get(ii,jj));
                result += 1 / sqrt(mat_data[ii + (jj * width)]);
            }
        }


    return result;
}

template<typename T>
float test_simd(T ** mat){
    float result = 0;
    std::cout << "OUT? \n";
#pragma omp parallel for simd collapse(2)
    for(int ii = arr_size - 1 ; ii >= 0; ii --) {
        for (int jj = arr_size - 1; jj >= 0; jj--) {
            result += 1 / sqrt(mat[ii][jj]);
        }
    }

    std::cout << "done\n";
    return result;


}

template<typename T>
void test_omp_m(T mat){
    float result1 = test_simd(mat);
#pragma omp parallel for collapse(4)
    for(int i = 0; i < arr_size; i ++) {
        for (int j = 0; j < arr_size; j++) {
            for (int ii = arr_size - 1; ii >= 0; ii--) {
                for (int jj = arr_size - 1; jj >= 0; jj--) {
                    mat->set(i, j, 1/sqrt(mat->get(ii,jj)));
                }
            }
        }
    }

    std::cout << result1;
}


void test_omp_m(float ** arr){
float result1 = test_simd(arr);
#pragma omp parallel for collapse(4)
    for(int i = 0; i < arr_size; i ++){
        for(int j = 0; j < arr_size; j ++){
            for(int ii = arr_size - 1 ; ii >= 0; ii --){
                for(int jj = arr_size -1; jj >= 0; jj--){

                    arr[i][j] = 1 / sqrt(arr[ii][jj]);
                }
            }
        }
    }
    std::cout << result1;
}
float test_simd(float * arr){
    float result = 0.0;
#pragma omp parallel for  collapse()
    for(int ii = arr_size - 1 ; ii >= 0; ii --) {
        for (int jj = arr_size - 1; jj >= 0; jj--) {
            result += 1 / sqrt( arr[index(ii,jj,arr_size)]);
        }
    }
    return result;
}

void test_omp_m(float * arr){

}
//template <typename T>
//auto test_mymat(SMatrix<T> mat,int i, int j, int num_ops){
//    while(num_ops > 0){
//        num_ops --;
//        int ii = gen(rng);
//        int jj = gen(rng);
//        int ix = gen(rng);
//        int jx = gen(rng);
//        mat.set(i,j, test_math)
//    }
//}
//template <typename T>
//auto test_mymat(T ** heap_array, int i, int j, int num_ops){
//
//}

template < typename T>
void pdur(T start, T end, const char * name){

    auto duration = duration_cast<microseconds>(end-start);
    std::cout << "Bench Result " << name << duration.count() << "microseconds \n";
}
typedef std::chrono::duration<long double,std::milli> milliseconds_type;
template <typename T>
auto gd(T start, T end){

    auto elapsed =  (end-start);
    return duration_cast<milliseconds>(elapsed).count();
    //return std::chrono::duration<long long,std::milli>(elapsed).count();


}
template <typename AT>
AT ** gen_heap(){
    auto heap_array = (AT **) malloc(sizeof (AT *) * arr_size);
    for(int j = 0; j < arr_size; j ++){
        heap_array[j] = (AT *) malloc(sizeof(AT) * arr_size);
    }
    return heap_array;
}

#include <vector>
template <typename t>
auto  avg( std::vector<t> & v){
    auto avg = v[0];
    for(int i = 1; i < v.size(); i ++ ){
        avg += i;
    }
    return (long double) avg / (long double) v.size();
}
template<typename T>
long double var(std::vector<T>  & v){
    auto mean = avg(v);
    auto sum = 0.0L;
    auto temp = 0.0L;
    for (auto e : v){
        temp = pow((long double) e - mean, 2);
        sum += temp;
    }
    return (long double) sum / (long double) (v.size() - 2);

}
template<typename TPS>
void print_stats(TPS starts, TPS ends, const std::string &rtype, const std::string &run_name){
    //we will do an average
    auto durs = std::vector<long long>();
    for(int i = 0; i < test_runs; i ++){
        auto t = gd(starts[i], ends[i]);
        durs.push_back(t);

        //durs.push_back(gd<microseconds>(starts[i], ends[i]));
    }
    long double avg_time;
    long double var_time;
    if(test_runs == 1){
        avg_time = durs[0];
        var_time = -1;
        std::cout <<"\n1 run total: for " << run_name <<  " in " << rtype << " took: " << durs[0] <<"\n";
    }else if (test_runs == 2){
        avg_time = avg(durs);
        var_time = -1;
        std::cout <<"2 runs total for" << run_name << " in " << rtype <<    "  INDV Runs: \n";
        std::cout << "Total 1: " << durs[0] << " Total 2: " << durs [1] << "\n";

    }else {
        avg_time = avg(durs);
        var_time = var(durs);
    }
    std::cout << "Using " << run_name << " Run " << rtype << " took " << avg_time << " S | VAR: " << var_time << " MS\n";

}

void print_stat_groups(std::string run_var, int run_type){
    //full, math, init, random in std:string array
    auto rt = run_types[run_type];
    switch(run_type){
        case(0):
            //full
            print_stats(run_starts, run_ends, rt, run_var);
            break;

        case(1):
            //math:
            print_stats(math_starts, math_ends, rt, run_var);
            break;
        case(2):
            print_stats(init_starts, init_ends, rt, run_var);
            break;
        case(3):
            print_stats(random_starts, random_ends, rt, run_var);
            break;
        default:
            std::cout << "No implementation for run type " << run_type << "\n";
            break;
    }



}

TEST(matrix, mat_performance){
    int math_tests = 8;
    std::vector<long long> total_sums;
    std::vector<std::string> total_name;
    std::vector<long long> math_sums;
    std::vector<std::string> math_names;


//    std::string run_types [4] = {"Full Run", "Math Op", "Init op", "Random op"};
//    std::string run_vars [2] = {"Heap Array", "Custom Matrix"};
#define atv(ar,tm) ar[rnum] = tm
    {
        total_sums.push_back(0);
        math_sums.push_back(0);
        total_name.push_back("Heap Alloc Array");

        auto rnum = test_runs;
        int i = 0;
        while(rnum --) {
            std :: cout  << "Heap at " << i << "\n";
            i ++;

            auto start= high_resolution_clock::now();
            // test the heap array:

            auto init_start = high_resolution_clock::now();

            auto heap_array = gen_heap<float>();
            //    int ** heap_array= (int **) malloc(sizeof(int*) *  arr_size);
            //    for(int j = 0; j < arr_size; j ++){
            //            heap_array[j] = (int*) malloc(sizeof(int) * arr_size);
            //    }
            auto init_end = high_resolution_clock::now();
            auto rand_start = init_end;

            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    heap_array[i][j] = rand();
                }
            }
            auto rand_end = high_resolution_clock::now();
            auto math_start = rand_end;

            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    int ii = gen(rng);
                    int jj = gen(rng);
                    int ix = gen(rng);
                    int jx = gen(rng);
                    heap_array[i][j] = test_math(heap_array[ii][jj], heap_array[ix][jx]);
                }

            }
            //test_multi_math(heap_array,math_tests);
            test_omp_m(heap_array);
            auto math_end = high_resolution_clock::now();
            auto end = high_resolution_clock::now();
            std::cout << gd(start, high_resolution_clock::now()) << "---\n";
            atv(run_starts, start);
            atv(run_ends, end);
            atv(init_starts, init_start);
            atv(init_ends, init_end);
            atv(random_starts, rand_start);
            atv(random_ends, rand_end);
            atv(math_starts, math_start);
            atv(math_ends, math_end);

            total_sums[0] = total_sums[0] + gd(start,end);
            math_sums[0] = math_sums[0] + gd(math_start, math_end);
            delete(heap_array);
        }
        //stats
        for(int i = 0; i < 4; i ++)
            print_stat_groups(run_vars[0], i);


    }
    // my matrix perf:
    std::cout << "\n\n";
    {

        total_sums.push_back(0);
        total_name.push_back("Heap Alloc Squashed Matrix");
        math_sums.push_back(0);
        auto rnum = test_runs;


        int i = 0;
        while(rnum --) {
            auto start= high_resolution_clock::now();
            std :: cout  << "Heap at " << i << "\n";
            i ++;
            auto init_start = high_resolution_clock::now();
            //auto matrix = new SMatrix<float>(arr_size, arr_size);
            auto matrix = (float *) malloc(sizeof(float)* arr_size);
            auto init_end = high_resolution_clock::now();
            auto rand_start = init_end;


            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    //matrix->set(i, j, rand());
                    matrix[index(i,j,arr_size)] = rand();
                }
            }

            auto rand_end = high_resolution_clock::now();
            auto math_start = rand_end;

            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    int ii = gen(rng);
                    int jj = gen(rng);
                    int ix = gen(rng);
                    int jx = gen(rng);
                    matrix[index(i,j,arr_size)] =
                    test_math(matrix[index(ii,jj,arr_size)],matrix[index(ix,jx,arr_size)]);


                }
            }
            //test_multi_math(matrix, math_tests);
            test_omp_m(matrix);
            auto math_end = high_resolution_clock::now();
            std::cout << gd(start, high_resolution_clock::now()) << "---\n";
            auto end = high_resolution_clock::now();
            total_sums[1] = total_sums[1] + gd(start,end);
            math_sums[1] = math_sums[1] + gd(math_start, math_end);
            atv(run_starts, start);
            atv(run_ends, end);
            atv(init_starts, init_start);
            atv(init_ends, init_end);
            atv(random_starts, rand_start);
            atv(random_ends, rand_end);
            atv(math_starts, math_start);
            atv(math_ends, math_end);
            delete(matrix);

        }
        for(int i = 0; i < 4; i ++)
            print_stat_groups(run_vars[1], i);



    }
    std::cout << "\n\n";
    {
       total_sums.push_back(0);
        total_name.push_back("Custom Heap Matrix");
        math_sums.push_back(0);
        auto rnum = test_runs;

        int i = 0;
        while (rnum --){
            auto start= high_resolution_clock::now();
            std::cout << "CM at " << i << "\n";
            i ++;
            i ++;
            auto init_start = high_resolution_clock::now();
            DYMatrix<float> *matrix = new DYMatrix<float>(arr_size, arr_size);
            auto init_end = high_resolution_clock::now();
            auto rand_start = init_end;




            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    matrix->set(i, j, rand());
                }
            }

            auto rand_end = high_resolution_clock::now();
            auto math_start = rand_end;

            for (int i = 0; i < arr_size; i++) {
                for (int j = 0; j < arr_size; j++) {
                    int ii = gen(rng);
                    int jj = gen(rng);
                    int ix = gen(rng);
                    int jx = gen(rng);
                    matrix->set(i,j, test_math(matrix->get(ii,jj),matrix->get(ix,jx)));


                }
            }
            //test_multi_math(matrix, math_tests);
            test_omp_m(matrix);
            auto math_end = high_resolution_clock::now();
            auto end = high_resolution_clock::now();
            std::cout << gd(start, high_resolution_clock::now()) << "---\n";
            total_sums[2] = total_sums[2] + gd(start,end);
            math_sums[2] = math_sums[2] + gd(math_start, math_end);
            atv(run_starts, start);
            atv(run_ends, end);
            atv(init_starts, init_start);
            atv(init_ends, init_end);
            atv(random_starts, rand_start);
            atv(random_ends, rand_end);
            atv(math_starts, math_start);
            atv(math_ends, math_end);
            delete(matrix);

        }
        for(int i = 0; i < 4; i ++)
            print_stat_groups(run_vars[2], i);

    }



    std::cout << "\n Totals: \n ";
    for(int i = 0; i < total_sums.size(); i ++){
        std::cout << "Total Time of "<< total_name[i] << " took: " << total_sums[i] << " S \n";
        std::cout << "Math Time took: " << math_sums[i] <<" S \n\n";
    }
}