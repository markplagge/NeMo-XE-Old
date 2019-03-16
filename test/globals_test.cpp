//
// Created by Mark Plagge on 2019-03-16.
//

#include <gtest/gtest.h>
#include <gtest/gtest.h>
#include "../src/include/globals.h"


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